#ifndef INCLUDE_MENU_H_
#define INCLUDE_MENU_H_


#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    uint8_t selected_index;
    uint8_t option_count;
    bool joystick_ready;

} menu_t;

/* DEKLARACIJE FUNKCIJ */
void Menu_Init(menu_t *menu, uint8_t option_count);
void Menu_MoveUp(menu_t *menu);
void Menu_MoveDown(menu_t *menu);
bool Menu_HandleInput(menu_t *menu);


#endif /* INCLUDE_MENU_H_ */
