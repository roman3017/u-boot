// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 roman3017 <rbacik@hotmail.com>
 *
 */

#include <common.h>
#include <spl.h>

int board_init(void)
{
	debug("%s\n",__func__);
	return 0;
}

#ifdef CONFIG_SPL_BUILD
DECLARE_GLOBAL_DATA_PTR;

void spl_board_init(void)
{
	debug("%s\n",__func__);
}
u32 spl_boot_device(void)
{
#ifdef CONFIG_SPL_MMC_SUPPORT
	return BOOT_DEVICE_MMC1;
#endif
	return BOOT_DEVICE_NONE;
}

#ifdef CONFIG_SPL_OS_BOOT
int spl_start_uboot(void)
{
	debug("%s\n",__func__);
	return 0;
}
void spl_perform_fixups(struct spl_image_info *spl_image)
{
	spl_image->os = IH_OS_LINUX;
	spl_image->arg = (void *)CONFIG_SYS_SPL_ARGS_ADDR;
	spl_image->entry_point = CONFIG_SYS_SPL_KERNEL_ADDR;
}
void __noreturn jump_to_image_linux(struct spl_image_info *spl_image)
{
	printf("Entering kernel: 0x%x, arg pointer: 0x%x\n", (u32)spl_image->entry_point, (u32)spl_image->arg);
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
	debug("%s\n",__func__);
	return -1;
}
const void *fdt_getprop(const void *fdt, int nodeoffset, const char *name, int *lenp)
{
	debug("%s\n",__func__);
	return NULL;
}
int fdt_first_subnode(const void *fdt, int offset)
{
	debug("%s\n",__func__);
	return -1;
}
int fdt_next_subnode(const void *fdt, int offset)
{
	debug("%s\n",__func__);
	return -1;
}
int fdt_path_offset(const void *fdt, const char *path)
{
	debug("%s\n",__func__);
	return -1;
}
int fdtdec_setup_mem_size_base(void)
{
	debug("%s\n",__func__);
	gd->ram_base = CONFIG_SYS_SPL_RAM_BASE;
	gd->ram_size = CONFIG_SYS_SPL_RAM_SIZE;
	gd->ram_top = gd->ram_base + gd->ram_size - 1;
	return 0;
}
int fdtdec_setup_memory_banksize(void)
{
	debug("%s\n",__func__);
	gd->bd->bi_memstart = gd->ram_base;
	gd->bd->bi_memsize = gd->ram_size;
	gd->bd->bi_dram[0].start = gd->ram_base;
	gd->bd->bi_dram[0].size = gd->ram_size;
	return 0;
}
int dm_scan_fdt_dev(struct udevice *dev)
{
	debug("%s\n",__func__);
	return 0;
}
#endif /* !CONFIG_SPL_OF_LIBFDT */
#endif /* CONFIG_SPL_BUILD */
