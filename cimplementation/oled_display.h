#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H

bool oled_init();
void oled_clear();
void oled_show_message(const std::string &msg);

#endif
