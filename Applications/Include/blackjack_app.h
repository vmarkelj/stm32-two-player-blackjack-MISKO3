#ifndef INCLUDE_BLACKJACK_APP_H_
#define INCLUDE_BLACKJACK_APP_H_

#include "device_mode.h"
#include "menu.h"
#include "blackjack.h"
#include "game_controller.h"

typedef enum
{
    BLACKJACK_APP_STATE_MODE_SELECT,
    BLACKJACK_APP_STATE_RUNNING,
	BLACKJACK_APP_STATE_NO_FUNDS

} blackjack_app_state_t;

typedef enum
{
    RESULT_LED_NONE = 0,
    RESULT_LED_WIN,
    RESULT_LED_LOSE,
    RESULT_LED_PUSH,
    RESULT_LED_MIXED,
    RESULT_LED_SPLIT_PUSH

} blackjack_result_led_t;

typedef struct
{
	blackjack_app_state_t state;
	device_mode_t device_mode;
	menu_t device_mode_menu;
	menu_t action_menu;
	menu_t round_over_menu;
	game_controller_t controller;
	blackjack_game_t game;
	uint16_t slave_bet_selection;
	bool slave_bet_confirmed;
	uint8_t previous_current_player;
	bool slave_action_confirmed;
	uint16_t local_bet_selection;
	bool slave_round_confirmed;
	bool master_initial_sync_sent;

	bool multiplayer_connected;
	bool join_message_sent;
	uint32_t join_retry_tick;
	uint32_t round_result_tick;
	bool round_result_finished;

}blackjack_app_t;

/* DEKLARACIJE FUNKCIJ */
void BlackjackApp_Init(blackjack_app_t *app);
void BlackjackApp_Update(blackjack_app_t *app);
#endif /* INCLUDE_BLACKJACK_APP_H_ */
