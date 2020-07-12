/*
 * spinal_lib_mac.c --  the Simple Network Utility
 *
 * Copyright (C) 2001 Alessandro Rubini and Jonathan Corbet
 * Copyright (C) 2001 O'Reilly & Associates
 *
 * The source code in this file can be freely used, adapted,
 * and redistributed in source or binary form, so long as an
 * acknowledgment appears in derived source files.  The citation
 * should list that the code comes from the book "Linux Device
 * Drivers" by Alessandro Rubini and Jonathan Corbet, published
 * by O'Reilly & Associates.   No warranty is attached;
 * we cannot take responsibility for errors or fitness for use.
 *
 * $Id: spinal_lib_mac.c,v 1.21 2004/11/05 02:36:03 rubini Exp $
 */
//#define DEBUG
#include <common.h>
#include <miiphy.h>
#include <asm/io.h>
#include <linux/delay.h>

#define SPINAL_LIB_MAC_FRAME_SIZE_MAX 2000

#define SPINAL_LIB_MAC_CTRL 0x00
#define SPINAL_LIB_MAC_TX   0x10
#define SPINAL_LIB_MAC_TX_AVAILABILITY   0x14
#define SPINAL_LIB_MAC_RX   0x20
#define SPINAL_LIB_MAC_RX_STATS   0x2C

#define SPINAL_LIB_MAC_CTRL_TX_RESET 0x00000001
#define SPINAL_LIB_MAC_CTRL_TX_READY 0x00000002
#define SPINAL_LIB_MAC_CTRL_TX_ALIGN 0x00000004

#define SPINAL_LIB_MAC_CTRL_RX_RESET   0x00000010
#define SPINAL_LIB_MAC_CTRL_RX_PENDING 0x00000020
#define SPINAL_LIB_MAC_CTRL_RX_ALIGN 0x00000040

#define BUFSIZE (SPINAL_LIB_MAC_FRAME_SIZE_MAX >> 2)

struct spinal_lib_mac_priv {
    u16 reserved;
    u32 buf[BUFSIZE];
};

static u32 spinal_lib_mac_tx_availability(void __iomem *base){
    return readl(base + SPINAL_LIB_MAC_TX_AVAILABILITY);
}

static u32 spinal_lib_mac_tx_ready(void __iomem *base){
    return readl(base + SPINAL_LIB_MAC_CTRL) & SPINAL_LIB_MAC_CTRL_TX_READY;
}

static void spinal_lib_mac_tx_u32(void __iomem *base, u32 data){
    writel(data, base + SPINAL_LIB_MAC_TX);
}

#if 0
static u32 spinal_lib_mac_rx_stats(void __iomem *base){
    return readl(base + SPINAL_LIB_MAC_RX_STATS);
}
#endif

static u32 spinal_lib_mac_rx_pending(void __iomem *base){
    return readl(base + SPINAL_LIB_MAC_CTRL) & SPINAL_LIB_MAC_CTRL_RX_PENDING;
}

static u32 spinal_lib_mac_rx_u32(void __iomem *base){
    return readl(base + SPINAL_LIB_MAC_RX);
}

static void spinal_lib_mac_reset_set(void __iomem *base){
    writel(SPINAL_LIB_MAC_CTRL_TX_RESET | SPINAL_LIB_MAC_CTRL_RX_RESET | SPINAL_LIB_MAC_CTRL_TX_ALIGN | SPINAL_LIB_MAC_CTRL_RX_ALIGN, base + SPINAL_LIB_MAC_CTRL);
}

static void spinal_lib_mac_reset_clear(void __iomem *base){
    writel(SPINAL_LIB_MAC_CTRL_TX_ALIGN | SPINAL_LIB_MAC_CTRL_RX_ALIGN, base + SPINAL_LIB_MAC_CTRL);
}

static int spinal_lib_mac_start(struct udevice *dev)
{
	return 0;
}

static int spinal_lib_mac_send(struct udevice *dev, void *packet, int length)
{
	struct eth_pdata *pdata = dev_get_platdata(dev);
    void __iomem *base = (void *)pdata->iobase;
    u32 bits = length*8+16;
    u32 word_count = (bits+31)/32;
    u32 *ptr;

    while(!spinal_lib_mac_tx_ready(base));
    spinal_lib_mac_tx_u32(base, bits);
    ptr = (u32*)((u32)packet-2);

    while(word_count){
        u32 tockens = spinal_lib_mac_tx_availability(base);
        if(tockens > word_count) tockens = word_count;
        word_count -= tockens;
        while(tockens--){
            spinal_lib_mac_tx_u32(base, *ptr++);
        }
    }

	return 0;
}

static int spinal_lib_mac_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct eth_pdata *pdata = dev_get_platdata(dev);
	struct spinal_lib_mac_priv *priv = dev_get_priv(dev);
    void __iomem *base = (void *)pdata->iobase;

    if (!spinal_lib_mac_rx_pending(base)) return 0;

    u32 bits = spinal_lib_mac_rx_u32(base);
    u32 len = (bits-16)/8;
    u32 word_count = (bits+31)/32;
    u32 *ptr;

    if (word_count > BUFSIZE) {
        debug("%s:%d word_count %d > buf %d\n",__func__,__LINE__,word_count,BUFSIZE);
        while(word_count--) spinal_lib_mac_rx_u32(base);
        return -EMSGSIZE;
    }

    *packetp = (uchar *)priv->buf;
    ptr = (u32*)((u32)(*packetp)-2);
    while(word_count--){
        *ptr++ = spinal_lib_mac_rx_u32(base);
    }

	return len;
}

static void spinal_lib_mac_stop(struct udevice *dev)
{
}

static int spinal_lib_mac_probe(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_platdata(dev);
    spinal_lib_mac_reset_set((void *)pdata->iobase);
    udelay(10);
    spinal_lib_mac_reset_clear((void *)pdata->iobase);
	return 0;
}

static const struct eth_ops spinal_lib_mac_ops = {
	.start			= spinal_lib_mac_start,
	.send			= spinal_lib_mac_send,
	.recv			= spinal_lib_mac_recv,
	.stop			= spinal_lib_mac_stop,
};

static int spinal_lib_mac_ofdata_to_platdata(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_platdata(dev);
	pdata->iobase = devfdt_get_addr(dev);
	return 0;
}

static const struct udevice_id spinal_lib_mac_of_match[] = {
	{ .compatible = "spinal,lib_mac" },
	{ }
};

U_BOOT_DRIVER(eth_spinal_lib_mac) = {
	.name = "eth_spinal_lib_mac",
	.id	= UCLASS_ETH,
	.of_match = spinal_lib_mac_of_match,
	.ofdata_to_platdata = spinal_lib_mac_ofdata_to_platdata,
	.probe = spinal_lib_mac_probe,
	.ops = &spinal_lib_mac_ops,
	.priv_auto_alloc_size = sizeof(struct spinal_lib_mac_priv),
	.platdata_auto_alloc_size = sizeof(struct eth_pdata),
};
