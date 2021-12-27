/*
	uart_tcp.h

	Copyright 2008, 2009 Michel Pollet <buserror@gmail.com>

 	This file is part of simavr.

	simavr is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	simavr is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with simavr.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef __UART_TCP_H___
#define __UART_TCP_H___

#include <pthread.h>
#include "sim_network.h"
#include "sim_irq.h"
#include "fifo_declare.h"

enum {
	IRQ_UART_TCP_BYTE_IN = 0,
	IRQ_UART_TCP_BYTE_OUT,
	IRQ_UART_TCP_COUNT
};

DECLARE_FIFO(uint8_t,uart_tcp_fifo, 512);

typedef struct uart_tcp_t {
	avr_irq_t *	irq;		// irq list
	struct avr_t *avr;		// keep it around so we can pause it

	pthread_t	thread;
	int 		s;			// socket we chat on
	struct sockaddr_in peer;

	int 		server_fd;  // TCP socket for listening
	int 		client_fd;  // TCP socket for communicating
	struct sockaddr_in server;
	struct sockaddr_in client;

	int			xon;
	uart_tcp_fifo_t in;
	uart_tcp_fifo_t out;
} uart_tcp_t;

void uart_tcp_init(struct avr_t * avr, uart_tcp_t * b);

void uart_tcp_connect(uart_tcp_t * p, char uart);

#endif /* __UART_TCP_H___ */
