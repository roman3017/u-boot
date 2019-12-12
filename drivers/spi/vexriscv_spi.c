// SPDX-License-Identifier: GPL-2.0+
/*
 * 
 * Based on Linux spi_spinal_lib_driver by charles.papon.90@gmail.com
 * Copyright (C) 2019 roman3017 <rbacik@hotmail.com>
 *
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <malloc.h>
#include <fdtdec.h>
#include <spi.h>
#include <asm/io.h>



#define SPI_CMD_WRITE (1 << 8)
#define SPI_CMD_READ (1 << 9)
#define SPI_CMD_SS (1 << 11)

#define SPI_RSP_VALID (1 << 31)

#define SPI_STATUS_CMD_INT_ENABLE = (1 << 0)
#define SPI_STATUS_RSP_INT_ENABLE = (1 << 1)
#define SPI_STATUS_CMD_INT_FLAG = (1 << 8)
#define SPI_STATUS_RSP_INT_FLAG = (1 << 9)

#define SPI_MODE_CPOL (1 << 0)
#define SPI_MODE_CPHA (1 << 1)

struct vexriscv_spi_regs {
	u32 data;
	u32 buffer;
	u32 config;
	u32 interrupt;
	u32 reserved[4];
	u32 clk_div;
	u32 ss_setup;
	u32 ss_hold;
	u32 ss_disable;
	u32 ss_active_high;
};

struct vexriscv_spi_privdata {
	struct vexriscv_spi_regs *regs;
	u32 clock;
	u32 frequency;
	u32 num_cs;
	u32 rsp_fifo_depth;
	u32 cmd_fifo_depth;
	u32 ss_active_high;
	u32 mode;
};

static u32 spi_spinal_lib_cmd_availability(struct vexriscv_spi_privdata *hw){
	return readl(&hw->regs->buffer) & 0xFFFF;
}

static u32 spi_spinal_lib_rsp_occupancy(struct vexriscv_spi_privdata *hw)
{
	return readl(&hw->regs->buffer) >> 16;
}

static void spi_spinal_lib_cmd(struct vexriscv_spi_privdata *hw, u32 cmd)
{
	writel(cmd, &hw->regs->data);
}

static void spi_spinal_lib_cmd_wait(struct vexriscv_spi_privdata *hw)
{
	while(spi_spinal_lib_cmd_availability(hw) == 0)
		udelay(1);
}

static u32 spi_spinal_lib_rsp(struct vexriscv_spi_privdata *hw)
{
	return readl(&hw->regs->data);
}

//static void spi_spinal_lib_rsp_wait(struct vexriscv_spi_privdata *hw)
//{
//	while(spi_spinal_lib_rsp_occupancy(hw) == 0)
//	udelay(1);
//}

static u32 spi_spinal_lib_rsp_pull(struct vexriscv_spi_privdata *hw)
{
	u32 rsp;
	while(((s32)(rsp = spi_spinal_lib_rsp(hw))) < 0);
	return rsp;
}

static void spi_spinal_lib_set_cs(struct vexriscv_spi_privdata *hw, u32 cs, bool high)
{
	spi_spinal_lib_cmd(hw, cs | SPI_CMD_SS | 
		((high != 0) ^ ((hw->mode & SPI_CS_HIGH) != 0) ? 0x00 : 0x80));
	spi_spinal_lib_cmd_wait(hw);
}

static int vexriscv_spi_ofdata_to_platdata(struct udevice *dev)
{
	debug("!!!%s:%d\n",__func__,__LINE__);
	struct vexriscv_spi_privdata *priv = dev_get_priv(dev);

	priv->regs = (void *)dev_read_addr(dev);
	if (!priv->regs) {
		printf("%s: could not map device address\n", __func__);
		return -EINVAL;
	}
	priv->num_cs = dev_read_u32_default(dev, "num-cs", 1);
	priv->clock = dev_read_u32_default(dev, "clock-frequency",
		50000000);
	priv->frequency = dev_read_u32_default(dev, "spi-max-frequency",
		100000);
	priv->rsp_fifo_depth = dev_read_u32_default(dev, "rsp_fifo_depth", 256);
	priv->cmd_fifo_depth = dev_read_u32_default(dev, "cmd_fifo_depth", 256);

	return 0;
}

static int vexriscv_spi_probe(struct udevice *dev)
{
	debug("!!!%s:%d\n",__func__,__LINE__);
	struct vexriscv_spi_privdata *priv = dev_get_priv(dev);

	priv->ss_active_high = 0;

	/* program defaults into the registers */
	writel(0, &priv->regs->config);
	writel(3, &priv->regs->interrupt);
	writel(3, &priv->regs->clk_div);
	writel(3, &priv->regs->ss_disable);
	writel(3, &priv->regs->ss_setup);
	writel(3, &priv->regs->ss_hold);
	return 0;
}

static int vexriscv_spi_remove(struct udevice *dev)
{
	debug("!!!%s:%d\n",__func__,__LINE__);
	return 0;
}

static int vexriscv_spi_xfer(struct udevice *dev, unsigned int bitlen,
			    const void *dout, void *din, unsigned long flags)
{
	debug("!!!%s:%d\n",__func__,__LINE__);
	struct udevice *bus = dev->parent;
	struct vexriscv_spi_privdata *spi = dev_get_priv(bus);
	struct dm_spi_slave_platdata *slave = dev_get_parent_platdata(dev);
	const u8 *tx_ptr = dout;
	u8 *rx_ptr = din;
	u32 len = (bitlen + 7) / 8;
	u32 count = 0;
	u32 tx_count = 0;

	if (flags & SPI_XFER_BEGIN)
		spi_spinal_lib_set_cs(spi, slave->cs, spi->ss_active_high & BIT(slave->cs) ? 1 : 0);

	if(spi->cmd_fifo_depth > 1 && spi->rsp_fifo_depth > 1) {
		u32 cmd = (tx_ptr ? SPI_CMD_WRITE : 0) | SPI_CMD_READ;
		u32 token = min(spi->cmd_fifo_depth, spi->rsp_fifo_depth);
		while (count < len) {
			{	//rsp
				u32 burst;
				u8 *ptr, *end;

				burst = spi_spinal_lib_rsp_occupancy(spi);
				ptr = rx_ptr + count;
				end = ptr + burst;
				if(rx_ptr) {while(ptr != end) {*ptr++ = spi_spinal_lib_rsp(spi);}}
				else {while(ptr != end) {ptr++; volatile x = spi_spinal_lib_rsp(spi);}}
				count += burst;
				token += burst;
			}

			{	//cmd
				u32 burst;
				const u8 *ptr, *end;
				burst = min(len - tx_count, token);
				ptr = tx_ptr + tx_count;
				end = ptr + burst;
				if(tx_ptr) {while(ptr != end) {writel(cmd | *ptr++, &spi->regs->data);}}
				else {while(ptr != end) {ptr++; writel(cmd, &spi->regs->data);}}
				tx_count += burst;
				token -= burst;
			}
		}
	} else {
		u32 cmd = (tx_ptr ? SPI_CMD_WRITE : 0) | SPI_CMD_READ;
		while (count < len) {
			u32 data = tx_ptr ? tx_ptr[count] : 0;
			writel(cmd | data, &spi->regs->data);
			data = spi_spinal_lib_rsp_pull(spi);
			if (rx_ptr) rx_ptr[count] = data;
			count++;
		}
	}

	if (flags & SPI_XFER_END)
		spi_spinal_lib_set_cs(spi, slave->cs, spi->ss_active_high & BIT(slave->cs) ? 0 : 1);

	return 0;
}

static int vexriscv_spi_set_speed(struct udevice *dev, uint speed)
{
	debug("!!!%s:%d\n",__func__,__LINE__);
	struct vexriscv_spi_privdata *priv = dev_get_priv(dev);
	u32 clk_divider = (priv->clock/speed/2)-1;
	writel(clk_divider, &priv->regs->clk_div);
	priv->frequency = speed;
	return 0;
}

static int vexriscv_spi_set_mode(struct udevice *dev, uint mode)
{
	debug("!!!%s:%d\n",__func__,__LINE__);
	struct vexriscv_spi_privdata *priv = dev_get_priv(dev);
	struct dm_spi_slave_platdata *slave = dev_get_platdata(dev);
	u32 config = 0;

	priv->mode = mode;
	if (mode & SPI_CS_HIGH)
		priv->ss_active_high |= BIT(slave->cs);
	else
		priv->ss_active_high &= ~BIT(slave->cs);

	writel(priv->ss_active_high, &priv->regs->ss_active_high);

	if (mode & SPI_CPOL)
		config |= SPI_MODE_CPOL;
	if (mode & SPI_CPHA)
		config |= SPI_MODE_CPHA;

	writel(config, &priv->regs->config);

	while(spi_spinal_lib_rsp_occupancy(priv))
		spi_spinal_lib_rsp(priv); //Flush rsp

	return 0;
}

static int vexriscv_cs_info(struct udevice *dev, uint cs,
			  struct spi_cs_info *info)
{
	debug("!!!%s:%d\n",__func__,__LINE__);
	struct vexriscv_spi_privdata *priv = dev_get_priv(dev);

	if (cs < priv->num_cs)
		return 0;

	return -EINVAL;
}

static const struct dm_spi_ops vexriscv_spi_ops = {
	.xfer	= vexriscv_spi_xfer,
	.set_speed	= vexriscv_spi_set_speed,
	.set_mode	= vexriscv_spi_set_mode,
	.cs_info	= vexriscv_cs_info,
};

static const struct udevice_id vexriscv_spi_ids[] = {
	{ .compatible = "vexriscv,spi" },
	{ }
};

U_BOOT_DRIVER(vexriscv_spi) = {
	.name	= "vexriscv_spi",
	.id	= UCLASS_SPI,
	.of_match = vexriscv_spi_ids,
	.ops	= &vexriscv_spi_ops,
	.ofdata_to_platdata = vexriscv_spi_ofdata_to_platdata,
	.platdata_auto_alloc_size	= sizeof(struct dm_spi_slave_platdata),
	.priv_auto_alloc_size = sizeof(struct vexriscv_spi_privdata),
	.probe	= vexriscv_spi_probe,
	.remove	= vexriscv_spi_remove,
};
