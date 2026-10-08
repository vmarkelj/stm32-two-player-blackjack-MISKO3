#include "blackjack_app.h"
#include "communication.h"
#include "kbd.h"
#include "LED.h"

static uint8_t result_led_step = 0;
static uint8_t result_led_win_mask = 0;
static uint32_t result_led_tick = 0;
static bool result_led_animation_started = false;


static void BlackjackApp_ResetSessionState(blackjack_app_t *app)
{
    app->slave_bet_confirmed = false;
    app->slave_action_confirmed = false;
    app->slave_bet_selection = 25;
    app->local_bet_selection = 25;
    app->action_menu.selected_index = 0;
    app->round_over_menu.selected_index = 0;
    app->previous_current_player = 0xFF;
    app->controller.exit_requested = false;
    app->game.state = BLACKJACK_STATE_IDLE;
    app->game.current_player = 0;
    app->game.current_hand = 0;
    app->master_initial_sync_sent = false;
    app->multiplayer_connected = false;
    app->join_message_sent = false;
    app->join_retry_tick = 0;
    app -> round_result_tick = 0;
    app -> round_result_finished = false;

}

static void Blackjack_ResetResultLEDs(void)
{
    result_led_step = 0;
    result_led_win_mask = 0;
    result_led_tick = 0;
    result_led_animation_started = false;

    LEDs_write(0x00);
}

static blackjack_result_led_t Blackjack_GetResultLEDAnimation(
    blackjack_player_result_t result_0,
    blackjack_player_result_t result_1,
    bool is_split)
{
    // ------------------------------------------------
    // NORMAL HAND
    // ------------------------------------------------

    if(!is_split)
    {
        if(result_0 == PLAYER_RESULT_WIN)
        {
            return RESULT_LED_WIN;
        }

        if(result_0 == PLAYER_RESULT_LOSE)
        {
            return RESULT_LED_LOSE;
        }

        if(result_0 == PLAYER_RESULT_PUSH)
        {
            return RESULT_LED_PUSH;
        }

        return RESULT_LED_NONE;
    }


    // ------------------------------------------------
    // SPLIT HAND
    // ------------------------------------------------

    // Both win
    if(result_0 == PLAYER_RESULT_WIN &&
       result_1 == PLAYER_RESULT_WIN)
    {
        return RESULT_LED_WIN;
    }


    // Both lose
    if(result_0 == PLAYER_RESULT_LOSE &&
       result_1 == PLAYER_RESULT_LOSE)
    {
        return RESULT_LED_LOSE;
    }


    // One wins, one loses
    if((result_0 == PLAYER_RESULT_WIN &&
        result_1 == PLAYER_RESULT_LOSE) ||

       (result_0 == PLAYER_RESULT_LOSE &&
        result_1 == PLAYER_RESULT_WIN))
    {
        return RESULT_LED_MIXED;
    }


    // Both push
    if(result_0 == PLAYER_RESULT_PUSH &&
       result_1 == PLAYER_RESULT_PUSH)
    {
        return RESULT_LED_SPLIT_PUSH;
    }


    // Win + Push
    if(result_0 == PLAYER_RESULT_WIN ||
       result_1 == PLAYER_RESULT_WIN)
    {
        return RESULT_LED_WIN;
    }


    // Lose + Push
    if(result_0 == PLAYER_RESULT_LOSE ||
       result_1 == PLAYER_RESULT_LOSE)
    {
        return RESULT_LED_LOSE;
    }


    return RESULT_LED_NONE;
}

static void Blackjack_UpdateResultLEDs(blackjack_result_led_t animation)
{
    uint32_t now = HAL_GetTick();

    // ------------------------------------------------
    // START NEW ANIMATION
    // ------------------------------------------------

    if(!result_led_animation_started)
    {
        result_led_animation_started = true;

        result_led_step = 0;
        result_led_win_mask = 0;
        result_led_tick = now;

        LEDs_write(0x00);
    }


    // ------------------------------------------------
    // WIN
    //
    // LED7 -> LED0 one by one
    // 150 ms between LEDs
    //
    // Then:
    // OFF 500 ms
    // ON  500 ms
    // OFF 500 ms
    // ON  500 ms
    // OFF
    // ------------------------------------------------

    if(animation == RESULT_LED_WIN)
    {
        // LED7 -> LED0
        if(result_led_step < 8)
        {
            if((now - result_led_tick) >= 150)
            {
                result_led_win_mask |=
                    (1U << (7 - result_led_step));

                LEDs_write(result_led_win_mask);

                result_led_tick = now;
                result_led_step++;
            }
        }

        // Turn everything off after sweep
        else if(result_led_step == 8)
        {
            LEDs_write(0x00);

            result_led_tick = now;
            result_led_step++;
        }

        // Stay OFF for 500 ms,
        // then first flash ON
        else if(result_led_step == 9 &&
                (now - result_led_tick) >= 500)
        {
            LEDs_write(0xFF);

            result_led_tick = now;
            result_led_step++;
        }

        // First flash OFF
        else if(result_led_step == 10 &&
                (now - result_led_tick) >= 500)
        {
            LEDs_write(0x00);

            result_led_tick = now;
            result_led_step++;
        }

        // Second flash ON
        else if(result_led_step == 11 &&
                (now - result_led_tick) >= 500)
        {
            LEDs_write(0xFF);

            result_led_tick = now;
            result_led_step++;
        }

        // Second flash OFF
        else if(result_led_step == 12 &&
                (now - result_led_tick) >= 500)
        {
            LEDs_write(0x00);

            result_led_step++;
        }
    }


    // ------------------------------------------------
    // LOSE
    //
    // All LEDs ON for 500 ms
    // then OFF
    // ------------------------------------------------

    else if(animation == RESULT_LED_LOSE)
    {
        if(result_led_step == 0)
        {
            LEDs_write(0xFF);

            result_led_tick = now;
            result_led_step = 1;
        }

        else if(result_led_step == 1 &&
                (now - result_led_tick) >= 1000)
        {
            LEDs_write(0x00);

            result_led_step = 2;
        }
    }


    // ------------------------------------------------
    // NORMAL PUSH
    //
    // LED7, LED6, LED1, LED0
    //
    // 11000011 = 0xC3
    //
    // ON for 500 ms
    // then OFF
    // ------------------------------------------------

    else if(animation == RESULT_LED_PUSH)
    {
        if(result_led_step == 0)
        {
            LEDs_write(0xC3);

            result_led_tick = now;
            result_led_step = 1;
        }

        else if(result_led_step == 1 &&
                (now - result_led_tick) >= 1000)
        {
            LEDs_write(0x00);

            result_led_step = 2;
        }
    }


    // ------------------------------------------------
    // SPLIT: ONE WIN + ONE LOSE
    //
    // LED6, LED5, LED2, LED1
    //
    // 01100110 = 0x66
    //
    // ON for 500 ms
    // then OFF
    // ------------------------------------------------

    else if(animation == RESULT_LED_MIXED)
    {
        if(result_led_step == 0)
        {
            LEDs_write(0x66);

            result_led_tick = now;
            result_led_step = 1;
        }

        else if(result_led_step == 1 &&
                (now - result_led_tick) >= 1000)
        {
            LEDs_write(0x00);

            result_led_step = 2;
        }
    }


    // ------------------------------------------------
    // SPLIT: BOTH PUSH
    //
    // LED7 and LED0
    //
    // 10000001 = 0x81
    //
    // ON for 500 ms
    // then OFF
    // ------------------------------------------------

    else if(animation == RESULT_LED_SPLIT_PUSH)
    {
        if(result_led_step == 0)
        {
            LEDs_write(0x81);

            result_led_tick = now;
            result_led_step = 1;
        }

        else if(result_led_step == 1 &&
                (now - result_led_tick) >= 1000)
        {
            LEDs_write(0x00);

            result_led_step = 2;
        }
    }


    // ------------------------------------------------
    // NONE / INVALID
    // ------------------------------------------------

    else
    {
        LEDs_write(0x00);
    }
}

void BlackjackApp_Init(blackjack_app_t *app)
{
	app->state = BLACKJACK_APP_STATE_MODE_SELECT;

	Menu_Init(&app->device_mode_menu, 3);
	Menu_Init(&app->action_menu, 4);
	Menu_Init(&app->round_over_menu, 2);
	Blackjack_Init(&app->game);
	GameController_Init(&app -> controller);
	app->slave_bet_selection = 25;
	app->slave_bet_confirmed = false;
	app -> previous_current_player = 0XFF;
	app->slave_action_confirmed = false;
	app->local_bet_selection = 25;
	app -> slave_round_confirmed = false;
	app->master_initial_sync_sent = false;

	app->multiplayer_connected = false;
	app->join_message_sent = false;
	app->join_retry_tick = 0;
	app -> round_result_tick = 0;
	app -> round_result_finished = false;

	LED_init();
	LEDs_write(0x00);
}



void BlackjackApp_Update(blackjack_app_t *app)
{
	if(app->state == BLACKJACK_APP_STATE_MODE_SELECT)
	{
		bool confirmed = Menu_HandleInput(&app -> device_mode_menu);

		if(confirmed)
		{
			if(app->device_mode_menu.selected_index == 0)
			{
			    app->device_mode = DEVICE_MODE_SINGLE_PLAYER;
			    Blackjack_SetPlayerCount(&app->game, 1);
			}

			else if(app->device_mode_menu.selected_index == 1)
			{
			    app->device_mode = DEVICE_MODE_MASTER;
			    Blackjack_SetPlayerCount(&app->game, 2);
			}

			else if(app->device_mode_menu.selected_index == 2)
			{
			    app->device_mode = DEVICE_MODE_SLAVE;
			    Blackjack_SetPlayerCount(&app->game, 2);
			}

			app->state = BLACKJACK_APP_STATE_RUNNING;
			GameController_Init(&app->controller);
			Blackjack_ResetResultLEDs();

			if(app->device_mode == DEVICE_MODE_SINGLE_PLAYER)
			{
			    Blackjack_StartRound(&app->game);
			}
			else
			{
			    app->game.state = BLACKJACK_STATE_IDLE;
			    app->multiplayer_connected = false;
			    app->join_message_sent = false;
			    app->join_retry_tick = 0;
			}
		}
	}

	else if(app -> state == BLACKJACK_APP_STATE_RUNNING)
	{
		uint8_t local_player_index;


		if(app->device_mode == DEVICE_MODE_MASTER ||
		   app->device_mode == DEVICE_MODE_SINGLE_PLAYER)
		{
		    local_player_index = 0;
		}
		else
		{
		    local_player_index = 1;
		}



		if(app->game.state == BLACKJACK_STATE_PLAYER_TURN && !app->game.player_turn_finished[local_player_index] && (app->device_mode != DEVICE_MODE_SLAVE || !app->slave_action_confirmed))
		{
		    bool confirmed = Menu_HandleInput(&app->action_menu);

		    if(confirmed)
		    {
		        communication_player_action_t selected_action = PLAYER_ACTION_HIT;

		        if(app->action_menu.selected_index == 1)
		        {
		            selected_action = PLAYER_ACTION_STAND;
		        }
		        else if(app->action_menu.selected_index == 2)
		        {
		            selected_action = PLAYER_ACTION_DOUBLE;
		        }
		        else if(app->action_menu.selected_index == 3)
		        {
		            selected_action = PLAYER_ACTION_SPLIT;
		        }

		        if(app->device_mode == DEVICE_MODE_SINGLE_PLAYER ||
		           app->device_mode == DEVICE_MODE_MASTER)
		        {
		            app->controller.cmd_message.player_index = local_player_index;

		            if(selected_action == PLAYER_ACTION_HIT)
		            {
		                app->controller.cmd_message.command = GAME_CMD_HIT;
		            }
		            else if(selected_action == PLAYER_ACTION_STAND)
		            {
		                app->controller.cmd_message.command = GAME_CMD_STAND;
		            }
		            else if(selected_action == PLAYER_ACTION_DOUBLE)
		            {
		                app->controller.cmd_message.command = GAME_CMD_DOUBLE;
		            }
		            else if(selected_action == PLAYER_ACTION_SPLIT)
		            {
		                app->controller.cmd_message.command = GAME_CMD_SPLIT;
		            }
		        }
		        else
		        {
		            communication_message_t message;

		            message.type = PLAYER_SEND_ACTION;
		            message.player_index = local_player_index;
		            message.player_action = selected_action;

		            Communication_SendMessage(&message);

		            app->slave_action_confirmed = true;
		        }
		    }
		}



		if(app->game.state == BLACKJACK_STATE_BETTING &&
		   app->device_mode != DEVICE_MODE_SLAVE &&
		   !app->controller.bet_confirmed[local_player_index])
		{
		    buttons_enum_t button = KBD_get_pressed_button();

		    if(button == BTN_GOR)
		    {
		        app->local_bet_selection += 25;
		    }
		    else if(button == BTN_DOL)
		    {
		        if(app->local_bet_selection > 25)
		        {
		            app->local_bet_selection -= 25;
		        }
		    }
		    else if(button == BTN_OK)
		    {
		        app->controller.cmd_message.player_index =
		            local_player_index;

		        app->controller.cmd_message.command =
		            GAME_CMD_BET;

		        app->controller.cmd_message.value =
		            app->local_bet_selection;
		    }
		}

		if(app->game.state == BLACKJACK_STATE_ROUND_OVER)
		{
		    blackjack_player_result_t result_0 =
		        app->game.player_result[local_player_index][0];

		    blackjack_player_result_t result_1 =
		        app->game.player_result[local_player_index][1];

		    bool is_split =
		        app->game.player_hands[local_player_index][1].card_count > 0;

		    if(!app->round_result_finished &&
		       (HAL_GetTick() - app->round_result_tick) >= 2000)
		    {
		        app->round_result_finished = true;
		    }

		    if(app->round_result_finished &&
		       app->device_mode == DEVICE_MODE_SINGLE_PLAYER &&
		       app->game.player_balance[0] < 25)
		    {
		        app->state = BLACKJACK_APP_STATE_NO_FUNDS;
		        return;
		    }

		    if(app->round_result_finished &&
		       app->device_mode == DEVICE_MODE_MASTER &&
		       (app->game.player_balance[0] < 25 ||
		        app->game.player_balance[1] < 25))
		    {
		        communication_message_t message;

		        message.type = MASTER_GAME_OVER_NO_FUNDS;
		        message.player_index = 1;

		        Communication_SendMessage(&message);

		        app->state = BLACKJACK_APP_STATE_NO_FUNDS;
		        return;
		    }


		    // ------------------------------------------------
		    // RESULT LED ANIMATION
		    // ------------------------------------------------

		    if(result_0 != PLAYER_RESULT_NONE)
		    {
		        if(!is_split || result_1 != PLAYER_RESULT_NONE)
		        {
		            blackjack_result_led_t animation =
		                Blackjack_GetResultLEDAnimation(
		                    result_0,
		                    result_1,
		                    is_split);

		            Blackjack_UpdateResultLEDs(animation);
		        }
		    }


		    // ------------------------------------------------
		    // PLAY AGAIN / EXIT MENU
		    // ------------------------------------------------

		    if(app->round_result_finished && (app->device_mode != DEVICE_MODE_SLAVE || !app->slave_round_confirmed))
		    {
		        bool confirmed =
		            Menu_HandleInput(&app->round_over_menu);

		        if(confirmed)
		        {
		            communication_player_action_t selected_round_action =
		                PLAYER_ACTION_PLAY_AGAIN;

		            if(app->round_over_menu.selected_index == 1)
		            {
		                selected_round_action =
		                    PLAYER_ACTION_EXIT;
		            }


		            // ----------------------------------------
		            // SINGLE PLAYER / MASTER
		            // ----------------------------------------

		            if(app->device_mode == DEVICE_MODE_SINGLE_PLAYER ||
		               app->device_mode == DEVICE_MODE_MASTER)
		            {
		                app->controller.cmd_message.player_index =
		                    local_player_index;

		                if(selected_round_action ==
		                   PLAYER_ACTION_PLAY_AGAIN)
		                {
		                    app->controller.cmd_message.command =
		                        GAME_CMD_PLAY_AGAIN;
		                }
		                else if(selected_round_action ==
		                        PLAYER_ACTION_EXIT)
		                {
		                    app->controller.cmd_message.command =
		                        GAME_CMD_EXIT;
		                }
		            }


		            // ----------------------------------------
		            // SLAVE
		            // ----------------------------------------

		            else
		            {
		                communication_message_t message;

		                message.type =
		                    PLAYER_SEND_ACTION;

		                message.player_index =
		                    local_player_index;

		                message.player_action =
		                    selected_round_action;

		                Communication_SendMessage(&message);

		                app->slave_round_confirmed = true;
		            }
		        }
		    }
		}

		switch(app -> device_mode)
			          {
			              case DEVICE_MODE_SINGLE_PLAYER:
			              {
			            	  blackjack_state_t previous_state = app->game.state;

			                  GameController_Update(&app -> game, &app -> controller);

			                  if(previous_state != BLACKJACK_STATE_ROUND_OVER && app->game.state == BLACKJACK_STATE_ROUND_OVER)
			                  {
			                      app->round_over_menu.selected_index = 0;
			                      app->round_result_tick = HAL_GetTick();
			                      app->round_result_finished = false;

			                  }

			  			    if(app->controller.exit_requested)
			  			    {
			  			    	BlackjackApp_ResetSessionState(app);
			  			        app->state = BLACKJACK_APP_STATE_MODE_SELECT;
			  			        break;
			  			    }

			  			  if(previous_state != BLACKJACK_STATE_BETTING &&
			  			     app->game.state == BLACKJACK_STATE_BETTING)
			  			  {
			  			      Blackjack_ResetResultLEDs();
			  			      app->local_bet_selection = 25;
			  			  }

			  			  	  break;
			              }

			              case DEVICE_MODE_MASTER:
			              {

			            	  if(!app->multiplayer_connected)
			            	  {
			            	      communication_message_t message;

			            	      if(Communication_GetMessage(&message))
			            	      {
			            	          if(message.type == PLAYER_JOIN)
			            	          {
			            	              communication_message_t assign_message;

			            	              assign_message.type = PLAYER_ASSIGN_INDEX;
			            	              assign_message.player_index = 1;

			            	              Communication_SendMessage(&assign_message);

			            	              app->multiplayer_connected = true;

			            	              Blackjack_StartRound(&app->game);

			            	              app->master_initial_sync_sent = false;
			            	          }
			            	      }

			            	      break;
			            	  }

			            	  if(!app->master_initial_sync_sent)
			            	  {

			            	      communication_message_t balance_message;
			            	      balance_message.type = MASTER_SEND_BALANCE;
			            	      balance_message.player_index = 1;
			            	      balance_message.balance_value = app->game.player_balance[1];
			            	      Communication_SendMessage(&balance_message);

			            	      communication_message_t bet_message;
			            	      bet_message.type = MASTER_SEND_BET;
			            	      bet_message.player_index = 1;
			            	      bet_message.bet_value = app->game.player_bet[1][0];
			            	      Communication_SendMessage(&bet_message);

			            	      for(uint8_t hand_index = 0; hand_index < BLACKJACK_MAX_HANDS; hand_index++)
			            	      {
			            	          communication_message_t hand_message;

			            	          hand_message.type = MASTER_SEND_STATE;
			            	          hand_message.player_index = 1;
			            	          hand_message.current_hand = hand_index;
			            	          hand_message.player_finished = app->game.player_turn_finished[1];
			            	          hand_message.player_state = app->game.player_state[1][hand_index];

			            	          Communication_SendMessage(&hand_message);
			            	      }

			            	      communication_message_t state_message;
			            	      state_message.type = MASTER_SEND_STATE;
			            	      state_message.player_index = 1;
			            	      state_message.game_state = (uint8_t)app->game.state;
			            	      state_message.current_hand = app->game.player_current_hand[1];
			            	      state_message.player_finished = app->game.player_turn_finished[1];
			            	      Communication_SendMessage(&state_message);

			            	      app->master_initial_sync_sent = true;
			            	  }


			            	  communication_message_t message;

			            	  if(app->controller.cmd_message.command == GAME_CMD_NONE)
			            	  {
			            	  if(Communication_GetMessage(&message))
			            	  {

			            		  if(message.type == PLAYER_JOIN)
			            		     {
			            		         communication_message_t assign_message;

			            		         assign_message.type = PLAYER_ASSIGN_INDEX;
			            		         assign_message.player_index = 1;

			            		         Communication_SendMessage(&assign_message);

			            		         app->master_initial_sync_sent = false;
			            		     }

			            		  if(message.type == PLAYER_SEND_BET)
			            		  {
			            			 app -> controller.cmd_message.player_index = message.player_index;
			            			 app -> controller.cmd_message.command = GAME_CMD_BET;
			            			 app -> controller.cmd_message.value = message.bet_value;
			            		  }

			            		  else if(message.type == PLAYER_SEND_ACTION)
			            		  {
			            			  if(message.player_action == PLAYER_ACTION_HIT)
			            			  {
			            				  app ->  controller.cmd_message.player_index = message.player_index;
			            				  app ->  controller.cmd_message.command = GAME_CMD_HIT;
			            			  }

			            			  else if(message.player_action == PLAYER_ACTION_STAND)
			            			  {
			            				  app -> controller.cmd_message.player_index = message.player_index;
			            				  app -> controller.cmd_message.command = GAME_CMD_STAND;
			            			  }

			            			  else if(message.player_action == PLAYER_ACTION_DOUBLE)
			            			  {
			            				  app -> controller.cmd_message.player_index = message.player_index;
			            				  app -> controller.cmd_message.command = GAME_CMD_DOUBLE;
			            			  }

			            			  else if(message.player_action == PLAYER_ACTION_SPLIT)
			            			  {
			            				  app -> controller.cmd_message.player_index = message.player_index;
			            				  app -> controller.cmd_message.command = GAME_CMD_SPLIT;
			            			  }


			            			  else if(message.player_action == PLAYER_ACTION_PLAY_AGAIN)
			            			  {
			            				  app -> controller.cmd_message.player_index = message.player_index;
			            				  app -> controller.cmd_message.command = GAME_CMD_PLAY_AGAIN;
			            			  }

			            			  else if(message.player_action == PLAYER_ACTION_EXIT)
			            			  {
			            			      app->controller.cmd_message.player_index = message.player_index;
			            			      app->controller.cmd_message.command = GAME_CMD_EXIT;
			            			  }

			            		  }
			            	  }
			            	  }

			            	  blackjack_state_t previous_state = app -> game.state;
			            	  uint8_t action_player = app->controller.cmd_message.player_index;
			            	  uint8_t action_hand = app->game.player_current_hand[action_player];
			            	  uint8_t previous_card_count = app->game.player_hands[action_player][action_hand].card_count;
			            	  bool split_command = (app -> controller.cmd_message.command == GAME_CMD_SPLIT);
			            	  bool double_command = (app->controller.cmd_message.command == GAME_CMD_DOUBLE);

			            	  bool slave_action_command =
			            	      (action_player == 1 &&
			            	       (app->controller.cmd_message.command == GAME_CMD_HIT ||
			            	        app->controller.cmd_message.command == GAME_CMD_STAND ||
			            	        app->controller.cmd_message.command == GAME_CMD_DOUBLE ||
			            	        app->controller.cmd_message.command == GAME_CMD_SPLIT));

			            	  uint16_t previous_slave_balance = app->game.player_balance[1];
			            	  uint16_t previous_slave_bet = app->game.player_bet[1][0];

			            	  uint8_t previous_dealer_card_count = app->game.dealer_hand.card_count; //karte dealerja si zapomnimo vnaprej (lažja implementacija razkrivanja dealerjeve skrite karte)
			            	  uint16_t previous_slave_split_bet = app->game.player_bet[1][1];
			            	  blackjack_player_state_t previous_slave_state[BLACKJACK_MAX_HANDS];

			            	  for(uint8_t hand_index = 0; hand_index < BLACKJACK_MAX_HANDS; hand_index++)
			            	  {
			            	      previous_slave_state[hand_index] = app->game.player_state[1][hand_index];
			            	  }

			            	  uint8_t previous_slave_hand = app->game.player_current_hand[1];
			            	  bool previous_slave_finished = app->game.player_turn_finished[1];

			            	  GameController_Update(&app -> game, &app -> controller);

			            	  if(action_player == 1)
			            	  {
			            	      if(double_command && !app->controller.double_succeeded)
			            	      {
			            	          communication_message_t reject_message;

			            	          reject_message.type = MASTER_ACTION_REJECTED;
			            	          reject_message.player_index = 1;

			            	          Communication_SendMessage(&reject_message);
			            	      }

			            	      if(split_command && !app->controller.split_succeeded)
			            	      {
			            	          communication_message_t reject_message;

			            	          reject_message.type = MASTER_ACTION_REJECTED;
			            	          reject_message.player_index = 1;

			            	          Communication_SendMessage(&reject_message);
			            	      }
			            	  }

			            	  for(uint8_t hand_index = 0; hand_index < BLACKJACK_MAX_HANDS; hand_index++)
			            	  {
			            	      if(previous_slave_state[hand_index] != app->game.player_state[1][hand_index])
			            	      {
			            	          communication_message_t message;

			            	          message.type = MASTER_SEND_PLAYER_STATE;
			            	          message.player_index = 1;
			            	          message.hand_index = hand_index;
			            	          message.player_state = (uint8_t)app->game.player_state[1][hand_index];

			            	          Communication_SendMessage(&message);
			            	      }
			            	  }

			            	  if(previous_slave_split_bet != app->game.player_bet[1][1])
			            	  {
			            	      communication_message_t message;

			            	      message.type = MASTER_SEND_SPLIT_BET;
			            	      message.player_index = 1;
			            	      message.bet_value = app->game.player_bet[1][1];

			            	      Communication_SendMessage(&message);
			            	  }

			            	  if(app->game.dealer_hand.card_count > previous_dealer_card_count)		//pošiljanje novih dealerjevih kart
			            	  {
			            		  for(uint8_t card_index = previous_dealer_card_count; card_index < app->game.dealer_hand.card_count; card_index++)
			            		  {
			            		      communication_message_t dealer_message;

			            		      dealer_message.type = MASTER_SEND_DEALER_CARD;
			            		      dealer_message.player_index = 1;
			            		      dealer_message.card_rank = app->game.dealer_hand.cards[card_index].rank;
			            		      dealer_message.card_suit = app->game.dealer_hand.cards[card_index].suit;

			            		      Communication_SendMessage(&dealer_message);
			            		  }
			            	  }

			            	  if(previous_slave_balance != app->game.player_balance[1])
			            	  {
			            	      communication_message_t message;

			            	      message.type = MASTER_SEND_BALANCE;
			            	      message.player_index = 1;
			            	      message.balance_value = app->game.player_balance[1];

			            	      Communication_SendMessage(&message);
			            	  }

			            	  if(previous_slave_bet != app->game.player_bet[1][0])
			            	  {
			            	      communication_message_t message;

			            	      message.type = MASTER_SEND_BET;
			            	      message.player_index = 1;
			            	      message.bet_value = app->game.player_bet[1][0];

			            	      Communication_SendMessage(&message);
			            	  }

			            	  if(previous_state != BLACKJACK_STATE_ROUND_OVER && app->game.state == BLACKJACK_STATE_ROUND_OVER)
			            	  {
			            	      app->round_over_menu.selected_index = 0;

			            	      app->round_result_tick = HAL_GetTick();
			            	      app->round_result_finished = false;

			            	      for(uint8_t hand_index = 0; hand_index < BLACKJACK_MAX_HANDS; hand_index++)
			            	      {
			            	          communication_message_t message;

			            	          message.type = MASTER_SEND_RESULT;
			            	          message.player_index = 1;
			            	          message.hand_index = hand_index;
			            	          message.player_result = app->game.player_result[1][hand_index];

			            	          Communication_SendMessage(&message);
			            	      }
			            	  }

			            	  if(app->controller.exit_requested)
			            	  {
			            	      communication_message_t message;

			            	      message.type = MASTER_EXIT_TO_MENU;
			            	      message.player_index = 1;

			            	      Communication_SendMessage(&message);

			            	      BlackjackApp_ResetSessionState(app);
			            	      app->state = BLACKJACK_APP_STATE_MODE_SELECT;
			            	      break;
			            	  }

			            	  if(previous_state != BLACKJACK_STATE_BETTING && app->game.state == BLACKJACK_STATE_BETTING)
			            	  {
			            		  Blackjack_ResetResultLEDs();
			            	      app->local_bet_selection = 25;
			            	  }

			            	  if(previous_state == BLACKJACK_STATE_BETTING && app->game.state != BLACKJACK_STATE_BETTING)
			            	  {
			            		  hand_t *slave_hand = &app -> game.player_hands[1][0];

			            		  for(uint8_t i = 0; i < slave_hand->card_count; i++)
			            		  {
			            			  communication_message_t message;
			            			  message.type = MASTER_SEND_CARD;
			            			  message.player_index = 1;
			            			  message.hand_index = 0;
			            			  message.card_rank = slave_hand -> cards[i].rank;
			            			  message.card_suit = slave_hand -> cards[i].suit;

			            			  Communication_SendMessage(&message);
			            		  }
			            	  }

			            	  else if(app->game.player_hands[action_player][action_hand].card_count > previous_card_count && action_player == 1 && !split_command)
			            	  {
			            		  communication_message_t message;
			            		  card_t new_card = app->game.player_hands[action_player][action_hand].cards[previous_card_count];

		            			  message.type = MASTER_SEND_CARD;
		            			  message.player_index = 1;
		            			  message.hand_index = action_hand;
		            			  message.card_rank = new_card.rank;
		            			  message.card_suit = new_card.suit;

		            			  Communication_SendMessage(&message);
			            	  }

			            	  if(split_command && action_player == 1 && app -> controller.split_succeeded)
			            	  {

		            			  communication_message_t split_message;
		            			  split_message.type = MASTER_PLAYER_SPLIT;
		            			  split_message.player_index = 1;
		            			  Communication_SendMessage(&split_message);

			            		  for(uint8_t hand_index = 0; hand_index < BLACKJACK_MAX_HANDS; hand_index++)
			            		  {

			            			  	 for(uint8_t card_index = 0; card_index < app -> game.player_hands[1][hand_index].card_count ; card_index++)
			            			  	 {
			            			  		 card_t split_card = app -> game.player_hands[1][hand_index].cards[card_index];

			            			  		 communication_message_t message;
			            			  		 message.type = MASTER_SEND_CARD;
			            			  		 message.player_index = 1;
			            			  		 message.hand_index = hand_index;
			            			  		 message.card_rank = split_card.rank;
			            			  		 message.card_suit = split_card.suit;

			            			  		Communication_SendMessage(&message);
			            			  	 }
			            		  }
			            	  }

			            	  if(previous_state != app->game.state || previous_slave_hand != app->game.player_current_hand[1] || previous_slave_finished != app->game.player_turn_finished[1] ||
			            	     (split_command && app->controller.split_succeeded) || slave_action_command)
			            	  {
			            	      communication_message_t message;

			            	      message.type = MASTER_SEND_STATE;
			            	      message.player_index = 1;
			            	      message.game_state = (uint8_t)app->game.state;
			            	      message.current_hand = app->game.player_current_hand[1];
			            	      message.player_finished = app->game.player_turn_finished[1];

			            	      Communication_SendMessage(&message);
			            	  }

			                  break;
			              }

			              case DEVICE_MODE_SLAVE:
			              {

			            	  if(!app->multiplayer_connected)
			            	  {
			            	      uint32_t now = HAL_GetTick();

			            	      if(!app->join_message_sent ||
			            	         (now - app->join_retry_tick) >= 500)
			            	      {
			            	          communication_message_t join_message;

			            	          join_message.type = PLAYER_JOIN;
			            	          join_message.player_index = 1;

			            	          Communication_SendMessage(&join_message);

			            	          app->join_message_sent = true;
			            	          app->join_retry_tick = now;
			            	      }

			            	      communication_message_t message;

			            	      if(Communication_GetMessage(&message))
			            	      {
			            	          if(message.type == PLAYER_ASSIGN_INDEX)
			            	          {
			            	              app->multiplayer_connected = true;
			            	          }
			            	      }

			            	      break;
			            	  }

			            	  communication_message_t message;

			            	  if(app->game.state == BLACKJACK_STATE_BETTING && !app->slave_bet_confirmed)
			            	  {
			            	      buttons_enum_t button = KBD_get_pressed_button();

			            	      if(button == BTN_GOR)
			            	      {
			            	          app->slave_bet_selection += 25;
			            	      }

			            	      else if(button == BTN_DOL)
			            	      {
			            	          if(app->slave_bet_selection > 25)
			            	          {
			            	              app->slave_bet_selection -= 25;
			            	          }
			            	      }

			            	      else if(button == BTN_OK)
			            	      {
			            	          communication_message_t message;

			            	          message.type = PLAYER_SEND_BET;
			            	          message.player_index = 1;
			            	          message.bet_value = app->slave_bet_selection;

			            	          Communication_SendMessage(&message);

			            	          app->slave_bet_confirmed = true;
			            	      }
			            	  }

			            	  if(Communication_GetMessage(&message))
			            	  {
			            		  if(message.type == MASTER_SEND_CARD)
			            		  {
			            			  card_t received_card;
			            			  received_card.rank = message.card_rank;
			            			  received_card.suit = message.card_suit;

			            			  Hand_AddCard(&app -> game.player_hands[message.player_index][message.hand_index], received_card);
			            		  }

			            		  else if(message.type == MASTER_PLAYER_SPLIT)
			            		  {
			            		      for(uint8_t hand_index = 0; hand_index < BLACKJACK_MAX_HANDS; hand_index++)
			            		      {
			            		          Hand_Init(&app->game.player_hands[message.player_index][hand_index]);
			            		      }
			            		      app->action_menu.selected_index = 0;
			            		  }



			            		  else if(message.type == MASTER_SEND_STATE)
			            		  {
			            		      blackjack_state_t old_state = app->game.state;
			            		      uint8_t old_hand = app->game.player_current_hand[1];
			            		      bool old_finished = app->game.player_turn_finished[1];

			            		      app->game.state = (blackjack_state_t)message.game_state;
			            		      app->game.player_current_hand[1] = message.current_hand;
			            		      app->game.player_turn_finished[1] = message.player_finished;

			            		      if(app->game.state == BLACKJACK_STATE_PLAYER_TURN &&
			            		         !app->game.player_turn_finished[1])
			            		      {
			            		          app->slave_action_confirmed = false;
			            		          app->action_menu.selected_index = 0;
			            		      }

			            		      if(old_state != BLACKJACK_STATE_ROUND_OVER &&
			            		         app->game.state == BLACKJACK_STATE_ROUND_OVER)
			            		      {
			            		          app->slave_round_confirmed = false;
			            		          app->round_over_menu.selected_index = 0;
			            		          app->round_result_tick = HAL_GetTick();
			            		          app->round_result_finished = false;
			            		      }

			            		      if(old_hand != app->game.player_current_hand[1] ||
			            		         old_finished != app->game.player_turn_finished[1])
			            		      {
			            		          app->slave_action_confirmed = false;
			            		      }

			            		      if(old_state != BLACKJACK_STATE_BETTING &&
			            		         app->game.state == BLACKJACK_STATE_BETTING)
			            		      {
			            		          Blackjack_ResetResultLEDs();

			            		          app->slave_bet_confirmed = false;
			            		          app->slave_round_confirmed = false;
			            		          app->slave_bet_selection = 25;
			            		          app->round_over_menu.selected_index = 0;

			            		          Hand_Init(&app->game.dealer_hand);

			            		          for(uint8_t hand_index = 0; hand_index < BLACKJACK_MAX_HANDS; hand_index++)
			            		          {
			            		              Hand_Init(&app->game.player_hands[1][hand_index]);
			            		              app->game.player_result[1][hand_index] = PLAYER_RESULT_NONE;
			            		          }

			            		          app->game.player_current_hand[1] = 0;
			            		          app->game.player_turn_finished[1] = false;
			            		      }
			            		  }

			            		  else if(message.type == MASTER_GAME_OVER_NO_FUNDS)
			            		  {
			            			  app->round_result_finished = false;
			            			  app->slave_round_confirmed = false;

			            		      app->state = BLACKJACK_APP_STATE_NO_FUNDS;
			            		  }

			            		  else if(message.type == MASTER_EXIT_TO_MENU)
			            		  {

			            			      Blackjack_ResetResultLEDs();
			            			      BlackjackApp_ResetSessionState(app);
			            			      Blackjack_Init(&app->game);

			            			      app->state = BLACKJACK_APP_STATE_MODE_SELECT;
			            			      break;
			            		  }

			            		  else if(message.type == MASTER_SEND_RESULT)
			            		  {
			            		      app->game.player_result[message.player_index][message.hand_index] = (blackjack_player_result_t)message.player_result;
			            		  }

			            		  else if(message.type == MASTER_SEND_DEALER_CARD)
			            		  {
			            		      card_t dealer_card;

			            		      dealer_card.rank = message.card_rank;
			            		      dealer_card.suit = message.card_suit;

			            		      Hand_AddCard(&app->game.dealer_hand, dealer_card);
			            		  }

			            		  else if(message.type == MASTER_SEND_BALANCE)
			            		  {
			            		      app->game.player_balance[message.player_index] = message.balance_value;
			            		  }

			            		  else if(message.type == MASTER_SEND_BET)
			            		  {
			            		      app->game.player_bet[message.player_index][0] = message.bet_value;
			            		  }

			            		  else if(message.type == MASTER_SEND_SPLIT_BET)
			            		  {
			            		      app->game.player_bet[message.player_index][1] = message.bet_value;
			            		  }

			            		  else if(message.type == MASTER_SEND_PLAYER_STATE)
			            		  {
			            		      app->game.player_state[message.player_index][message.hand_index] = (blackjack_player_state_t)message.player_state;
			            		  }

			            		  else if(message.type == MASTER_ACTION_REJECTED)
			            		  {
			            		      app->slave_action_confirmed = false;
			            		  }

			            	  }
			                  break;
			              }
			          }


	}

	else if(app->state == BLACKJACK_APP_STATE_NO_FUNDS)
	{
	    /*
	     * SLAVE:
	     * Stay on the popup and keep listening for the master's
	     * MASTER_EXIT_TO_MENU message.
	     */
	    if(app->device_mode == DEVICE_MODE_SLAVE)
	    {
	        communication_message_t message;

	        if(Communication_GetMessage(&message))
	        {
	            if(message.type == MASTER_EXIT_TO_MENU)
	            {
	                Blackjack_ResetResultLEDs();
	                BlackjackApp_ResetSessionState(app);
	                Blackjack_Init(&app->game);

	                KBD_flush();

	                app->state = BLACKJACK_APP_STATE_MODE_SELECT;
	            }
	        }

	        return;
	    }

	    /*
	     * SINGLE PLAYER / MASTER:
	     * OK dismisses the popup.
	     */
	    if(KBD_get_pressed_button() == BTN_OK)
	    {
	        if(app->device_mode == DEVICE_MODE_MASTER)
	        {
	            communication_message_t message;

	            message.type = MASTER_EXIT_TO_MENU;
	            message.player_index = 1;

	            Communication_SendMessage(&message);
	        }

	        Blackjack_ResetResultLEDs();
	        BlackjackApp_ResetSessionState(app);
	        Blackjack_Init(&app->game);

	        KBD_flush();

	        app->state = BLACKJACK_APP_STATE_MODE_SELECT;
	    }

	    return;
	}
}
