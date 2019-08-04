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
#define CONFIG_SYS_MALLOC_LEN		SZ_8K
#define CONFIG_ENV_SIZE			SZ_4K
#define	CONFIG_EXTRA_ENV_SETTINGS	"initrd_high=0xffffffff\0" \
                                  "fdt_high=0xffffffff\0"

#endif /* __CONFIG_H */
