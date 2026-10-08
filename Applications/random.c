#include "random.h"
#include "main.h" //main.h dodamo zato da ima datoteka dostop do funkcije RNG

uint32_t Random_GetNumber(void)
{
	uint32_t random_number;

	HAL_RNG_GenerateRandomNumber(&hrng, &random_number);

	return random_number;
}


