/* define spi-uart debug constrains */
/* code for MRST early printk */
#ifndef _SPI_UART
#define _SPI_UART

#include "types.h"

typedef volatile unsigned short vu16;
typedef volatile unsigned int	vu32;

#define MRST_REGBASE_SPI0	0xff128000
#define MRST_REGBASE_SPI1	0xff128400
#define MRST_REGBASE_SPI2	0xff128800

struct mrst_spi_reg {
	vu32 ctrlr0;	/* control reg 0 */
	vu32 ctrlr1;	/* control reg 1 */
	vu32 ssienr;	/* SSI enable reg */
	vu32 mwcr;	/* Microwire control reg */

	vu32 ser;	/* slave enable reg */
	vu32 baudr;
	vu32 txftlr;
	vu32 rxftlr;

	vu32 txflr;
	vu32 rxflr;
	vu32 sr;
	vu32 imr;

	vu32 isr;
	vu32 risr;
	vu32 txoicr;
	vu32 rxoicr;

	vu32 rxuicr;
	vu32 msticr;
	vu32 icr;
	vu32 dmacr;

	vu32 dmatdlr;
	vu32 dmardlr;
	vu32 idr;
	vu32 ssi_comp_version;

	vu32 dr[16];	/* 16 bits access for each 32bit space */
};

/* bit fields in CTRLR0 */
#define SPI_DFS_OFFSET			0
#define SPI_FRF_OFFSET			4
#define FRF_SPI				0x0
#define FRF_SSP				0x1
#define FRF_MICROWIRE			0x2
#define FRF_RESV			0x3
#define SPI_SCPH_OFFSET			6
#define SPI_SCOL_OFFSET			7
#define SPI_TMOD_OFFSET			8
#define	TMOD_TR				0x0		/* xmit & recv */
#define	TMOD_TO				0x1		/* xmit only */
#define	TMOD_RO				0x2		/* recv only */
#define	TMOD_EPROMREAD			0x3		/* eeprom read mode */

#define SPI_SLVOE_OFFSET		10
#define SPI_SRL_OFFSET			11
#define SPI_CFS_OFFSET			12

/* bit fields in SR, 7 bits */
#define SR_MASK				0x7f		/* cover 7 bits */
#define SR_BUSY				(1 << 0)	
#define SR_TF_NOT_FULL			(1 << 1)
#define SR_TF_EMPT			(1 << 2)	
#define SR_RF_NOT_EMPT			(1 << 3)	
#define SR_RF_FULL			(1 << 4)	
#define SR_TX_ERR			(1 << 5)	
#define SR_DCOL				(1 << 6)	

/* bit fields in ISR, IMR, RISR, 7 bits */
#define SPI_INT_TXEI			(1 << 0)
#define SPI_INT_TXOI			(1 << 1)
#define SPI_INT_RXUI			(1 << 2)
#define SPI_INT_RXOI			(1 << 3)
#define SPI_INT_RXFI			(1 << 4)
#define SPI_INT_MSTI			(1 << 5)

extern void bs_spi_printk(const char *str);

#endif
