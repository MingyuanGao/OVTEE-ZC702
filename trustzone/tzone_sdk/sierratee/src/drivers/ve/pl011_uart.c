/* 
 * OpenVirtualization: 
 * For additional details and support contact developer@sierraware.com.
 * Additional documentation can be found at www.openvirtualization.org
 * 
 * Copyright (C) 2011 SierraWare
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

/* 
 * PL011 UART driver implementation
 */

#if defined(CONFIG_EB_BOARD)  || defined(CONFIG_VE_BOARD)

#include <uart.h>
#include <pl011_uart.h>
#include <sw_types.h>
#include <sw_board.h>
#include <sw_modinit.h>
#ifdef CONFIG_SHELL
#include <shell_command_manager.h>
#include <shell_process_task.h>
#endif

/*
 * UART ADDR array.
 */
sw_uint UART_ADDR[] = {UART0_ADDR_PA, UART1_ADDR_PA, UART2_ADDR_PA, UART3_ADDR_PA};


/**
 * @brief 
 *
 * @param id
 *
 * @return 
 */
static inline sw_uint get_uart_base_addr(sw_uint id);

/**
 * @brief 
 *
 * @param reg_offs
 * @param uartid
 *
 * @return 
 */
sw_uint read_uart(sw_uint reg_offs, sw_uint uartid)
{
	volatile sw_uint * reg_ptr = (sw_uint*)(get_uart_base_addr(uartid) | reg_offs);
	return *reg_ptr;
}

/**
 * @brief 
 *
 * @param reg_offs
 * @param value
 * @param uartid
 */
void write_uart(sw_uint reg_offs, sw_uint value, sw_uint uartid)
{
	volatile sw_uint * reg_ptr = (sw_uint*)(get_uart_base_addr(uartid) | reg_offs);
	*reg_ptr = value;
}


/**
 * @brief 
 *
 * @param id
 *
 * @return 
 */
static inline sw_uint get_uart_base_addr(sw_uint id)
{
	if(id <= 3)
		return UART_ADDR[id];
	else {
		sw_seterrno(SW_EINVAL);
		return SW_ERROR;
	}
}

/**
 * @brief 
 *
 * @param c
 */
void serial_putc(char c)
{
	/* Wait until there is space in the FIFO */
	while (read_uart(UART_PL01x_FR, UART_ID) & UART_PL01x_FR_TXFF);

	/* Send the character */
	write_uart(UART_PL01x_DR, c, UART_ID);
}

/**
 * @brief 
 *
 * @param c
 */
void serial_puts(char * c)
{
	int index = 0;

	while (c[index] != '\0')
	{
		serial_putc(c[index]);
		index++;
	}
}

/**
 * @brief 
 *
 * @param uart_id
 */
void serial_init(sw_uint uart_id)
{
	int lcrh_reg;
	/* First, disable everything */
	write_uart(UART_PL011_CR, 0x0, uart_id);

	lcrh_reg = read_uart(UART_PL011_LCRH, uart_id);
	lcrh_reg &= ~UART_PL011_LCRH_FEN;
	write_uart(UART_PL011_LCRH, lcrh_reg, uart_id);

	/*
	 * Set baud rate
	 *
	 baudrate = 115200
	 UART clock = 24000000
	 * IBRD = UART_CLK / (16 * BAUD_RATE)
	 * FBRD = RND((64 * MOD(UART_CLK,(16 * BAUD_RATE))) 
	 *    / (16 * BAUD_RATE))
	 */

	write_uart(UART_PL011_IBRD, 13, uart_id);

	/* Set the UART to be 8 bits, 1 stop bit, 
	 * no parity, fifo enabled 
	 */
	write_uart(UART_PL011_LCRH, (UART_PL011_LCRH_WLEN_8 | UART_PL011_LCRH_FEN), uart_id);

	/* Finally, enable the UART */
	write_uart(UART_PL011_CR, (UART_PL011_CR_UARTEN | UART_PL011_CR_TXE
				| UART_PL011_CR_LBE), uart_id);
	write_uart(UART_PL011_IFLS, UART_PL011_IFLS_RX4_8, uart_id);

	write_uart(UART_PL01x_DR, 0, uart_id);

	while (read_uart(UART_PL01x_FR, uart_id) & UART_PL01x_FR_BUSY);

	write_uart(UART_PL011_CR, (UART_PL011_CR_UARTEN | 
				UART_PL011_CR_RXE | UART_PL011_CR_TXE), uart_id);

	write_uart(UART_PL011_ICR, (UART_PL011_RIS_OERIS |
				UART_PL011_RIS_BERIS | UART_PL011_RIS_PERIS |
				UART_PL011_RIS_FERIS), uart_id);

	write_uart(UART_PL011_IMSC, (UART_PL011_MIS_RXMIS | 
				UART_PL011_MIS_RTMIS), uart_id);


	return;
}

#ifdef CONFIG_SHELL
/**
* @brief 
*
* @param irq
* @param data
*/
void serial_irq_handler(sw_uint irq, void *data)
{
	sw_uint mis_status, check_val;
	sw_uint uart_id;
	char ch;	
	int task_id;
	struct sw_task *task ;
	struct shell_global *task_data;
	task_id = (int)data;
	task = get_task(task_id);
	if(task != NULL){
		task_data = (struct shell_global *)task->tls->private_data;
		uart_id = task_data->uart_id;

		mis_status = read_uart(UART_PL011_MIS, uart_id);
		check_val = (UART_PL011_RIS_RXRIS | UART_PL011_RIS_RTRIS);

		if(mis_status){
			write_uart(UART_PL011_ICR, mis_status & ~ (UART_PL011_RIS_RXRIS |
						UART_PL011_RIS_TXRIS | UART_PL011_RIS_RTRIS), uart_id);
			if(mis_status & (UART_PL011_RIS_RXRIS | UART_PL011_RIS_RTRIS))
				ch = receive_characters(uart_id);
			if(mis_status & UART_PL011_RIS_TXRIS)
				transmit_characters(uart_id);
			do {
				mis_status = read_uart(UART_PL011_MIS, uart_id);
				if(mis_status)	
					read_uart(UART_PL01x_DR, uart_id);
			}while(mis_status);

			task_data->data = ch;			
			schedule_task_id(task_id);
		}
	}
	return;
}

/**
 * @brief 
 *
 * @param uart_id
 *
 * @return
 */
char receive_characters(sw_uint uart_id)
{
	char ch;
	sw_uint flag_status;		
	flag_status = read_uart(UART_PL01x_FR, uart_id);

	ch = read_uart(UART_PL01x_DR, uart_id);
	if(ch == KEY_ESCAPE) {
		ch =  read_uart(UART_PL01x_DR, uart_id);
		if(ch == KEY_ESCAPE_SEQUENCE) {
			ch = read_uart(UART_PL01x_DR, uart_id);
			if(!read_uart(UART_PL01x_DR, uart_id)) {
				if(ch == KEY_UP)
					ch = MASK_KEY_UP;
				else if(ch == KEY_DOWN)
					ch = MASK_KEY_DOWN;
				else if(ch == KEY_RIGHT)
					ch = MASK_KEY_RIGHT;
				else if(ch == KEY_LEFT)
					ch = MASK_KEY_LEFT;
				else if(ch == KEY_END)
					ch = MASK_KEY_END;
				else if(ch == KEY_HOME)
					ch = MASK_KEY_HOME;
				return ch;
			}
			return NULL;
		}
	}
	return ch;
}

/**
 * @brief 
 *
 * @param uart_id
 */
void transmit_characters(sw_uint uart_id)
{
	return;
}
#endif

#endif
