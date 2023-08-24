#include "Menu.hpp"

/* MENU ITEM */

MR::MenuItem::MenuItem(MR::MenuType type)
  : _type(type)
{

}

void MR::MenuItem::setFocus(bool focus)
{
  _isFocused = focus;
}

void MR::MenuItem::setDisplayCallback(SubMenuDisplayCallback callback, uint8_t yOffset)
{
  _callback = callback;
  _yOffset = yOffset;
}

void MR::MenuItem::setDisplayCallback(SubMenuDisplayCallback callback, SubMenuDisplayCallback subMenuCallback, uint8_t yOffset)
{
  _callback = callback;
  _subMenuCallback = subMenuCallback;
  _yOffset = yOffset;
}

void MR::MenuItem::executeCallback()
{
  if (_callback != NULL) {
    (*_callback)(_isFocused, _yOffset);
  }
}

void MR::MenuItem::executeSubMenuCallback()
{
  if (_subMenuCallback != NULL) {
    (*_subMenuCallback)(_isFocused, MENU_Y_OFFSET);
  }
}

MR::MenuType MR::MenuItem::getType() const
{
  return _type;
}

void MR::MenuItem::setType(const MR::MenuType &type)
{
  _type = type;
}

/* MENU */

void MR::Menu::onIndexChanged(const int8_t newIndex)
{
  if (_displaySubMenu) return;

  _items[_index].setFocus(false);

  if (_index + newIndex >= _menuSize) {
    _index = 0;
  } else if (_index + newIndex < 0) {
    _index = _menuSize - 1;
  } else {
    _index += newIndex;
  }

  _items[_index].setFocus(true);
  _displaySubMenu = false;
  _shouldDraw = true;
}

void MR::Menu::onButtonPressed(const MR::ButtonEventType &eventType)
{
  /**
   * Logic for the button event is the following:
   *
   * In the main menu, it is used to enter submenus or enable interaction with the current menu item
   * If in a menu, releasing the button will exit the submenu
  */
  if (!_displaySubMenu && eventType == MR::ButtonEventType::RELEASED) {
    if (_items[_index].getType() == MR::MenuType::SubMenu) {
      _displaySubMenu = true;
      _shouldDraw = true;
    }
  } else if (_displaySubMenu && eventType == MR::ButtonEventType::RELEASED) {
    _displaySubMenu = false;
    _shouldDraw = true;
  }
}

void MR::Menu::addMenuItem(MenuType type, SubMenuDisplayCallback callback)
{
  _items[_menuSize].setType(type);
  _items[_menuSize].setFocus(false);
  _items[_menuSize].setDisplayCallback(callback, MENU_Y_OFFSET + (_menuSize != 0 ?  MENU_ITEM_Y_OFFSET * _menuSize : 0));
  _shouldDraw = true;
  _menuSize += 1;
}

void MR::Menu::addMenuItem(MenuType type, SubMenuDisplayCallback callback, SubMenuDisplayCallback subMenuCallback)
{
  _items[_menuSize].setFocus(false);
  _items[_menuSize].setType(type);
  _items[_menuSize].setDisplayCallback(callback, subMenuCallback, MENU_Y_OFFSET + (_menuSize != 0 ?  MENU_ITEM_Y_OFFSET * _menuSize : 0));
  _shouldDraw = true;
  _menuSize += 1;
}

void MR::Menu::display()
{
  if (_shouldDraw) {
    if (!_displaySubMenu) {
      for (uint8_t i = 0; i < _menuSize; ++i) {
        _items[i].executeCallback();
      }
    } else {
      Serial.print("Displaying sub menu of index : ");
      Serial.println(_index);
      _items[_index].executeSubMenuCallback();
    }

    _shouldDraw = false;
  }
}

bool MR::Menu::shouldRedraw() const
{
  return _shouldDraw;
}

uint8_t MR::Menu::getMenuSize() const
{
  return _menuSize;
}