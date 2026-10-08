#include "menu.h"
#include "joystick.h"
#include "kbd.h"

void Menu_Init(menu_t *menu, uint8_t option_count)
{
	menu -> selected_index = 0;
	menu -> option_count = option_count;
	menu->joystick_ready = true;

}

void Menu_MoveUp(menu_t *menu)
{
	if(menu -> selected_index > 0)
	{
		menu -> selected_index--;
	}
}

void Menu_MoveDown(menu_t *menu)
{
    if(menu->selected_index < menu->option_count - 1)
    {
        menu->selected_index++;
    }
}

bool Menu_HandleInput(menu_t *menu)		//branje vrednosti joystick pozicij GOR = y je blizu 100 DOL = y je blizu 0
{
	 uint8_t y = JOY_get_axis_position(Y);

	    if(menu->joystick_ready)
	    {
	        if(y > 60)
	        {
	            Menu_MoveUp(menu);
	            menu->joystick_ready = false;
	        }
	        else if(y < 40)
	        {
	            Menu_MoveDown(menu);
	            menu->joystick_ready = false;
	        }
	    }
	    else
	    {
	        if(y >= 40 && y <= 60)
	        {
	            menu->joystick_ready = true;
	        }
	    }

	    if(KBD_get_pressed_button() == BTN_OK)
	    {
	        return true;
	    }

	    return false;
}
