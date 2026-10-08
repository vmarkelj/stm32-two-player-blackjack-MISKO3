#include "deck.h"
#include "random.h"

void Deck_Init(deck_t *deck)		//inicializacija kupčka kart
{
	uint8_t index = 0;

	for(card_suit_t suit = SUIT_CLUBS; suit <= SUIT_HEARTS ; suit++)				//zunanji for gre čez vse znake kart
	{
		for(card_rank_t rank = RANK_ACE; rank <= RANK_KING ; rank++)		//notranji for gre čez range kart
		{
			deck ->cards[index].rank = rank;		//vsaki karti pripiše rang
			deck ->cards[index].suit = suit;		//vsaki karti pripiše znak
			index++;
		}


	}

	deck -> next_card = 0; // nastavi spremenljivko next_card na prvo kart v vrsti
}

void Deck_Shuffle(deck_t *deck)  // funkcija ki bo mešala karte po Fisher-Yates algoritmu
{
	card_t temp;

	for(int i = DECK_SIZE - 1; i > 0; i--)
	{
		int j = Random_GetNumber() % (i + 1);

		temp = deck -> cards[i];
		deck ->cards[i] = deck -> cards[j];
		deck -> cards[j] = temp;

	}

	deck -> next_card = 0;
}


card_t Deck_Draw(deck_t *deck)
{
	card_t card;

	card = deck->cards[deck->next_card];

	deck -> next_card++;
	return card;

}


uint8_t Deck_CardsRemaining(deck_t *deck)
{
	return DECK_SIZE - deck -> next_card;		// od velikosti kupčka kart odštejemo next_card kar nam poda število preostalih kart
}

