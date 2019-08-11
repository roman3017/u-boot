// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 roman3017 <rbacik@hotmail.com>
 *
 */

#include <common.h>
#include <dm.h>
#include <spl.h>

int board_init(void)
{
	return 0;
}

#ifdef CONFIG_SPL_BUILD
u32 spl_boot_device(void)
{
	return BOOT_DEVICE_XIP;
}

int spl_start_uboot(void)
{
	return 0;
}

void __noreturn jump_to_image_linux(struct spl_image_info *spl_image)
{
	spl_image->arg = CONFIG_SYS_FDT_BASE;
	spl_image->entry_point = CONFIG_SYS_LINUX_BASE;
	debug("Entering kernel, arg pointer: 0x%x\n", spl_image->arg);
	void (*kernel)(ulong hart, void *dtb);
	kernel = (void (*)(ulong, void *))spl_image->entry_point;
	kernel(gd->arch.boot_hart, spl_image->arg);
}

void board_init_f(ulong dummy)
{
	debug("%s\n",__func__);
}

/* Do not enable SPL_OF_LIBFDT without SPL_OF_CONTROL */
#if !defined(CONFIG_SPL_OF_LIBFDT) && !defined(CONFIG_SPL_OF_CONTROL)
int fdtdec_setup_memory_banksize(void)
{
	debug("%s\n",__func__);
	return 0;
}
const void *fdt_getprop(const void *fdt, int nodeoffset, const char *name, int *lenp)
{
	debug("%s\n",__func__);
	return 0;
}
ulong fdt_getprop_u32(const void *fdt, int node, const char *prop)
{
	debug("%s\n",__func__);
	return 0;
}
#endif /* !CONFIG_SPL_OF_LIBFDT && !CONFIG_SPL_OF_CONTROL */
#endif /* CONFIG_SPL_BUILD */