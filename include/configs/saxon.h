/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2019 roman3017 <rbacik@hotmail.com>
 *
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <linux/sizes.h>

/* assuming Text Base is not bottom of memory for INIT_SP to work */
#define CONFIG_SYS_LOAD_ADDR CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_INIT_SP_ADDR		CONFIG_SYS_LOAD_ADDR
#define CONFIG_SYS_MALLOC_LEN		64*SZ_1K
#define CONFIG_ENV_SIZE			SZ_4K
#define	CONFIG_EXTRA_ENV_SETTINGS	"initrd_high=0xffffffff\0" \
                                  "fdt_high=0xffffffff\0"
#ifdef CONFIG_SPL_BUILD
#define CONFIG_SPL_MAX_SIZE SZ_32K
#define CONFIG_SPL_BSS_START_ADDR (IMAGE_TEXT_BASE+SZ_16K+SZ_8K)
#define CONFIG_SPL_BSS_MAX_SIZE SZ_8K
#define CONFIG_SYS_FDT_BASE 0x80ff0000
#define CONFIG_SYS_UBOOT_BASE 0x80200000
#define CONFIG_SYS_LINUX_BASE 0x80400000
#endif /* CONFIG_SPL_BUILD */
#endif /* __CONFIG_H */
