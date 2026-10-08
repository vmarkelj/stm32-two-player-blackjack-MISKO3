#ifndef INCLUDE_HAND_H_
#define INCLUDE_HAND_H_

#include "deck.h"
#include <stdbool.h>
#include <stdint.h>

#define HAND_MAX_CARDS 11U

typedef struct
{
	card_t cards[HAND_MAX_CARDS];
	uint8_t card_count;
}hand_t;


/* DEKLARACIJE FUNKCIJ */
void Hand_Init(hand_t *hand);
void Hand_AddCard(hand_t *hand, card_t card);
uint8_t Hand_GetValue(const hand_t *hand);
bool Hand_IsBust(const hand_t *hand);
bool Hand_IsBlackjack(const hand_t *hand);

#endif /* INCLUDE_HAND_H_ */
