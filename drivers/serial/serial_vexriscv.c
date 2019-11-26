// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 roman3017 <rbacik@hotmail.com>
 */
#define DEBUG
#include <common.h>
#include <clk.h>
#include <debug_uart.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <serial.h>

DECLARE_GLOBAL_DATA_PTR;

#define STATUS_TX 16
#define STATUS_RX 24

#define UNUSED(x) (void)(x)

struct vexriscv_uart_regs {
	u32 data;
	u32 status;
	u32 div;
	u32 frame;
};

static void set_div(struct vexriscv_uart_regs *regs, int freq, int baudrate)
{
  int div;

	div = (freq + baudrate - 1) / baudrate;
  writel(div, &regs->div);
}

static void set_frame(struct vexriscv_uart_regs *regs, int par, int bits, int stop)
{
  writel((bits << 0) | (par << 8) | (stop << 16), &regs->frame);
}

#ifdef CONFIG_DEBUG_UART_VEXRISCV
static inline void _debug_uart_init(void)
{
	struct vexriscv_uart_regs *regs;

  regs = (struct vexriscv_uart_regs *)CONFIG_DEBUG_UART_BASE;
	set_div(regs, CONFIG_DEBUG_UART_CLOCK, CONFIG_BAUDRATE);
	set_frame(regs, 0, 7, 0);
}

static inline void _debug_uart_putc(int ch)
{
	struct vexriscv_uart_regs *regs;

  regs = (struct vexriscv_uart_regs *)CONFIG_DEBUG_UART_BASE;
	while (0 == ((readl(&regs->status)>>STATUS_TX) & 0xff))
		;
	writel(ch, &regs->data);
}

DEBUG_UART_FUNCS

#endif /* CONFIG_DEBUG_UART_VEXRISCV */

struct vexriscv_uart_platdata {
	struct vexriscv_uart_regs *regs;
	u32 clock;
  u32 baudrate;
};

static int vexriscv_setbrg(struct udevice *dev, int baudrate)
{
	struct vexriscv_uart_platdata *platdata;
  int freq;

  platdata = dev_get_platdata(dev);
	freq = platdata->clock;
  set_div(platdata->regs, freq, baudrate);
  platdata->baudrate = baudrate;
	return 0;
}

static int vexriscv_getc(struct udevice *dev)
{
	struct vexriscv_uart_platdata *platdata;

  platdata = dev_get_platdata(dev);
	if (0 == ((readl(&platdata->regs->status)>>STATUS_RX) & 0xff))
		return -EAGAIN;
	return readl(&platdata->regs->data);
}

static int vexriscv_putc(struct udevice *dev, const char ch)
{
	struct vexriscv_uart_platdata *platdata;

  platdata = dev_get_platdata(dev);
	if (0 == ((readl(&platdata->regs->status)>>STATUS_TX) & 0xff))
		return -EAGAIN;
	writel(ch, &platdata->regs->data);
  return 0;
}

static int vexriscv_pending(struct udevice *dev, bool input)
{
	struct vexriscv_uart_platdata *platdata;

  platdata = dev_get_platdata(dev);
  return ((readl(&platdata->regs->status)>>STATUS_RX) & 0xff);
}

static int vexriscv_clear(struct udevice *dev)
{
  UNUSED(dev);
  return -EINVAL;
}

static int vexriscv_getconfig(struct udevice *dev, uint *serial_config)
{
	struct vexriscv_uart_platdata *platdata;
  int frame;

  platdata = dev_get_platdata(dev);
  frame = readl(&platdata->regs->frame);
  *serial_config = SERIAL_CONFIG((0xff & (frame >> 8)), ((0xff & (frame >> 0))-4), (0xff && (frame >> 16)));
  return 0;
}

static int vexriscv_setconfig(struct udevice *dev, uint config)
{
	struct vexriscv_uart_platdata *platdata;

  platdata = dev_get_platdata(dev);
  set_frame(platdata->regs, SERIAL_GET_PARITY(config), SERIAL_GET_BITS(config)+4, SERIAL_GET_STOP(config));
  return 0;
}

static int vexriscv_getinfo(struct udevice *dev, struct serial_device_info *info)
{
	struct vexriscv_uart_platdata *platdata;

  platdata = dev_get_platdata(dev);
  info->type = SERIAL_CHIP_UNKNOWN;
  info->addr_space = SERIAL_ADDRESS_SPACE_IO;
  info->baudrate = platdata->baudrate;
  info->addr = (ulong)platdata->regs;
  info->reg_offset = 0;
  info->reg_shift = 0;
  return 0;
}

#if CONFIG_IS_ENABLED(OF_CONTROL) && !CONFIG_IS_ENABLED(OF_PLATDATA)
static int vexriscv_ofdata_to_platdata(struct udevice *dev)
{
	struct vexriscv_uart_platdata *platdata;

  platdata = dev_get_platdata(dev);
	platdata->regs = (void *)dev_read_addr(dev);
	if (IS_ERR(platdata->regs))
		return PTR_ERR(platdata->regs);
	if (IS_ERR_VALUE(dev_read_u32(dev, "clock-frequency", &platdata->clock))) {
		debug("Timer clock-frequency not defined\n");
	}
	return 0;
}
#endif /* OF_CONTROL && !OF_PLATDATA */

static int vexriscv_probe(struct udevice *dev)
{
  UNUSED(dev);
	debug("!!!%s\n",__func__);
	return 0;
}

static const struct dm_serial_ops vexriscv_ops = {
	.setbrg = vexriscv_setbrg,
	.getc = vexriscv_getc,
	.putc = vexriscv_putc,
	.pending = vexriscv_pending,
  .clear = vexriscv_clear,
  .getconfig = vexriscv_getconfig,
  .setconfig = vexriscv_setconfig,
  .getinfo = vexriscv_getinfo,
};

#if CONFIG_IS_ENABLED(OF_CONTROL) && !CONFIG_IS_ENABLED(OF_PLATDATA)
static const struct udevice_id vexriscv_of_match[] = {
	{ .compatible = "vexriscv,uart" },
	{ }
};
#endif /* OF_CONTROL && !OF_PLATDATA */

U_BOOT_DRIVER(vexriscv_serial) = {
	.name	= "vexriscv_serial",
	.id	= UCLASS_SERIAL,
#if CONFIG_IS_ENABLED(OF_CONTROL) && !CONFIG_IS_ENABLED(OF_PLATDATA)
	.of_match = vexriscv_of_match,
	.ofdata_to_platdata = vexriscv_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct vexriscv_uart_platdata),
#endif
	.probe = vexriscv_probe,
	.ops	= &vexriscv_ops,
#if !CONFIG_IS_ENABLED(OF_CONTROL)
	.flags	= DM_FLAG_PRE_RELOC,
#endif
};

static const struct vexriscv_uart_platdata vexriscv_serial_info_non_fdt = {
  .regs = (void *)CONFIG_DEBUG_UART_BASE,
  .baudrate = CONFIG_BAUDRATE,
	.clock = CONFIG_DEBUG_UART_CLOCK,
};
U_BOOT_DEVICE(vexriscv_serial_non_fdt) = {
  .name = "vexriscv_serial",
  .platdata = &vexriscv_serial_info_non_fdt,
};
