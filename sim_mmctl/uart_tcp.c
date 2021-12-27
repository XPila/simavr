/*
	uart_tcp.c

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

#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/fcntl.h>

#include "uart_tcp.h"
#include "avr_uart.h"
#include "sim_time.h"
#include "sim_hex.h"

DEFINE_FIFO(uint8_t,uart_tcp_fifo);


static int set_nonblocking_socket(int sock)
{
#ifdef __MINGW32__
// Windows with MinGW
	u_long mode = 1;  // 1 to enable non-blocking socket
	return ioctlsocket(sock, FIONBIO, &mode);
#else //__MINGW32__
	return fcntl(client_fd, F_SETFL, fcntl(sock, F_GETFL, 0) | O_NONBLOCK);
#endif //__MINGW32__
}

/*
 * called when a byte is send via the uart on the AVR
 */
static void uart_tcp_in_hook(struct avr_irq_t * irq, uint32_t value, void * param)
{
	uart_tcp_t * p = (uart_tcp_t*)param;
//	printf("uart_tcp_in_hook %02x\n", value);
	uart_tcp_fifo_write(&p->in, value);
}

static void uart_tcp_flush_incoming(uart_tcp_t * p)
{
	// try to empty our fifo, the uart_tcp_xoff_hook() will be called when
	// other side is full
	while (p->xon && !uart_tcp_fifo_isempty(&p->out)) {
		uint8_t byte = uart_tcp_fifo_read(&p->out);
//		printf("uart_tcp_xon_hook send %02x\n", byte);
		avr_raise_irq(p->irq + IRQ_UART_TCP_BYTE_OUT, byte);
	}
}

avr_cycle_count_t uart_tcp_flush_timer(struct avr_t * avr, avr_cycle_count_t when, void * param)
{
	uart_tcp_t * p = (uart_tcp_t*)param;
	uart_tcp_flush_incoming(p);
	/* always return a cycle NUMBER not a cycle count */
	return p->xon ? when + avr_hz_to_cycles(p->avr, 1000) : 0;
}

/*
 * Called when the uart has room in it's input buffer. This is called repeateadly
 * if necessary, while the xoff is called only when the uart fifo is FULL
 */
static void uart_tcp_xon_hook(struct avr_irq_t * irq, uint32_t value, void * param)
{
	uart_tcp_t * p = (uart_tcp_t*)param;
//	if (!p->xon)
//		printf("uart_tcp_xon_hook\n");
	p->xon = 1;
	uart_tcp_flush_incoming(p);
	if (p->xon)
		avr_cycle_timer_register(p->avr, avr_hz_to_cycles(p->avr, 1000), uart_tcp_flush_timer, param);
}

/*
 * Called when the uart ran out of room in it's input buffer
 */
static void uart_tcp_xoff_hook(struct avr_irq_t * irq, uint32_t value, void * param)
{
	uart_tcp_t * p = (uart_tcp_t*)param;
	if (p->xon)
		printf("uart_tcp_xoff_hook\n");
	p->xon = 0;
	avr_cycle_timer_cancel(p->avr, uart_tcp_flush_timer, param);
}

static void * uart_tcp_thread(void * param)
{
	uart_tcp_t * p = (uart_tcp_t*)param;
	printf("uart_tcp_thread start\n");

	while (1) {
		fd_set read_set, write_set;
		int max = p->client_fd + 1;
		FD_ZERO(&read_set);
		FD_ZERO(&write_set);

		FD_SET(p->client_fd, &read_set);
		if (!uart_tcp_fifo_isempty(&p->in))
			FD_SET(p->client_fd, &write_set);

		struct timeval timo = { 0, 500 };	// short, but not too short interval
		int ret = select(max, &read_set, &write_set, NULL, &timo);
//		int ret = select(max, &read_set, NULL, NULL, &timo);

		if (!ret)
			continue;
//		printf("uart_tcp_thread select\n");

	#if 1
		if (FD_ISSET(p->client_fd, &read_set)) {
			uint8_t buffer[512];
			ssize_t r = recv(p->client_fd, buffer, sizeof(buffer)-1, 0);

//			hdump("tcp recv", buffer, r);
//			printf("xon=%d\n",p->xon);
			// write them in fifo
			uint8_t * src = buffer;
			while (r-- && !uart_tcp_fifo_isfull(&p->out))
				uart_tcp_fifo_write(&p->out, *src++);
			if (r > 0)
				printf("TCP dropped %zu bytes\n", r);
		}
	#endif
	#if 1
		if (FD_ISSET(p->client_fd, &write_set)) {
			uint8_t buffer[512];
			// write them in fifo
			uint8_t * dst = buffer;
			while (!uart_tcp_fifo_isempty(&p->in) && dst < (buffer+sizeof(buffer)))
				*dst++ = uart_tcp_fifo_read(&p->in);
			socklen_t len = dst - buffer;
			if (len > 0)
			{
				//printf("len=%d\n",len);
				size_t r = send(p->client_fd, buffer, len, 0);
				//*dst = 0;
				//printf("r=%d %s\n",r,buffer);
				//hdump("tcp send", buffer, r);
			}
		}
	#endif
#if 0
		fd_set read_set, write_set;
		int max = p->s + 1;
		FD_ZERO(&read_set);
		FD_ZERO(&write_set);

		FD_SET(p->s, &read_set);
		if (!uart_tcp_fifo_isempty(&p->in))
			FD_SET(p->s, &write_set);

		struct timeval timo = { 0, 500 };	// short, but not too short interval
		int ret = select(max, &read_set, &write_set, NULL, &timo);

		if (!ret)
			continue;

		if (FD_ISSET(p->s, &read_set)) {
			uint8_t buffer[512];

			socklen_t len = sizeof(p->peer);
			ssize_t r = recvfrom(p->s, buffer, sizeof(buffer)-1, 0, (struct sockaddr*)&p->peer, &len);

		//	hdump("tcp recv", buffer, r);

			// write them in fifo
			uint8_t * src = buffer;
			while (r-- && !uart_tcp_fifo_isfull(&p->out))
				uart_tcp_fifo_write(&p->out, *src++);
			if (r > 0)
				printf("TCP dropped %zu bytes\n", r);
		}
		if (FD_ISSET(p->s, &write_set)) {
			uint8_t buffer[512];
			// write them in fifo
			uint8_t * dst = buffer;
			while (!uart_tcp_fifo_isempty(&p->in) && dst < (buffer+sizeof(buffer)))
				*dst++ = uart_tcp_fifo_read(&p->in);
			socklen_t len = dst - buffer;
			/*size_t r = */sendto(p->s, buffer, len, 0, (struct sockaddr*)&p->peer, sizeof(p->peer));
		//	hdump("tcp send", buffer, r);
		}
#endif
	}
	return NULL;
}

static const char * irq_names[IRQ_UART_TCP_COUNT] = {
	[IRQ_UART_TCP_BYTE_IN] = "8<uart_tcp.in",
	[IRQ_UART_TCP_BYTE_OUT] = "8>uart_tcp.out",
};

void uart_tcp_init(struct avr_t * avr, uart_tcp_t * p)
{
	p->avr = avr;
	p->irq = avr_alloc_irq(&avr->irq_pool, 0, IRQ_UART_TCP_COUNT, irq_names);
	avr_irq_register_notify(p->irq + IRQ_UART_TCP_BYTE_IN, uart_tcp_in_hook, p);

	if ((p->server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		fprintf(stderr, "socket=%d", p->server_fd);
		fprintf(stderr, "%s: socket create error: %s", __FUNCTION__, strerror(errno));
		return ;
	}

	p->server.sin_family = AF_INET;
	p->server.sin_addr.s_addr = INADDR_ANY;
	p->server.sin_port = htons(8888);

	if (bind(p->server_fd,(struct sockaddr *)&p->server , sizeof(p->server)) < 0)
	{
		fprintf(stderr, "%s: socket bind error: %s", __FUNCTION__, strerror(errno));
		return ;
	}

	listen(p->server_fd, 3);

	printf("uart_tcp_init bridge on port %d\n", 8888);
	printf("waiting for incoming connections...\n");

	int size = sizeof(struct sockaddr_in);
	if ((p->client_fd = accept(p->server_fd, (struct sockaddr *)&p->client, (socklen_t*)&size)) < 0)
	{
		fprintf(stderr, "%s: socket accept error: %s", __FUNCTION__, strerror(errno));
		return ;
	}

	printf("accepted connection from TODO\n");

//	if (set_nonblocking_socket(p->client_fd) != 0)
//	{
//		fprintf(stderr, "%s: socket fcntl error: %s", __FUNCTION__, strerror(errno));
//		return 0;
//	}

/*	if ((p->s = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
		fprintf(stderr, "%s: Can't create socket: %s", __FUNCTION__, strerror(errno));
		return ;
	}

	struct sockaddr_in address = { 0 };
	address.sin_family = AF_INET;
	address.sin_port = htons (4321);

	if (bind(p->s, (struct sockaddr *) &address, sizeof(address))) {
		fprintf(stderr, "%s: Can not bind socket: %s", __FUNCTION__, strerror(errno));
		return ;
	}

	printf("uart_tcp_init bridge on port %d\n", 4321);*/

	pthread_create(&p->thread, NULL, uart_tcp_thread, p);

}

void uart_tcp_connect(uart_tcp_t * p, char uart)
{
	// disable the stdio dump, as we are sending binary there
	uint32_t f = 0;
	avr_ioctl(p->avr, AVR_IOCTL_UART_GET_FLAGS(uart), &f);
	f &= ~AVR_UART_FLAG_STDIO;
	avr_ioctl(p->avr, AVR_IOCTL_UART_SET_FLAGS(uart), &f);

	avr_irq_t * src = avr_io_getirq(p->avr, AVR_IOCTL_UART_GETIRQ(uart), UART_IRQ_OUTPUT);
	avr_irq_t * dst = avr_io_getirq(p->avr, AVR_IOCTL_UART_GETIRQ(uart), UART_IRQ_INPUT);
	avr_irq_t * xon = avr_io_getirq(p->avr, AVR_IOCTL_UART_GETIRQ(uart), UART_IRQ_OUT_XON);
	avr_irq_t * xoff = avr_io_getirq(p->avr, AVR_IOCTL_UART_GETIRQ(uart), UART_IRQ_OUT_XOFF);
	if (src && dst) {
		avr_connect_irq(src, p->irq + IRQ_UART_TCP_BYTE_IN);
		avr_connect_irq(p->irq + IRQ_UART_TCP_BYTE_OUT, dst);
	}
	if (xon)
		avr_irq_register_notify(xon, uart_tcp_xon_hook, p);
	if (xoff)
		avr_irq_register_notify(xoff, uart_tcp_xoff_hook, p);
}

