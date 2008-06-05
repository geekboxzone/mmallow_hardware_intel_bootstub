/* define bootstub constrains here, like memory map etc. 
 */

#ifndef _BOOT_STUB_HEAD
#define _BOOT_STUB_HEAD

#define CMDLINE_OFFSET		0x100000
#define BZIMAGE_SIZE_OFFSET	0x100100
#define INITRD_SIZE_OFFSET	0x100104
#define STACK_OFFSET		0x101000
#define BZIMAGE_OFFSET		0x102000

#define SETUP_HEADER_OFFSET (BZIMAGE_OFFSET + 0x1F1)
#define SETUP_HEADER_SIZE (0x0202 + *(unsigned char*)(0x0201+BZIMAGE_OFFSET))
#define BOOT_PARAMS_OFFSET 0x8000
#define SETUP_SIGNATURE 0x5a5aaa55

#define GDT_ENTRY_BOOT_CS       2
#define __BOOT_CS               (GDT_ENTRY_BOOT_CS * 8)

#define GDT_ENTRY_BOOT_DS       (GDT_ENTRY_BOOT_CS + 1)
#define __BOOT_DS               (GDT_ENTRY_BOOT_DS * 8)

#define GDT_ENTRY(flags, base, limit)           \
        (((u64)(base & 0xff000000) << 32) |     \
         ((u64)flags << 40) |                   \
         ((u64)(limit & 0x00ff0000) << 32) |    \
         ((u64)(base & 0x00ffffff) << 16) |     \
         ((u64)(limit & 0x0000ffff)))

#endif

/* code for MRST early printk */
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

