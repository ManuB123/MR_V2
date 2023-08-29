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
  typedef void (*IndexChangedCallback)(const int8_t);

  enum ButtonEventType {
    DEFAULT = 0,
    PRESSED = 1,
    RELEASED = 2,
  };

  enum MenuType : uint8_t {
    Default = 0,
    SubMenu  = 1,
    Scrollable = 2,
  };

  class MenuItem {
    public:
      MenuItem(MenuType type = MenuType::Default);
      ~MenuItem() = default;

      void setFocus(bool focus);
      void setDisplayCallback(SubMenuDisplayCallback callback, uint8_t yOffset);
      void setDisplayCallback(SubMenuDisplayCallback callback, SubMenuDisplayCallback subMenuCallback, uint8_t yOffset);
      void setIndexChangedCallback(IndexChangedCallback callback);
      void executeCallback();
      void executeSubMenuCallback();
      void setType(const MenuType &type);
      MenuType getType() const;

      void handleIndexChangeEvent(const int8_t newIndex);

    private:
      MenuType _type;
      bool _isFocused = false;
      SubMenuDisplayCallback _callback = NULL;
      SubMenuDisplayCallback _subMenuCallback = NULL;
      IndexChangedCallback _indexChangedCallback = NULL;
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
      void addMenuItem(MenuType type, SubMenuDisplayCallback callback, IndexChangedCallback indexCallback);
      void addMenuItem(MenuType type, SubMenuDisplayCallback callback, SubMenuDisplayCallback subMenuCallback);
      void display();
      void setIdle(const bool idle);
      bool isIdle() const;
      bool shouldRedraw() const;
      void markAsRedraw();
      int8_t editingIndex() const;
      uint8_t getMenuSize() const;

    private:
      MenuItem _items[MAX_MENU_SIZE];
      
      bool _shouldDraw = false;
      bool _displaySubMenu = false;
      bool _isIdle = false;

      uint8_t _menuSize = 0;
      uint8_t _index = 0;
      int8_t _editingIndex = -1;
  };
};
