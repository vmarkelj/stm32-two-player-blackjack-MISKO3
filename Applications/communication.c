#include "communication.h"
#include "SCI.h"


static uint8_t Communication_CalculateChecksum(uint8_t *data, uint8_t length)
{
    uint8_t checksum = 0;

    for(uint8_t i = 0; i < length; i++)
    {
        checksum ^= data[i];
    }

    return checksum;
}


bool Communication_GetMessage(communication_message_t *message)   //koda za sprejemanje sporočila poslanega z slave miška
{
    uint8_t byte;
    uint8_t packet[COMM_PACKET_SIZE];

    while(SCI_RX_buffer_get_data_size() >= COMM_PACKET_SIZE)
    {
        SCI_RX_buffer_get_byte(&byte);

        if(byte != COMM_START_BYTE)
        {
            continue;
        }

        packet[0] = byte;

             if(SCI_RX_buffer_get_bytes(&packet[1], COMM_PACKET_SIZE - 1) != BUFFER_OK)
             {
                 return false;
             }

             uint8_t calculated_checksum =
                     Communication_CalculateChecksum(packet, COMM_PACKET_SIZE - 1);

             if(calculated_checksum != packet[COMM_PACKET_SIZE - 1])
             {
                 continue;
             }

             if(packet[1] > MASTER_GAME_OVER_NO_FUNDS)
             {
                 continue;
             }

             message->type = (communication_message_type_t)packet[1];  // dodajanje tipa communication_message_type_t prispelemu sporočilu

             message->player_index = packet[2];

             switch(message->type)
             {
                 case PLAYER_JOIN:
                 {
                	 return true;
                 }

                 case PLAYER_ASSIGN_INDEX:
                 {
                	 return true;
                 }

                 case PLAYER_SEND_BET:
                 {
                	   message->bet_value = ((uint16_t)packet[3] << 8) | packet[4];		//rekonstrukcija dveh 8 bitnih paketov v eno 16bitno število ki predstavlja stavo

                	    return true;
                 }

                 case PLAYER_SEND_ACTION:
                 {
                	    if(packet[3] > PLAYER_ACTION_EXIT)				//preverjanje ali je akcija ena izmed definiranih akcij
                	    {
                	        continue;
                	    }

                	    message->player_action = (communication_player_action_t)packet[3];

                	    return true;
                 }

                 case MASTER_SEND_CARD:
                 {
                     message->card_rank = packet[3];
                     message->card_suit = packet[4] & 0x03;
                     message->hand_index = packet[4] >> 2;
                     return true;
                 }

                 case MASTER_PLAYER_SPLIT:
                 {
                	 return true;
                 }

                 case MASTER_SEND_STATE:
                 {
                     message -> game_state = packet[3];
                     message->current_hand = packet[4] & 0x0F;
                     message->player_finished = (packet[4] >> 4) & 0x01;
                     return true;
                 }

                 case MASTER_EXIT_TO_MENU:
                 {
                     return true;
                 }

                 case MASTER_SEND_BALANCE:
                 {
                     message -> balance_value = ((uint16_t)packet[3] << 8) | packet[4];
                     return true;
                 }

                 case MASTER_SEND_BET:
                 {
                     message -> bet_value = ((uint16_t)packet[3] << 8) | packet[4];
                     return true;
                 }

                 case MASTER_SEND_RESULT:
                 {
                     message -> player_result = packet[3];
                     message -> hand_index = packet[4];
                     return true;
                 }

                 case MASTER_SEND_DEALER_CARD:
                 {
                     message -> card_rank = (card_rank_t)packet[3];
                     message -> card_suit = (card_suit_t)packet[4];
                     return true;
                 }

             	case MASTER_SEND_SPLIT_BET:
             	{
             		message->bet_value = ((uint16_t)packet[3] << 8) | packet[4];
             		return true;
             	}

             	case MASTER_SEND_PLAYER_STATE:
             	{
             	    message->player_state = packet[3];
             	    message->hand_index = packet[4];
             	    return true;
             	}

             	case MASTER_ACTION_REJECTED:
             	{
             	    return true;
             	}

             	case MASTER_GAME_OVER_NO_FUNDS:
             	{
             		return true;
             	}
             }

         }

    return false;
}


void Communication_SendMessage(communication_message_t *message)
{
    uint8_t packet[COMM_PACKET_SIZE];

    packet[0] = COMM_START_BYTE;
    packet[1] = (uint8_t)message->type;
    packet[2] = message->player_index;

    switch(message -> type)
    {
    	case PLAYER_JOIN:
    	{
    	    packet[3] = 0;
    	    packet[4] = 0;

    		break;
    	}

    	case PLAYER_ASSIGN_INDEX:
    	{
    	    packet[3] = 0;
    	    packet[4] = 0;

    		break;
    	}

    	case PLAYER_SEND_BET:
    	{
    	    packet[3] = (uint8_t)(message->bet_value >> 8);
    	    packet[4] = (uint8_t)(message->bet_value & 0xFF);

    		break;
    	}

    	case PLAYER_SEND_ACTION:
    	{
    	    packet[3] = (uint8_t)message->player_action;
    	    packet[4] = 0;

    		break;
    	}

    	case MASTER_SEND_CARD:
    	{
    	    packet[3] = message->card_rank;
    	    packet[4] = (message->hand_index << 2) | message->card_suit;

    	    break;
    	}

    	case MASTER_PLAYER_SPLIT:
    	{
    		packet[3] = 0;
    		packet[4] = 0;

    		break;
    	}

    	case MASTER_SEND_STATE:
    	{
    		packet[3] = message->game_state;
    		packet[4] = (message->current_hand & 0x0F) | ((message->player_finished & 0x01) << 4);
    		break;
    	}

    	case MASTER_EXIT_TO_MENU:
    	{
    	    packet[3] = 0;
    	    packet[4] = 0;
    	    break;
    	}

    	case MASTER_SEND_BALANCE:
    	{
    		packet[3] = (uint8_t)(message->balance_value >> 8);
    		packet[4] = (uint8_t)message->balance_value;
    		break;
    	}

    	case MASTER_SEND_BET:
    	{
    		packet[3] = (uint8_t)(message->bet_value >> 8);
    		packet[4] = (uint8_t)message->bet_value;
    		break;
    	}

    	case MASTER_SEND_RESULT:
    	{
    		packet[3] = message->player_result;
    		packet[4] = message->hand_index;
    		break;
    	}

    	case MASTER_SEND_DEALER_CARD:
    	{
    	    packet[3] = message->card_rank;
    	    packet[4] = message->card_suit;
    	    break;
    	}

    	case MASTER_SEND_SPLIT_BET:
    	{
    		packet[3] = (uint8_t)(message->bet_value >> 8);
    		packet[4] = (uint8_t)message->bet_value;
    		break;
    	}

    	case MASTER_SEND_PLAYER_STATE:
    	{
    	    packet[3] = message->player_state;
    	    packet[4] = message->hand_index;
    	    break;
    	}

    	case MASTER_ACTION_REJECTED:
    	{
    	    packet[3] = 0;
    	    packet[4] = 0;
    	    break;
    	}

    	case MASTER_GAME_OVER_NO_FUNDS:
    	{
    	    packet[3] = 0;
    	    packet[4] = 0;
    	    break;
    	}
    }

    packet[COMM_PACKET_SIZE - 1] = Communication_CalculateChecksum(packet, COMM_PACKET_SIZE - 1);

    SCI_send_bytes(packet, COMM_PACKET_SIZE);
}

