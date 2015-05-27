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
 * uart registers Declaration
 */

#ifndef _XILINX_UART_H
#define _XILINX_UART_H

/* UART Registers offset*/
#define UART_CONTROL_REG_OFFSET         	0x00 /* Control Register [8:0] */
#define UART_MODE_REG_OFFSET          		0x04 /* Mode Register [10:0] */
#define UART_INTRPT_STAT_REG_OFFSET			0x14 /* Channel Interrupt Status Register*/
#define UART_BAUD_RATE_GEN_OFFSET     		0x18 /* Baud Rate Generator [15:0] */
#define UART_FIFO_OFFSET        			0x30 /* FIFO [15:0] or [7:0] */
#define UART_BAUD_RATE_DIV_OFFSET     		0x34 /* Baud Rate Divider [7:0] */
#define UART_INTRPT_EN_REG_OFFSET   		0x8  /*Interrupt Enable Register */
#define UART_STATUS_REG_OFFSET          	0x2C /* Channel Status [11:0] */
#define UART_INTRPT_EN_REG_OFFSET   		0x8  /*Interrupt Enable Register */
#define UART_INTRPT_DIS_REG_OFFSET  		0xC  /*Interrupt Disable Register*/
#define UART_INTRPT_MASK_REG_OFFSET 		0x10 /*Interrupt Mask Register*/
#define UART_TR_RX_FIFO_REG_OFFSET  		0x30 /* Transmit and Receive FIFO*/
#define UART_RX_FIFO_TRIG_LVL_REG_OFFSET	0x20 /* RX FIFO Trigger Level*/
#define UART_RX_TIMEOUT_REG_OFFSET         	0x1C /* RX TimeOut*/


#define UART_CONTROL_REG_VALUE           	0x17 /*Value taken from First Stage
													Bootloader code */
#define UART_CONTROL_REG_TX_EN     			0x00000010  /* TX enabled */
#define UART_CONTROL_REG_RX_EN       		0x00000004  /* RX enabled */
#define UART_MODE_REG_PARITY_NONE  			0x00000020  /* No parity mode */
#define UART_STATUS_REG_TXFULL   			0x00000010  /* TX FIFO full */
#define UART_STATUS_REG_TXEMPTY				0x00000008  /* TX FIFO empty */
#define UART_STATUS_REG_RXFULL      		0x4			/* RX FIFO full*/
#define UART_STATUS_REG_RXEMPTY     		0x2         /* RX FIFO empty*/
#define UART_STATUS_REG_RXOVR       		0x1         /* RX FIFO Trigger*/
#define UART_MASK_INTRPT            		0x000003FF

#define UART_BAUD_115K						0x56	/* 115200 based on 33.33MHz / 63 clock */
#define UART_BAUDDIV_115K					0x4
#define UART_BAUD_RATE						115200
#define UART_FREQ							50000000
#define MAX_ERROR_RATE      				3 /* The maximum percentage error allowed between
								 				the original value & calculated value */

#endif


