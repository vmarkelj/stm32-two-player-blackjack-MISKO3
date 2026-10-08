#include "blackjack.h"
#include "stm32g4xx_hal.h"


void Blackjack_Init(blackjack_game_t *game)		//inicializacija igre
{
	Deck_Init(&game -> deck);
	Deck_Shuffle(&game->deck);
	Hand_Init(&game -> dealer_hand);

    for(uint8_t i = 0; i < BLACKJACK_MAX_PLAYERS; i++)
    {
    	for(uint8_t h = 0; h < BLACKJACK_MAX_HANDS; h++)
    	{

    		 Hand_Init(&game->player_hands[i][h]);
    		 game -> player_result[i][h] = PLAYER_RESULT_NONE;
    		 game -> player_bet[i][h] = 0;

    		if(h == 0)
    		{
    			 game -> player_state[i][h] = PLAYER_STATE_PLAYING;
    		}
    		else
    		{
    			game -> player_state[i][h] = PLAYER_STATE_INACTIVE;
    		}


    	}

        game -> player_balance[i] = 1000;
        game -> player_active[i] = false;
        game->player_current_hand[i] = 0;
        game->player_turn_finished[i] = false;


    }



	game -> state = BLACKJACK_STATE_IDLE; 		//nastavitev stanja na idle
	game -> current_player = 0;
	game -> player_count = 0;
}

void Blackjack_SetPlayerCount(blackjack_game_t *game, uint8_t count)
{
    if(count >=1 && count <= BLACKJACK_MAX_PLAYERS)
    {
        game -> player_count = count;

        for(uint8_t i = 0; i < game -> player_count; i++)
        {
        	game -> player_active[i] = true;
        }
    }
}

void Blackjack_StartRound(blackjack_game_t *game)
{
	Hand_Init(&game ->dealer_hand);		// inicializacija rok dealerja
	game -> current_player = 0;
	game -> current_hand = 0;
	game -> state = BLACKJACK_STATE_BETTING;

	for(uint8_t i = 0; i < game -> player_count; i++)			//inicializacija rok igralcev in nastavitev stanj ter rezultatov
	{
		game->player_current_hand[i] = 0;
		game->player_turn_finished[i] = false;

		for(uint8_t h = 0; h < BLACKJACK_MAX_HANDS; h++)
		{
		    Hand_Init(&game->player_hands[i][h]);
		    game -> player_result[i][h] = PLAYER_RESULT_NONE;
		    game -> player_bet[i][h] = 0;

		    if(h == 0)
		    {
		    	game -> player_state[i][h] = PLAYER_STATE_PLAYING;
		    }

		    else
		    {
		    	game -> player_state[i][h] = PLAYER_STATE_INACTIVE;
		    }
		}


	}

}

bool Blackjack_PlaceBet(blackjack_game_t *game,uint8_t player, uint16_t amount)
{
	if(player >= game -> player_count)
	{
		return false;
	}

	if(game->player_bet[player][0] != 0)
	{
	    return false;
	}

	if(amount == 0 || amount > game -> player_balance[player])
	{
		return false;
	}

	game -> player_bet[player][0] = amount;
	game -> player_balance[player] -= amount;


	return true;

}


void Blackjack_DealInitialCards(blackjack_game_t *game)
{
	if(Deck_CardsRemaining(&game -> deck) < BLACKJACK_RESHUFFLE_THRESHOLD)		// preverimo ali premešamo kupček
		{
			Deck_Shuffle(&game -> deck);
		}

	for(uint8_t round = 0; round < 2; round++)					//razdelimo 2 karti na igralca
	{
		for(uint8_t i = 0; i < game->player_count; i++)
		 {
		    Hand_AddCard(&game -> player_hands[i][0], Deck_Draw(&game -> deck));
		 }

		 Hand_AddCard(&game -> dealer_hand, Deck_Draw(&game -> deck));
	}


	for(uint8_t i = 0; i < game->player_count; i++)					//preverjanje ali je kdo od igralcev dobil blackjack
	{
	    if(Hand_IsBlackjack(&game -> player_hands[i][0]))
	    {
	        game -> player_state[i][0] = PLAYER_STATE_BLACKJACK;
	    }
	}

	if(Hand_IsBlackjack(&game -> dealer_hand))						//preverjanje ali je dealer dobil blackjack
	{
		for(uint8_t i = 0; i < game -> player_count; i++)
		{
			if(Hand_IsBlackjack(&game -> player_hands[i][0]))			//če dealer dobi blackjack prav tako pa nekdo od igralcev sta izenačena
			{
				game->player_result[i][0] = PLAYER_RESULT_PUSH;
			}
			else
			{
				game->player_result[i][0] = PLAYER_RESULT_LOSE;		//ostali igralci izgubijo
			}
		}

		Blackjack_Payout(game);
		game -> state = BLACKJACK_STATE_ROUND_OVER;					//runda se zaključi
		return;
	}

	bool any_player_active = false;

	for(uint8_t i = 0; i < game->player_count; i++)
	{
	    game->player_current_hand[i] = 0;

	    if(game->player_state[i][0] == PLAYER_STATE_PLAYING)
	    {
	        game->player_turn_finished[i] = false;
	        any_player_active = true;
	    }
	    else
	    {
	        game->player_turn_finished[i] = true;
	    }
	}

	if(any_player_active)
	{
	    game->state = BLACKJACK_STATE_PLAYER_TURN;
	}
	else
	{
	    game->state = BLACKJACK_STATE_DEALER_TURN;
	}

}





void Blackjack_PlayerHit(blackjack_game_t *game, uint8_t player) //funkcija hit
{
    uint8_t hand = game->player_current_hand[player];

    Hand_AddCard(
        &game->player_hands[player][hand],
        Deck_Draw(&game->deck)
    );

    uint8_t hand_value =
        Hand_GetValue(&game->player_hands[player][hand]);

    if(hand_value == 21)
    {
        game->player_state[player][hand] = PLAYER_STATE_STAND;
        Blackjack_NextTurn(game, player);
    }
    else if(hand_value > 21)
    {
        game->player_state[player][hand] = PLAYER_STATE_BUST;
        Blackjack_NextTurn(game, player);
    }
}


void Blackjack_PlayerStand(blackjack_game_t *game, uint8_t player) //funkcija stand
{
    uint8_t hand = game->player_current_hand[player];

    game->player_state[player][hand] = PLAYER_STATE_STAND;

    Blackjack_NextTurn(game, player);
}


bool Blackjack_PlayerDouble(blackjack_game_t *game, uint8_t player)
{
    uint8_t hand = game->player_current_hand[player];

    if(game->player_hands[player][hand].card_count != 2)
    {
        return false;
    }

    if(game->player_balance[player] >= game->player_bet[player][hand])
    {
        game->player_bet[player][hand] *= 2;
        game->player_balance[player] -= game->player_bet[player][hand] / 2;

        Hand_AddCard(&game->player_hands[player][hand], Deck_Draw(&game->deck));

        if(Hand_GetValue(&game->player_hands[player][hand]) > 21)
        {
            game->player_state[player][hand] = PLAYER_STATE_BUST;
        }
        else
        {
            game->player_state[player][hand] = PLAYER_STATE_STAND;
        }

        Blackjack_NextTurn(game, player);

        return true;
    }

    return false;
}


bool Blackjack_PlayerSplit(blackjack_game_t *game, uint8_t player)
{
    uint8_t hand = game->player_current_hand[player];

    if(hand != 0)
    {
        return false;
    }

    if(game->player_hands[player][hand].card_count != 2)
    {
        return false;
    }

    if(game->player_hands[player][hand].cards[0].rank != game->player_hands[player][hand].cards[1].rank)
    {
        return false;
    }

    bool split_aces = game->player_hands[player][hand].cards[0].rank == RANK_ACE;

    if(game->player_balance[player] < game->player_bet[player][hand])
    {
        return false;
    }

    game->player_bet[player][1] = game->player_bet[player][hand];
    game->player_balance[player] -= game->player_bet[player][1];

    game->player_hands[player][1].cards[0] = game->player_hands[player][hand].cards[1];

    game->player_hands[player][0].card_count = 1;
    game->player_hands[player][1].card_count = 1;

    Hand_AddCard(&game->player_hands[player][0], Deck_Draw(&game->deck));
    Hand_AddCard(&game->player_hands[player][1], Deck_Draw(&game->deck));

    uint8_t hand_value0 = Hand_GetValue(&game->player_hands[player][0]);
    uint8_t hand_value1 = Hand_GetValue(&game->player_hands[player][1]);

    game->player_current_hand[player] = 0;

    if(split_aces)
    {
        game->player_state[player][0] = PLAYER_STATE_STAND;
        game->player_state[player][1] = PLAYER_STATE_STAND;

        Blackjack_NextTurn(game, player);
    }
    else
    {
        if(hand_value0 == 21)
        {
            game->player_state[player][0] = PLAYER_STATE_STAND;
        }
        else
        {
            game->player_state[player][0] = PLAYER_STATE_PLAYING;
        }

        if(hand_value1 == 21)
        {
            game->player_state[player][1] = PLAYER_STATE_STAND;
        }
        else
        {
            game->player_state[player][1] = PLAYER_STATE_PLAYING;
        }
    }

    return true;
}


void Blackjack_NextTurn(blackjack_game_t *game, uint8_t player)
{
    uint8_t hand = game->player_current_hand[player];

    if(hand == 0 && game->player_state[player][1] == PLAYER_STATE_PLAYING)
    {
        game->player_current_hand[player] = 1;
        return;
    }

    game->player_turn_finished[player] = true;

    bool all_players_finished = true;

    for(uint8_t i = 0; i < game->player_count; i++)
    {
        if(!game->player_turn_finished[i])
        {
            all_players_finished = false;
            break;
        }
    }

    if(all_players_finished)
    {
        game->state = BLACKJACK_STATE_DEALER_TURN;
    }
}


void Blackjack_DealerTurn(blackjack_game_t *game)
{
    static uint32_t dealer_draw_tick = 0;
    static bool dealer_waiting = false;

    uint8_t dealer_hand_value = Hand_GetValue(&game->dealer_hand);



    if(dealer_hand_value < 17)
    {
        if(!dealer_waiting)
        {
            dealer_draw_tick = HAL_GetTick();
            dealer_waiting = true;
            return;
        }

        if((HAL_GetTick() - dealer_draw_tick) >= 800)
        {
            Hand_AddCard(&game->dealer_hand,
                         Deck_Draw(&game->deck));

            dealer_waiting = false;
        }

        return;
    }



    dealer_waiting = false;



    for(uint8_t i = 0; i < game->player_count; i++)
    {
        for(uint8_t h = 0; h < BLACKJACK_MAX_HANDS; h++)
        {
            if(game->player_hands[i][h].card_count == 0)
            {
                continue;
            }

            uint8_t player_hand_value =
                Hand_GetValue(&game->player_hands[i][h]);

            if(game->player_state[i][h] == PLAYER_STATE_BUST)
            {
                game->player_result[i][h] = PLAYER_RESULT_LOSE;
            }

            else if(game->player_state[i][h] == PLAYER_STATE_BLACKJACK)
            {
                game->player_result[i][h] = PLAYER_RESULT_WIN;
            }

            else if(dealer_hand_value > 21)
            {
                game->player_result[i][h] = PLAYER_RESULT_WIN;
            }

            else if(player_hand_value > dealer_hand_value)
            {
                game->player_result[i][h] = PLAYER_RESULT_WIN;
            }

            else if(player_hand_value < dealer_hand_value)
            {
                game->player_result[i][h] = PLAYER_RESULT_LOSE;
            }

            else
            {
                game->player_result[i][h] = PLAYER_RESULT_PUSH;
            }
        }
    }


    Blackjack_Payout(game);

    game->state = BLACKJACK_STATE_ROUND_OVER;
}


void Blackjack_Payout(blackjack_game_t *game)				//funkcija izplačila
{
	for(uint8_t i = 0;i < game -> player_count ; i++)
	{
		for(uint8_t h = 0; h < BLACKJACK_MAX_HANDS; h++)
		{
			if(game -> player_hands[i][h].card_count == 0)
			{
				continue;
			}

			if(game -> player_result[i][h] == PLAYER_RESULT_PUSH)
			{
				game -> player_balance[i] += game -> player_bet[i][h];
			}

			else if(game -> player_result[i][h] == PLAYER_RESULT_WIN)
			{

				if(game -> player_state[i][h] == PLAYER_STATE_BLACKJACK)
				{
					game -> player_balance[i] += game -> player_bet[i][h] * 5 / 2;
				}

				else
				{
				game -> player_balance[i] += game -> player_bet[i][h] * 2;
				}
			}
		}
	}
}
