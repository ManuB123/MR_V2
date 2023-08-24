#include <sam.h>
#include <Encoder.h>
// #include <DMXSerial.h>
#include <SAMD21turboPWM.h>
#include <FlashStorage.h>
#include <Adafruit_SSD1306.h>
#include "Menu.hpp"

#define Beta  3950 // Changer cela à la valeur Beta de votre thermistance
#define Temp0  25 // Température à laquelle la résistance est R0 (25°C pour la plupart des thermistances)
#define R0  10000 // Ch
#define SerieResistor  10000
#define SAMD21turboPWM_A  6
#define SELECTION  7
#define RELAIS  3
//Adafruit_SSD1306 display(128, 32, &Wire, -1);
Encoder myEnc(4, 5);
#define BUTTON_PIN 3
MR::Menu mainMenu;

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


const char* commandeModes[] = {"Local", "DMX", "DALI"};

long oldPosition = 0;
enum MR::ButtonEventType buttonPressed = MR::ButtonEventType::DEFAULT;
unsigned long buttonPressedTimestamp;

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
    display.println("> Commande");
  } else {
    display.println("Commande");
  }
}

void ModeDisplayCallback(bool inFocus, uint8_t yOffset)
{
  display.setCursor(0, yOffset);

  if (inFocus) {
    display.println("> Mode");
  } else {
    display.println("Mode");
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

void setup() {
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_I2C_ADDRESS);
  display.setFont();
  display.setTextSize(1);
  display.clearDisplay();
  display.setTextColor(WHITE);
  Serial.begin(9600);
  while (!Serial);

  /* Main menu setup */

  mainMenu.addMenuItem(MR::MenuType::SubMenu, &FabricantDisplayCallback, &FabricantSubMenuDisplayCallback);
  mainMenu.addMenuItem(MR::MenuType::Default, &CommandeDisplayCallback);
  mainMenu.addMenuItem(MR::MenuType::Default, &ModeDisplayCallback);
  mainMenu.addMenuItem(MR::MenuType::SubMenu, &DiagnosticDisplayCallback, &DiagnosticSubMenuCallback);
  mainMenu.onIndexChanged(0);


  // pinMode(SELECTION, OUTPUT);  // Définir la broche comme sortie
  // digitalWrite(SELECTION, LOW);

  // pinMode(3, OUTPUT);  // Définir la broche comme sortie
  // digitalWrite(3, HIGH);

  // if (EEPROM.read(initializedFlagAddress) != initializedMagicNumber) {
  //   EEPROM.write(commandeModeIndexAddress, 0); 
  //   EEPROM.write(adresseDMXAddress, 0);
  //   EEPROM.write(consigneAddress, 0);
  //   EEPROM.write(initializedFlagAddress, initializedMagicNumber);
  // }

  // commandeModeIndex = EEPROM.read(commandeModeIndexAddress);
  // adresseDMX = EEPROM.read(adresseDMXAddress);
  // consigne = EEPROM.read(consigneAddress);

  // pinMode(SAMD21turboPWM_A, OUTPUT);  
  // analogWrite(SAMD21turboPWM_A, consigne);

  // DMXSerial.init(DMXReceiver);
}

void refreshScreen() {
  /**
   * Only refresh screen when needed.
   * In our case the possible reasons are the following:
   *  - Index changed
   *  - Button pressed
   *  - Data from sensor changed
  */
  if (mainMenu.shouldRedraw()) {
    unsigned long before = millis();
    display.clearDisplay();
    display.setCursor(0,0);
    mainMenu.display();
    display.display();
    unsigned long after = millis();

    Serial.print("Drawing took : ");
    Serial.print(after - before);
    Serial.println("ms");
  }
}

void saveParameters()
{
  Serial.println("Button pressed for more than 5 seconds");
}

void loop() {
  int32_t newEncoderPosition = myEnc.read() / 4;

  if (buttonPressed != MR::ButtonEventType::PRESSED && digitalRead(BUTTON_PIN) == LOW) {
    Serial.println("Button pressed");
    buttonPressed = MR::ButtonEventType::PRESSED;
    buttonPressedTimestamp = millis();
    mainMenu.onButtonPressed(buttonPressed);
  } else if (buttonPressed == MR::ButtonEventType::PRESSED && digitalRead(BUTTON_PIN) == HIGH) {
    Serial.println("Button released");
    buttonPressed = MR::ButtonEventType::RELEASED;
    Serial.print("Button pressed for ");
    Serial.print(millis() - buttonPressedTimestamp);
    Serial.println("ms");
    buttonPressedTimestamp = millis();
    mainMenu.onButtonPressed(buttonPressed);
    buttonPressed = MR::ButtonEventType::DEFAULT;
  }

  if (newEncoderPosition != oldPosition) {
    mainMenu.onIndexChanged(newEncoderPosition - oldPosition);
    oldPosition = newEncoderPosition;
  }

  refreshScreen();
}
