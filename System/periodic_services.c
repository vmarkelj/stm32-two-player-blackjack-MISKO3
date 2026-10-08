/*
 * periodic_services.c
 *
 *  Created on: 23 Apr 2026
 *      Author:
 */

#include "periodic_services.h"
#include "stm32g4xx_ll_tim.h"
#include "kbd.h"

typedef struct
{

TIM_TypeDef *TIMx;

}periodic_services_handle_t;

periodic_services_handle_t periodic_services;

void PSERV_init(void)
{
	periodic_services.TIMx = TIM6;
	LL_TIM_EnableCounter(periodic_services.TIMx);
}

void PSERV_enable(void)
{
	LL_TIM_EnableIT_UPDATE(periodic_services.TIMx);

}

void PSERV_disable(void)
{
	LL_TIM_DisableIT_UPDATE(periodic_services.TIMx);
}

void PSERV_run_services_Callback(void)
{
	KBD_scan();

	//KBD_demo_toggle_LEDs_if_buttons_pressed();
}
