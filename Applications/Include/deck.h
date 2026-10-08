#ifndef DECK_H
#define DECK_H

#include <stdint.h> // knjižnica za uporabo tipov števil s fiksno širino (uint8_t, uint16_t, itd.)

typedef enum
{
	SUIT_CLUBS = 0,
	SUIT_SPADES,		// definicija znakov kart
	SUIT_DIAMONDS,		//enum uporabiš ko ima vsaka spremenljivka le eno fiksno vrednost
	SUIT_HEARTS
}card_suit_t;

typedef enum
{
	RANK_ACE = 1,
	RANK_2,
	RANK_3,				//definicija rangov kart
	RANK_4,				//z dodajanjem vrednosti 1 polega RANK_ACE poskrbimo,
	RANK_5,				//da so od tu naprej vsi ostali rangi oštevilčeni v vrstnem redu 1-13
	RANK_6,
	RANK_7,
	RANK_8,
	RANK_9,
	RANK_10,
	RANK_JACK,
	RANK_QUEEN,
	RANK_KING,
}card_rank_t;

typedef struct
{
	card_suit_t suit;		//definicija karte, ki ima dve vrednosti (rang in znak)
	card_rank_t rank;
}card_t;

#define DECK_SIZE 52U

typedef struct
{
	card_t cards[DECK_SIZE];		//definicija paketa kart
	uint8_t next_card;				//next_card si nam pomaga zapomniti kje v paketu premešanih kart se trenutno nahajamo
}deck_t;

/* DEKLARACIJE FUNKCIJ */

void Deck_Init(deck_t *deck);
void Deck_Shuffle(deck_t *deck);
card_t Deck_Draw(deck_t *deck);
uint8_t Deck_CardsRemaining(deck_t *deck);

#endif
