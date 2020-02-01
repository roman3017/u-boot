// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 dolu1990 <charles.papon.90@gmail.com>
 */

#include <common.h>
#include <clk.h>
#include <debug_uart.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <serial.h>
#include <asm/sbi.h>

#define UNUSED(x) (void)(x)

#ifdef CONFIG_DEBUG_UART_RISCV_SBI
static inline void _debug_uart_init(void)
{

}

static inline void _debug_uart_putc(int ch)
{
  UNUSED(ch);

  sbi_console_putchar(ch);
}

DEBUG_UART_FUNCS

#endif /* CONFIG_DEBUG_UART_riscv_sbi */

struct riscv_sbi_uart_platdata {
	int getc_buffer;
};

static int riscv_sbi_setbrg(struct udevice *dev, int baudrate)
{
	UNUSED(dev);
	UNUSED(baudrate);
	return 0;
}

static int riscv_sbi_getc(struct udevice *dev)
{
	struct riscv_sbi_uart_platdata *platdata;

  platdata = dev_get_platdata(dev);
  if (platdata->getc_buffer == -1) platdata->getc_buffer = sbi_console_getchar();
  if (platdata->getc_buffer == -1)
    return -EAGAIN;
  int ch = platdata->getc_buffer;
  platdata->getc_buffer = -1;
  return ch;
}

static int riscv_sbi_putc(struct udevice *dev, const char ch)
{
  UNUSED(dev);
  sbi_console_putchar(ch);
  return 0;
}

static int riscv_sbi_pending(struct udevice *dev, bool input)
{
  struct riscv_sbi_uart_platdata *platdata;
  platdata = dev_get_platdata(dev);

  if(platdata->getc_buffer == -1) platdata->getc_buffer = sbi_console_getchar();
  return platdata->getc_buffer != -1 ? 1 : 0;
}

static int riscv_sbi_clear(struct udevice *dev)
{
  UNUSED(dev);
  return -EINVAL;
}

static int riscv_sbi_getconfig(struct udevice *dev, uint *serial_config)
{
  *serial_config = SERIAL_DEFAULT_CONFIG;
  return 0;
}

static int riscv_sbi_setconfig(struct udevice *dev, uint config)
{
  UNUSED(dev);
  UNUSED(config);
  return 0;
}

static int riscv_sbi_getinfo(struct udevice *dev, struct serial_device_info *info)
{
  UNUSED(dev);
  info->type = SERIAL_CHIP_UNKNOWN;
  info->addr_space = SERIAL_ADDRESS_SPACE_IO;
  info->baudrate = 0;
  info->addr = 0;
  info->reg_offset = 0;
  info->reg_shift = 0;
  return 0;
}

#if CONFIG_IS_ENABLED(OF_CONTROL) && !CONFIG_IS_ENABLED(OF_PLATDATA)
static int riscv_sbi_ofdata_to_platdata(struct udevice *dev)
{
  UNUSED(dev);
  return 0;
}
#endif /* OF_CONTROL && !OF_PLATDATA */

static int riscv_sbi_probe(struct udevice *dev)
{
  struct riscv_sbi_uart_platdata *platdata;

  platdata = dev_get_platdata(dev);
  platdata->getc_buffer = -1;

  debug("!!!%s:%d\n",__func__,__LINE__);
  return 0;
}

static int riscv_sbi_bind(struct udevice *dev)
{
  UNUSED(dev);
	debug("!!!%s:%d\n",__func__,__LINE__);
	return 0;
}
static const struct dm_serial_ops riscv_sbi_ops = {
	.setbrg = riscv_sbi_setbrg,
	.getc = riscv_sbi_getc,
	.putc = riscv_sbi_putc,
	.pending = riscv_sbi_pending,
  .clear = riscv_sbi_clear,
  .getconfig = riscv_sbi_getconfig,
  .setconfig = riscv_sbi_setconfig,
  .getinfo = riscv_sbi_getinfo,
};

#if CONFIG_IS_ENABLED(OF_CONTROL) && !CONFIG_IS_ENABLED(OF_PLATDATA)
static const struct udevice_id riscv_sbi_of_match[] = {
	{ .compatible = "riscv_sbi,uart" },
	{ }
};
#endif /* OF_CONTROL && !OF_PLATDATA */

U_BOOT_DRIVER(riscv_sbi_serial) = {
	.name	= "riscv_sbi_serial",
	.id	= UCLASS_SERIAL,
#if CONFIG_IS_ENABLED(OF_CONTROL) && !CONFIG_IS_ENABLED(OF_PLATDATA)
	.of_match = riscv_sbi_of_match,
	.ofdata_to_platdata = riscv_sbi_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct riscv_sbi_uart_platdata),
#endif
	.bind = riscv_sbi_bind,
	.probe = riscv_sbi_probe,
	.ops	= &riscv_sbi_ops,
#if !CONFIG_IS_ENABLED(OF_CONTROL)
	.flags	= DM_FLAG_PRE_RELOC,
#endif
};

#if !CONFIG_IS_ENABLED(OF_CONTROL) || CONFIG_IS_ENABLED(OF_PLATDATA)
static const struct riscv_sbi_uart_platdata riscv_sbi_serial_info_non_fdt = {
  .getc_buffer = -1,
};
U_BOOT_DEVICE(riscv_sbi_serial_non_fdt) = {
  .name = "riscv_sbi_serial",
  .platdata = &riscv_sbi_serial_info_non_fdt,
};
#endif /*!CONFIG_IS_ENABLED(OF_CONTROL) || CONFIG_IS_ENABLED(OF_PLATDATA)*/
