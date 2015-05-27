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
 * uart registers implementation
 */

#if defined(CONFIG_ZYNQ7_BOARD)
#include <uart.h>

#include <xilinx_uart.h>
#include <sw_types.h>
#include <sw_board.h>
#ifdef CONFIG_SHELL
#include <shell_command_manager.h>
#include <shell_process_task.h>
#endif

sw_uint secure_uart_base = SECURE_UART_BASE_PA;

/**
 * @brief Function to read uart registers
 *
 * @param reg_addr - The address of the register
 * @param uart_id - This is currently ignored.
 */ 

#define SYNCHRONIZE __asm__ __volatile__ ("dmb" : : : "memory")

sw_uint read_uart(sw_uint reg_addr, sw_uint uart_id)
{
	SYNCHRONIZE;
	return(*(volatile sw_uint*)reg_addr);
}


/**
 * @brief Function to write to uart registers
 *
 * @param reg_addr - The address of the register
 * @param value - The value that needs to be written
 * @param uart_id - This is currently ignored.
 */ 


void write_uart(sw_uint reg_addr, sw_uint value, sw_uint uart_id)
{
	*(volatile sw_uint*)reg_addr = value;
	SYNCHRONIZE;
}


/**
 * @brief Function to write a character to uart.
 *
 * @param c - The character that needs to be written to uart.
 */ 

void serial_putc(char ch)
{
	while(read_uart((secure_uart_base + UART_STATUS_REG_OFFSET),0) 
			& UART_STATUS_REG_TXFULL);
	/*{
	  dmb();
	  }*/
	write_uart((secure_uart_base + UART_FIFO_OFFSET),ch,0);
}


/**
 * @brief Function to write a character to uart.
 *
 * @param c - The character buffer that needs to be written to uart.
 */ 

void serial_puts(char * c)
{
	int index = 0;

	while (c[index] != '\0')
	{
		serial_putc(c[index]);
		index++;
	}
	/* Clear the interrupts after writing into UART */
	write_uart((secure_uart_base + UART_INTRPT_STAT_REG_OFFSET), 0xFFFFFFFF, 0);
}


/**
 * @brief Function to init the uart subsystem
 *
 * @param uart_id - This is currently ignored.
 */

void serial_init(sw_uint uart_id)
{
	/*if (read_uart((secure_uart_base + UART_MODE_REG_OFFSET),0) != 0)
	  return;*/
	/* Enable the transmitter and receiver, the mode for no parity, 1 stop,
	   8 data bits, and baud rate of 9600 
	   */
	sw_uint baudRate = UART_BAUD_RATE;
	sw_uint inputClock = UART_FREQ;
	sw_short_int loop_cntr = 0;
	sw_uint calculated_baud_rate = 0;
	sw_uint calculated_baud_rate_generator = 0;
	sw_uint best_baud_rate_divisor = 0;
	sw_uint best_baud_rate_generator = 0;
	sw_uint best_error = 0xFFFFFFFF;
	sw_uint calculated_error = 0;

	/*loop to calculate best baud rate divisor value.
	 * code inspired by code from first stage bootloader */
	for(loop_cntr=4;loop_cntr<255;loop_cntr++) {
		calculated_baud_rate_generator = inputClock/(baudRate*(loop_cntr+1));
		calculated_baud_rate =
			inputClock/(calculated_baud_rate_generator*(loop_cntr+1));
		calculated_error = ((baudRate > calculated_baud_rate) ?
				(baudRate - calculated_baud_rate) :
				(calculated_baud_rate - baudRate));
		if(best_error > calculated_error) {
			best_baud_rate_generator = calculated_baud_rate_generator;
			best_baud_rate_divisor = loop_cntr;
			best_error = calculated_error;
		}
	}
	if(((best_error * 100)/baudRate) > MAX_ERROR_RATE) {
		return;
	}
	write_uart((secure_uart_base + UART_BAUD_RATE_GEN_OFFSET),
			best_baud_rate_generator,0); 
	write_uart((secure_uart_base + UART_BAUD_RATE_DIV_OFFSET),
			best_baud_rate_divisor,0);
	write_uart((secure_uart_base + UART_CONTROL_REG_OFFSET),UART_CONTROL_REG_VALUE,0);
	write_uart((secure_uart_base + UART_MODE_REG_OFFSET),UART_MODE_REG_PARITY_NONE,0);

	/*Set the Trigger level to 1; RX FIFO Trigger Interrupt will be generated
	 * when the FIFO buffer reaches this level */
	write_uart((secure_uart_base + UART_RX_FIFO_TRIG_LVL_REG_OFFSET), 0x1, 0);

	/* Clear all the interrupts in Interrupt Status Register if there is any*/
	write_uart((secure_uart_base + UART_INTRPT_STAT_REG_OFFSET), 0xFFFFFFFF, 0);

	/* Enable RX FIFO Trigger Interrupt */
	write_uart((secure_uart_base + UART_INTRPT_EN_REG_OFFSET), 
			   (UART_STATUS_REG_RXOVR),0);
	
}

#ifdef CONFIG_SHELL

char uart_receive(sw_uint uart_id){
	sw_uint intrpt_status_reg;
	char data;
	/* Read Interrupt Status Register; Check whether RX FIFO trigger interrupt
	 * is generated, if so read the TR_RX_FIFO register till the FIFO buffer
	 * becomes empty*/

	intrpt_status_reg = read_uart((secure_uart_base + UART_INTRPT_STAT_REG_OFFSET), 0);
	while((intrpt_status_reg & UART_STATUS_REG_RXOVR) && 
		!(intrpt_status_reg & UART_STATUS_REG_RXEMPTY)){
		data = read_uart((secure_uart_base + UART_TR_RX_FIFO_REG_OFFSET), 0);
		intrpt_status_reg = read_uart((secure_uart_base + UART_INTRPT_STAT_REG_OFFSET), 0);		
	}

	return data;
}

char uart_receive_character(sw_uint uart_id){
	char ch;
	ch = uart_receive(uart_id);
    if(ch == KEY_ESCAPE){
    	if(ch == KEY_ESCAPE_SEQUENCE) {
        	ch = uart_receive(uart_id);
            	if(!uart_receive(uart_id)) {
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


void serial_irq_handler(sw_uint irq, void *data){
	char ch;
    int task_id;
	sw_uint uart_id;
    struct sw_task *task ;
    struct shell_global *task_data;
	sw_uint isr_reg;

    task_id = (int)data;
    task = get_task(task_id);

    if(task != NULL){
        task_data = (struct shell_global *)task->tls->private_data;
        uart_id = task_data->uart_id;

		/* Read Interrupt status register to check whether there is any
		 * interrupt generated */
		isr_reg = read_uart((secure_uart_base + UART_INTRPT_STAT_REG_OFFSET), 0);
		if(isr_reg){
			/* Disable all the interrupts so that no more interrupt is generated
			 * while handling this interrupt*/
			write_uart((secure_uart_base + UART_INTRPT_DIS_REG_OFFSET), 
						UART_MASK_INTRPT,0);
			
			if(isr_reg & UART_STATUS_REG_RXOVR){
				/* Enable RX Empty Interrupt */
				write_uart((secure_uart_base + UART_INTRPT_EN_REG_OFFSET),
                           (UART_STATUS_REG_RXEMPTY),0);

				/* Clear the RX Empty Interrupt if there is any in Interrupt
				 * Status Register*/
				write_uart((secure_uart_base + UART_INTRPT_STAT_REG_OFFSET), 
						(UART_STATUS_REG_RXEMPTY),0);

				ch = uart_receive_character(uart_id);
				/* Clear the RX FIFO Trigger Interrupt which is generated*/
				write_uart((secure_uart_base + UART_INTRPT_STAT_REG_OFFSET), 
						(UART_STATUS_REG_RXOVR),0);
				/* Disable the RX Empty Interrupt; Otherwise the interrupt will
				 * be continuously triggered*/
				write_uart((secure_uart_base + UART_INTRPT_DIS_REG_OFFSET),
                           (UART_STATUS_REG_RXEMPTY),0);
			}
			/* Enable the RX FIFO Trigger interrupt */
			write_uart((secure_uart_base + UART_INTRPT_EN_REG_OFFSET),
					(UART_STATUS_REG_RXOVR),0);
		}
		task_data->data = ch;
    	schedule_task_id(task_id);
	}
}

#endif

#endif

