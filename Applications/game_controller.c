#include "game_controller.h"
#include "kbd.h"

void GameController_Init(game_controller_t *controller)
{
	for(uint8_t i = 0; i < BLACKJACK_MAX_PLAYERS; i++)
	{
		controller -> bet_confirmed[i] = false;
		controller -> round_response[i] = GAME_ROUND_RESPONSE_NONE;
	}

	controller -> cmd_message.player_index = 0;
	controller -> cmd_message.value = 0;
	controller -> cmd_message.command = GAME_CMD_NONE;
	controller -> split_succeeded = false;
	controller -> exit_requested = false;
	controller -> double_succeeded = false;
}

static bool GameController_AllBetsConfirmed(blackjack_game_t *game, game_controller_t *controller)
{
    for(uint8_t i = 0; i < game->player_count; i++)
    {
        if(!controller -> bet_confirmed[i])
        {
            return false;
        }
    }

    return true;
}

static game_round_decision_t GameController_GetRoundDecision( blackjack_game_t *game, game_controller_t *controller)
{
    for(uint8_t i = 0; i < game->player_count; i++)
    {
        if(controller->round_response[i] == GAME_ROUND_RESPONSE_EXIT)
        {
            return GAME_ROUND_DECISION_EXIT;
        }
    }

    for(uint8_t i = 0; i < game->player_count; i++)
    {
        if(controller->round_response[i] == GAME_ROUND_RESPONSE_NONE)
        {
            return GAME_ROUND_DECISION_WAITING;
        }
    }

    return GAME_ROUND_DECISION_PLAY_AGAIN;
}

void GameController_Update(blackjack_game_t *game, game_controller_t *controller)
{
    switch(game->state)
    {
        case BLACKJACK_STATE_IDLE:
        {
            break;
        }

        case BLACKJACK_STATE_BETTING:
        {
            if(controller->cmd_message.command == GAME_CMD_BET)
            {
                if(Blackjack_PlaceBet(game, controller->cmd_message.player_index, controller->cmd_message.value))
                {
                    controller->bet_confirmed[controller->cmd_message.player_index] = true;
                }

                controller->cmd_message.command = GAME_CMD_NONE;
            }
            else if(controller->cmd_message.command != GAME_CMD_NONE)
            {
                controller->cmd_message.command = GAME_CMD_NONE;
            }

            if(GameController_AllBetsConfirmed(game, controller))
            {
                Blackjack_DealInitialCards(game);
            }

            break;
        }

        case BLACKJACK_STATE_PLAYER_TURN:
        {
            uint8_t player = controller->cmd_message.player_index;

            if(player >= game->player_count)
            {
                controller->cmd_message.command = GAME_CMD_NONE;
                break;
            }

            if(game->player_turn_finished[player])
            {
                controller->cmd_message.command = GAME_CMD_NONE;
                break;
            }

            if(controller->cmd_message.command == GAME_CMD_HIT)
            {
                Blackjack_PlayerHit(game, player);
                controller->cmd_message.command = GAME_CMD_NONE;
            }
            else if(controller->cmd_message.command == GAME_CMD_STAND)
            {
                Blackjack_PlayerStand(game, player);
                controller->cmd_message.command = GAME_CMD_NONE;
            }
            else if(controller->cmd_message.command == GAME_CMD_DOUBLE)
            {
                controller->double_succeeded = false;

                if(Blackjack_PlayerDouble(game, player))
                {
                    controller->double_succeeded = true;
                }

                controller->cmd_message.command = GAME_CMD_NONE;
            }
            else if(controller->cmd_message.command == GAME_CMD_SPLIT)
            {
                controller->split_succeeded = false;

                if(Blackjack_PlayerSplit(game, player))
                {
                    controller->split_succeeded = true;
                }

                controller->cmd_message.command = GAME_CMD_NONE;
            }
            else if(controller->cmd_message.command != GAME_CMD_NONE)
            {
                controller->cmd_message.command = GAME_CMD_NONE;
            }

            break;
        }

        case BLACKJACK_STATE_DEALER_TURN:
        {
            Blackjack_DealerTurn(game);
            break;
        }

        case BLACKJACK_STATE_ROUND_OVER:
        {
            if(controller->cmd_message.command == GAME_CMD_PLAY_AGAIN)
            {
                controller->round_response[controller->cmd_message.player_index] = GAME_ROUND_RESPONSE_PLAY_AGAIN;
                controller->cmd_message.command = GAME_CMD_NONE;
            }
            else if(controller->cmd_message.command == GAME_CMD_EXIT)
            {
                controller->round_response[controller->cmd_message.player_index] = GAME_ROUND_RESPONSE_EXIT;
                controller->cmd_message.command = GAME_CMD_NONE;
            }
            else if(controller->cmd_message.command != GAME_CMD_NONE)
            {
                controller->cmd_message.command = GAME_CMD_NONE;
            }

            game_round_decision_t decision = GameController_GetRoundDecision(game, controller);

            if(decision == GAME_ROUND_DECISION_WAITING)
            {
                break;
            }
            else if(decision == GAME_ROUND_DECISION_PLAY_AGAIN)
            {
                for(uint8_t i = 0; i < game->player_count; i++)
                {
                    controller->round_response[i] = GAME_ROUND_RESPONSE_NONE;
                    controller->bet_confirmed[i] = false;
                }

                Blackjack_StartRound(game);
            }
            else if(decision == GAME_ROUND_DECISION_EXIT)
            {
                controller->exit_requested = true;
                break;
            }

            break;
        }
    }
}

