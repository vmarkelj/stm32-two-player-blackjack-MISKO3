#ifndef INCLUDE_BLACKJACK_UI_H_
#define INCLUDE_BLACKJACK_UI_H_

#include "blackjack_app.h"

typedef enum
{
    CARD_SIZE_NORMAL = 0,
    CARD_SIZE_SMALL
} blackjack_ui_card_size_t;

/* DEKLARACIJE FUNKCIJ */
void BlackjackUI_Update(blackjack_app_t *app);

#endif /* INCLUDE_BLACKJACK_UI_H_ */
