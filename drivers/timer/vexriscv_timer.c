// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 roman3017 <rbacik@hotmail.com>
 */

#include <common.h>
#include <asm/io.h>
#include <dm.h>
#include <clk.h>
#include <timer.h>

DECLARE_GLOBAL_DATA_PTR;

struct vexriscv_platdata {
	fdt_addr_t regs;
	u32 clock_rate;
};

static int vexriscv_get_count(struct udevice *dev, u64 *count)
{
	struct vexriscv_platdata *platdata;
	u32 hi, lo;

  platdata = dev_get_platdata(dev);
	do {
		hi = readl((void *)(platdata->regs + 0x4));
		lo = readl((void *)(platdata->regs + 0x0));
	} while (readl((void *)(platdata->regs + 0x4)) != hi);
	*count = (((u64)hi) << 32) | lo;

	return 0;
}

#if CONFIG_IS_ENABLED(OF_CONTROL) && !CONFIG_IS_ENABLED(OF_PLATDATA)
static int vexriscv_ofdata_to_platdata(struct udevice *dev)
{
	struct vexriscv_platdata *platdata;
	struct timer_dev_priv *uc_priv;
	int ret;

  platdata = dev_get_platdata(dev);
	platdata->regs = dev_read_addr(dev);
	if (IS_ERR((void *)platdata->regs))
		return PTR_ERR((void *)platdata->regs);
	uc_priv = dev_get_uclass_priv(dev);
	ret = dev_read_u32(dev, "clock-frequency", (u32 *)&uc_priv->clock_rate);
	if (IS_ERR_VALUE(ret)) {
		debug("Timer clock-frequency not defined\n");
	} else {
		platdata->clock_rate = uc_priv->clock_rate;
	}

	return 0;
}
#endif

static int vexriscv_probe(struct udevice *dev)
{
	struct vexriscv_platdata *platdata;
	struct timer_dev_priv *uc_priv;

	debug("%s:%d\n",__func__,__LINE__);
	platdata = dev_get_platdata(dev);
  writel(0xFFFFFFFF, (void *)(platdata->regs + 0xC));
  writel(0xFFFFFFFF, (void *)(platdata->regs + 0x8));
  writel(0x7FFFFFFF, (void *)(platdata->regs + 0xC));
	uc_priv = dev_get_uclass_priv(dev);
	uc_priv->clock_rate = platdata->clock_rate;

	return 0;
}

static const struct timer_ops vexriscv_ops = {
	.get_count	= vexriscv_get_count,
};

#if CONFIG_IS_ENABLED(OF_CONTROL) && !CONFIG_IS_ENABLED(OF_PLATDATA)
static const struct udevice_id vexriscv_of_match[] = {
	{ .compatible = "vexriscv,timer" },
	{}
};
#endif

U_BOOT_DRIVER(vexriscv_timer) = {
	.name		= "vexriscv_timer",
	.id		= UCLASS_TIMER,
	.ops		= &vexriscv_ops,
	.probe		= vexriscv_probe,
#if CONFIG_IS_ENABLED(OF_CONTROL) && !CONFIG_IS_ENABLED(OF_PLATDATA)
	.of_match	= vexriscv_of_match,
	.ofdata_to_platdata = vexriscv_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct vexriscv_platdata),
#endif
};

static const struct vexriscv_platdata timer_vexriscv_info_non_fdt = {
  .regs = (fdt_addr_t)0x10008000,
	.clock_rate = 50000000,
};
U_BOOT_DEVICE(timer_vexriscv_non_fdt) = {
  .name = "vexriscv_timer",
  .platdata = &timer_vexriscv_info_non_fdt,
};
