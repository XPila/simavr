// mmctl_shr16.c

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sim_time.h"
#include "avr_ioport.h"

#include "mmctl_shr16.h"


static void mmctl_shr16_pin_changed_hook(struct avr_irq_t* irq, uint32_t value, void* param)
{
	mmctl_shr16_t* b = (mmctl_shr16_t*) param;
	uint16_t data_out_old = b->data_out;
	uint8_t pins_old = b->pins;
	switch (irq->irq)
	{
	case IRQ_MMCTL_SHR16_ALL:
		for (int i = 0; i < 3; i++)
			mmctl_shr16_pin_changed_hook(b->irq + IRQ_MMCTL_SHR16_D + i, ((value >> i) & 1), param);
		break;
	case IRQ_MMCTL_SHR16_D:
		b->pin_d = value;
		break;
	case IRQ_MMCTL_SHR16_L:
		b->pin_l = value;
		break;
	case IRQ_MMCTL_SHR16_C:
		b->pin_c = value;
		if (value) b->data_shr = (b->data_shr << 1) | b->pin_d;
		break;
	}
	if (b->pin_l) b->data_out = b->data_shr;
	uint16_t data_out_changed = b->data_out ^ data_out_old;
	if (data_out_changed)
	{
		avr_raise_irq(b->irq + IRQ_MMCTL_SHR16_DATA_OUT, b->data_out);
//		printf("MMCTL SHR16 OUT = %04x\n", b->data_out);
	}
//	uint8_t pins_changed = b->pins ^ pins_old;
//	for (int i = 0; i < 3; i++)
//		if (pins_changed & (1 << i))
//			printf("MMCTL SHR16 PIN %c is %d\n", "DLC"[i], (b->pins & (1 << i))?1:0);
}

static const char * irq_names[IRQ_MMCTL_SHR16_COUNT] =
{
	[IRQ_MMCTL_SHR16_ALL] = "3=mmctl_shr16.pins",
	[IRQ_MMCTL_SHR16_D] = "<mmctl_shr16.D",
	[IRQ_MMCTL_SHR16_L] = "<mmctl_shr16.L",
	[IRQ_MMCTL_SHR16_C] = "<mmctl_shr16.C",
	[IRQ_MMCTL_SHR16_DATA_OUT] = "16>mmctl_shr16.DATA_OUT",
};

void mmctl_shr16_init(struct avr_t* avr, mmctl_shr16_t* b)
{
	memset(b, 0, sizeof(*b));
	b->avr = avr;
	b->irq = avr_alloc_irq(&avr->irq_pool, 0, IRQ_MMCTL_SHR16_COUNT, irq_names);
	for (int i = 0; i < IRQ_MMCTL_SHR16_INPUT_COUNT; i++)
		avr_irq_register_notify(b->irq + i, mmctl_shr16_pin_changed_hook, b);
}

void mmctl_shr16_connect(mmctl_shr16_t* p)
{
	avr_connect_irq(avr_io_getirq(p->avr, AVR_IOCTL_IOPORT_GETIRQ('B'), 5), p->irq + IRQ_MMCTL_SHR16_D);
	avr_connect_irq(avr_io_getirq(p->avr, AVR_IOCTL_IOPORT_GETIRQ('B'), 6), p->irq + IRQ_MMCTL_SHR16_L);
	avr_connect_irq(avr_io_getirq(p->avr, AVR_IOCTL_IOPORT_GETIRQ('C'), 7), p->irq + IRQ_MMCTL_SHR16_C);
}
