// mmctl_shr16.h
#ifndef __MMCTL_SHR16_H___
#define __MMCTL_SHR16_H___

#include "sim_irq.h"


enum {
    IRQ_MMCTL_SHR16_ALL = 0,
    IRQ_MMCTL_SHR16_D,
    IRQ_MMCTL_SHR16_L,
    IRQ_MMCTL_SHR16_C,
    IRQ_MMCTL_SHR16_INPUT_COUNT,
    IRQ_MMCTL_SHR16_DATA_OUT = IRQ_MMCTL_SHR16_INPUT_COUNT,
    IRQ_MMCTL_SHR16_COUNT
};

typedef struct mmctl_shr16_t
{
	avr_irq_t* irq;
	struct avr_t* avr;
	union
	{
		struct
		{
			uint8_t pin_d:1;
			uint8_t pin_l:1;
			uint8_t pin_c:1;
		};
		uint8_t pins;
	};
	uint16_t data_shr;
	uint16_t data_out;
} mmctl_shr16_t;


void mmctl_shr16_init(struct avr_t* avr, mmctl_shr16_t* b);

void mmctl_shr16_connect(mmctl_shr16_t* p);


#endif // __MMCTL_SHR16_H___
