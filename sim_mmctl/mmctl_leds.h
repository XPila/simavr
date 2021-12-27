// mmctl_leds.h
#ifndef __MMCTL_LEDS_H___
#define __MMCTL_LEDS_H___

#include "sim_irq.h"
#include "mmctl_shr16.h"


enum
{
	MMCTL_LED_AG = 0,
	MMCTL_LED_AR,
	MMCTL_LED_BG,
	MMCTL_LED_BR,
	MMCTL_LED_CG,
	MMCTL_LED_CR,
	MMCTL_LED_DG,
	MMCTL_LED_DR,
	MMCTL_LED_EG,
	MMCTL_LED_ER,
	MMCTL_LED_RX,
	MMCTL_LED_TX,
	MMCTL_LED_COUNT,
};

enum {
    IRQ_MMCTL_LEDS_ALL = 0,
    IRQ_MMCTL_LEDS_RX,
    IRQ_MMCTL_LEDS_TX,
    IRQ_MMCTL_LEDS_SHR16,
    IRQ_MMCTL_LEDS_INPUT_COUNT,
    IRQ_MMCTL_LEDS_DATA_OUT = IRQ_MMCTL_LEDS_INPUT_COUNT,
    IRQ_MMCTL_LEDS_COUNT
};

typedef struct mmctl_leds_t
{
	avr_irq_t* irq;
	struct avr_t* avr;
	uint16_t leds;
} mmctl_leds_t;


void mmctl_leds_init(struct avr_t* avr, mmctl_leds_t* b);

void mmctl_leds_connect(mmctl_leds_t* p, mmctl_shr16_t* s);


#endif // __MMCTL_LEDS_H___
