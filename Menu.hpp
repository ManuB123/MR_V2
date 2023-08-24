#pragma once

#ifdef ARDUINO
  #include <Arduino.h>
  #include <Adafruit_SSD1306.h>
#endif

#include "stdint.h"

#define MAX_MENU_SIZE 4
#define MENU_Y_OFFSET 17
#define MENU_ITEM_Y_OFFSET 10

namespace MR {
  typedef void (*SubMenuDisplayCallback)(bool, uint8_t);

  enum ButtonEventType {
    DEFAULT = 0,
    PRESSED = 1,
    RELEASED = 2,
  };

  enum MenuType : uint8_t {
    Default = 0,
    SubMenu  = 1,
  };

  class MenuItem {
    public:
      MenuItem(MenuType type = MenuType::Default);
      ~MenuItem() = default;

      void setFocus(bool focus);
      void setDisplayCallback(SubMenuDisplayCallback callback, uint8_t yOffset);
      void setDisplayCallback(SubMenuDisplayCallback callback, SubMenuDisplayCallback subMenuCallback, uint8_t yOffset);
      void executeCallback();
      void executeSubMenuCallback();
      void setType(const MenuType &type);
      MenuType getType() const;

    private:
      MenuType _type;
      bool _isFocused = false;
      SubMenuDisplayCallback _callback = NULL;
      SubMenuDisplayCallback _subMenuCallback = NULL;
      uint8_t _yOffset;
  };

  class Menu
  {
    public:
      Menu() {};
      ~Menu() = default;

      void onIndexChanged(const int8_t newIndex);
      void onButtonPressed(const ButtonEventType &eventType);
      void addMenuItem(MenuType type, SubMenuDisplayCallback callback);
      void addMenuItem(MenuType type, SubMenuDisplayCallback callback, SubMenuDisplayCallback subMenuCallback);
      void display();
      bool shouldRedraw() const;
      uint8_t getMenuSize() const;

    private:
      MenuItem _items[MAX_MENU_SIZE];
      bool _shouldDraw = false;
      uint8_t _menuSize = 0;
      uint8_t _index = 0;
      bool _displaySubMenu = false;

  };
};
