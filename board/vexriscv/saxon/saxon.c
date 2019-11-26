// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 roman3017 <rbacik@hotmail.com>
 *
 */
#define DEBUG
#include <common.h>
#include <cpu.h>
#include <dm.h>
#include <spl.h>

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
int dm_scan_fdt_dev(struct udevice *dev)
{
	debug("!!!%s\n",__func__);
	return -1;
}
int fdtdec_setup_memory_banksize(void)
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

static const struct driver_info root_info = {
	.name		= "root_driver",
};
U_BOOT_DEVICE(root_non_fdt) = {
  .name = "root_driver",
  .platdata = &root_info,
};

static const struct cpu_platdata riscv_cpu_info = {
  .cpu_id = 0,
  .timebase_freq = 50000000,
};
U_BOOT_DEVICE(riscv_cpu_non_fdt) = {
  .name = "riscv_cpu",
	.platdata = &riscv_cpu_info,
};

U_BOOT_DEVICE(sifive_spi) = {
	.name	= "sifive_spi",
};

U_BOOT_DEVICE(mmc_spi) = {
	.name = "mmc_spi",
};
#endif /* !CONFIG_SPL_OF_LIBFDT && !CONFIG_SPL_OF_CONTROL */
#endif /* CONFIG_SPL_BUILD */