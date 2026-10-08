#include "hand.h"


void Hand_Init(hand_t *hand)	//inicializacija roke - na začetku imaš 0 kart
{
	hand -> card_count = 0;
}


void Hand_AddCard(hand_t *hand, card_t card)		//funkcija za dodajanje karte
{
	if(hand -> card_count < HAND_MAX_CARDS)
	{
		hand -> cards[hand -> card_count] = card;
		hand -> card_count++;
	}
}


uint8_t Hand_GetValue(const hand_t *hand)		//računanje vrednosti celotne roke
{
	uint8_t value = 0;
	uint8_t ace_count = 0; 									// s to spremenljivko si zapomnimo koliko asov imamo v roki
															//in odločimo ali je as vreden 1 ali 11
	for(uint8_t i = 0; i < hand -> card_count; i++)
	{
		if (hand -> cards[i].rank == RANK_ACE)				//na začetku asu dodamo vrednost 1. Če bo možno bomo kasneje dodali še 10
		{													// in tako bo imel vrednost 11
			value++;
			ace_count++;
		}

		else if(hand -> cards[i].rank >= RANK_2 && hand -> cards[i].rank <= RANK_10)
		{
			value += hand -> cards[i].rank;
		}

		else
		{
			value += 10;
		}

	}

	if(ace_count > 0 && (value + 10) <= 21)		//odločimo ali lahko asu dodamo vrednost 10
	{
		value += 10;
	}

	return value;
}


bool Hand_IsBust(const hand_t *hand)		//preverimo ali smo prekoračili mejo
{
	if(Hand_GetValue(hand) > 21)
	{
		return true;
	}

	else
	{
		return false;
	}
}


bool Hand_IsBlackjack(const hand_t *hand)						//preverimo ali smo dobili blackjack
{
	if(hand -> card_count == 2 && Hand_GetValue(hand) == 21)
	{
		return true;
	}

	else
	{
		return false;
	}
}
