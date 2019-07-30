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

static int vexriscv_ofdata_to_platdata(struct udevice *dev)
{
	struct vexriscv_platdata *platdata;
	
  platdata = dev_get_platdata(dev);
	platdata->regs = dev_read_addr(dev);
	if (IS_ERR((void *)platdata->regs))
		return PTR_ERR((void *)platdata->regs);

	return 0;
}

static int vexriscv_probe(struct udevice *dev)
{
	struct vexriscv_platdata *platdata;
	struct timer_dev_priv *uc_priv;
	int ret;

	platdata = dev_get_platdata(dev);
  writel(0xFFFFFFFF, (void *)(platdata->regs + 0xC));
  writel(0xFFFFFFFF, (void *)(platdata->regs + 0x8));
  writel(0x7FFFFFFF, (void *)(platdata->regs + 0xC));

	uc_priv = dev_get_uclass_priv(dev);
	ret = dev_read_u32(dev, "clock-frequency", (u32 *)&uc_priv->clock_rate);
	if (IS_ERR_VALUE(ret)) {
		debug("Timer clock-frequency not defined\n");
	}

	return 0;
}

static const struct timer_ops vexriscv_ops = {
	.get_count	= vexriscv_get_count,
};

static const struct udevice_id vexriscv_of_match[] = {
	{ .compatible = "vexriscv,timer0" },
	{}
};

U_BOOT_DRIVER(vexriscv_timer) = {
	.name		= "vexriscv_timer",
	.id		= UCLASS_TIMER,
	.ops		= &vexriscv_ops,
	.probe		= vexriscv_probe,
	.of_match	= vexriscv_of_match,
	.ofdata_to_platdata = vexriscv_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct vexriscv_platdata),
};
