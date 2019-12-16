// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 roman3017 <rbacik@hotmail.com>
 *
 */

#include <common.h>
#include <cpu.h>
#include <dm.h>
#include <mmc.h>
#include <spi.h>
#include <spl.h>

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	return 0;
}

#ifdef CONFIG_SPL_BUILD
u32 spl_boot_device(void)
{
#ifdef CONFIG_SPL_RAM_SUPPORT
	return BOOT_DEVICE_RAM;
#endif
#ifdef CONFIG_SPL_XIP_SUPPORT
	return BOOT_DEVICE_XIP;
#endif
#ifdef CONFIG_SPL_MMC_SUPPORT
	return BOOT_DEVICE_MMC1;
#endif
	return BOOT_DEVICE_NONE;
}

int spl_start_uboot(void)
{
#ifdef CONFIG_SPL_OS_BOOT
	return 0;
#endif
	return 1;
}

#ifdef CONFIG_SPL_RAM_SUPPORT
struct image_header *spl_get_load_buffer(ssize_t offset, size_t size)
{
	return (struct image_header *)(CONFIG_SYS_LINUX_BASE + offset);
}
#endif

#ifdef CONFIG_SPL_OS_BOOT
void __noreturn jump_to_image_linux(struct spl_image_info *spl_image)
{
	spl_image->arg = (void *)CONFIG_SYS_FDT_BASE;
	spl_image->entry_point = CONFIG_SYS_LINUX_BASE;
	debug("Entering kernel, arg pointer: 0x%x\n", (u32)spl_image->arg);
	typedef void (*image_entry_arg_t)(ulong, void *)
		__attribute__ ((noreturn));
	image_entry_arg_t kernel =
		(image_entry_arg_t)spl_image->entry_point;
	kernel(gd->arch.boot_hart, spl_image->arg);
}
#endif /* CONFIG_SPL_OS_BOOT */

/* DO NOT enable SPL_OF_LIBFDT without SPL_OF_CONTROL */
#if !defined(CONFIG_SPL_OF_LIBFDT)
s32 fdtdec_get_int(const void *blob, int node, const char *prop_name, s32 default_val)
{
	debug("!!!%s\n",__func__);
	return -1;
}
const void *fdt_getprop(const void *fdt, int nodeoffset, const char *name, int *lenp)
{
	debug("!!!%s\n",__func__);
	return NULL;
}
int fdt_first_subnode(const void *fdt, int offset)
{
	debug("!!!%s\n",__func__);
	return -1;
}
int fdt_next_subnode(const void *fdt, int offset)
{
	debug("!!!%s\n",__func__);
	return -1;
}
int fdt_path_offset(const void *fdt, const char *path)
{
	debug("!!!%s\n",__func__);
	return -1;
}
#endif /* !CONFIG_SPL_OF_LIBFDT */

#endif /* CONFIG_SPL_BUILD */

#if !CONFIG_IS_ENABLED(OF_CONTROL) || CONFIG_IS_ENABLED(OF_PLATDATA)

int fdtdec_setup_mem_size_base(void)
{
	debug("!!!%s\n",__func__);
	gd->ram_base = 0x80000000;
	gd->ram_size = 0x02000000;
	gd->ram_top = gd->ram_base + gd->ram_size - 1;
	return 0;
}
int fdtdec_setup_memory_banksize(void)
{
	debug("!!!%s\n",__func__);
	return 0;
}

int fdtdec_get_alias_seq(const void *blob, const char *base, int offset, int *seqp)
{
	debug("!!!%s\n",__func__);
	return 0;
}
int dm_scan_fdt_dev(struct udevice *dev)
{
	debug("!!!%s\n",__func__);
	return -1;
}

static const struct cpu_platdata riscv_cpu_info = {
  .cpu_id = 0,
  .timebase_freq = 50000000,
};
U_BOOT_DEVICE(riscv_cpu_non_fdt) = {
  .name = "riscv_cpu",
	.platdata = &riscv_cpu_info,
};

/* TODO: move this struct to header file */
struct mmc_spi_plat {
	struct mmc_config cfg;
	struct mmc mmc;
};

static const struct mmc_spi_plat mmc_spi_info_non_fdt = {
	.cfg = {
		.f_min = 0,
		.f_max = 25000000,
	},
	.mmc = {

	},
};
U_BOOT_DEVICE(mmc_spi_non_fdt) = {
	.name = "mmc_spi",
	.platdata = &mmc_spi_info_non_fdt,
};

static const struct dm_spi_slave_platdata vexriscv_spi_info_non_fdt = {
};

U_BOOT_DEVICE(vexriscv_spi_non_fdt) = {
  .name = "vexriscv_spi",
  .platdata = &vexriscv_spi_info_non_fdt,
};
/*
vexriscv_spi_non_fdt (UCLASS_SPI);
	mmc_spi_non_fdt (UCLASS_MMC);
*/
#endif /*!CONFIG_IS_ENABLED(OF_CONTROL) || CONFIG_IS_ENABLED(OF_PLATDATA)*/
