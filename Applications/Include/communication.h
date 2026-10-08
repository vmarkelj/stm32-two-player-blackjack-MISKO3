#ifndef INCLUDE_COMMUNICATION_H_
#define INCLUDE_COMMUNICATION_H_

#include <stdbool.h>
#include "buf.h"
#include "deck.h"

#define COMM_START_BYTE 0xAA
#define COMM_PACKET_SIZE 6		// start byte - message type - player index - data1 - data2 - checksum


typedef enum
{
	PLAYER_JOIN = 0,
	PLAYER_ASSIGN_INDEX,
	PLAYER_SEND_BET,
	PLAYER_SEND_ACTION,
	MASTER_SEND_CARD,
	MASTER_PLAYER_SPLIT,
	MASTER_SEND_STATE,
	MASTER_EXIT_TO_MENU,
	MASTER_SEND_BALANCE,
	MASTER_SEND_BET,
	MASTER_SEND_RESULT,
	MASTER_SEND_DEALER_CARD,
	MASTER_SEND_SPLIT_BET,
	MASTER_SEND_PLAYER_STATE,
	MASTER_ACTION_REJECTED,
	MASTER_GAME_OVER_NO_FUNDS
}communication_message_type_t;

typedef enum
{
	PLAYER_ACTION_HIT = 0,
	PLAYER_ACTION_STAND,
	PLAYER_ACTION_DOUBLE,
	PLAYER_ACTION_SPLIT,
	PLAYER_ACTION_PLAY_AGAIN,
	PLAYER_ACTION_EXIT
}communication_player_action_t;

typedef struct				//v tem struktu so opisane informacije ki jih slave potrebuje za normalno igro
{
	communication_message_type_t type;
	uint8_t player_index;
	uint16_t bet_value;
	communication_player_action_t player_action;

	card_rank_t card_rank;
	card_suit_t card_suit;
	uint8_t hand_index;
	uint8_t game_state;
	uint8_t current_player;
	uint16_t balance_value;
	uint8_t player_result;
	uint8_t player_state;
	uint8_t current_hand;
	uint8_t player_finished;
}communication_message_t;

/* DEKLARACIJE FUNKCIJ */
bool Communication_GetMessage(communication_message_t *message);
void Communication_SendMessage(communication_message_t *message);


#endif /* INCLUDE_COMMUNICATION_H_ */
