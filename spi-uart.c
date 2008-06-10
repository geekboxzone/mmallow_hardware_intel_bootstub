/*
 * spi-uart debug routings 
 * Copyright (C) 2008, Feng Tang <feng.tang@intel.com> Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 
 * 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include "spi-uart.h"

#define MRST_SPI_TIMEOUT	0x200000
static spi_inited = 0;
static struct mrst_spi_reg *pspi = 0;

static void spi_init()
{
	u32 ctrlr0;

	pspi = (struct mrst_spi_reg *)MRST_REGBASE_SPI0;

	/* disable SPI controller first */
	pspi->ssienr = 0x0;

	/* set control param, 16 bits, transmit only mode */
	ctrlr0 = pspi->ctrlr0;

	ctrlr0 &= 0xfcc0;
	ctrlr0 |= (0xf | (FRF_SPI << SPI_FRF_OFFSET)
		       | (TMOD_TO << SPI_TMOD_OFFSET));
	pspi->ctrlr0 = ctrlr0;
	
	/* set a default baud rate, 115200 */
	/* feng, need make sure SPIC and MAXIM3110 match */
	//spi_enable_clk(32);
	pspi->baudr = 0x32;

	/* need set the transmit threshhol? */
	pspi->txftlr = 0x3;

	/* disable all INT for early phase */
	pspi->imr &= 0xffffff00;
	
	/* select one slave SPI device */
	pspi->ser = 0x1;

	/* enable the HW, this should be the last step for HW init */
	pspi->ssienr |= 0x1;

	spi_inited = 1;
}

/* set the ratio rate, INT */
static void max3110_write_config(void)
{
	u16 config;

	/* 115200, TM not set, no parity, 8bit word */
	config = 0xc000; 	
	pspi->dr[0] = config;
}

/* transfer char to a eligibal word and send to max3110 */
static int max3110_write_data(char c)
{
	u16 data;
	u8  config;

	data = 0x8000 | c;
	pspi->dr[0] = data;
}

/* slave select should be called in the read/write function */
static int spi_max3110_putc(char c)
{
	unsigned int timeout;
	u32 sr;
	u32 test;

	/* read RX FIFO out if there is any */
	while ((pspi->sr & SR_RF_NOT_EMPT) && pspi->rxflr ) {
		timeout = MRST_SPI_TIMEOUT;
		while (timeout--) {
			 if (!(pspi->sr & SR_BUSY))
				break;
		}

		if (timeout == 0xffffffff)
			return -1;
		test = pspi->dr[0];
	}

	timeout = MRST_SPI_TIMEOUT;
	/* early putc need make sure the TX FIFO is empty */
	while (timeout--) {
		sr = pspi->sr;
		if ( (sr & SR_BUSY) || !(sr & SR_TF_EMPT))
			continue;
		else
			break;
	}

	if (timeout == 0xffffffff)
		return -1;

	max3110_write_data(c);

	return 0;
}


void bs_spi_printk(const char *str)
{
	if (!spi_inited)
		spi_init();

	if (!str)
		return;

	/* 
	 * here we assume only 1 write_config is enough,
	 * if not will call it for each putc 
	 */
	max3110_write_config();

	while (*str) {
		if (*str == '\n')
			spi_max3110_putc('\r');
		spi_max3110_putc(*str++);
	}
}
