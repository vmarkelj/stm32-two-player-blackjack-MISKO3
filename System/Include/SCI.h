/*
 * SCI.h
 *
 *  Created on:
 *      Author:
 */

#ifndef INCLUDE_SCI_H_
#define INCLUDE_SCI_H_



// ----------- Include other modules (for public) -------------

// Vključimo nizko-nivojsko LL knjižnjico, da dobimo podporo za delo z USART vmesnikom.

#include "stm32g4xx_ll_usart.h"
#include "buf.h"


// Pri implementaciji sistemskih funkcij serijskega vmesnika SCI bomo potrebovali sledeče nizko-nivojske funkcije
// za upravljanje USART periferne enote:
//
//	    LL_USART_TransmitData8(USARTx, podatek);
//		uint8_t podatek = LL_USART_ReceiveData8(USARTx);
//		LL_USART_IsActiveFlag_TXE_TXFNF(USARTx);
//   	LL_USART_IsActiveFlag_RXNE_RXFNE(USARTx);

// Iz zgornjega seznama funkcij lahko vidimo, da bo potrebno v "handle" strukturi SCI vmesnika hraniti
// samo en parameter, ki določa, katero USARTx periferno enoto bomo uporabili.



// "Javno" vključimo še standardno "stdio" knjižnico, v kateri se nahaja printf() funkcija.
// Tako zagotovimo, da bodo lahko tudi ostali moduli, ki uporabljajo SCI modul, lahko uporabljali
// prirejeno printf() funkcijo (ki smo jo priredili znotraj SCI modula).
#include <stdio.h>



// -------------------- Public definitions --------------------


// "Javno" definirajmo še pomožne "vrnjene kode" (angl. return codes), s katerimi bodo
// SCI funkcije sporočale, ali je bila operacija uspešna ali ne.
// Uporabimo naštevni tip.
typedef enum { SCI_ERROR = -1, SCI_NO_ERROR = 0} SCI_rtrn_codes_t;





// ---------------- Public function prototypes ----------------

void SCI_init(void);

void SCI_send_char(char c);
void SCI_send_byte(uint8_t data);

uint8_t SCI_is_data_waiting(void);

SCI_rtrn_codes_t SCI_read_char(char *c);
SCI_rtrn_codes_t SCI_read_byte(uint8_t *data);


void SCI_send_string(char *str);
void SCI_send_bytes(uint8_t *data, uint32_t size);


void SCI_demo_Hello_world(void);
void SCI_demo_echo_with_polling(void);
void SCI_demo_Hello_world_printf(void);


void SCI_send_string_IT(char *str);
void SCI_send_bytes_IT(uint8_t *data, uint32_t size);

void SCI_receive_byte_Callback(void);
void SCI_transmit_byte_Callback(void);

uint32_t SCI_RX_buffer_get_data_size(void);
uint32_t SCI_is_RX_buffer_not_empty(void);
buf_rtrn_codes_t SCI_RX_buffer_get_byte(uint8_t *data);
buf_rtrn_codes_t SCI_RX_buffer_get_bytes(uint8_t *data,  uint32_t size);
buf_rtrn_codes_t SCI_RX_buffer_flush(void);
buf_rtrn_codes_t SCI_TX_buffer_flush(void);


// - "demo" funkcije za demonstracijo komunikacije z uporabo prekinitev

void SCI_demo_receive_with_interrupts(void);
void SCI_demo_transmit_with_interrupts(void);
void SCI_demo_echo_with_interrupts(void);
#endif /* INCLUDE_SCI_H_ */






