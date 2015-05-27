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
 * Timer clock 111Mhz
 */

#if  defined(CONFIG_ZYNQ7_BOARD)
#include <sw_types.h>
#include <sw_io.h>

#include <sw_board.h>
#include <sw_timer.h>

#include <sw_gic.h>

#define XTTCPSS_CLK_CNTRL_OFFSET	0x00 /* Clock Control Reg, RW */
#define XTTCPSS_CNT_CNTRL_OFFSET	0x0C /* Counter Control Reg, RW */
#define XTTCPSS_COUNT_VAL_OFFSET	0x18 /* Counter Value Reg, RO */
#define XTTCPSS_INTR_VAL_OFFSET		0x24 /* Interval Count Reg, RW */
#define XTTCPSS_MATCH_1_OFFSET		0x30 /* Match 1 Value Reg, RW */
#define XTTCPSS_MATCH_2_OFFSET		0x3C /* Match 2 Value Reg, RW */
#define XTTCPSS_MATCH_3_OFFSET		0x48 /* Match 3 Value Reg, RW */
#define XTTCPSS_ISR_OFFSET		0x54 /* Interrupt Status Reg, RO */
#define XTTCPSS_IER_OFFSET		0x60 /* Interrupt Enable Reg, RW */

#define PRESCALE_EXPONENT 	11	/* 2 ^ PRESCALE_EXPONENT = PRESCALE */
#define PRESCALE 		2048	/* The exponent must match this */
#define CLK_CNTRL_PRESCALE (((PRESCALE_EXPONENT - 1) << 1) | 0x1)


#define XTTCPSS_CNT_CNTRL_DISABLE_MASK	0x1
#define XTTCPSS_RST_COUNTER             0x10

/**
 * @brief 
 *
 * @param timer_base
 * @param tim_mod_index
 */
void secure_timer_enable(sw_uint timer_base, sw_uint tim_mod_index)
{
	sw_uint ctrl_reg;

	ctrl_reg = sw_readl((void *)(timer_base + XTTCPSS_CNT_CNTRL_OFFSET));
	ctrl_reg |= XTTCPSS_RST_COUNTER;
	ctrl_reg &= ~XTTCPSS_CNT_CNTRL_DISABLE_MASK;
	sw_writel(ctrl_reg, (void *)(timer_base + XTTCPSS_CNT_CNTRL_OFFSET));

	if (timer_base == TICK_TIMER_BASE)
		sw_gic_unmask_interrupt(TICK_TIMER_IRQ);
	if (timer_base == FREE_RUNNING_TIMER_BASE)
		sw_gic_unmask_interrupt(FREE_RUNNING_TIMER_IRQ);
}

/**
 * @brief 
 *
 * @param timer_base
 * @param tim_mod_index
 */
void secure_timer_disable(sw_uint timer_base, sw_uint tim_mod_index)
{
	sw_uint ctrl_reg;

	ctrl_reg = sw_readl((void *)(timer_base + XTTCPSS_CNT_CNTRL_OFFSET));
	ctrl_reg |= XTTCPSS_CNT_CNTRL_DISABLE_MASK;
	sw_writel(ctrl_reg, (void *)(timer_base + XTTCPSS_CNT_CNTRL_OFFSET));

	if (timer_base == TICK_TIMER_BASE)
		sw_gic_mask_interrupt(TICK_TIMER_IRQ);
	if (timer_base == FREE_RUNNING_TIMER_BASE)
		sw_gic_mask_interrupt(FREE_RUNNING_TIMER_IRQ);
}

/**
 * @brief 
 *
 * @param timer_base
 */
void secure_timer_init(sw_uint unused)
{
	sw_uint timer_base;

	/* 
	 * Initialize both free running and event timer 
	 * timer_base is unused here.
	 */

	/* 
	 * Init free running timer
	 * Overflow mode, with overflow interrupt enabled
	 */
	timer_base = FREE_RUNNING_TIMER_BASE;
	/* Set mode and keep it disabled */
	sw_writel(0x01, (void *)(timer_base + XTTCPSS_CNT_CNTRL_OFFSET));
	sw_writel(CLK_CNTRL_PRESCALE, (void *)(timer_base 
				+ XTTCPSS_CLK_CNTRL_OFFSET));
	/* Enable required interrupts */
	sw_writel(0x10, (void *)(timer_base + XTTCPSS_IER_OFFSET)); 

	/* 
	 * Init tick timer, used for one shot timer
	 * Programmed as interval mode, interval interrupt enabled
	 *
	 * Infact we this is been used as one shot timer and not periodic,
	 * After every interval we disable it, and the interval is dynamic.
	 */

	timer_base = TICK_TIMER_BASE;
	sw_writel(0x23, (void *)(timer_base + XTTCPSS_CNT_CNTRL_OFFSET));
	/* Set mode and keep it disabled */
	sw_writel(CLK_CNTRL_PRESCALE, (void *)(timer_base + 
				XTTCPSS_CLK_CNTRL_OFFSET));
	/* Enable required interrupts */
	sw_writel(0x1, (void *)(timer_base + XTTCPSS_IER_OFFSET));
}

/**
 * @brief 
 *
 * @param timer_base
 * @param tim_mod_index
 *
 * @return 
 */
sw_uint secure_read_timer(sw_uint timer_base, sw_uint tim_mod_index)
{

	return sw_readl((void *)(timer_base + XTTCPSS_COUNT_VAL_OFFSET));
}

/**
 * @brief 
 *
 * @param timer_base
 * @param tim_mod_index
 * @param mode
 * @param cycles
 */
void secure_set_timer(sw_uint timer_base, sw_uint tim_mod_index, sw_uint mode, sw_uint cycles)
{
	sw_uint ctrl_reg;

	if (timer_base == TICK_TIMER_BASE) {
		/* 
		 * Disable the counter, set the interval
		 */
		ctrl_reg = sw_readl((void *)(TICK_TIMER_BASE + 
					XTTCPSS_CNT_CNTRL_OFFSET));
		ctrl_reg |= XTTCPSS_CNT_CNTRL_DISABLE_MASK;
		sw_writel(ctrl_reg, (void *)(TICK_TIMER_BASE + 
					XTTCPSS_CNT_CNTRL_OFFSET));
		sw_writel(cycles, (void *)(TICK_TIMER_BASE + 
					XTTCPSS_INTR_VAL_OFFSET));

	} 
	/* 
	 * do nothing for free running timer which is configured 
	 * during secure_timer_init 
	 */
}

/**
 * @brief 
 *
 * @param irq
 */
void secure_timer_irq_handler(int irq)
{
	sw_uint val;


	if (irq == TICK_TIMER_IRQ) {
		val = sw_readl((void *)(TICK_TIMER_BASE+ XTTCPSS_ISR_OFFSET));
		if(val){
			/* disable the timer, since this is used as one shot */
			secure_timer_disable(TICK_TIMER_BASE, 0);
			timer_interrupt(); 
			sw_writel(val, (void *)(TICK_TIMER_BASE + 
						XTTCPSS_ISR_OFFSET));		
		}
	} else if (irq == FREE_RUNNING_TIMER_IRQ) {
		val = sw_readl((void *)(FREE_RUNNING_TIMER_BASE + 
					XTTCPSS_ISR_OFFSET));
		if(val) {
			free_running_cntr_intr();
			sw_writel(val, (void *)(FREE_RUNNING_TIMER_BASE + 
						XTTCPSS_ISR_OFFSET));
		}
	}
}

/**
 * @brief 
 *
 * @return 
 */
sw_uint get_clock_period_us(void)
{
	return TIMER_PERIOD_US; 
}


/**
 * @brief 
 *
 * @return 
 */
sw_ulong get_timer_period(void)
{
	sw_timeval timer_period;
	timer_period.tval.sec = 1;
	timer_period.tval.nsec = 186201600;
	return timer_period.tval64;
}

/**
 * @brief 
 *
 * @return 
 */
sw_ulong get_clock_period(void)
{
	sw_timeval clock_period;
	clock_period.tval.sec = 0;
	clock_period.tval.nsec = 18000;
	return clock_period.tval64;
}



/**
 * @brief 
 *
 * @param timer_base
 *
 * @return 
 */
sw_uint read_timer_value(sw_uint timer_base)
{
	return secure_read_timer(timer_base, 0);
}

/**
 * @brief 
 *
 * @return 
 */
sw_uint read_freerunning_cntr(void)
{
	/* Increment type */
	return read_timer_value(FREE_RUNNING_TIMER_BASE);
}


#endif

