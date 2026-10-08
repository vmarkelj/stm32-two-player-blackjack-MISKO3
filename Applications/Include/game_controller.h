#ifndef INCLUDE_GAME_CONTROLLER_H_
#define INCLUDE_GAME_CONTROLLER_H_

#include "blackjack.h"


typedef enum
{
	GAME_CMD_NONE = 0,
	GAME_CMD_BET,
	GAME_CMD_HIT,
	GAME_CMD_STAND,
	GAME_CMD_DOUBLE,
	GAME_CMD_SPLIT,
	GAME_CMD_PLAY_AGAIN,
	GAME_CMD_EXIT
}game_command_t;

typedef enum
{
    GAME_ROUND_RESPONSE_NONE = 0,
    GAME_ROUND_RESPONSE_PLAY_AGAIN,
    GAME_ROUND_RESPONSE_EXIT

} game_round_response_t;

typedef enum
{
    GAME_ROUND_DECISION_WAITING = 0,
    GAME_ROUND_DECISION_PLAY_AGAIN,
    GAME_ROUND_DECISION_EXIT
}game_round_decision_t;

typedef struct
{
    uint8_t player_index;
    game_command_t command;
    uint16_t value;
}game_command_message_t;

typedef struct
{
	bool bet_confirmed[BLACKJACK_MAX_PLAYERS];
	game_round_response_t round_response[BLACKJACK_MAX_PLAYERS];
	game_command_message_t cmd_message;
	bool split_succeeded;
	bool exit_requested;
	bool double_succeeded;

}game_controller_t;

/* DEKLARACIJE FUNKCIJ */
void GameController_Init(game_controller_t *controller);
void GameController_Update(blackjack_game_t *game, game_controller_t *controller);

#endif /* INCLUDE_GAME_CONTROLLER_H_ */
