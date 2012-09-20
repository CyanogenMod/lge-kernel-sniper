/*
 * ifx_n721_spi.h -- Serial peheripheral interface framing layer for IFX modem.
 *
 * Copyright (C) 2009 Texas Instruments
 * Authors:	Umesh Bysani <bysani@ti.com> and
 *		Shreekanth D.H <sdh@ti.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef IFX_N721_SPI_H
#define IFX_N721_SPI_H

// SUPPORT EXTENDED FORMAT OF HEADER FOR SPI FRAMEs
#define MSPI_EXTENDED_HEADER

#define IFX_SPI_MAJOR			153	/* assigned */
#define IFX_N_SPI_MINORS		4	/* ... up to 256 */



// hgahn
//#define MRDY_CFG_REG     		AA9_3430_GPIO_149	/* Used from Schematic file */
//#define SRDY_CFG_REG     		AF11_3430_GPIO_14	/* Used from Schematic file */
#define MRDY_CFG_REG     		AF9_3430_GPIO_22	/* Used from Schematic file */
#define SRDY_CFG_REG     		AF11_3430_GPIO_21	/* Used from Schematic file */



// hgahn
//#define IFX_MRDY_GPIO			149	/* MRDY GPIO pin for IFX - According to Windows Mobile */
//#define IFX_SRDY_GPIO			14	/* SRDY GPIO pin for IFX - According to Windows Mobile */
#define IFX_MRDY_GPIO			22	/* MRDY GPIO pin for IFX - According to Windows Mobile */
#define IFX_SRDY_GPIO			21	/* SRDY GPIO pin for IFX - According to Windows Mobile */



#define MODEM_GPIO_AUDIO		95
#define MODEM_GPIO_RESET		103
#define MODEM_GPIO_PWRON		110


#ifdef MSPI_EXTENDED_HEADER
// NEW EXTENDED HEADER FORMAT FOR DYNAMIC SPI BUFFER FEAT
#define SPI_MAX_PACKET_LEN       1536
//#define SPI_MAX_PACKET_LEN       96
struct ifx_spi_frame_header {
		unsigned int unused_curr_data_size:12;
		unsigned int more:1;
		unsigned int res1:1;
		unsigned int res2:2;      
		unsigned int unused_next_data_size:12;
		unsigned int ri:1;
		unsigned int dcd:1;
		unsigned int cts_rts:1;
		unsigned int dsr_dtr:1;
		unsigned int curr_data_size:16;
		unsigned int next_data_size:16;
};
#define MAX_DYN_IFX_SPI_MAX_MUX_FRAMES 		5

#else
#define IFX_SPI_MAX_BUF_SIZE		2044	/* Max buffer size */
#define IFX_SPI_DEFAULT_BUF_SIZE	2044 	/* Default buffer size*/
#endif
//--]] LGE_UBIQUIX_MODIFIED_END : shyun@ubiquix.com [2011.07.20]- Config definition is modified.
#define IFX_SPI_HEADER_SIZE		    sizeof(struct ifx_spi_frame_header)
#define IFX_SPI_MAX_BUF_SIZE		(SPI_MAX_PACKET_LEN - IFX_SPI_HEADER_SIZE)	/* Max buffer size */
#define IFX_SPI_DEFAULT_BUF_SIZE	(SPI_MAX_PACKET_LEN - IFX_SPI_HEADER_SIZE) 	/* Default buffer size*/
//#define MAX_DYN_IFX_SPI_BUFFER_SIZE (IFX_SPI_MAX_BUF_SIZE * MAX_DYN_IFX_SPI_MAX_MUX_FRAMES)
#define DYN_IFX_SPI_BUFFER_SIZE     7680
#define MAX_DYN_IFX_SPI_BUFFER_SIZE (DYN_IFX_SPI_BUFFER_SIZE-IFX_SPI_HEADER_SIZE)
#define MSPI_SPI_FULL_FRAME_LENGTH  (MAX_DYN_IFX_SPI_BUFFER_SIZE + IFX_SPI_HEADER_SIZE)



#define SPI_MODE_0			(0|0)
#define SPI_MODE_1			(0|SPI_CPHA)
#define SPI_MODE_2			(SPI_CPOL|0)
#define SPI_MODE_3			(SPI_CPOL|SPI_CPHA)

#define SPI_CPHA			0x01
#define SPI_CPOL			0x02
#define SPI_CS_HIGH			0x04
#define SPI_LSB_FIRST			0x08
#define SPI_3WIRE			0x10
#define SPI_LOOP			0x20

/* Bit masks for spi_device.mode management.  Note that incorrect
 * settings for CS_HIGH and 3WIRE can cause *lots* of trouble for other
 * devices on a shared bus:  CS_HIGH, because this device will be
 * active when it shouldn't be;  3WIRE, because when active it won't
 * behave as it should.
 *
 * REVISIT should changing those two modes be privileged?
 */
#define SPI_MODE_MASK			(SPI_CPHA | SPI_CPOL | SPI_CS_HIGH \
					| SPI_LSB_FIRST | SPI_3WIRE | SPI_LOOP)

struct ifx_spi_platform_data {
	unsigned mrdy_gpio;
	unsigned srdy_gpio;
};

#endif /* IFX_N721_SPI_H */
