#include "blackjack_ui.h"
#include "lcd.h"
#include "ugui.h"
#include <string.h>
#include <stdio.h>
#include"menu_bg.h"
#include "game_bg.h"


static void BlackjackUI_DrawMenuBackground(void)
{
    ILI9341_SetDisplayWindow(0, 0, 320, 240);

    ILI9341_SendData(
        (LCD_IO_Data_t *)g_menu_bg_data,
        320 * 240);
}

static void BlackjackUI_DrawGameBackground(void)
{
    ILI9341_SetDisplayWindow(0, 0, 320, 240);

    ILI9341_SendData(
        (LCD_IO_Data_t *)g_game_bg_data,
        320 * 240);
}

static void BlackjackUI_RestoreGameBackgroundRegion(
    uint16_t x,
    uint16_t y,
    uint16_t width,
    uint16_t height)
{
    for(uint16_t row = 0; row < height; row++)
    {
        ILI9341_SetDisplayWindow(
            x,
            y + row,
            width,
            1);

        ILI9341_SendData(
            (LCD_IO_Data_t *)
            &g_game_bg_data[(y + row) * 320 + x],
            width);
    }
}

static uint8_t BlackjackUI_GetCardValue(card_t card)
{
    if(card.rank == RANK_ACE)
    {
        return 11;
    }
    else if(card.rank >= RANK_2 && card.rank <= RANK_10)
    {
        return card.rank;
    }
    else
    {
        return 10;
    }
}

static uint8_t BlackjackUI_GetSuitChar(card_suit_t suit)
{
    switch(suit)
    {
        case SUIT_HEARTS:   return 0x03;
        case SUIT_DIAMONDS: return 0x04;
        case SUIT_CLUBS:    return 0x05;
        case SUIT_SPADES:   return 0x06;
        default:            return '?';
    }
}


static UG_COLOR BlackjackUI_GetSuitColor(card_suit_t suit)
{
    if(suit == SUIT_HEARTS || suit == SUIT_DIAMONDS)
    {
        return C_RED;
    }

    return C_BLACK;
}

#define CARD_WIDTH_NORMAL   42
#define CARD_HEIGHT_NORMAL  62

#define CARD_WIDTH_SMALL    34
#define CARD_HEIGHT_SMALL   50

static void BlackjackUI_DrawChar180(char chr,					// zarotira desno spodnjo oznako karte na glavo
                                    uint16_t x,
                                    uint16_t y,
                                    UG_COLOR color,
                                    const UG_FONT *font)
{
    uint8_t bytes_per_row;
    uint16_t char_index;

    if(chr < font->start_char || chr > font->end_char)
    {
        return;
    }

    bytes_per_row = font->char_width / 8;

    if(font->char_width % 8)
    {
        bytes_per_row++;
    }

    char_index =
        (chr - font->start_char) *
        font->char_height *
        bytes_per_row;

    for(uint16_t row = 0; row < font->char_height; row++)
    {
        for(uint16_t col = 0; col < font->char_width; col++)
        {
            uint16_t byte_index =
                char_index +
                row * bytes_per_row +
                col / 8;

            uint8_t bit = col % 8;

            if(font->p[byte_index] & (1 << bit))
            {
                UG_DrawPixel(
                    x + (font->char_width - 1 - col),
                    y + (font->char_height - 1 - row),
                    color);
            }
        }
    }
}

static void BlackjackUI_DrawPip(uint16_t x,
                                uint16_t y,
                                uint8_t suit_char,
                                UG_COLOR color)
{
    UG_FontSelect(&FONT_8X8);

    UG_PutChar(suit_char,
               x,
               y,
               color,
               C_WHITE);
}

static void BlackjackUI_DrawPips(uint16_t x,
                                 uint16_t y,
                                 uint8_t value,
                                 uint8_t suit_char,
                                 UG_COLOR color,
                                 blackjack_ui_card_size_t size)
{
    uint16_t left;
    uint16_t center;
    uint16_t right;

    uint16_t top;
    uint16_t upper_mid;
    uint16_t middle;
    uint16_t lower_mid;
    uint16_t bottom;


    if(size == CARD_SIZE_SMALL)
    {
        // Small card: 34 x 50
        left   = x + 6;
        center = x + 13;
        right  = x + 20;

        top       = y + 13;
        upper_mid = y + 19;
        middle    = y + 24;
        lower_mid = y + 29;
        bottom    = y + 35;
    }
    else
    {
        // Normal card: 42 x 62
        left   = x + 8;
        center = x + 17;
        right  = x + 26;

        top       = y + 17;
        upper_mid = y + 25;
        middle    = y + 29;
        lower_mid = y + 37;
        bottom    = y + 45;
    }


    switch(value)
    {
        case 2:
            BlackjackUI_DrawPip(center, top,
                                suit_char, color);

            BlackjackUI_DrawPip(center, bottom,
                                suit_char, color);
            break;


        case 3:
            BlackjackUI_DrawPip(center, top,
                                suit_char, color);

            BlackjackUI_DrawPip(center, middle,
                                suit_char, color);

            BlackjackUI_DrawPip(center, bottom,
                                suit_char, color);
            break;


        case 4:
            BlackjackUI_DrawPip(left, top,
                                suit_char, color);

            BlackjackUI_DrawPip(right, top,
                                suit_char, color);

            BlackjackUI_DrawPip(left, bottom,
                                suit_char, color);

            BlackjackUI_DrawPip(right, bottom,
                                suit_char, color);
            break;


        case 5:
            BlackjackUI_DrawPip(left, top,
                                suit_char, color);

            BlackjackUI_DrawPip(right, top,
                                suit_char, color);

            BlackjackUI_DrawPip(center, middle,
                                suit_char, color);

            BlackjackUI_DrawPip(left, bottom,
                                suit_char, color);

            BlackjackUI_DrawPip(right, bottom,
                                suit_char, color);
            break;


        case 6:
            BlackjackUI_DrawPip(left, top,
                                suit_char, color);

            BlackjackUI_DrawPip(right, top,
                                suit_char, color);

            BlackjackUI_DrawPip(left, middle,
                                suit_char, color);

            BlackjackUI_DrawPip(right, middle,
                                suit_char, color);

            BlackjackUI_DrawPip(left, bottom,
                                suit_char, color);

            BlackjackUI_DrawPip(right, bottom,
                                suit_char, color);
            break;


        case 7:
            BlackjackUI_DrawPip(left, top,
                                suit_char, color);

            BlackjackUI_DrawPip(right, top,
                                suit_char, color);

            BlackjackUI_DrawPip(center, upper_mid,
                                suit_char, color);

            BlackjackUI_DrawPip(left, middle,
                                suit_char, color);

            BlackjackUI_DrawPip(right, middle,
                                suit_char, color);

            BlackjackUI_DrawPip(left, bottom,
                                suit_char, color);

            BlackjackUI_DrawPip(right, bottom,
                                suit_char, color);
            break;


        case 8:
            BlackjackUI_DrawPip(left, top,
                                suit_char, color);

            BlackjackUI_DrawPip(right, top,
                                suit_char, color);

            BlackjackUI_DrawPip(left, upper_mid,
                                suit_char, color);

            BlackjackUI_DrawPip(right, upper_mid,
                                suit_char, color);

            BlackjackUI_DrawPip(left, lower_mid,
                                suit_char, color);

            BlackjackUI_DrawPip(right, lower_mid,
                                suit_char, color);

            BlackjackUI_DrawPip(left, bottom,
                                suit_char, color);

            BlackjackUI_DrawPip(right, bottom,
                                suit_char, color);
            break;


        case 9:
            BlackjackUI_DrawPip(left, top,
                                suit_char, color);

            BlackjackUI_DrawPip(right, top,
                                suit_char, color);

            BlackjackUI_DrawPip(left, upper_mid,
                                suit_char, color);

            BlackjackUI_DrawPip(right, upper_mid,
                                suit_char, color);

            BlackjackUI_DrawPip(center, middle,
                                suit_char, color);

            BlackjackUI_DrawPip(left, lower_mid,
                                suit_char, color);

            BlackjackUI_DrawPip(right, lower_mid,
                                suit_char, color);

            BlackjackUI_DrawPip(left, bottom,
                                suit_char, color);

            BlackjackUI_DrawPip(right, bottom,
                                suit_char, color);
            break;


        case 10:
            BlackjackUI_DrawPip(left, top,
                                suit_char, color);

            BlackjackUI_DrawPip(right, top,
                                suit_char, color);

            BlackjackUI_DrawPip(center, upper_mid,
                                suit_char, color);

            BlackjackUI_DrawPip(left, upper_mid,
                                suit_char, color);

            BlackjackUI_DrawPip(right, upper_mid,
                                suit_char, color);

            BlackjackUI_DrawPip(left, lower_mid,
                                suit_char, color);

            BlackjackUI_DrawPip(right, lower_mid,
                                suit_char, color);

            BlackjackUI_DrawPip(center, lower_mid,
                                suit_char, color);

            BlackjackUI_DrawPip(left, bottom,
                                suit_char, color);

            BlackjackUI_DrawPip(right, bottom,
                                suit_char, color);
            break;
    }
}

static void BlackjackUI_DrawCard(uint16_t x,
                                 uint16_t y,
                                 card_t card,
                                 blackjack_ui_card_size_t size)
{
    uint8_t suit_char = BlackjackUI_GetSuitChar(card.suit);
    UG_COLOR suit_color = BlackjackUI_GetSuitColor(card.suit);

    char rank_text[3];

    uint16_t card_width;
    uint16_t card_height;

    if(size == CARD_SIZE_SMALL)
    {
        card_width  = CARD_WIDTH_SMALL;
        card_height = CARD_HEIGHT_SMALL;
    }
    else
    {
        card_width  = CARD_WIDTH_NORMAL;
        card_height = CARD_HEIGHT_NORMAL;
    }

    // ------------------------------------------------
    // CARD BACKGROUND
    // ------------------------------------------------

    UG_FillFrame(
        x,
        y,
        x + card_width,
        y + card_height,
        C_WHITE);

    UG_DrawFrame(
        x,
        y,
        x + card_width,
        y + card_height,
        C_BLACK);

    UG_SetBackcolor(C_WHITE);

    // ------------------------------------------------
    // DETERMINE RANK TEXT
    // ------------------------------------------------

    if(card.rank == RANK_ACE)
    {
        rank_text[0] = 'A';
        rank_text[1] = '\0';
    }
    else if(card.rank == RANK_JACK)
    {
        rank_text[0] = 'J';
        rank_text[1] = '\0';
    }
    else if(card.rank == RANK_QUEEN)
    {
        rank_text[0] = 'Q';
        rank_text[1] = '\0';
    }
    else if(card.rank == RANK_KING)
    {
        rank_text[0] = 'K';
        rank_text[1] = '\0';
    }
    else if(card.rank == RANK_10)
    {
        rank_text[0] = '1';
        rank_text[1] = '0';
        rank_text[2] = '\0';
    }
    else
    {
        rank_text[0] = '0' + card.rank;
        rank_text[1] = '\0';
    }

    // ------------------------------------------------
    // TOP-LEFT RANK
    // ------------------------------------------------

    UG_FontSelect(&FONT_7X12);
    UG_SetForecolor(C_BLACK);
    UG_SetBackcolor(C_WHITE);

    UG_PutString(
        x + 2,
        y + 2,
        rank_text);

    // ------------------------------------------------
    // BOTTOM-RIGHT RANK - ROTATED 180 DEGREES
    // ------------------------------------------------

    if(card.rank == RANK_10)
    {
        BlackjackUI_DrawChar180(
            '0',
            x + card_width - 9,
            y + card_height - 14,
            C_BLACK,
            &FONT_7X12);

        BlackjackUI_DrawChar180(
            '1',
            x + card_width - 16,
            y + card_height - 14,
            C_BLACK,
            &FONT_7X12);
    }
    else
    {
        BlackjackUI_DrawChar180(
            rank_text[0],
            x + card_width - 9,
            y + card_height - 14,
            C_BLACK,
            &FONT_7X12);
    }

    // ------------------------------------------------
    // ACE / FACE CARDS
    // ------------------------------------------------

    if(card.rank == RANK_ACE ||
       card.rank == RANK_JACK ||
       card.rank == RANK_QUEEN ||
       card.rank == RANK_KING)
    {
        if(size == CARD_SIZE_SMALL)
        {
            UG_FontSelect(&FONT_8X12);

            UG_PutChar(
                suit_char,
                x + (card_width - 8) / 2,
                y + (card_height - 12) / 2,
                suit_color,
                C_WHITE);
        }
        else
        {
            UG_FontSelect(&FONT_16X26);

            UG_PutChar(
                suit_char,
                x + (card_width - 16) / 2,
                y + (card_height - 26) / 2,
                suit_color,
                C_WHITE);
        }

        return;
    }

    // ------------------------------------------------
    // NUMBER CARDS 2 - 10
    // ------------------------------------------------

    BlackjackUI_DrawPips(
        x,
        y,
        card.rank,
        suit_char,
        suit_color,
        size);
}

static void BlackjackUI_DrawHiddenCard(uint16_t x, uint16_t y)
{
    UG_FillFrame(x,
                 y,
                 x + CARD_WIDTH_NORMAL,
                 y + CARD_HEIGHT_NORMAL,
                 C_BLACK);

    UG_DrawFrame(x,
                 y,
                 x + CARD_WIDTH_NORMAL,
                 y + CARD_HEIGHT_NORMAL,
                 C_WHITE);

    UG_SetBackcolor(C_BLACK);
    UG_SetForecolor(C_WHITE);
    UG_FontSelect(&FONT_16X26);

    UG_PutString(x + 13, y + 18, "?");
}

static void BlackjackUI_DrawCharTransparent(
    char chr,
    uint16_t x,
    uint16_t y,
    UG_COLOR color,
    const UG_FONT *font)
{
    uint8_t bytes_per_row;
    uint32_t char_index;

    if(chr < font->start_char || chr > font->end_char)
    {
        return;
    }

    bytes_per_row = font->char_width / 8;

    if(font->char_width % 8)
    {
        bytes_per_row++;
    }

    char_index =
        (chr - font->start_char) *
        font->char_height *
        bytes_per_row;

    for(uint16_t row = 0; row < font->char_height; row++)
    {
        for(uint16_t col = 0; col < font->char_width; col++)
        {
            uint32_t byte_index =
                char_index +
                row * bytes_per_row +
                col / 8;

            uint8_t bit = col % 8;

            if(font->p[byte_index] & (1U << bit))
            {
                UG_DrawPixel(
                    x + col,
                    y + row,
                    color);
            }
        }
    }
}

static void BlackjackUI_DrawStringTransparent(
    const char *text,
    uint16_t x,
    uint16_t y,
    UG_COLOR color,
    const UG_FONT *font)
{
    uint16_t current_x = x;

    while(*text != '\0')
    {
        BlackjackUI_DrawCharTransparent(
            *text,
            current_x,
            y,
            color,
            font);

        current_x += font->char_width + 1;
        text++;
    }
}


void BlackjackUI_Update(blackjack_app_t *app)
{
    static uint8_t previous_mode_index = 0xFF;
    static bool     static_ui_drawn    = false;			//zapomni si prejšnja stanja za prepoznavo, kdaj je potrebno na novo narisati določene dele
    static bool running_ui_drawn = false;

    static uint16_t previous_bet = 0xFFFF;
    static uint16_t previous_balance = 0xFFFF;			//zapomni si prejšnja stanja za prepoznavo, kdaj je potrebno na novo narisati določene dele
    static uint8_t previous_total = 0xFF;

    static uint8_t previous_player_card_count = 0xFF;
    static uint8_t previous_dealer_card_count = 0xFF;	//zapomni si prejšnja stanja za prepoznavo, kdaj je potrebno na novo narisati določene dele
    static bool previous_dealer_reveal = false;

    static uint8_t previous_dealer_total = 0xFF;
    static uint8_t previous_action_index = 0xFF;

    static bool round_over_ui_drawn = false;
    static uint8_t previous_round_over_index = 0xFF;
    static blackjack_state_t previous_ui_game_state = BLACKJACK_STATE_IDLE;
    static bool decision_popup_drawn = false;

    static bool no_funds_ui_drawn = false;

    static uint8_t previous_player_hand = 0xFF;
    static bool previous_player_split = false;

    if(app->state == BLACKJACK_APP_STATE_MODE_SELECT)
    {

    	running_ui_drawn = false;



    	previous_bet = 0xFFFF;
    	previous_balance = 0xFFFF;			//resetiranje ob prehodu nazaj v mode select
    	previous_total = 0xFF;

    	previous_player_card_count = 0xFF;
    	previous_dealer_card_count = 0xFF;			//resetiranje ob prehodu nazaj v mode select
    	previous_dealer_reveal = false;

    	previous_action_index = 0xFF;

    	round_over_ui_drawn = false;
    	previous_round_over_index = 0xFF;
    	decision_popup_drawn = false;

    	previous_ui_game_state = BLACKJACK_STATE_IDLE;

    	no_funds_ui_drawn = false;
    	previous_player_hand = 0xFF;
    	previous_player_split = false;

        static const char* mode_labels[3] = {
            "SINGLE PLAYER",
            "MASTER",
            "SLAVE"
        };



        // prvotni napisi na home menu
        if(!static_ui_drawn)
        {


        	BlackjackUI_DrawMenuBackground();

        	    previous_mode_index = 0xFF;
        	    static_ui_drawn = true;
        }

        // posodobi le napise ki so se spremenili ostalo pusti tako kot je
        if(app->device_mode_menu.selected_index != previous_mode_index)
        {
            static const uint16_t option_y[3] =
            {
                125,
                155,
                185
            };

            const uint16_t box_x1 = 72;
            const uint16_t box_x2 = 248;

            UG_FontSelect(&FONT_8X14);

            for(uint8_t i = 0;
                i < app->device_mode_menu.option_count;
                i++)
            {
                bool is_selected =
                    (i == app->device_mode_menu.selected_index);

                bool needs_redraw =
                    is_selected ||
                    (i == previous_mode_index) ||
                    (previous_mode_index == 0xFF);

                if(!needs_redraw)
                {
                    continue;
                }

                uint16_t y = option_y[i];

                UG_FillFrame(
                    box_x1,
                    y - 3,
                    box_x2,
                    y + 17,
                    is_selected ? C_GOLD : C_DARK_GREEN);

                UG_DrawFrame(
                    box_x1,
                    y - 3,
                    box_x2,
                    y + 17,
                    C_GOLD);

                const char *text = mode_labels[i];

                uint8_t len = strlen(text);

                uint16_t text_width =
                    len * 8;

                uint16_t text_x =
                    box_x1 +
                    ((box_x2 - box_x1) - text_width) / 2;

                UG_SetForecolor(
                    is_selected ? C_BLACK : C_WHITE);

                UG_SetBackcolor(
                    is_selected ? C_GOLD : C_DARK_GREEN);

                UG_PutString(
                    text_x,
                    y,
                    text);
            }

            previous_mode_index =
                app->device_mode_menu.selected_index;
        }
    }

    else if(app->state == BLACKJACK_APP_STATE_RUNNING)
    {
        static_ui_drawn = false;

        if(previous_ui_game_state == BLACKJACK_STATE_ROUND_OVER &&
           app->game.state != BLACKJACK_STATE_ROUND_OVER)
        {
            // Redraw the clean gameplay screen
            BlackjackUI_DrawGameBackground();

            // Force all dynamic elements to redraw
            previous_bet = 0xFFFF;
            previous_balance = 0xFFFF;

            previous_total = 0xFF;
            previous_dealer_total = 0xFF;

            previous_player_card_count = 0xFF;
            previous_player_hand = 0xFF;
            previous_player_split = false;
            previous_dealer_card_count = 0xFF;
            previous_dealer_reveal = false;

            previous_action_index = 0xFF;

            round_over_ui_drawn = false;
            previous_round_over_index = 0xFF;
            decision_popup_drawn = false;
        }


        // Force a fresh result screen for every new round
        if(previous_ui_game_state != BLACKJACK_STATE_BETTING &&
           app->game.state == BLACKJACK_STATE_BETTING)
        {
            round_over_ui_drawn = false;
            previous_round_over_index = 0xFF;
        }

        if(!running_ui_drawn)
        {
            BlackjackUI_DrawGameBackground();

            running_ui_drawn = true;
        }


        // ------------------------------------------------
        // Dynamic values
        // ------------------------------------------------

        // ------------------------------------------------
        // Dynamic values
        // ------------------------------------------------

        uint8_t local_player_index;
        uint16_t displayed_bet;
        char buffer[12];


        // Determine local player

        if(app->device_mode == DEVICE_MODE_MASTER ||
           app->device_mode == DEVICE_MODE_SINGLE_PLAYER)
        {
            local_player_index = 0;
        }
        else
        {
            local_player_index = 1;
        }


        // ------------------------------------------------
        // ACTION MENU
        // ------------------------------------------------

        // ------------------------------------------------
        // ACTION MENU
        // ------------------------------------------------

        if(app->game.state == BLACKJACK_STATE_PLAYER_TURN &&
           app->action_menu.selected_index != previous_action_index)
        {
            static const char *actions[4] =
            {
                "HIT",
                "STAND",
                "DOUBLE",
                "SPLIT"
            };

            // Centers of the four boxes in the background sprite
            static const uint16_t action_y[4] =
            {
                102,
                125,
                148,
                171
            };

            const uint16_t box_x1 = 244;
            const uint16_t box_x2 = 310;

            for(uint8_t i = 0; i < 4; i++)
            {
                if(previous_action_index == 0xFF ||
                   i == previous_action_index ||
                   i == app->action_menu.selected_index)
                {
                    bool selected =
                        (i == app->action_menu.selected_index);

                    /*
                     * Restore the complete button from the original
                     * background first. This removes the old selection
                     * without damaging the gold frame.
                     */
                    BlackjackUI_RestoreGameBackgroundRegion(
                        box_x1,
                        action_y[i] - 4,
                        box_x2 - box_x1 + 1,
                        20);

                    uint8_t len = strlen(actions[i]);

                    // FONT_8X12 + 1 pixel spacing used by our transparent helper
                    uint16_t text_width =
                        len * 9 - 1;

                    uint16_t text_x =
                        box_x1 +
                        ((box_x2 - box_x1 + 1) - text_width) / 2;

                    if(selected)
                    {
                        /*
                         * Fill only INSIDE the existing gold border.
                         * Leave the decorative border itself visible.
                         */
                        UG_FillFrame(
                            247,
                            action_y[i] - 2,
                            307,
                            action_y[i] + 13,
                            C_GOLD);

                        BlackjackUI_DrawStringTransparent(
                            actions[i],
                            text_x,
                            action_y[i],
                            C_BLACK,
                            &FONT_8X12);
                    }
                    else
                    {
                        BlackjackUI_DrawStringTransparent(
                            actions[i],
                            text_x,
                            action_y[i],
                            C_GOLD,
                            &FONT_8X12);
                    }
                }
            }

            previous_action_index =
                app->action_menu.selected_index;
        }




        // ------------------------------------------------
        // BET
        // ------------------------------------------------

        if(app->game.state == BLACKJACK_STATE_IDLE ||
           app->game.state == BLACKJACK_STATE_BETTING)
        {
            if(app->device_mode == DEVICE_MODE_SLAVE)
            {
                displayed_bet = app->slave_bet_selection;
            }
            else
            {
                displayed_bet = app->local_bet_selection;
            }
        }
        else
        {
            displayed_bet = app->game.player_bet[local_player_index][app->game.player_current_hand[local_player_index]];
        }

        if(displayed_bet != previous_bet)
        {
        	BlackjackUI_RestoreGameBackgroundRegion(
        	    264,
        	    25,
        	    50,
        	    18);

            snprintf(buffer,
                     sizeof(buffer),
                     "%u",
                     displayed_bet);

            uint16_t bet_text_width = strlen(buffer) * 9 - 1;
               uint16_t bet_x = 277 - (bet_text_width / 2);

               BlackjackUI_DrawStringTransparent(
                   buffer,
                   bet_x,
                   29,
                   C_GOLD,
                   &FONT_8X12);

               previous_bet = displayed_bet;
        }


        // ------------------------------------------------
        // BALANCE
        // ------------------------------------------------

        // ------------------------------------------------
        // BALANCE
        // ------------------------------------------------

        uint16_t current_balance =
            app->game.player_balance[local_player_index];

        if(current_balance != previous_balance)
        {
            BlackjackUI_RestoreGameBackgroundRegion(
                256,
                63,
                60,
                18);

            snprintf(buffer,
                     sizeof(buffer),
                     "%u",
                     current_balance);

            uint16_t balance_text_width =
                strlen(buffer) * 9 - 1;

            uint16_t balance_x =
                277 - (balance_text_width / 2);

            BlackjackUI_DrawStringTransparent(
                buffer,
                balance_x,
                63,
                C_GOLD,
                &FONT_8X12);

            previous_balance = current_balance;
        }


        // ------------------------------------------------
        // PLAYER HAND
        // ------------------------------------------------

        hand_t *player_hand =
            &app->game.player_hands[local_player_index][app->game.player_current_hand[local_player_index]];

        uint8_t player_total =
            Hand_GetValue(player_hand);


        // Player sum
        if(player_total != previous_total)
        {
        	BlackjackUI_RestoreGameBackgroundRegion(172, 140, 40, 24);

            snprintf(buffer,
                     sizeof(buffer),
                     "%u",
                     player_total);

            BlackjackUI_DrawStringTransparent(
                buffer,
                174,
                144,
                C_GOLD,
                &FONT_8X12);

            previous_total = player_total;
        }


        // ------------------------------------------------
        // DEALER HAND / DEALER SUM
        // ------------------------------------------------

        hand_t *dealer_hand =
            &app->game.dealer_hand;

        bool reveal_dealer_cards =
            (app->game.state == BLACKJACK_STATE_DEALER_TURN) ||
            (app->game.state == BLACKJACK_STATE_ROUND_OVER);

        uint8_t dealer_total;

        if(reveal_dealer_cards)
        {
            dealer_total =
                Hand_GetValue(dealer_hand);
        }
        else if(dealer_hand->card_count > 0)
        {
            dealer_total =
                BlackjackUI_GetCardValue(dealer_hand->cards[0]);
        }
        else
        {
            dealer_total = 0;
        }


        // Dealer sum
        if(dealer_total != previous_dealer_total)
        {
        	BlackjackUI_RestoreGameBackgroundRegion(172, 13, 40, 18);

            snprintf(buffer,
                     sizeof(buffer),
                     "%u",
                     dealer_total);

            BlackjackUI_DrawStringTransparent(
                buffer,
                174,
                15,
                C_GOLD,
                &FONT_8X12);

            previous_dealer_total = dealer_total;
        }


        // ------------------------------------------------
        // PLAYER CARDS
        // ------------------------------------------------

        bool player_is_split = app->game.player_hands[local_player_index][1].card_count > 0;

        if(player_hand->card_count != previous_player_card_count ||
           app->game.player_current_hand[local_player_index] != previous_player_hand ||
           player_is_split != previous_player_split)
        {
            // Clear previous player cards
            BlackjackUI_RestoreGameBackgroundRegion(5, 155, 208, 73);

            if(!player_is_split)
            {
                uint16_t player_spacing = 30;

                if(player_hand->card_count > 1)
                {
                    uint16_t max_spacing = 158 / (player_hand->card_count - 1);

                    if(max_spacing < player_spacing)
                    {
                        player_spacing = max_spacing;
                    }
                }

                for(uint8_t i = 0; i < player_hand->card_count; i++)
                {
                    BlackjackUI_DrawCard(
                        24 + i * player_spacing,
                        158,
                        player_hand->cards[i],
                        CARD_SIZE_NORMAL);
                }
            }
            else
            {
                hand_t *hand_0 = &app->game.player_hands[local_player_index][0];
                hand_t *hand_1 = &app->game.player_hands[local_player_index][1];

                const uint16_t hand0_x = 12;
                const uint16_t hand1_x = 116;
                const uint16_t hand_y = 158;

                uint16_t hand0_spacing = 24;
                uint16_t hand1_spacing = 24;

                if(hand_0->card_count > 1)
                {
                    uint16_t max_spacing = 51 / (hand_0->card_count - 1);

                    if(max_spacing < hand0_spacing)
                    {
                        hand0_spacing = max_spacing;
                    }
                }

                if(hand_1->card_count > 1)
                {
                    uint16_t max_spacing = 51 / (hand_1->card_count - 1);

                    if(max_spacing < hand1_spacing)
                    {
                        hand1_spacing = max_spacing;
                    }
                }

                for(uint8_t i = 0; i < hand_0->card_count; i++)
                {
                    BlackjackUI_DrawCard(
                        hand0_x + i * hand0_spacing,
                        hand_y,
                        hand_0->cards[i],
                        CARD_SIZE_NORMAL);
                }

                for(uint8_t i = 0; i < hand_1->card_count; i++)
                {
                    BlackjackUI_DrawCard(
                        hand1_x + i * hand1_spacing,
                        hand_y,
                        hand_1->cards[i],
                        CARD_SIZE_NORMAL);
                }

                if(app->game.player_current_hand[local_player_index] == 0)
                {
                    UG_DrawFrame(8, 155, 105, 224, C_GOLD);
                }
                else
                {
                    UG_DrawFrame(112, 155, 209, 224, C_GOLD);
                }
            }

            previous_player_card_count = player_hand->card_count;
            previous_player_hand = app->game.player_current_hand[local_player_index];
            previous_player_split = player_is_split;
        }


        // ------------------------------------------------
        // DEALER CARDS
        // ------------------------------------------------

        if(dealer_hand->card_count != previous_dealer_card_count ||
           reveal_dealer_cards != previous_dealer_reveal)
        {
            uint16_t dealer_spacing = 30;

            if(dealer_hand->card_count > 1)
            {
                uint16_t max_spacing = 127 / (dealer_hand->card_count - 1);

                if(max_spacing < dealer_spacing)
                {
                    dealer_spacing = max_spacing;
                }
            }

            // Clear previous dealer cards
            BlackjackUI_RestoreGameBackgroundRegion(10, 30, 171, 63);

            for(uint8_t i = 0; i < dealer_hand->card_count; i++)
            {
                uint16_t card_x = 24 + i * dealer_spacing;

                if(i == 1 && !reveal_dealer_cards)
                {
                    BlackjackUI_DrawHiddenCard(
                        card_x,
                        30);
                }
                else
                {
                    BlackjackUI_DrawCard(
                        card_x,
                        30,
                        dealer_hand->cards[i],
                        CARD_SIZE_NORMAL);
                }
            }

            previous_dealer_card_count = dealer_hand->card_count;
            previous_dealer_reveal = reveal_dealer_cards;
        }

        if(app->game.state == BLACKJACK_STATE_ROUND_OVER)
        {
        	if(!app->round_result_finished)
        	{
        	    if(!round_over_ui_drawn)
        	    {
        	    	BlackjackUI_RestoreGameBackgroundRegion(
        	    	    80,
        	    	    92,
        	    	    120,
        	    	    50);

        	    	UG_DrawFrame(
        	    	    85,
        	    	    96,
        	    	    195,
        	    	    136,
        	    	    C_GOLD);

        	        bool is_split =
        	            app->game.player_hands[local_player_index][1].card_count > 0;

        	        if(is_split)
        	        {
        	            blackjack_player_result_t result_0 =
        	                app->game.player_result[local_player_index][0];

        	            blackjack_player_result_t result_1 =
        	                app->game.player_result[local_player_index][1];

        	            const char *text_0 = "";
        	            const char *text_1 = "";

        	            if(result_0 == PLAYER_RESULT_WIN)
        	                text_0 = "WIN";
        	            else if(result_0 == PLAYER_RESULT_LOSE)
        	                text_0 = "LOSE";
        	            else if(result_0 == PLAYER_RESULT_PUSH)
        	                text_0 = "PUSH";

        	            if(result_1 == PLAYER_RESULT_WIN)
        	                text_1 = "WIN";
        	            else if(result_1 == PLAYER_RESULT_LOSE)
        	                text_1 = "LOSE";
        	            else if(result_1 == PLAYER_RESULT_PUSH)
        	                text_1 = "PUSH";

        	            char result_text[20];

        	            UG_FontSelect(&FONT_8X12);
        	            UG_SetForecolor(C_GOLD);
        	            UG_SetBackcolor(C_DARK_GREEN);

        	            snprintf(
        	                result_text,
        	                sizeof(result_text),
        	                "HAND 1: %s",
        	                text_0);

        	            UG_PutString(
        	                88,
        	                106,
        	                result_text);

        	            snprintf(
        	                result_text,
        	                sizeof(result_text),
        	                "HAND 2: %s",
        	                text_1);

        	            UG_PutString(
        	                88,
        	                124,
        	                result_text);
        	        }
        	        else
        	        {
        	            blackjack_player_result_t result =
        	                app->game.player_result[local_player_index][0];

        	            UG_FontSelect(&FONT_16X26);
        	            UG_SetBackcolor(C_DARK_GREEN);

        	            if(result == PLAYER_RESULT_WIN)
        	            {
        	                BlackjackUI_DrawStringTransparent(
        	                    "WIN",
        	                    116,
        	                    104,
        	                    C_GREEN,
        	                    &FONT_16X26);
        	            }
        	            else if(result == PLAYER_RESULT_LOSE)
        	            {
        	                BlackjackUI_DrawStringTransparent(
        	                    "LOSE",
        	                    108,
        	                    104,
        	                    C_RED,
        	                    &FONT_16X26);
        	            }
        	            else if(result == PLAYER_RESULT_PUSH)
        	            {
        	                BlackjackUI_DrawStringTransparent(
        	                    "PUSH",
        	                    108,
        	                    104,
        	                    C_CYAN,
        	                    &FONT_16X26);
        	            }
        	        }

        	        round_over_ui_drawn = true;
        	    }
        	}

        	else
        	{
        	    // ------------------------------------------------
        	    // REMOVE RESULT POPUP
        	    // ------------------------------------------------

        	    if(!decision_popup_drawn)
        	    {
        	        BlackjackUI_RestoreGameBackgroundRegion(
        	            80,
        	            92,
        	            120,
        	            50);

        	        decision_popup_drawn = true;
        	        previous_round_over_index = 0xFF;
        	    }


        	    // ------------------------------------------------
        	    // PLAY AGAIN / EXIT POPUP
        	    // ------------------------------------------------

        	    static const char *options[2] =
        	    {
        	        "PLAY AGAIN",
        	        "EXIT"
        	    };

        	    static const uint16_t option_y[2] =
        	    {
        	        109,
        	        131
        	    };

        	    const uint16_t popup_x1 = 85;
        	    const uint16_t popup_y1 = 98;
        	    const uint16_t popup_x2 = 205;
        	    const uint16_t popup_y2 = 150;

        	    const uint16_t box_x1 = 91;
        	    const uint16_t box_x2 = 199;

        	    // Draw popup border
        	    UG_DrawFrame(
        	        popup_x1,
        	        popup_y1,
        	        popup_x2,
        	        popup_y2,
        	        C_GOLD);


        	    if(app->round_over_menu.selected_index !=
        	       previous_round_over_index)
        	    {
        	        for(uint8_t i = 0; i < 2; i++)
        	        {
        	            if(previous_round_over_index == 0xFF ||
        	               i == previous_round_over_index ||
        	               i == app->round_over_menu.selected_index)
        	            {
        	                bool selected =
        	                    (i == app->round_over_menu.selected_index);

        	                // Restore only the inside of this option
        	                BlackjackUI_RestoreGameBackgroundRegion(
        	                    box_x1,
        	                    option_y[i] - 2,
        	                    box_x2 - box_x1 + 1,
        	                    17);

        	                uint8_t len = strlen(options[i]);

        	                uint16_t text_width =
        	                    len * 9 - 1;

        	                uint16_t text_x =
        	                    box_x1 +
        	                    ((box_x2 - box_x1 + 1) - text_width) / 2;

        	                if(selected)
        	                {
        	                    UG_FillFrame(
        	                        box_x1,
        	                        option_y[i] - 2,
        	                        box_x2,
        	                        option_y[i] + 13,
        	                        C_GOLD);

        	                    BlackjackUI_DrawStringTransparent(
        	                        options[i],
        	                        text_x,
        	                        option_y[i],
        	                        C_BLACK,
        	                        &FONT_8X12);
        	                }
        	                else
        	                {
        	                    BlackjackUI_DrawStringTransparent(
        	                        options[i],
        	                        text_x,
        	                        option_y[i],
        	                        C_GOLD,
        	                        &FONT_8X12);
        	                }
        	            }
        	        }

        	        previous_round_over_index =
        	            app->round_over_menu.selected_index;


        	    }
        	}
        }

        previous_ui_game_state = app->game.state;
    }

    else if(app->state == BLACKJACK_APP_STATE_NO_FUNDS)
            {
                if(!no_funds_ui_drawn)
                {
                    BlackjackUI_DrawGameBackground();

                    UG_DrawFrame(
                        70,
                        78,
                        250,
                        162,
                        C_GOLD);

                    BlackjackUI_DrawStringTransparent(
                        "GAME OVER",
                        84,
                        92,
                        C_RED,
                        &FONT_16X26);

                    BlackjackUI_DrawStringTransparent(
                        "NO FUNDS",
                        125,
                        126,
                        C_GOLD,
                        &FONT_8X12);

                    BlackjackUI_DrawStringTransparent(
                        "PRESS OK",
                        125,
                        144,
                        C_WHITE,
                        &FONT_8X12);

                    no_funds_ui_drawn = true;
                }
            }
}



