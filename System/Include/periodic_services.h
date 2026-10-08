/*
 * periodic_services.h
 *
 *  Created on: 23 Apr 2026
 *      Author:
 */

#ifndef INCLUDE_PERIODIC_SERVICES_H_
#define INCLUDE_PERIODIC_SERVICES_H_


//		a) void LL_TIM_EnableCounter(TIM_TypeDef *TIMx);
//		b) void LL_TIM_EnableIT_UPDATE(TIM_TypeDef *TIMx);
//		c) void LL_TIM_DisableIT_UPDATE(TIM_TypeDef *TIMx);
//		d) uint32_t LL_TIM_IsEnabledIT_UPDATE(TIM_TypeDef *TIMx);
//		e) uint32_t LL_TIM_IsActiveFlag_UPDATE(TIM_TypeDef *TIMx);
//		f) void LL_TIM_ClearFlag_UPDATE(TIM_TypeDef *TIMx);



void PSERV_init(void);
void PSERV_enable(void);
void PSERV_disable(void);
void PSERV_run_services_Callback(void);
#endif /* INCLUDE_PERIODIC_SERVICES_H_ */
