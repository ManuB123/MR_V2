#include <sam.h>
#include <Encoder.h>

#include <SAMD21turboPWM.h>
#include <Adafruit_SSD1306.h>
#include "src/LXSAMD21DMX.h"
#include "src/Menu.hpp"
#include "src/Vault.hpp"

#define Beta  3950 // Changer cela à la valeur Beta de votre thermistance
#define Temp0  25 // Température à laquelle la résistance est R0 (25°C pour la plupart des thermistances)
#define R0  10000 // Ch
#define SerieResistor  10000
#define SELECTION  7
#define RELAIS  3
Encoder myEnc(3, 4);
#define BUTTON_PIN 2
MR::Menu mainMenu;

#define PWM_PIN  8
TurboPWM PWM_A;


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_I2C_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define DMX_MAX_ADRESS 513
#define MAX_LOCAL_CONSIGNE 100
int dmxFlag = 0;
int dmxReceivedAddress = 0;

int8_t commandeIndex = 0;
const char* commandeModes[] = {"Local", "DMX", "DALI"};
#define LOCAL_COMMAND_INDEX 0
#define DMX_COMMAND_INDEX 1
#define DALI_COMMAND_INDEX 2
#define COMMAND_ARRAY_SIZE 3

int8_t modeIndex = 0;
const char* modeDisplayText[] = {"Consigne", "Adresse DMX"};
uint8_t modeArraySize = sizeof(modeDisplayText) / sizeof(modeDisplayText[0]);
int16_t modeAddressValues[COMMAND_ARRAY_SIZE] = {0, 0, 0};
uint8_t lastDMXMainSlotValue = 0;

long oldPosition = 0;
enum MR::ButtonEventType buttonPressed = MR::ButtonEventType::DEFAULT;
uint32_t buttonPressedTimestamp;
#define SAVE_PARAMETERS_MINIMAL_TIME 5000 // In ms
#define SAVE_PARAMETERS_DISPLAY_DELAY 2000 // In ms

int32_t lastActivityTimestamp = 0;
bool ignoreNextActivityEvent = false;
#define IDLE_TRESHOLD 60000 // In ms

/* Display and scroll events callbacks */

/**
 * To handles generic menu items, we have 2 item categories, a SubMenu and a Scrollable and 3 types of callbacks:
 *   - A display callback: Manage how the item is displayed. It has 2 parameters a boolean that tell if the menu item is in focus and the y offset in the list of items.
 *   - A SubMenu specific callback: Manage how the submenu should be displayed
 *   - A Scrollable specific callback: When in focus, manage index changed events to handle how a specific value should be changed.
 * A SubMenu is an item that can be pressed with the button.
 * A Scrollable is an item that has a value that can be modified
*/
void FabricantDisplayCallback(bool inFocus, uint8_t yOffset)
{
  display.setCursor(0, yOffset);

  if (inFocus) {
    display.println("> Fabricant");
  } else {
    display.println("Fabricant");
  }
}

void FabricantSubMenuDisplayCallback(bool inFocus, uint8_t yOffset)
{
  (void)inFocus;
  display.setCursor(0, yOffset);
  display.println("Beresheet");
  display.println("Couleur LED: 3000K");
  display.println("Date de fab: 07/23");
  display.println("Puissance: 21W");
}

void CommandeDisplayCallback(bool inFocus, uint8_t yOffset)
{
  display.setCursor(0, yOffset);

  if (inFocus) {
    display.print("> Commande :");

    /*  Highlight value when editing  */
    if (mainMenu.editingIndex() != -1) {
      display.setTextColor(BLACK, WHITE);
    }

    display.print(' ');
    display.println(commandeModes[commandeIndex]);
    display.setTextColor(WHITE, BLACK);
  } else {
    display.print("Commande : ");
    display.println(commandeModes[commandeIndex]);
  }
}

void ModeDisplayCallback(bool inFocus, uint8_t yOffset)
{
  display.setCursor(0, yOffset);

  if (inFocus) {
    display.print("> ");
    display.print(modeDisplayText[modeIndex]);

    /*  Highlight value when editing  */
    if (mainMenu.editingIndex() != -1) {
      display.setTextColor(BLACK, WHITE);
    }

    display.print(' ');
    display.println(modeAddressValues[commandeIndex]);
    display.setTextColor(WHITE, BLACK);
  } else {
    display.print(modeDisplayText[modeIndex]);
    display.print(' ');
    display.println(modeAddressValues[commandeIndex]);
  }
}

void DiagnosticDisplayCallback(bool inFocus, uint8_t yOffset)
{
  display.setCursor(0, yOffset);

  if (inFocus) {
    display.println("> Diagnostic");
  } else {
    display.println("Diagnostic");
  }
}

void DiagnosticSubMenuCallback(bool inFocus, uint8_t yOffset)
{
  (void)inFocus;
  display.setCursor(0, yOffset);
  display.println("Courant : 0 A");
  display.println("Tension : 0 V");
  display.println("Puissance : 0 W");
  display.println("Temperature : 0 C");
}

void CommandScrollCallback(const int8_t newIndex)
{
  if (commandeIndex + newIndex < 0) {
    commandeIndex = 0;
  } else if (commandeIndex + newIndex >= COMMAND_ARRAY_SIZE) {
    commandeIndex = COMMAND_ARRAY_SIZE - 1;
  } else {
    commandeIndex += newIndex;
  }

  if (commandeIndex == 0) {
    modeIndex = 0;
  } else {
    modeIndex = 1;
  }
}

void ModeScrollCallback(const int8_t newIndex)
{
  /* DMX address ranges from 1 to 513 whereas the consigne value should range from 0 to 100 */
  if (modeAddressValues[commandeIndex] + newIndex < 0) {
    modeAddressValues[commandeIndex] = 1;
  } else if (modeAddressValues[commandeIndex] + newIndex > (commandeIndex == LOCAL_COMMAND_INDEX ? MAX_LOCAL_CONSIGNE : DMX_MAX_ADRESS)) {
    modeAddressValues[commandeIndex] = (commandeIndex == LOCAL_COMMAND_INDEX ? MAX_LOCAL_CONSIGNE : DMX_MAX_ADRESS);
  } else {
    modeAddressValues[commandeIndex] += newIndex;
  }
}

/* End of display and scroll events callbacks */

void loadStoredData()
{
  /**
   * Read stored data from the internal flash memory
  */
  commandeIndex = MR::Vault::readSelectedCommand();
  modeIndex = (commandeIndex == 0) ? 0 : 1;
  modeAddressValues[LOCAL_COMMAND_INDEX] = MR::Vault::readConsigneAddress();
  modeAddressValues[DMX_COMMAND_INDEX] = MR::Vault::readDMXAddress();
  modeAddressValues[DALI_COMMAND_INDEX] = MR::Vault::readDALIAddress();
  lastDMXMainSlotValue = MR::Vault::readLastDmxValue();
}

void gotDMXCallback(int slots) {
  dmxFlag = slots;
  dmxReceivedAddress = slots - 11;

  if (dmxReceivedAddress != modeAddressValues[DMX_COMMAND_INDEX]) {
    dmxFlag = 0;
  }
}

void setup() {
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_I2C_ADDRESS);
  display.setFont();
  display.setTextSize(1);
  display.clearDisplay();
  display.setTextColor(WHITE);
  Serial.begin(115200);

  loadStoredData();
  lastActivityTimestamp = millis();
  ignoreNextActivityEvent = false;
  Serial.println("Data loaded");

  /* Main menu setup */

  mainMenu.addMenuItem(MR::MenuType::SubMenu, &FabricantDisplayCallback, &FabricantSubMenuDisplayCallback);
  mainMenu.addMenuItem(MR::MenuType::Scrollable, &CommandeDisplayCallback, &CommandScrollCallback);
  mainMenu.addMenuItem(MR::MenuType::Scrollable, &ModeDisplayCallback, &ModeScrollCallback);
  mainMenu.addMenuItem(MR::MenuType::SubMenu, &DiagnosticDisplayCallback, &DiagnosticSubMenuCallback);

  /* Hack the menu to start the device on the last selected command value, ready to be edited */
  mainMenu.onIndexChanged(2);
  mainMenu.onButtonPressed(MR::ButtonEventType::RELEASED);

  /* DMX */
  SAMD21DMX.setDataReceivedCallback(&gotDMXCallback);
  SAMD21DMX.startInput();

  /* PWM */
  PWM_A.setClockDivider(1, false);
  PWM_A.timer(2, 1, 1500, false);   // Use timer 2 for pin 8, 16 kHz
}

void refreshScreen() {
  /* If no activity was recorded for a minute, the screen will shutdown */
  if (mainMenu.isIdle()) {
    return;
  }

  /**
   * Only refresh screen when needed.
   * In our case the possible reasons are the following:
   *  - Index changed
   *  - Button pressed
   *  - Data from sensor changed
  */
  if (mainMenu.shouldRedraw()) {
    uint32_t before = millis();
    display.clearDisplay();
    display.setCursor(0,0);
    mainMenu.display();
    display.display();
    uint32_t after = millis();

    Serial.print("Drawing took : ");
    Serial.print(after - before);
    Serial.println("ms");
  }
}

void saveParameters()
{
  uint8_t saveBufferSize = sizeof(commandeIndex) + sizeof(lastDMXMainSlotValue) + sizeof(modeAddressValues);
  uint8_t saveBuffer[saveBufferSize] = {0};
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Parametres a jour");
  display.display();
  Serial.println("Button pressed for more than 5 seconds");

  /**
   * So far we have 4 values to store in the internal flash of the MCU:
   *   - The command that was last chosen on 1 byte
   *   - The consigne value on 2 bytes
   *   - The DMX address on 2 bytes
   *   - The DALI DMX address on 2 bytes
  */
  saveBuffer[0] = commandeIndex;
  saveBuffer[1] = modeAddressValues[LOCAL_COMMAND_INDEX] >> 8;
  saveBuffer[2] = modeAddressValues[LOCAL_COMMAND_INDEX] & 0xFF;
  saveBuffer[3] = modeAddressValues[DMX_COMMAND_INDEX] >> 8;
  saveBuffer[4] = modeAddressValues[DMX_COMMAND_INDEX] & 0xFF;
  saveBuffer[5] = modeAddressValues[DALI_COMMAND_INDEX] >> 8;
  saveBuffer[6] = modeAddressValues[LOCAL_COMMAND_INDEX] & 0xFF;
  saveBuffer[7] = lastDMXMainSlotValue;
  MR::Vault::writeData(saveBuffer, saveBufferSize);

  delay(2000);
  mainMenu.markAsRedraw();
}

void loop() {
  int32_t newEncoderPosition = myEnc.read() / 4;
  bool updateRequired = false;

  /* Check if the button has been pressed for more than 5 seconds, if so, save the parameters to the internal flash */
  if (buttonPressed == MR::ButtonEventType::PRESSED && millis() - buttonPressedTimestamp > SAVE_PARAMETERS_MINIMAL_TIME) {
    buttonPressedTimestamp = millis();
    saveParameters();
    buttonPressed = MR::ButtonEventType::DEFAULT;
    mainMenu.onButtonPressed(buttonPressed);
  }

  /**
   * Handle the 3 different states: Pressed, released and default
   * We generally use the Released event.
  */
  if (buttonPressed != MR::ButtonEventType::PRESSED && digitalRead(BUTTON_PIN) == LOW) {
    Serial.println("Button pressed");
    buttonPressed = MR::ButtonEventType::PRESSED;
    buttonPressedTimestamp = millis();
    mainMenu.onButtonPressed(buttonPressed);
  } else if (buttonPressed == MR::ButtonEventType::PRESSED && digitalRead(BUTTON_PIN) == HIGH) {
    Serial.println("Button released");
    buttonPressed = MR::ButtonEventType::RELEASED;
    buttonPressedTimestamp = millis();
    mainMenu.onButtonPressed(buttonPressed);
    buttonPressed = MR::ButtonEventType::DEFAULT;
  }

  if (newEncoderPosition != oldPosition) {
    /**
     * We dont notify the menu that an index was changed just after the screen was brought back in normal mode
     * This way we dont randmly update values
    */
    if (!ignoreNextActivityEvent) {
      mainMenu.onIndexChanged(newEncoderPosition - oldPosition);
    } else {
      ignoreNextActivityEvent = false;
    }
    oldPosition = newEncoderPosition;
    updateRequired = true;
    lastActivityTimestamp = millis();
  }

  /**
   * If no activity has been recorded for at least 60 seconds, we put the screen in idle mode.
   * In idle mode, the screen is turned off to save power consumption
   * When the value of the encoder is changed, we switch back the screen to normal operating mode
  */
  if (!mainMenu.isIdle() && millis() - lastActivityTimestamp >= IDLE_TRESHOLD) {
    mainMenu.setIdle(true);
    Serial.println("Screen put in idle mode");
    display.ssd1306_command(SSD1306_DISPLAYOFF);
    ignoreNextActivityEvent = true;
  } else if (mainMenu.isIdle() && millis() - lastActivityTimestamp < IDLE_TRESHOLD) {
    mainMenu.setIdle(false);
    Serial.println("Screen put in normal mode");
    display.ssd1306_command(SSD1306_DISPLAYON);
  }

  /* DMX */
  if (commandeIndex == 1) {
    if (dmxFlag > 0) {
      lastDMXMainSlotValue = SAMD21DMX.getSlot(dmxReceivedAddress);
      float factor = ((float)lastDMXMainSlotValue / 255) * 1000;
      PWM_A.analogWrite(PWM_PIN, (uint32_t)factor);
    } else {
      // PWM_A.analogWrite(PWM_PIN, 0);
      if (updateRequired) {
        Serial.println("Switching from DMX mode");
      } 
    }
  } /* Local consigne */ 
  else if (commandeIndex == 0) {
    uint8_t consigneValue = modeAddressValues[LOCAL_COMMAND_INDEX];
    if (updateRequired) {
      if (consigneValue == 100) {
        PWM_A.analogWrite(PWM_PIN, 1000);
      } else {
        PWM_A.analogWrite(PWM_PIN, consigneValue * 10);
      }
    }
  }

  refreshScreen();
}
