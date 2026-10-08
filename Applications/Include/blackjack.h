#ifndef INCLUDE_BLACKJACK_H_
#define INCLUDE_BLACKJACK_H_

#include "hand.h"
#include "deck.h"

typedef enum
{
	BLACKJACK_STATE_IDLE = 0,
	BLACKJACK_STATE_BETTING,
	BLACKJACK_STATE_PLAYER_TURN,
	BLACKJACK_STATE_DEALER_TURN,
	BLACKJACK_STATE_ROUND_OVER
}blackjack_state_t;

typedef enum
{
	PLAYER_STATE_PLAYING = 0,
	PLAYER_STATE_STAND,
	PLAYER_STATE_BUST,
	PLAYER_STATE_BLACKJACK,
	PLAYER_STATE_INACTIVE
}blackjack_player_state_t;

typedef enum
{
    PLAYER_RESULT_NONE = 0,
    PLAYER_RESULT_WIN,
    PLAYER_RESULT_LOSE,
    PLAYER_RESULT_PUSH
} blackjack_player_result_t;

#define BLACKJACK_MAX_PLAYERS 2U
#define BLACKJACK_RESHUFFLE_THRESHOLD 26U
#define BLACKJACK_MAX_HANDS 2

typedef struct
{
	blackjack_state_t state;
	blackjack_player_state_t player_state[BLACKJACK_MAX_PLAYERS][BLACKJACK_MAX_HANDS];
	blackjack_player_result_t player_result[BLACKJACK_MAX_PLAYERS][BLACKJACK_MAX_HANDS];
	hand_t player_hands[BLACKJACK_MAX_PLAYERS][BLACKJACK_MAX_HANDS];
	hand_t dealer_hand;
	deck_t deck;
	uint8_t player_count;
	uint8_t current_player;
	uint8_t current_hand;
	uint16_t player_bet[BLACKJACK_MAX_PLAYERS][BLACKJACK_MAX_HANDS];
	uint16_t player_balance[BLACKJACK_MAX_PLAYERS];
	bool player_active[BLACKJACK_MAX_PLAYERS];

	uint8_t player_current_hand[BLACKJACK_MAX_PLAYERS];
	bool player_turn_finished[BLACKJACK_MAX_PLAYERS];

}blackjack_game_t;


/* DEKLARACIJE FUNKCIJ */
void Blackjack_Init(blackjack_game_t *game);
void Blackjack_SetPlayerCount(blackjack_game_t *game, uint8_t count);
void Blackjack_StartRound(blackjack_game_t *game);
bool Blackjack_PlaceBet(blackjack_game_t *game,uint8_t player, uint16_t amount);
void Blackjack_DealInitialCards(blackjack_game_t *game);
void Blackjack_PlayerHit(blackjack_game_t *game, uint8_t player);
void Blackjack_PlayerStand(blackjack_game_t *game, uint8_t player);
bool Blackjack_PlayerDouble(blackjack_game_t *game, uint8_t player);
bool Blackjack_PlayerSplit(blackjack_game_t *game, uint8_t player);
void Blackjack_NextTurn(blackjack_game_t *game, uint8_t player);
void Blackjack_DealerTurn(blackjack_game_t *game);
void Blackjack_Payout(blackjack_game_t *game);

#endif /* INCLUDE_BLACKJACK_H_ */
