#pragma once

#ifdef ARDUINO
  #include <Arduino.h>
  #include <FlashAsEEPROM.h>
#endif

#define SELECTED_COMMAND_OFFSET 0
#define SELECTED_COMMAND_SIZE 1

#define CONSIGNE_ADDRESS_OFFSET (SELECTED_COMMAND_OFFSET + SELECTED_COMMAND_SIZE)
#define CONSIGNE_COMMAND_SIZE 2

#define DMX_ADDRESS_OFFSET (CONSIGNE_ADDRESS_OFFSET + CONSIGNE_COMMAND_SIZE)
#define DMX_COMMAND_SIZE 2

#define DALI_ADDRESS_OFFSET (DMX_ADDRESS_OFFSET + DMX_COMMAND_SIZE)
#define DALI_COMMAND_SIZE 2

#define LAST_DMX_VALUE_OFFSET (DALI_ADDRESS_OFFSET + DALI_COMMAND_SIZE)
#define LAST_DMX_VALUE_SIZE 1

#include "stdint.h"

namespace MR {
  class Vault {
    public:
      static uint8_t readSelectedCommand()
      {
        uint8_t res = 0;

        if (EEPROM.isValid())
        {
          res = EEPROM.read(SELECTED_COMMAND_OFFSET);
        }

        return res;
      };

      static int16_t readConsigneAddress()
      {
        int16_t res = 0;

        if (EEPROM.isValid())
        {
          uint8_t first = EEPROM.read(CONSIGNE_ADDRESS_OFFSET);
          uint8_t second = EEPROM.read(CONSIGNE_ADDRESS_OFFSET + 1);

          res = first << 8 | second;
        }

        return res;
      };

      static int16_t readDMXAddress()
      {
        int16_t res = 0;

        if (EEPROM.isValid())
        {
          uint8_t first = EEPROM.read(DMX_ADDRESS_OFFSET);
          uint8_t second = EEPROM.read(DMX_ADDRESS_OFFSET + 1);

          res = first << 8 | second;
        }

        return res;
      };

      static int16_t readDALIAddress()
      {
        int16_t res = 0;

        if (EEPROM.isValid())
        {
          uint8_t first = EEPROM.read(DALI_ADDRESS_OFFSET);
          uint8_t second = EEPROM.read(DALI_ADDRESS_OFFSET + 1);

          res = first << 8 | second;
        }

        return res;
      };

      static uint8_t readLastDmxValue()
      {
        uint8_t res = 0;

        if (EEPROM.isValid())
        {
          res = EEPROM.read(LAST_DMX_VALUE_OFFSET);
        }

        return res;
      };

      static bool writeData(uint8_t *buffer, uint8_t size)
      {
        for (uint8_t i = 0; i < size; ++i) {
          EEPROM.write(i, buffer[i]);
        }

        EEPROM.commit();
      };
  };
} // namespace MR
