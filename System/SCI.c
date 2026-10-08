/*
 * SCI.c
 *
 *  Created on:
 *      Author:
 */


/* **************** MODULE DESCRIPTION *************************

Ta modul implementira serijski komunikacijski vmesnik (SCI) na sistemskem nivoju.
SCI vmesnik nudi serijsko povezavo z osebnim računalnikom preko periferne USART enote.
Z drugumi besedami: SCI vmesnik je implementiran z USART vmesnikom (lahko bi namreč
uporabili kak drug serijski vmesnik za implementacijo SCI vmesnika).

Podprta sta dva tipa prenosa podatkov preko SCI:

	1. prenos znakov (uporablja se za prenos besedilnih sporočil),

	2. prenos binarnih podatkov (uporablja se za prenos binarnih podatkov).


Poleg tega sta na voljo dve vrsti funkcij za pošiljanje "send":

	1. za pošiljanje enega samega znaka ali enega bajta podatkov

	2. za pošiljanje niza znakov ali zaporedja bajtov


POZOR: za uspešno uporabo modula je potrebno predhodno poskrbeti
za ustrezno nizko-nivojsko inicializacijo USART vmesnika.

************************************************************* */






// ----------- Include other modules (for private) -------------

#include "SCI.h"




// ---------------------- Private definitions ------------------


// ------ Definicje v zvezi z SCI -------


// Definiramo tip "handle" strukture "SCI_handle_t", ki jo bomo uporabili za upravljanje SCI.
// V strukturi je pravzaprav potrebno pomniti le, katera USART periferna enota bo uporabljena.


typedef struct
{
	USART_TypeDef			*port;

} SCI_handle_t;



// In sedaj na podlagi zgoraj definiranega tipa definiramo še globalno strukturno spremenljivko
// z imenom "SCI".

SCI_handle_t SCI;

// --- SCI receive (RX) buffer definitions ----

// Pripraviti je potrebno podatkovne strukture, s katerimi bomo implementirali
// sprejemni medpomnilnik.

// Definirajmo makro parameter, ki bo specificiral dolžino medpomnilnika.
#define SCI_RX_BUF_LEN 	512

// In definirajmo še dve podatkovni strukturi, ki sta potrebni za implementacijo
// medpomnilnika.
uint8_t SCI_RX_buffer[SCI_RX_BUF_LEN];			// tabela bajtov, ki bo hranila podatke
buf_handle_t SCI_RX_buf_handle;					// "handle" struktura za RX medpomnilnik




// --- SCI transmit (TX) buffer definitions ---

// Pripraviti je potrebno podatkovne strukture, s katerimi bomo implementirali
// sprejemni medpomnilnik.

// Definirajmo makro parameter, ki bo specificiral dolžino medpomnilnika.
#define SCI_TX_BUF_LEN 	512

// In definirajmo še dve podatkovni strukturi, ki sta potrebni za implementacijo
// medpomnilnika.
uint8_t SCI_TX_buffer[SCI_TX_BUF_LEN];			// tabela bajtov, ki bo hranila podatke
buf_handle_t SCI_TX_buf_handle;					// "handle" struktura za TX medpomnilnik




// ------ printf() customization definitions -----


// V zgornjem delu SCI.c datoteke definiramo prototip funkcije
//
//		int _write(int file, char *ptr, int len)
//
// s pomočjo katere bomo lahko priredili uporabo printf() funkcije tako, da bo
// sporočila pošiljala preko SCI vmesnika (tj. re-definirali bomo pošiljanje sporočil
// preko standardnega izhoda "stdout").
 int _write(int file, char *ptr, int len);







// -------------- Public function implementations --------------


// Funkcija SCI_init() poskrbi za inicializacijo SCI "handle" strukture SCI
// in modificira dolžino medpomnilnika printf() funkcije.
void SCI_init(void)
{

	// 0. Poskrbite, da je ustrezna nizko-nivojska strojna oprema že inicializirana

			// Za inicializacijo USART vmesnika, ki ga nameravamo uporabiti za implementacijo
			// SCI komunikacije, je poskrbljeno s pomočjo avtomatsko generirane CubeMX programske kode
			// znotraj funkcije MX_USARTx_UART_Init() v "main.c"!



	// 1. Določite, katera USART periferna enota bo uporabljena za implementacijo SCI vmesnika.

		// USART vmesnik specificiramo s pred-definiranim makrojem iz
	    // "CMSIS STM32G474xx Device Peripheral Access" knjižnice (datoteka "stm32g474xx.h").
		// Razmišljamo podobno kot pri portih. Za primer: GPIOx --> GPIOF.
		// Pri serijskem vmesniku je situacija podobna: USARTx --> USART[številka vmesnika].

		SCI.port = USART1;
						// Namig: držite CTRL in kliknite na makro, ki ste ga uporabili.
						// Videli boste, da je res definiran v CMSIS knjižnici "stm32g474xx.h".


	// 2. modifikacija printf() medpomnilnika

		// Dolžino medpomnilnika, ki se uporablja za pošiljanje sporočil na
		// standardni izhod "stdout" s pomočjo printf() funkcije,
		// bomo nastavili na 0. Na ta način dosežemo najboljšo odzivnost
		// printf() funkcije, saj se tako ne čaka, da se medpomnilnik napolne,
		// preden se pošlje sporočilo. Sporočila bodo tako poslana takoj, ko se
		// prejme vsaj en znak.
		setvbuf(stdout, NULL, _IONBF, 0);

		// Opomba: uporabimo funkcijo setvbuf() iz modula "stdio.h" iz t.i. "newlib" knjižnice,
		//
		//		int setvbuf(FILE *fp, char *buf, int mode, size_t size);
		//
		// s katero specificiramo dolžino medpomnilnika za standardni izhod "stdout" (prvi argument funkcije).
		// Za 3. vhodni parameter uporabimo že pred-definirani makro _IONBF (I/O No Buffer), ki bo povzročil,
		// da se za standardni izhod "stdout" ne uporablja medpomnilnika, temveč
		// se znake takoj pošlje na standardni izhod.

		// 3. Inicializacija SCI medpomnilnikov  (RX and TX)
					BUF_init( &SCI_RX_buf_handle, SCI_RX_buffer, SCI_RX_BUF_LEN);	// RX SCI medpomnilnik
					BUF_init( &SCI_TX_buf_handle, SCI_TX_buffer, SCI_TX_BUF_LEN);	// TX SCI medpomnilnik



				// 4. Omogočitev prekinitev ob prejemu novega podatka

					// Kot zadnji korak inicializacije SCI vmesnika poskrbimo, da se vklopijo
					// prekinitve ob sprejemu novega podatka preko USART enote, s katero
					// implementiramo SCI vmesnik. Uporabimo seveda primerno LL funkcijo,
					// ki omogoči prekinitve ob postavitvi RXNE zastavice.


					LL_USART_EnableIT_RXNE_RXFNE(SCI.port);



					// V vednost: prekinitve ob sprejemu novega podatka lahko vklopimo šele,
					// ko že imamo pripravljen sprejemni RX medpomnilnik, kamor se bodo ob
					// prekinitvah shranjevali novoprejeti podatki.

}





// Kot zanimivost: spodaj bomo implementirali SCI komunikacijo po principu "od spodaj navzgor"
// (anlg. bottom-up approach): najprej bomo implementirani najbolj elementarne, temeljne funkcije
// za komunikacijo, ki pošljejo en sam znak oziroma bajt. Potem pa bomo te funkcije uporabili,
// da z njimi implementiramo pošiljanje daljših nizov podatkov.





// ------ Funkcije za pošiljanje enega samega bajta podatkov -------



// Funkcija SCI_send_char() pošlje en sam ZNAK preko SCI vmesnika (uporabno, kadar delamo z besedilnimi sporočili).
// Ker SCI vmesnik implementiramo z USART vmesnikom, je potrebno pravzaprav znak poslati z oddajnikom USART enote.
// Pri tem si pomagamo z LL funkcijo.
void SCI_send_char(char c)
{
	// S čakanjem v zanki zagotovimo, da USART oddajnik lahko sprejme nov znak v oddajni register.
	// Za ugotavljanje, ali je že mogoče pisati v oddajni register USART vmesnika, uporabimo ustrezno
	// nizko-nivojsko LL funkcijo za delo z USART-om.
	while(!LL_USART_IsActiveFlag_TXE_TXFNF(SCI.port));

	// In nato z uporabo USART vmesnika pošljemo en znak, shranjen v spremenljivki "c".
	// Za pošiljane podatka preko USART vmesnika prav tako uporabimo
	// ustrezno nizko-nivojsko LL funkcijo.

	LL_USART_TransmitData8(SCI.port, c);

}



// Funkcija SCI_send_byte() pošlje en sam BAJT preko SCI vmesnika (uporabno, kadar pošiljamo binarne podatke).
// Ker SCI vmesnik implementiramo z USART vmesnikom, je potrebno pravzaprav bajt podatkov poslati z oddajnikom USART enote.
// Pri tem si pomagamo z LL funkcijo. Implementacija te funkcije je skoraj indentična implementaciji funkcije zgoraj.
void SCI_send_byte(uint8_t data)
{
	// S čakanjem v zanki zagotovimo, da USART oddajnik lahko sprejme nov bajt v oddajni register.
	// Za ugotavljanje, ali je že mogoče pisati v oddajni register USART vmesnika, uporabimo ustrezno
		// nizko-nivojsko LL funkcijo za delo z USART-om.

	while(!LL_USART_IsActiveFlag_TXE_TXFNF(SCI.port));

	// In nato z uporabo USART vmesnika pošljemo en bajt podatkov, shranjenih v spremenljivki "data".
	// Za pošiljane podatka preko USART vmesnika prav tako uporabimo
	// ustrezno nizko-nivojsko LL funkcijo.

	LL_USART_TransmitData8(SCI.port, data);

}







// ------ Funkcije za pošiljanje niza znakov ali zaporedja bajtov ----------

// Te funkcije lahko enostavno implementiramo s pomočjo funkcij za pošiljanje
// enega samega znaka oziroma bajta podaktkov, ki smo jih definirali zgoraj.


// Funkcija SCI_send_string() pošlje znakovni niz (tj. string) preko SCI vmesnika.
void SCI_send_string(char *str)
{
	// Pomožna spremenljivka - indeks znaka, ki ga pošiljamo. Indeks "i" je
	// potrebno pred prvo uporabo smiselno inicializirati.
	// Kateri znak pošljemo najprej? Kakšna je vrednost indeksa tega znaka?
	uint32_t i = 0;

	// Pošlji celoten niz znakov tako, da pošiljaš znak za znakom
	// vse dokler ne naletiš na vrednost 0 (null terminated string).
	// Premislite: "null terminating" znaka ni potrebno pošiljati preko SCI vmesnika.
	while(str[i] != '\0')
	{
		SCI_send_char(str[i]);
		i++;
	}


}




// Funkcija SCI_send_bytes() pošlje zaporedje bajtov preko SCI vmesnika.
// Lokacija podatkov zapošiljanje je podana z vhodnim argumentom "*data",
// dolžina podatkov za pošiljanje pa je podano z vhodnim argumentom "size".
void SCI_send_bytes(uint8_t *data, uint32_t size)
{
	// Ker je dolžina podatkov znana vnaprej, uporabimo "for" zanko.
	for(int i = 0; i < size; i++)
	{
		// In znotraj nje pošiljamo bajt za bajtom.

		SCI_send_byte(data[i]);
	}
}










// ------ Funkcije za sprejemanje enega samega bajta podatkov -------


// Najprej si pripravimo pomožno funkcijo, s katero bomo lahko preverjali, ali je v SCI vmesniku
// na voljo nov prejeti podatek.


// Funkcija SCI_is_data_waiting() preveri, če je v SCI vmesniku na voljo nov prejeti podatek.
// Ker SCI vmesnik implementiramo z USART vmesnikom, je potrebno pravzaprav preveriti, ali je v sprejemniku USART-a
// na voljo nov podatek.
uint8_t SCI_is_data_waiting(void)
{
	// V premislek: LL funkcija, ki jo boste uporabili za preverjanje prejetega podatka vrača vrednost
	// tipa "uint32_t", sistemska funkcija SCI_is_data_waiting() pa vrača vrednost tipa uint8_t.
	// Torej boste imeli tu opravka s pretvorbo tipov, ki jo za vas lahko izvede prevajalnik avtomatsko.

	return LL_USART_IsActiveFlag_RXNE_RXFNE(SCI.port);

}




// Funkcija SCI_read_char() POSKUSI prebrati en ZNAK iz SCI vmesnika in
// ga shraniti na lokacijo, ki jo določa vhodni argument funkcije *c.
// Ker SCI vmesnik implementiramo z USART vmesnikom, je potrebno pravzaprav podatek prebrati iz
// sprejemnika USART vmesnika. Poskus branja podatka iz SCI vmesnika je
// lahko uspešen (return code = SCI_NO_ERROR) ali pa neuspešen (return code = SCI_ERROR).
SCI_rtrn_codes_t SCI_read_char(char *c)
{
	// Najprej preverimo, če sploh je v SCI vmesniku na voljo nov prejeti podatek.
	// Uporabimo zgoraj definirano SCI funkcijo.
	if (SCI_is_data_waiting())
	{

		// In če je na voljo nov prejeti podatek, ta podatek preberemo
		// in ga shranimo na specificirano mesto, ki ga določa kazalec "c".
		// Preberemo ga seveda iz USART vmesnika s pomočjo LL funkcije.

		 *c = LL_USART_ReceiveData8(SCI.port);


		// In ker je bilo branje novega prejetega podatka uspešno, vrnemo
		// kodo SCI_NO_ERROR.
		return SCI_NO_ERROR;

	}
	else
	{
		// Če pa ni na voljo novega prejetega podatka, pa vrnemo kodo SCI_ERROR
		// in NE spreminjamo vrednosti spremenljivke *c.
		return SCI_ERROR;
	}

}





// Funkcija SCI_read_byte() POSKUSI prebrati en BAJT iz SCI vmesnika in
// ga shraniti na lokacijo, ki jo določa vhodni argument funkcije *data.
// Ker SCI vmesnik implementiramo z USART vmesnikom, je potrebno pravzaprav podatek prebrati iz
// sprejemnika USART vmesnika. Poskus branja podatka iz SCI vmesnika je
// lahko uspešen (return code = SCI_NO_ERROR) ali pa neuspešen (return code = SCI_ERROR).
SCI_rtrn_codes_t SCI_read_byte(uint8_t *data)
{
	// Najprej preverimo, če sploh je v SCI vmesniku na voljo nov prejeti podatek.
	// Uporabimo zgoraj definirano SCI funkcijo.
	if ( SCI_is_data_waiting() )
	{

		// In če je na voljo nov prejeti podatek, ta podatek preberemo
		// in ga shranimo na specificirano mesto, ki ga določa kazalec "data".
		// Preberemo ga seveda iz USART vmesnika s pomočjo LL funkcije.

		 *data = LL_USART_ReceiveData8(SCI.port);


		// In ker je bilo branje novega prejetega podatka uspešno, vrnemo
		// kodo SCI_NO_ERROR.
		return SCI_NO_ERROR;

	}
	else
	{
		// Če pa ni na voljo novega prejetega podatka, pa vrnemo kodo SCI_ERROR
		// in ne spreminjamo vrednosti spremenljivke *data.
		return SCI_ERROR;
	}
}








// -------------- Private function implementations -------------



// ----------- printf() customization ----------


// Na tem mestu pa REDEFINIRAMO "newlib" funkcijo _write(). Funkcija _write()
// se namreč uporablja za pošiljanje sporočil s printf() funkcijo. S tem ko
// redefiniramo _write() funkcijo pravzaprav priredimo uporabo printf() funkcije
// tako, da pošilja sporočila na točno tak način, kakor to sami želimo.
// In mi želimo uporabljati funkcijo printf() tako, da bomo s pomočjo nje
// pošiljali formatirana sporočila preko SCI vmesnika.
int _write(int file, char *ptr, int len)
{
	// Vhodna argumenta _write funkcije "*ptr" in "len" specificirata,
	// kje se nahajajo podatki, ki jih printf() funkcija želi poslati,
	// ter kako dolgi so ti podatki (v smislu števila bajtov).

	// Sedaj pa uporabimo našo SCI sistemsko funkcijo SCI_send_bytes()
	// da z njo pošljemo te podatke, na katere kaže "ptr".
	// Ker naša funkcija SCI_send_bytes() uporablja drugačen tip
	// vhodnih argumentov, je potrebno poskrbeti za eksplicitno
	// pretvorbo med tipi podatkov (angl. type-casting).
	SCI_send_bytes( (uint8_t*) ptr, (uint32_t) len );



	// Funkcija _write() mora vrniti število uspešno poslanih
	// znakov. Tu poenostavimo in privzamemo, da je bilo uspešno
	// poslanih vseh "len" znakov.
	return len;
}


// --------------------------------------------------------------











// ------- Testne funkcije ---------


// Funkcija SCI_demo_Hello_world() naj preko SCI vmesnika pošlje
// klasično sporočilo "Hello world!".
void SCI_demo_Hello_world(void)
{
	SCI_send_string("Hello World!");
}



// Funkcija SCI_demo_Hello_world_printf() naj preko SCI vmesnika pošlje
// sporočilo "Hello world!" s pomočjo prirejene funkcije printf().
// Sporočila naj bodo izpisana vsaka v svojo vrstico in naj bodo
// sledeče oblike:
//
//		1 : Hello printf() world!
//		2 : Hello printf() world!
//		3 : Hello printf() world!
//
// in tako dalje izpišite 10 vrstic.
// Sporočila naj se pošiljajo na vsako sekundo.
void SCI_demo_Hello_world_printf(void)
{

	for(int i = 0; i <= 10; i++)
	{
		printf("Hello printf() world!");
	}

}




// Funkcija SCI_demo_echo_with_polling() naj demonstrira t.i. "echo funkcionalnost"
// s pristopom poizvedovanja (angl. polling).
//
// To pomeni da funkcija vsake toliko časa, ko ima na vojo "procesorski čas",
// preveri, če je bil preko SCI vmesnika sprejet nov znak in če je bil,
// naj ta znak takoj pošlje preko SCI vmesnika nazaj. Tako ste pravzaprav
// naredili nekakšen "odmev znakov", ki prihajajo preko SCI vmesnika.
// Demonstracija bo ključnega pomena, saj bomo z njo lahko pokazali
// pomembno težavo, ki jo prinese pristop poizvedovanja (angl. polling).
void SCI_demo_echo_with_polling(void)
{

	// Pomožna spremenljivka, kamor bomo shranili prejeti znak.
	char c;


	// Spremenljivke, s pomočjo katerih bomo simulirali "zaposlenost procesorja" ("CPU busy").
	float a_0 = 0.5;
	float a_n;
	float sum = 0;

	#define N_MAX		50		// Za uporabo: večji kot bo N_MAX, več simuliranega dela ima procesor.


	// V neskončni zanki
	while(1)
	{

		// s tehniko poizvedovanja ("polling") prevrejamo, ali je preko SCI vmesnika prispel
		// nov znak
		if ( SCI_read_char(&c) == SCI_NO_ERROR)
		{
			// in če je prispel, ga takoj pošljemo preko SCI vmesnika nazaj (tj. "echo").

			SCI_send_char(c);
		}



		// Potem pa simuliramo "zaposlenost procesorja" ("CPU busy") z drugimi opravili.
		// V našem primeru bomo to simulirali tako, da bomo računali privh N_MAX členov
		// geometrijske vsote.
		//
		// Na začetku, ko preverjate, kako dobro deluje "echo funkcionalnost",
		// pustite N_MAX nastavljen na 50. Kasneje pa lahko preverite, kaj se
		// dogaja, ko vrednost N_MAX večate oziroma manjšate. Lahko tudi poskusite zakomentirati
		// spodnjo kodo.

		for(uint32_t n=0; n<N_MAX; n++)
		{
			 sum = sum + a_n;
			 a_n = a_n * a_0;
		}


	}

}

// VAJA 8

// ******** Dodati je potrebno funkciji za pošiljanje več podatkov s pomočjo prekinitev:


// ------ Funkcije za pošiljanje niza znakov ali zaporedja bajtov s pomočjo prekinitev -------


// Funkcija SCI_send_string_IT() pošlje znakovni niz (tj. string) preko SCI vmesnika s pomočjo prekinitev.
void SCI_send_string_IT(char *str)
{
	// pomožna spremenljivka
	uint32_t i = 0;		// indeks


	// Najprej je potrebno celoten znakovni niz "*str" shraniti v oddajni medpomnilnik.
	// Ker ni znana dolžina znakovnega niza, se z "while" zanko sprehodimo preko vseh znakov niza
	// do t.i. "null character-ja"
	while( str[i] != '0')
	{
		// in jih enega po enega shranjujemo v oddajni mepodmnilnik.
		BUF_store_byte( &SCI_TX_buf_handle, (uint8_t) str[i] );

		// Indeks se premakne na naslednji znak v nizu.

		i++;
	}


	// Če je potrebno poslati vsaj en znak,
	if ( BUF_get_data_size(&SCI_TX_buf_handle) > 0 )
	{
		// zgolj omogočimo prekinitve za primer, ko je oddajni podatkovni register TDR prost (TXE).
		// To storimo z ustrezno LL funkcijo.



			LL_USART_EnableIT_TXE_TXFNF(SCI.port);

		// Tako smo omogočili, da se bodo podatki iz oddajnega medpomnilnika poslali
		// avtomatsko znotraj prekinitvene rutine s klicem funkcije SCI_transmit_byte_Callback().
	}

}




// Funkcija SCI_send_bytes_IT() pošlje zaporedje bajtov preko SCI vmesnika s pomočjo prekinitev.
// Lokacija podatkov zapošiljanje je podana z vhodnim argumentom "*data",
// dolžina podatkov za pošiljanje pa je podano z vhodnim argumentom "size".
void SCI_send_bytes_IT(uint8_t *data, uint32_t size)
{

	// Najprej je potrebno celotno zaporedje bajtov z naslova "data" shraniti v oddajni medpomnilnik.
	// Ker je poznana dolžina "data" podatkov, lahko uporabimo BUF_store_bytes().
	BUF_store_bytes( &SCI_TX_buf_handle, data, size );


	// Če je potrebno poslati vsaj en bajt,
	if ( size > 0 )
	{
		// zgolj omogočimo prekinitve za primer, ko je oddajni podatkovni register TDR prost (TXE).
		// To storimo z ustrezno LL funkcijo.
		LL_USART_EnableIT_TXE_TXFNF(SCI.port);


		// Tako smo omogočili, da se bodo podatki iz oddajnega medpomnilnika poslali
		// avtomatsko znotraj prekinitvene rutine s klicem funkcije SCI_transmit_byte_Callback().
	}

}







// ************ Dodati je potrebno funkcije za delo z SCI medpomnilniki:


// --------- Pomožne funkcije za delo z SCI medpomnilniki -------------

// Spodaj so implementirane funkcije za osnovno upravljanje z SCI RX in TX medpomnilikoma.



// ------ RX buffer -------

// Funkcija SCI_RX_buffer_get_data_size() vrne število sprejetih bajtov,
// ki jih trenutno hrani sprejemni RX SCI medpomnilnilk.
uint32_t SCI_RX_buffer_get_data_size(void)
{
	return BUF_get_data_size (&SCI_RX_buf_handle);
}


// Funkcija SCI_is_RX_buffer_not_empty() vrne neničelno vrednost, če se v
// sprejemnem RX SCI medpomnilnilku nahajajo podatki.
uint32_t SCI_is_RX_buffer_not_empty(void)
{
	return ( SCI_RX_buffer_get_data_size() > 0 );
}


// Funkcija SCI_RX_buffer_get_byte() prebere EN BAJT podatkov iz sprejemnega RX SCI medpomnilnika
// in jih vpiše na naslov, ki ga specificira kazalec "data".
buf_rtrn_codes_t SCI_RX_buffer_get_byte(uint8_t *data)
{
	return BUF_get_byte(&SCI_RX_buf_handle, data );
}



// Funkcija SCI_RX_buffer_get_bytes() prebere podatke iz sprejemnega RX SCI medpomnilnika
// in jih vpiše na naslov, ki ga specificira kazalec "data". Dolžino prebranih podatkov
// specificira parameter "size".
buf_rtrn_codes_t SCI_RX_buffer_get_bytes(uint8_t *data,  uint32_t size)
{
	return BUF_get_bytes( &SCI_RX_buf_handle, data, size );
}

// Funkcija SCI_RX_buffer_flush() zbriše obstoječe podatke, ki se nahajajo
// v sprejemnem RX SCI medpomnilniku.
buf_rtrn_codes_t SCI_RX_buffer_flush(void)
{
	return BUF_flush( &SCI_RX_buf_handle );
}




// ------ TX buffer -------

// Funkcija SCI_TX_buffer_flush() zbriše obstoječe podatke, ki se nahajajo
// v oddajnem TX SCI medpomnilniku.
buf_rtrn_codes_t SCI_TX_buffer_flush(void)
{
	return BUF_flush( &SCI_TX_buf_handle );
}







// *********** Dodati je potrebno prekinitveni "callback" funkciji:


// ------- "callback" funkcije, ki implementirajo prekinitvene rutine ---------

// Funkcija SCI_receive_byte_Callback() je "callback" funkcija, ki jo bomo poklicali
// ob prekinitvah, ko bo sprejet nov bajt preko USARTa, s katerim implementiramo sistemski SCI vmesnik.
void SCI_receive_byte_Callback(void)
{
	// Pomožna spremenljivka za novo-sprejeti podatek.
	uint8_t received_data;

	// Ko preko USART vmesnika prejmemo nov podatek, ga shranimo v sprejemni RX medpomnilnik SCI vmesnika.

	// Najprej z uporabo LL funkcije preberemo novo-sprejeti podatek in ga shranimo v pomožno spremenljivko.
	received_data = LL_USART_ReceiveData8(SCI.port);

	// Nato pa ta podatek shranimo v sprejemni RX medpomnilnik SCI vmesnika za nadaljno obdelavo kasneje.
	BUF_store_byte( &SCI_RX_buf_handle , received_data);
}





// Funkcija SCI_transmit_byte_Callback() je "callback" funkcija, ki jo bomo poklicali
// ob prekinitvah, ko bo oddajni register USART vmesnika prost (tj. bo lahko prejel nov
// podatek za pošiljanje).
void SCI_transmit_byte_Callback(void)
{
	// pomožna spremenljivke
	uint8_t 			data_to_transmit;		// sem shranimo podatek, ki ga želimo poslati
	buf_rtrn_codes_t	return_code;			// sem shranimo rezultat poskusa branja iz medpomnilnika


	// Najprej poskusimo prebrati naslednji podatek, ki ga želimo poslati.
	// Zapomnimo si "vrnjeno kodo" (angl. return code), ki jo vrne BUF_ funkcija.
	return_code = BUF_get_byte( &SCI_TX_buf_handle, &data_to_transmit );

	// S pomočjo "vrnjene kode" ugotovimo, če sedaj imamo na voljo naslednji podatek za pošiljanje.
	if ( return_code == BUFFER_OK )
	{
		// In če je na voljo naslednji podatek, ga kar vpišemo v oddajni podatkovni register
		// USART vmesnika s pomočjo LL funkcije.
		LL_USART_TransmitData8(SCI.port, data_to_transmit);

	}


	// Po vsakem podatku, ki ga pošljemo, je potrebno preveriti, če smo morda
	// poslali zadnji podatek. To je pomembno, saj moramo v tem primeru ustaviti
	// "avtomatsko" pošiljanje podatkov s pomočjo prekinitev.
	if ( BUF_get_data_size( &SCI_TX_buf_handle ) == 0)
	{
		// In če smo res poslali zadnji podatek, potem moramo onemogočiti nadaljne
		// prekinitve ob sprostitvi oddajnega podatkovnega registra (TXE),
		// saj nimamo več kaj pošiljati. Tako onemogočimo avtomatsko
		// pošiljanje iz oddajnega TX medpomnilnika SCI vmesnika s pomočjo prekinitev.
		// Uporabimo ustrezno LL funkcijo za onemogočitev prekinitve.

		LL_USART_DisableIT_TXE_TXFNF(SCI.port);
	}

}

























