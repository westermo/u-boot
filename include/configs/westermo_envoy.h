/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2018 NXP
 */

#ifndef __IMX8MN_EVK_H
#define __IMX8MN_EVK_H

#include <linux/sizes.h>
#include <asm/arch/imx-regs.h>

/* # SPL/Boot Related */

#ifdef CONFIG_SPL_BUILD

#define CONFIG_SPL_MAX_SIZE (208 * SZ_1K)

#define CONFIG_MALLOC_F_ADDR		0x184000
#define CONFIG_SPL_STACK		0x187FF0
#define CONFIG_SPL_BSS_START_ADDR	0x0095e000
#define CONFIG_SPL_BSS_MAX_SIZE		SZ_8K
#define CONFIG_SYS_SPL_MALLOC_START	0x42200000
#define CONFIG_SYS_SPL_MALLOC_SIZE	SZ_64K

/* For RAW image gives a error info not panic */
#define CONFIG_SPL_ABORT_ON_RAW_IMAGE

#define CONFIG_SYS_I2C
#define CONFIG_POWER
#define CONFIG_POWER_I2C
#define CONFIG_POWER_PCA9450

#endif

/* In spite of the name, this denotes an offset on the QSPI
 * FLASH. ...Yeah, I know...  */
#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR	(0x61000 / 512)
#define CONFIG_SYS_UBOOT_BASE	\
	(QSPI0_AMBA_BASE + CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR * 512)


/* # Hardware */
/* ## DRAM */
#define PHYS_SDRAM	0x40000000
#define PHYS_SDRAM_SIZE	SZ_512M

/* ## QSPI FLASH */
#define FSL_FSPI_FLASH_SIZE		SZ_128M
#define FSL_FSPI_FLASH_NUM		1
#define FSPI0_BASE_ADDR			0x30bb0000
#define FSPI0_AMBA_BASE			0x0
#define CONFIG_FSPI_QUAD_SUPPORT
#define CONFIG_SYS_FSL_FSPI_AHB

/* ## Networking */
#define CONFIG_ETHPRIME                 "eth0"
#define FEC_QUIRK_ENET_MAC

/* ## SD Card */
#define CONFIG_FSL_USDHC

/* ## Serial */
#define CONFIG_MXC_UART_BASE		UART1_BASE_ADDR

/* ## I2C */
#define CONFIG_SYS_I2C_SPEED		100000


/* # System Configuration */
#define CONFIG_SYS_SDRAM_BASE	PHYS_SDRAM

#define CONFIG_SYS_MONITOR_LEN	SZ_512K
#define CONFIG_SYS_BOOTM_LEN	SZ_64M
#define CONFIG_SYS_MALLOC_LEN	SZ_32M

#define CONFIG_SYS_INIT_RAM_ADDR	PHYS_SDRAM
#define CONFIG_SYS_INIT_RAM_SIZE        SZ_512K
#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

#define CONFIG_LOADADDR		0x50000000
#define CONFIG_SYS_LOAD_ADDR	CONFIG_LOADADDR

#define CONFIG_ENV_OVERWRITE
#define CONFIG_ENV_SPI_BUS	CONFIG_SF_DEFAULT_BUS
#define CONFIG_ENV_SPI_CS	CONFIG_SF_DEFAULT_CS
#define CONFIG_ENV_SPI_MODE	CONFIG_SF_DEFAULT_MODE
#define CONFIG_ENV_SPI_MAX_HZ	CONFIG_SF_DEFAULT_SPEED

#define CONFIG_SYS_MEMTEST_START    PHYS_SDRAM
#define CONFIG_SYS_MEMTEST_END      (CONFIG_SYS_MEMTEST_START + (PHYS_SDRAM_SIZE >> 1))

#define CONFIG_IMX_BOOTAUX

#define CONFIG_EXTRA_ENV_SETTINGS					\
	"bootcmd_mfg=boot\0"						\
	"bootdelay=0\0"							\
	"itbboot=bootm 0x50000000#0000-lynx-2510\0"			\
	"flashboot=sf probe; sf read 0x50000000 0x200000 0x03800000; run itbboot\0" \
	"netboot=dhcp netbox-os-envoy.itb; run itbboot\0"		\
	"bootargs=console=ttymxc0,115200 root=/dev/ram ramdisk_size=32000 quiet\0"

#define CONFIG_BOOTCOMMAND \
	"run flashboot"


/* Not used, but arch/arm/mach-imx/Makefile unconditionally builds a
 * file that requires this. */
#define CONFIG_SYS_MMC_ENV_DEV	-1

#endif
