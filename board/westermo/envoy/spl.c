/*
 * Copyright 2018-2019 NXP
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <cpu_func.h>
#include <hang.h>
#include <spl.h>
#include <asm/io.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx8mn_pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/arch/ddr.h>

#include <power/pmic.h>
#ifdef CONFIG_POWER_PCA9450
#include <power/pca9450.h>
#else
#include <power/bd71837.h>
#endif
#include <asm/mach-imx/gpio.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <fsl_esdhc_imx.h>

DECLARE_GLOBAL_DATA_PTR;

int spl_board_boot_device(enum boot_device boot_dev_spl)
{
#ifdef CONFIG_SPL_BOOTROM_SUPPORT
	return BOOT_DEVICE_BOOTROM;
#else
	switch (boot_dev_spl) {
	case SD1_BOOT:
	case MMC1_BOOT:
	case SD2_BOOT:
	case MMC2_BOOT:
		return BOOT_DEVICE_MMC1;
	case SD3_BOOT:
	case MMC3_BOOT:
		return BOOT_DEVICE_MMC2;
	case QSPI_BOOT:
		return BOOT_DEVICE_NOR;
	case NAND_BOOT:
		return BOOT_DEVICE_NAND;
	case USB_BOOT:
		return BOOT_DEVICE_BOARD;
	default:
		return BOOT_DEVICE_NONE;
	}
#endif
}

void spl_dram_init(void)
{
	ddr_init(&dram_timing);
}

#define I2C_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_HYS | PAD_CTL_PUE | PAD_CTL_PE)
#define PC MUX_PAD_CTRL(I2C_PAD_CTRL)
struct i2c_pads_info i2c_pad_info1 = {
	.scl = {
		.i2c_mode = IMX8MN_PAD_I2C1_SCL__I2C1_SCL | PC,
		.gpio_mode = IMX8MN_PAD_I2C1_SCL__GPIO5_IO14 | PC,
		.gp = IMX_GPIO_NR(5, 14),
	},
	.sda = {
		.i2c_mode = IMX8MN_PAD_I2C1_SDA__I2C1_SDA | PC,
		.gpio_mode = IMX8MN_PAD_I2C1_SDA__GPIO5_IO15 | PC,
		.gp = IMX_GPIO_NR(5, 15),
	},
};


/* #ifdef CONFIG_POWER */
#define I2C_PMIC	0
#ifdef CONFIG_POWER_PCA9450
int power_init_board(void)
{
	struct pmic *p;
	int ret;

	ret = power_pca9450b_init(I2C_PMIC);
	if (ret)
		printf("power init failed");
	p = pmic_get("PCA9450");
	pmic_probe(p);

	/* BUCKxOUT_DVS0/1 control BUCK123 output */
	pmic_reg_write(p, PCA9450_BUCK123_DVS, 0x29);

	/* increase VDD_SOC/VDD_DRAM to typical value 0.95V before first DRAM access */
	/* Set DVS1 to 0.85v for suspend */
	/* Enable DVS control through PMIC_STBY_REQ and set B1_ENMODE=1 (ON by PMIC_ON_REQ=H) */
	pmic_reg_write(p, PCA9450_BUCK1OUT_DVS0, 0x1C);
	pmic_reg_write(p, PCA9450_BUCK1OUT_DVS1, 0x14);
	pmic_reg_write(p, PCA9450_BUCK1CTRL, 0x59);

	/* set VDD_SNVS_0V8 from default 0.85V */
	pmic_reg_write(p, PCA9450_LDO2CTRL, 0xC0);

	/* enable LDO4 to 1.2v */
	pmic_reg_write(p, PCA9450_LDO4CTRL, 0x44);

	/* set WDOG_B_CFG to cold reset */
	pmic_reg_write(p, PCA9450_RESET_CTRL, 0xA1);

	return 0;
}
#else
int power_init_board(void)
{
	struct pmic *p;
	int ret;

	ret = power_bd71837_init(I2C_PMIC);
	if (ret)
		printf("power init failed");

	p = pmic_get("BD71837");
	pmic_probe(p);


	/* decrease RESET key long push time from the default 10s to 10ms */
	pmic_reg_write(p, BD71837_PWRONCONFIG1, 0x0);

	/* unlock the PMIC regs */
	pmic_reg_write(p, BD71837_REGLOCK, 0x1);

	/* Set VDD_ARM to typical value 0.85v for 1.2Ghz */
	pmic_reg_write(p, BD71837_BUCK2_VOLT_RUN, 0xf);

#ifdef CONFIG_IMX8M_DDR4
	/* Set VDD_SOC/VDD_DRAM to typical value 0.85v for nominal mode */
	pmic_reg_write(p, BD71837_BUCK1_VOLT_RUN, 0xf);
#endif

	/* Set VDD_SOC 0.85v for suspend */
	pmic_reg_write(p, BD71837_BUCK1_VOLT_SUSP, 0xf);

#ifdef CONFIG_IMX8M_DDR4
	/* increase NVCC_DRAM_1V2 to 1.2v for DDR4 */
	pmic_reg_write(p, BD71837_BUCK8_VOLT, 0x28);
#endif

	/* lock the PMIC regs */
	pmic_reg_write(p, BD71837_REGLOCK, 0x11);

	return 0;
}
#endif
/* #endif */

void spl_board_init(void)
{
	puts("Normal Boot\n");
}

#ifdef CONFIG_SPL_LOAD_FIT
int board_fit_config_name_match(const char *name)
{
	/* Just empty function now - can't decide what to choose */
	debug("%s: %s\n", __func__, name);

	return 0;
}
#endif

void board_init_f(ulong dummy)
{
	int ret;

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	arch_cpu_init();

	board_early_init_f();

	timer_init();

	preloader_console_init();

	ret = spl_init();
	if (ret) {
		debug("spl_init() failed: %d\n", ret);
		hang();
	}

	enable_tzc380();

	/* Adjust pmic voltage to 1.0V for 800M */
	setup_i2c(0, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info1);

	power_init_board();

	/* DDR initialization */
	spl_dram_init();

	board_init_r(NULL, 0);
}

int do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	puts("resetting ...\n");

	reset_cpu(WDOG1_BASE_ADDR);

	return 0;
}

#ifdef CONFIG_SPL_MMC_SUPPORT

#define UBOOT_RAW_SECTOR_OFFSET 0x40
unsigned long spl_mmc_get_uboot_raw_sector(struct mmc *mmc)
{
	u32 boot_dev = spl_boot_device();
	switch (boot_dev) {
		case BOOT_DEVICE_MMC1:
			return CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR;
		case BOOT_DEVICE_MMC2:
			return CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR - UBOOT_RAW_SECTOR_OFFSET;
	}
	return CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR;
}
#endif
