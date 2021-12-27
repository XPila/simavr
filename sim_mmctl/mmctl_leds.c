// mmctl_leds.c

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sim_time.h"
#include "avr_ioport.h"

#include "mmctl_leds.h"


static const char* led_names[MMCTL_LED_COUNT] =
{
	[MMCTL_LED_AG] = "AG",
	[MMCTL_LED_AR] = "AR",
	[MMCTL_LED_BG] = "BG",
	[MMCTL_LED_BR] = "BR",
	[MMCTL_LED_CG] = "CG",
	[MMCTL_LED_CR] = "CR",
	[MMCTL_LED_DG] = "DG",
	[MMCTL_LED_DR] = "DR",
	[MMCTL_LED_EG] = "EG",
	[MMCTL_LED_ER] = "ER",
	[MMCTL_LED_RX] = "RX",
	[MMCTL_LED_TX] = "TX",
};

static void mmctl_leds_pin_changed_hook(struct avr_irq_t* irq, uint32_t value, void* param)
{
	mmctl_leds_t* b = (mmctl_leds_t*) param;
	//printf("mmctl_leds_pin_changed_hook %u'\n", value);
	uint16_t leds_old = b->leds;
	switch (irq->irq)
	{
	case IRQ_MMCTL_LEDS_ALL:
		break;
	case IRQ_MMCTL_LEDS_RX:
		b->leds = (b->leds & ~(1 << MMCTL_LED_RX)) | (value?0:(1 << MMCTL_LED_RX));
		break;
	case IRQ_MMCTL_LEDS_TX:
		b->leds = (b->leds & ~(1 << MMCTL_LED_TX)) | (value?0:(1 << MMCTL_LED_TX));
		break;
	case IRQ_MMCTL_LEDS_SHR16:
		b->leds = (b->leds & ~(0x03ff)) | ((value & 0xff00) >> 8) | ((value & 0x00c0) << 2);
//		printf("MMCTL LED SHR16 = %04x\n", value);
		break;
	}
	uint16_t leds_changed = b->leds ^ leds_old;
	if (leds_changed)
	{
//		printf("MMCTL LEDS = %04x\n", b->leds);
		avr_raise_irq(b->irq + IRQ_MMCTL_LEDS_DATA_OUT, b->leds);
		for (int i = 0; i < MMCTL_LED_COUNT; i++)
			if (leds_changed & (1 << i))
				printf("MMCTL LED %s is %s\n", led_names[i], (b->leds & (1 << i))?"ON":"OFF");
	}
}

static const char * irq_names[IRQ_MMCTL_LEDS_COUNT] =
{
	[IRQ_MMCTL_LEDS_ALL] = "2=mmctl_leds.pins",
	[IRQ_MMCTL_LEDS_RX] = "<mmctl_leds.RX",
	[IRQ_MMCTL_LEDS_TX] = "<mmctl_leds.TX",
	[IRQ_MMCTL_LEDS_SHR16] = "8<mmctl_leds.SHR16",
	[IRQ_MMCTL_LEDS_DATA_OUT] = "12>mmctl_leds.DATA_OUT",
};

void mmctl_leds_init(struct avr_t* avr, mmctl_leds_t* b)
{
	memset(b, 0, sizeof(*b));
	b->avr = avr;
	b->irq = avr_alloc_irq(&avr->irq_pool, 0, IRQ_MMCTL_LEDS_COUNT, irq_names);
	for (int i = 0; i < IRQ_MMCTL_LEDS_INPUT_COUNT; i++)
		avr_irq_register_notify(b->irq + i, mmctl_leds_pin_changed_hook, b);
}

void mmctl_leds_connect(mmctl_leds_t* p, mmctl_shr16_t* s)
{
	avr_connect_irq(avr_io_getirq(p->avr, AVR_IOCTL_IOPORT_GETIRQ('B'), 0), p->irq + IRQ_MMCTL_LEDS_RX);
	avr_connect_irq(avr_io_getirq(p->avr, AVR_IOCTL_IOPORT_GETIRQ('D'), 5), p->irq + IRQ_MMCTL_LEDS_TX);
	avr_connect_irq(s->irq + IRQ_MMCTL_SHR16_DATA_OUT, p->irq + IRQ_MMCTL_LEDS_SHR16);
}
