/*
 * Copyright (c) 2012, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */


#include "sw.h"
#include "ssdk_init.h"
#include "fal_init.h"
#include "fal_misc.h"
#include "fal_mib.h"
#include "fal_port_ctrl.h"
#include "fal_portvlan.h"
#include "fal_fdb.h"
#include "fal_stp.h"
#include "fal_igmp.h"
#include "fal_qos.h"
#include "hsl.h"
#include "hsl_dev.h"
#include "ssdk_init.h"
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/phy.h>
#include <linux/platform_device.h>
#include <linux/types.h>
//#include <asm/mach-types.h>
#include <linux/kconfig.h>
#include <generated/autoconf.h>
#include <linux/netdevice.h>
#include <linux/phy.h>
#include <linux/delay.h>
#include <linux/string.h>
#if defined(CONFIG_OF) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,0))
#include <linux/switch.h>
#include <linux/of.h>
#else
#include <net/switch.h>
#include <linux/ar8216_platform.h>
#endif
#include "ssdk_plat.h"
#include "ref_vlan.h"
#include "ref_fdb.h"
#include "ref_mib.h"
#include "ref_port_ctrl.h"
#include "ref_misc.h"
#include "ref_uci.h"
#include "shell.h"



#define ISIS_CHIP_ID 0x18
#define ISIS_CHIP_REG 0
#define SHIVA_CHIP_ID 0x1f
#define SHIVA_CHIP_REG 0x10

/*
 * Using ISIS's address as default
  */
static int switch_chip_id = ISIS_CHIP_ID;
static int switch_chip_reg = ISIS_CHIP_REG;

static void
qca_phy_read_port_link(struct qca_phy_priv *priv, int port,
		      struct switch_port_link *port_link)
{
	a_uint32_t port_status, port_speed;

	memset(port_link, 0, sizeof(*port_link));

	port_status = priv->mii_read(AR8327_REG_PORT_STATUS(port));

    port_link->link = 1;
    port_link->aneg = !!(port_status & AR8327_PORT_STATUS_LINK_AUTO);
	if (port_link->aneg) {
		port_link->link = !!(port_status & AR8327_PORT_STATUS_LINK_UP);
		if (port_link->link == 0) {
			return;
        }
	}

	port_speed = (port_status & AR8327_PORT_STATUS_SPEED) >>
		            AR8327_PORT_STATUS_SPEED_S;
    if(port_speed == AR8327_PORT_SPEED_10M) {
        port_link->speed = SWITCH_PORT_SPEED_10;
    } else if(port_speed == AR8327_PORT_SPEED_100M) {
        port_link->speed = SWITCH_PORT_SPEED_100;
    } else if(port_speed == AR8327_PORT_SPEED_1000M) {
        port_link->speed = SWITCH_PORT_SPEED_1000;
    } else {
        port_link->speed = SWITCH_PORT_SPEED_UNKNOWN;
    }

	port_link->duplex = !!(port_status & AR8327_PORT_STATUS_DUPLEX);
	port_link->tx_flow = !!(port_status & AR8327_PORT_STATUS_TXFLOW);
	port_link->rx_flow = !!(port_status & AR8327_PORT_STATUS_RXFLOW);
}

static void
qca_ar8327_phy_fixup(struct qca_phy_priv *priv, int phy)
{
	switch (priv->revision) {
	case 1:
		/* 100m waveform */
		priv->phy_dbg_write(0, phy, 0, 0x02ea);
		/* turn on giga clock */
		priv->phy_dbg_write(0, phy, 0x3d, 0x68a0);
		break;

	case 2:
		priv->phy_mmd_write(0, phy, 0x7, 0x3c);
		priv->phy_mmd_write(0, phy, 0x4007, 0x0);
		/* fallthrough */
	case 4:
		priv->phy_mmd_write(0, phy, 0x3, 0x800d);
		priv->phy_mmd_write(0, phy, 0x4003, 0x803f);

		priv->phy_dbg_write(0, phy, 0x3d, 0x6860);
		priv->phy_dbg_write(0, phy, 0x5, 0x2c46);
		priv->phy_dbg_write(0, phy, 0x3c, 0x6000);
		break;
	}
}

#if defined(CONFIG_OF) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,0))
static int
qca_ar8327_hw_init(struct qca_phy_priv *priv)
{
	struct ar8327_platform_data *plat_data;
	struct device_node *np = NULL;
	const __be32 *paddr;
	a_uint32_t reg, value, i;
	a_int32_t len;

	np = priv->phy->dev.of_node;
	if(!np)
		return -EINVAL;

	/*First software reset S17 chip*/
	value = priv->mii_read(AR8327_REG_CTRL);
	value |= 0x80000000;
	priv->mii_write(AR8327_REG_CTRL, value);

	/* Configure switch register from DT information */
	paddr = of_get_property(np, "qca,ar8327-initvals", &len);
	if (!paddr || len < (2 * sizeof(*paddr))) {
		printk("len:%d < 2 * sizeof(*paddr):%d\n", len, 2 * sizeof(*paddr));
		return -EINVAL;
	}

	len /= sizeof(*paddr);

	for (i = 0; i < len - 1; i += 2) {
		reg = be32_to_cpup(paddr + i);
		value = be32_to_cpup(paddr + i + 1);
		priv->mii_write(reg, value);
	}

	value = priv->mii_read(AR8327_REG_PORT_LOOKUP(0));
	value &= ~0x5e;
	priv->mii_write(AR8327_REG_PORT_LOOKUP(0), value);

	value = priv->mii_read(AR8327_REG_PORT_LOOKUP(1));
	value &= ~0x21;
	priv->mii_write(AR8327_REG_PORT_LOOKUP(1), value);

	value = priv->mii_read(AR8327_REG_PORT_LOOKUP(2));
	value &= ~0x21;
	priv->mii_write(AR8327_REG_PORT_LOOKUP(2), value);

	value = priv->mii_read(AR8327_REG_PORT_LOOKUP(3));
	value &= ~0x21;
	priv->mii_write(AR8327_REG_PORT_LOOKUP(3), value);

	value = priv->mii_read(AR8327_REG_PORT_LOOKUP(4));
	value &= ~0x21;
	priv->mii_write(AR8327_REG_PORT_LOOKUP(4), value);

	value = priv->mii_read(AR8327_REG_PORT_LOOKUP(5));
	value &= ~0x5e;
	priv->mii_write(AR8327_REG_PORT_LOOKUP(5), value);

	value = priv->mii_read(AR8327_REG_PORT_LOOKUP(6));
	value &= ~0x21;
	priv->mii_write(AR8327_REG_PORT_LOOKUP(6), value);

	value = 0x20001;
	priv->mii_write(AR8327_REG_PORT_VLAN0(0), value);

	value = 0x10001;
	priv->mii_write(AR8327_REG_PORT_VLAN0(1), value);

	value = 0x10001;
	priv->mii_write(AR8327_REG_PORT_VLAN0(2), value);

	value = 0x10001;
	priv->mii_write(AR8327_REG_PORT_VLAN0(3), value);

	value = 0x10001;
	priv->mii_write(AR8327_REG_PORT_VLAN0(4), value);

	value = 0x20001;
	priv->mii_write(AR8327_REG_PORT_VLAN0(5), value);

	value = 0x10001;
	priv->mii_write(AR8327_REG_PORT_VLAN0(6), value);

	for (i = 0; i < AR8327_NUM_PHYS; i++) {
		qca_ar8327_phy_fixup(priv, i);

		/* start autoneg*/
		priv->phy_write(0, i, MII_ADVERTISE, ADVERTISE_ALL |
						     ADVERTISE_PAUSE_CAP | ADVERTISE_PAUSE_ASYM);
		priv->phy_write(0, i, MII_CTRL1000, ADVERTISE_1000FULL);
		priv->phy_write(0, i, MII_BMCR, BMCR_RESET | BMCR_ANENABLE);
	}

	msleep(1000);

	return 0;
}
#else
static a_uint32_t
qca_ar8327_get_pad_cfg(struct ar8327_pad_cfg *pad_cfg)
{
	a_uint32_t value = 0;

	if (pad_cfg == 0) {
		return 0;
    }

    if(pad_cfg->mode == AR8327_PAD_MAC2MAC_MII) {
		value = AR8327_PAD_CTRL_MAC_MII_EN;
		if (pad_cfg->rxclk_sel)
			value |= AR8327_PAD_CTRL_MAC_MII_RXCLK_SEL;
		if (pad_cfg->txclk_sel)
			value |= AR8327_PAD_CTRL_MAC_MII_TXCLK_SEL;

    } else if (pad_cfg->mode == AR8327_PAD_MAC2MAC_GMII) {
		value = AR8327_PAD_CTRL_MAC_GMII_EN;
		if (pad_cfg->rxclk_sel)
			value |= AR8327_PAD_CTRL_MAC_GMII_RXCLK_SEL;
		if (pad_cfg->txclk_sel)
			value |= AR8327_PAD_CTRL_MAC_GMII_TXCLK_SEL;

    } else if (pad_cfg->mode == AR8327_PAD_MAC_SGMII) {
		value = AR8327_PAD_CTRL_SGMII_EN;

		/* WAR for AP136 board. */
		value |= pad_cfg->txclk_delay_sel <<
		        AR8327_PAD_CTRL_RGMII_TXCLK_DELAY_SEL_S;
		value |= pad_cfg->rxclk_delay_sel <<
                AR8327_PAD_CTRL_RGMII_RXCLK_DELAY_SEL_S;
		if (pad_cfg->rxclk_delay_en)
			value |= AR8327_PAD_CTRL_RGMII_RXCLK_DELAY_EN;
		if (pad_cfg->txclk_delay_en)
			value |= AR8327_PAD_CTRL_RGMII_TXCLK_DELAY_EN;

    } else if (pad_cfg->mode == AR8327_PAD_MAC2PHY_MII) {
		value = AR8327_PAD_CTRL_PHY_MII_EN;
		if (pad_cfg->rxclk_sel)
			value |= AR8327_PAD_CTRL_PHY_MII_RXCLK_SEL;
		if (pad_cfg->txclk_sel)
			value |= AR8327_PAD_CTRL_PHY_MII_TXCLK_SEL;

    } else if (pad_cfg->mode == AR8327_PAD_MAC2PHY_GMII) {
		value = AR8327_PAD_CTRL_PHY_GMII_EN;
		if (pad_cfg->pipe_rxclk_sel)
			value |= AR8327_PAD_CTRL_PHY_GMII_PIPE_RXCLK_SEL;
		if (pad_cfg->rxclk_sel)
			value |= AR8327_PAD_CTRL_PHY_GMII_RXCLK_SEL;
		if (pad_cfg->txclk_sel)
			value |= AR8327_PAD_CTRL_PHY_GMII_TXCLK_SEL;

    } else if (pad_cfg->mode == AR8327_PAD_MAC_RGMII) {
		value = AR8327_PAD_CTRL_RGMII_EN;
		value |= pad_cfg->txclk_delay_sel <<
                 AR8327_PAD_CTRL_RGMII_TXCLK_DELAY_SEL_S;
		value |= pad_cfg->rxclk_delay_sel <<
                 AR8327_PAD_CTRL_RGMII_RXCLK_DELAY_SEL_S;
		if (pad_cfg->rxclk_delay_en)
			value |= AR8327_PAD_CTRL_RGMII_RXCLK_DELAY_EN;
		if (pad_cfg->txclk_delay_en)
			value |= AR8327_PAD_CTRL_RGMII_TXCLK_DELAY_EN;

    } else if (pad_cfg->mode == AR8327_PAD_PHY_GMII) {
		value = AR8327_PAD_CTRL_PHYX_GMII_EN;

    } else if (pad_cfg->mode == AR8327_PAD_PHY_RGMII) {
		value = AR8327_PAD_CTRL_PHYX_RGMII_EN;

    } else if (pad_cfg->mode == AR8327_PAD_PHY_MII) {
		value = AR8327_PAD_CTRL_PHYX_MII_EN;

	} else {
        value = 0;
    }

	return value;
}

#ifndef BOARD_AR71XX
static a_uint32_t
qca_ar8327_get_pwr_sel(struct qca_phy_priv *priv,
                                struct ar8327_platform_data *plat_data)
{
	struct ar8327_pad_cfg *cfg = NULL;
	a_uint32_t value;

	if (!plat_data) {
		return 0;
	}

	value = priv->mii_read(AR8327_REG_PAD_MAC_PWR_SEL);

	cfg = plat_data->pad0_cfg;

	if (cfg && (cfg->mode == AR8327_PAD_MAC_RGMII) &&
                cfg->rgmii_1_8v) {
		value |= AR8327_PAD_MAC_PWR_RGMII0_1_8V;
	}

	cfg = plat_data->pad5_cfg;
	if (cfg && (cfg->mode == AR8327_PAD_MAC_RGMII) &&
                cfg->rgmii_1_8v) {
		value |= AR8327_PAD_MAC_PWR_RGMII1_1_8V;
	}

	cfg = plat_data->pad6_cfg;
	if (cfg && (cfg->mode == AR8327_PAD_MAC_RGMII) &&
               cfg->rgmii_1_8v) {
		value |= AR8327_PAD_MAC_PWR_RGMII1_1_8V;
	}

	return value;
}
#endif

static a_uint32_t
qca_ar8327_set_led_cfg(struct qca_phy_priv *priv,
                              struct ar8327_platform_data *plat_data,
                              a_uint32_t pos)
{
	struct ar8327_led_cfg *led_cfg;
    a_uint32_t new_pos = pos;

	led_cfg = plat_data->led_cfg;
	if (led_cfg) {
		if (led_cfg->open_drain)
			new_pos |= AR8327_POS_LED_OPEN_EN;
		else
			new_pos &= ~AR8327_POS_LED_OPEN_EN;

		priv->mii_write(AR8327_REG_LED_CTRL_0, led_cfg->led_ctrl0);
		priv->mii_write(AR8327_REG_LED_CTRL_1, led_cfg->led_ctrl1);
		priv->mii_write(AR8327_REG_LED_CTRL_2, led_cfg->led_ctrl2);
		priv->mii_write(AR8327_REG_LED_CTRL_3, led_cfg->led_ctrl3);

		if (new_pos != pos) {
			new_pos |= AR8327_POS_POWER_ON_SEL;
		}
	}
    return new_pos;
}

static void
qca_ar8327_port_init(struct qca_phy_priv *priv, a_uint32_t port)
{
	struct ar8327_platform_data *plat_data;
	struct ar8327_port_cfg *port_cfg;
	a_uint32_t value;

	plat_data = priv->phy->dev.platform_data;
	if (plat_data == NULL) {
		return;
    }

	if (((port == 0) && plat_data->pad0_cfg) ||
	    ((port == 5) && plat_data->pad5_cfg) ||
	    ((port == 6) && plat_data->pad6_cfg)) {
        switch (port) {
        case 0:
            port_cfg = &plat_data->cpuport_cfg;
            break;
        case 5:
            port_cfg = &plat_data->port5_cfg;
            break;
        case 6:
            port_cfg = &plat_data->port6_cfg;
            break;
        }
	} else {
        return;
	}

	if (port_cfg->force_link == 0) {
		priv->mii_write(AR8327_REG_PORT_STATUS(port),
			            AR8327_PORT_STATUS_LINK_AUTO);
		return;
	}

	value = AR8327_PORT_STATUS_TXMAC | AR8327_PORT_STATUS_RXMAC;
	value|= port_cfg->duplex ? AR8327_PORT_STATUS_DUPLEX : 0;
	value |= port_cfg->rxpause ? AR8327_PORT_STATUS_RXFLOW : 0;
	value |= port_cfg->txpause ? AR8327_PORT_STATUS_TXFLOW : 0;

    if(port_cfg->speed == AR8327_PORT_SPEED_10) {
        value |= AR8327_PORT_SPEED_10M;
    } else if(port_cfg->speed == AR8327_PORT_SPEED_100) {
        value |= AR8327_PORT_SPEED_100M;
    } else if(port_cfg->speed == AR8327_PORT_SPEED_1000) {
        value |= AR8327_PORT_SPEED_1000M;
    }

	priv->mii_write(AR8327_REG_PORT_STATUS(port), value);
}

static int
qca_ar8327_hw_init(struct qca_phy_priv *priv)
{
	struct ar8327_platform_data *plat_data;
	a_uint32_t pos, new_pos;
	a_uint32_t value, i;

	plat_data = priv->phy->dev.platform_data;
	if (plat_data == NULL) {
		return -EINVAL;
	}
	/*First software reset S17 chip*/
	value = priv->mii_read(AR8327_REG_CTRL);
	value |= 0x80000000;
	priv->mii_write(AR8327_REG_CTRL, value);

	value = priv->mii_read(AR8327_REG_PORT_LOOKUP(0));
	value &= ~0x5e;
	priv->mii_write(AR8327_REG_PORT_LOOKUP(0), value);

	value = priv->mii_read(AR8327_REG_PORT_LOOKUP(1));
	value &= ~0x21;
	priv->mii_write(AR8327_REG_PORT_LOOKUP(1), value);

	value = priv->mii_read(AR8327_REG_PORT_LOOKUP(2));
	value &= ~0x21;
	priv->mii_write(AR8327_REG_PORT_LOOKUP(2), value);

	value = priv->mii_read(AR8327_REG_PORT_LOOKUP(3));
	value &= ~0x21;
	priv->mii_write(AR8327_REG_PORT_LOOKUP(3), value);

	value = priv->mii_read(AR8327_REG_PORT_LOOKUP(4));
	value &= ~0x21;
	priv->mii_write(AR8327_REG_PORT_LOOKUP(4), value);

	value = priv->mii_read(AR8327_REG_PORT_LOOKUP(5));
	value &= ~0x5e;
	priv->mii_write(AR8327_REG_PORT_LOOKUP(5), value);

	value = priv->mii_read(AR8327_REG_PORT_LOOKUP(6));
	value &= ~0x21;
	priv->mii_write(AR8327_REG_PORT_LOOKUP(6), value);

	value = 0x20001;
	priv->mii_write(AR8327_REG_PORT_VLAN0(0), value);

	value = 0x10001;
	priv->mii_write(AR8327_REG_PORT_VLAN0(1), value);

	value = 0x10001;
	priv->mii_write(AR8327_REG_PORT_VLAN0(2), value);

	value = 0x10001;
	priv->mii_write(AR8327_REG_PORT_VLAN0(3), value);

	value = 0x10001;
	priv->mii_write(AR8327_REG_PORT_VLAN0(4), value);

	value = 0x20001;
	priv->mii_write(AR8327_REG_PORT_VLAN0(5), value);

	value = 0x10001;
	priv->mii_write(AR8327_REG_PORT_VLAN0(6), value);

	value = qca_ar8327_get_pad_cfg(plat_data->pad0_cfg);
	priv->mii_write(AR8327_REG_PAD0_CTRL, value);

	value = qca_ar8327_get_pad_cfg(plat_data->pad5_cfg);
	priv->mii_write(AR8327_REG_PAD5_CTRL, value);

	value = qca_ar8327_get_pad_cfg(plat_data->pad6_cfg);
	priv->mii_write(AR8327_REG_PAD6_CTRL, value);

#ifndef BOARD_AR71XX
	value = qca_ar8327_get_pwr_sel(priv, plat_data);
	priv->mii_write(AR8327_REG_PAD_MAC_PWR_SEL, value);
#endif

	pos = priv->mii_read(AR8327_REG_POS);

    new_pos = qca_ar8327_set_led_cfg(priv, plat_data, pos);

#ifndef BOARD_AR71XX
	/*configure the SGMII*/
	if (plat_data->sgmii_cfg) {
		value = priv->mii_read(AR8327_REG_PAD_SGMII_CTRL);
		value &= ~(AR8327_PAD_SGMII_CTRL_MODE_CTRL);
		value |= ((plat_data->sgmii_cfg->sgmii_mode) <<
                  AR8327_PAD_SGMII_CTRL_MODE_CTRL_S);

		if (priv->version == QCA_VER_AR8337) {
			value |= (AR8327_PAD_SGMII_CTRL_EN_PLL |
			     AR8327_PAD_SGMII_CTRL_EN_RX |
			     AR8327_PAD_SGMII_CTRL_EN_TX);
		} else {
			value &= ~(AR8327_PAD_SGMII_CTRL_EN_PLL |
			       AR8327_PAD_SGMII_CTRL_EN_RX |
			       AR8327_PAD_SGMII_CTRL_EN_TX);
		}
		value |= AR8327_PAD_SGMII_CTRL_EN_SD;

		priv->mii_write(AR8327_REG_PAD_SGMII_CTRL, value);

		if (plat_data->sgmii_cfg->serdes_aen) {
			new_pos &= ~AR8327_POS_SERDES_AEN;
		} else {
			new_pos |= AR8327_POS_SERDES_AEN;
		}
	}
#endif

	priv->mii_write(AR8327_REG_POS, new_pos);

	for (i = 0; i < AR8327_NUM_PHYS; i++) {
		qca_ar8327_phy_fixup(priv, i);

		/* start autoneg*/
		priv->phy_write(0, i, MII_ADVERTISE, ADVERTISE_ALL |
						     ADVERTISE_PAUSE_CAP | ADVERTISE_PAUSE_ASYM);
		priv->phy_write(0, i, MII_CTRL1000, ADVERTISE_1000FULL);
		priv->phy_write(0, i, MII_BMCR, BMCR_RESET | BMCR_ANENABLE);
	}

	if(priv->version == QCA_VER_AR8337) {
        value = priv->mii_read(AR8327_REG_PAD5_CTRL);
        value |= AR8327_PAD_CTRL_RGMII_RXCLK_DELAY_EN;
        priv->mii_write(AR8327_REG_PAD5_CTRL, value);
    }

	msleep(1000);

	for (i = 0; i < AR8327_NUM_PORTS; i++) {
		qca_ar8327_port_init(priv, i);
    }

	return 0;
}
#endif

static int
qca_ar8327_sw_get_reg_val(struct switch_dev *dev,
                                    int reg, int *val)
{
	return 0;
}

static int
qca_ar8327_sw_set_reg_val(struct switch_dev *dev,
                                    int reg, int val)
{
	return 0;
}

static struct switch_attr qca_ar8327_globals[] = {
	{
		.name = "enable_vlan",
		.description = "Enable 8021q VLAN",
		.type = SWITCH_TYPE_INT,
		.set = qca_ar8327_sw_set_vlan,
		.get = qca_ar8327_sw_get_vlan,
		.max = 1
	},{
		.name = "max_frame_size",
		.description = "Set Max frame Size Of Mac",
		.type = SWITCH_TYPE_INT,
		.set = qca_ar8327_sw_set_max_frame_size,
		.get = qca_ar8327_sw_get_max_frame_size,
		.max = 9018
	},
	{
		.name = "reset_mibs",
		.description = "Reset All MIB Counters",
		.type = SWITCH_TYPE_NOVAL,
		.set = qca_ar8327_sw_set_reset_mibs,
	},
	{
		.name = "flush_arl",
		.description = "Flush All ARL table",
		.type = SWITCH_TYPE_NOVAL,
		.set = qca_ar8327_sw_atu_flush,
	},
	{
		.name = "dump_arl",
		.description = "Dump All ARL table",
		.type = SWITCH_TYPE_STRING,
		.get = qca_ar8327_sw_atu_dump,
	},
	{
		.name = "switch_ext",
		.description = "Switch extended configuration",
		.type = SWITCH_TYPE_EXT,
		.set = qca_ar8327_sw_switch_ext,
	},
};

static struct switch_attr qca_ar8327_port[] = {
	{
		.name = "reset_mib",
		.description = "Reset Mib Counters",
		.type = SWITCH_TYPE_NOVAL,
		.set = qca_ar8327_sw_set_port_reset_mib,
	},
	{
		.name = "mib",
		.description = "Get Mib Counters",
		.type = SWITCH_TYPE_STRING,
		.set = NULL,
		.get = qca_ar8327_sw_get_port_mib,
	},
};

static struct switch_attr qca_ar8327_vlan[] = {
	{
		.name = "vid",
		.description = "Configure Vlan Id",
		.type = SWITCH_TYPE_INT,
		.set = qca_ar8327_sw_set_vid,
		.get = qca_ar8327_sw_get_vid,
		.max = 4094,
	},
};

static const struct switch_dev_ops qca_ar8327_sw_ops = {
	.attr_global = {
		.attr = qca_ar8327_globals,
		.n_attr = ARRAY_SIZE(qca_ar8327_globals),
	},
	.attr_port = {
		.attr = qca_ar8327_port,
		.n_attr = ARRAY_SIZE(qca_ar8327_port),
	},
	.attr_vlan = {
		.attr = qca_ar8327_vlan,
		.n_attr = ARRAY_SIZE(qca_ar8327_vlan),
	},
	.get_port_pvid = qca_ar8327_sw_get_pvid,
	.set_port_pvid = qca_ar8327_sw_set_pvid,
	.get_vlan_ports = qca_ar8327_sw_get_ports,
	.set_vlan_ports = qca_ar8327_sw_set_ports,
	.apply_config = qca_ar8327_sw_hw_apply,
	.reset_switch = qca_ar8327_sw_reset_switch,
	.get_port_link = qca_ar8327_sw_get_port_link,
#ifndef BOARD_AR71XX
	.get_reg_val = qca_ar8327_sw_get_reg_val,
	.set_reg_val = qca_ar8327_sw_set_reg_val,
#endif
};

#define SSDK_MIB_CHANGE_WQ

static int
qca_phy_mib_task(struct qca_phy_priv *priv)
{
	qca_ar8327_sw_mib_task(&priv->sw_dev);
}

static void
qca_phy_mib_work_task(struct work_struct *work)
{
	struct qca_phy_priv *priv = container_of(work, struct qca_phy_priv,
                                            mib_dwork.work);

	mutex_lock(&priv->mib_lock);

    qca_phy_mib_task(priv);

	mutex_unlock(&priv->mib_lock);
#ifndef SSDK_MIB_CHANGE_WQ
	schedule_delayed_work(&priv->mib_dwork,
			      msecs_to_jiffies(QCA_PHY_MIB_WORK_DELAY));
#else
	queue_delayed_work_on(0, system_long_wq, &priv->mib_dwork,
					msecs_to_jiffies(QCA_PHY_MIB_WORK_DELAY));
#endif
}

static int
qca_phy_mib_work_start(struct qca_phy_priv *priv)
{
	mutex_init(&priv->mib_lock);
	priv->mib_counters = kzalloc(priv->sw_dev.ports * QCA_MIB_ITEM_NUMBER *
	      sizeof(*priv->mib_counters), GFP_KERNEL);
	if (!priv->mib_counters)
		return -ENOMEM;

	INIT_DELAYED_WORK(&priv->mib_dwork, qca_phy_mib_work_task);
#ifndef SSDK_MIB_CHANGE_WQ
	schedule_delayed_work(&priv->mib_dwork,
			               msecs_to_jiffies(QCA_PHY_MIB_WORK_DELAY));
#else
	queue_delayed_work_on(0, system_long_wq, &priv->mib_dwork,
					msecs_to_jiffies(QCA_PHY_MIB_WORK_DELAY));
#endif

	return 0;
}

static void
qca_phy_mib_work_stop(struct qca_phy_priv *priv)
{
	cancel_delayed_work_sync(&priv->mib_dwork);
}

static int
qca_phy_id_chip(struct qca_phy_priv *priv)
{
	a_uint32_t value, version;

	value = qca_ar8216_mii_read(AR8327_REG_CTRL);
	version = value & (AR8327_CTRL_REVISION |
                AR8327_CTRL_VERSION);
	priv->version = (version & AR8327_CTRL_VERSION) >>
                           AR8327_CTRL_VERSION_S;
	priv->revision = (version & AR8327_CTRL_REVISION);

    if((priv->version == QCA_VER_AR8327) ||
       (priv->version == QCA_VER_AR8337) ||
       (priv->version == QCA_VER_AR8227)) {
		return 0;

    } else {
		printk("unsupported QCA device\n");
		return -ENODEV;
	}
}

static int
qca_phy_config_init(struct phy_device *pdev)
{
	struct qca_phy_priv *priv = pdev->priv;
	struct switch_dev *sw_dev;
	int ret;

	if (pdev->addr != 0) {
        pdev->supported |= SUPPORTED_1000baseT_Full;
        pdev->advertising |= ADVERTISED_1000baseT_Full;
		return 0;
	}

	if (priv == NULL)
		return -ENOMEM;

	priv->phy = pdev;
	ret = qca_phy_id_chip(priv);
	if (ret != 0) {
	        return ret;
	}

	priv->mii_read = qca_ar8216_mii_read;
	priv->mii_write = qca_ar8216_mii_write;
	priv->phy_write = qca_ar8327_phy_write;
	priv->phy_dbg_write = qca_ar8327_phy_dbg_write;
	priv->phy_mmd_write = qca_ar8327_mmd_write;

	pdev->priv = priv;
	pdev->supported |= SUPPORTED_1000baseT_Full;
	pdev->advertising |= ADVERTISED_1000baseT_Full;

	sw_dev = &priv->sw_dev;
	sw_dev->ops = &qca_ar8327_sw_ops;
	sw_dev->name = "QCA AR8327 AR8337";
	sw_dev->vlans = AR8327_MAX_VLANS;
	sw_dev->ports = AR8327_NUM_PORTS;

	ret = register_switch(&priv->sw_dev, pdev->attached_dev);
	if (ret != 0) {
	        return ret;
	}

	ret = qca_ar8327_hw_init(priv);
	if (ret != 0) {
	        return ret;
	}

	qca_phy_mib_work_start(priv);

	return ret;
}

static int
qca_phy_read_status(struct phy_device *pdev)
{
	struct qca_phy_priv *priv = pdev->priv;
	struct switch_port_link port_link;
	int ret;

	if (pdev->addr != 0) {
		mutex_lock(&priv->reg_mutex);
		ret = genphy_read_status(pdev);
		mutex_unlock(&priv->reg_mutex);
		return ret;
	}

	qca_phy_read_port_link(priv, pdev->addr, &port_link);
	pdev->link = !!port_link.link;
	if (pdev->link == 0)
		return 0;

	if(port_link.speed == SWITCH_PORT_SPEED_10) {
        pdev->speed = SPEED_10;
    } else if (port_link.speed == SWITCH_PORT_SPEED_100) {
        pdev->speed = SPEED_100;
    } else if (port_link.speed == SWITCH_PORT_SPEED_1000) {
        pdev->speed = SPEED_1000;
    } else {
        pdev->speed = 0;
    }

    if(port_link.duplex) {
       pdev->duplex = DUPLEX_FULL;
    } else {
       pdev->duplex = DUPLEX_HALF;
    }

    pdev->state = PHY_RUNNING;
	netif_carrier_on(pdev->attached_dev);
	pdev->adjust_link(pdev->attached_dev);

	return ret;
}

static int
qca_phy_config_aneg(struct phy_device *pdev)
{
	if (pdev->addr != 0) {
		return genphy_config_aneg(pdev);
	}

	return 0;
}

static int
qca_phy_probe(struct phy_device *pdev)
{
	struct qca_phy_priv *priv;
	int ret;

	priv = kzalloc(sizeof(struct qca_phy_priv), GFP_KERNEL);
	if (priv == NULL) {
		return -ENOMEM;
	}

	pdev->priv = priv;
	priv->phy = pdev;
	mutex_init(&priv->reg_mutex);

	ret = qca_phy_id_chip(priv);
	return ret;
}

static void
qca_phy_remove(struct phy_device *pdev)
{
	struct qca_phy_priv *priv = pdev->priv;

	if ((pdev->addr == 0) && priv) {
        qca_phy_mib_work_stop(priv);
		kfree(priv->mib_counters);
		unregister_switch(&priv->sw_dev);
	}

	if (priv) {
		kfree(priv);
    }
}

static struct phy_driver qca_phy_driver = {
    .name		= "QCA AR8216 AR8236 AR8316 AR8327 AR8337",
	.phy_id		= 0x004d0000,
    .phy_id_mask= 0xffff0000,
	.probe		= qca_phy_probe,
	.remove		= qca_phy_remove,
	.config_init= &qca_phy_config_init,
	.config_aneg= &qca_phy_config_aneg,
	.read_status= &qca_phy_read_status,
	.features	= PHY_BASIC_FEATURES,
	.driver		= { .owner = THIS_MODULE },
};

static struct mii_bus *miibus = NULL;
extern ssdk_chip_type SSDK_CURRENT_CHIP_TYPE;

#define UCI_TAKEOVER
#ifdef UCI_TAKEOVER
#ifdef BOARD_AR71XX
#define SSDK_MAX_PHY  5
struct ssdk_phy_8216prv {
	struct switch_dev dev;
	struct phy_device *phy;
};

struct ssdk_phy_7240prv {
	struct mii_bus	*mii_bus;
	char *swdata;
	struct switch_dev swdev;
	int member1_1;
	int member1_2;
	char member2;
	bool vlan;
	a_uint16_t vlan_id[16];
	a_uint8_t  vlan_table[16];
	a_uint8_t vlan_tagged;
	a_uint16_t pvid[6];
};

struct ssdk_ag_member1 {
	char *member1[2];
	int member2;
	char *member3;
	int member4;
	char *member5;
	dma_addr_t member6;
	char *member7;
};

struct ssdk_ag {
	struct ssdk_ag_member1 member1_1 ____cacheline_aligned;
	struct ssdk_ag_member1 member1_2 ____cacheline_aligned;
	void *member2[5];
	int member3[2];
	struct net_device	*dev;
	char *member4;
	struct napi_struct	ssdk_napi;
	spinlock_t		ssdk_lock;
	int member5;
	char *member6[2];
	int member7;
	char *member8;
	dma_addr_t member9;
	bool member10;
	char *member11[2];
	void  *phy_priv;
};

struct switch_dev *old_sw_dev = NULL;
struct net_device    *sw_attach_dev = NULL;
struct qca_phy_priv qca_priv;
struct ssdk_phy_7240prv *ssdk_7240_prv = NULL;

void ssdk_uci_vlan_enable_set(char enable)
{
	ssdk_7240_prv->vlan = enable;
}

void ssdk_uci_vlan_ports_set(a_uint16_t *vlan_id, a_uint8_t tag,
			a_uint16_t *pvid, a_uint8_t *vlan_table)
{
	memcpy(ssdk_7240_prv->vlan_id, vlan_id, sizeof(ssdk_7240_prv->vlan_id));
	memcpy(ssdk_7240_prv->pvid, pvid, sizeof(ssdk_7240_prv->pvid));
	memcpy(ssdk_7240_prv->vlan_table, vlan_table, sizeof(ssdk_7240_prv->vlan_table));
	ssdk_7240_prv->vlan_tagged = tag;
}

void ssdk_uci_vlan_table_set(a_uint8_t vlan, a_uint16_t index)
{
	if(index < 16)
		ssdk_7240_prv->vlan_table[index] = vlan;
}

int ssdk_uci_takeover()
{
	int phy_addr = 0;
	struct ssdk_phy_8216prv *phy_8216prv = NULL;
	struct phy_device *phydev = NULL;
	struct qca_phy_priv *priv = &qca_priv;

	if((qca_ar8216_mii_read(0)&0xff00)>>8 != 0x02)
		return 0;

	memset(&qca_priv, 0, sizeof(qca_priv));
	mutex_init(&priv->reg_mutex);
	qca_phy_id_chip(priv);
	priv->mii_read = qca_ar8216_mii_read;
	priv->mii_write = qca_ar8216_mii_write;
	priv->phy_write = qca_ar8327_phy_write;
	priv->phy_dbg_write = qca_ar8327_phy_dbg_write;
	priv->phy_mmd_write = qca_ar8327_mmd_write;

	priv->sw_dev.ops = &qca_ar8327_sw_ops;
	priv->sw_dev.name = "QCA AR8327 AR8337";
	priv->sw_dev.vlans = AR8327_MAX_VLANS;
	priv->sw_dev.ports = AR8327_NUM_PORTS;

	/*try to get mii_bus phy_map*/
	for (phy_addr = 0; phy_addr < SSDK_MAX_PHY; phy_addr++) {
		if(miibus->phy_map[phy_addr]) {
			phydev = miibus->phy_map[phy_addr];
			phy_8216prv  = phydev->priv;
			if(!phy_8216prv)
				continue;
			if(phy_8216prv->dev.ports > 0) {
				/*got it*/

				old_sw_dev = &phy_8216prv->dev;
				sw_attach_dev = phydev->attached_dev;

				unregister_switch(old_sw_dev);
				printk("register switch!\n");
				if(register_switch(&priv->sw_dev, sw_attach_dev) < 0)
					printk("ssdk register switch fail!\n");

				break;
			}
		}
	}

	if(phy_addr >= SSDK_MAX_PHY) {
		/*not found it, try to get from ag71xx from eth1*/
		struct net_device *ag_dev = NULL;
		ag_dev = dev_get_by_name(&init_net, "eth1");
		if(ag_dev) {
			struct ssdk_ag *ag = NULL;

			ag = netdev_priv(ag_dev);
			if(ag) {
				//struct ssdk_phy_7240prv *prv = NULL;
				sw_attach_dev = ag->dev;
				ssdk_7240_prv = ag->phy_priv;
				if(ssdk_7240_prv) {
					old_sw_dev = &ssdk_7240_prv->swdev;
					unregister_switch(old_sw_dev);
					printk("register switch!!\n");
					if(register_switch(&priv->sw_dev, sw_attach_dev) < 0)
						printk("ssdk register switch fail!!\n");
					}
			}
		}
	}

	if(sw_attach_dev) {
		qca_phy_mib_work_start(&qca_priv);
	}

	return 0;
}
#endif
#endif


int
ssdk_plat_init(void)
{
	#ifdef UCI_TAKEOVER
	int rv = 0;
	#endif
    printk("ssdk_plat_init start\n");

    if(miibus_get())
	return -ENODEV;

    if(driver_find(qca_phy_driver.name, &mdio_bus_type)){
        printk("QCA PHY driver had been Registered\n");
        return 0;
    }

    printk("Register QCA PHY driver\n");
	#ifndef UCI_TAKEOVER
	return phy_driver_register(&qca_phy_driver);
	#else
	rv = phy_driver_register(&qca_phy_driver);
	#ifdef BOARD_AR71XX
	ssdk_uci_takeover();
	#endif
	return rv;
	#endif

}

void
ssdk_plat_exit(void)
{
    printk("ssdk_plat_exit\n");
#ifndef BOARD_AR71XX
    phy_driver_unregister(&qca_phy_driver);
#endif

#ifdef UCI_TAKEOVER
	#ifdef BOARD_AR71XX
	if(sw_attach_dev && old_sw_dev) {
		unregister_switch(&qca_priv.sw_dev);
		register_switch(old_sw_dev, sw_attach_dev);
		qca_phy_mib_work_stop(&qca_priv);
		kfree(qca_priv.mib_counters);
	}
	#endif
#endif

}

#ifdef BOARD_IPQ806X
#define IPQ806X_MDIO_BUS_NAME			"mdio-gpio"
#endif

#ifdef BOARD_AR71XX
#define IPQ806X_MDIO_BUS_NAME			"ag71xx-mdio"
#endif
#define MDIO_BUS_0						0
#define MDIO_BUS_1						1
#define IPQ806X_MDIO_BUS_NUM			MDIO_BUS_0


struct ag71xx_mdio {
	struct mii_bus		*mii_bus;
	int			mii_irq[PHY_MAX_ADDR];
	void __iomem		*mdio_base;
};




sw_error_t
ssdk_switch_init(a_uint32_t dev_id)
{
    a_uint32_t nr = 0;
    a_uint32_t i;
    hsl_dev_t *p_dev = NULL;

    p_dev = hsl_dev_ptr_get(dev_id);
    SW_RTN_ON_NULL(p_dev);

    /*fal_reset(dev_id);*/
    /*enable cpu and disable mirror*/
    fal_cpu_port_status_set(dev_id, A_TRUE);
    /* setup MTU */
    fal_frame_max_size_set(dev_id, 1518+8+2);
    /* Enable MIB counters */
    fal_mib_status_set(dev_id, A_TRUE);
    fal_igmp_mld_rp_set(dev_id, 0);

    for (i = 0; i < p_dev->nr_ports; i++)
    {
#ifdef BOARD_AR71XX
        if(i >= 6) {
            break;
        }
#endif
        if (i  != 0) {
            fal_port_link_forcemode_set(dev_id, i, A_FALSE);
        }
        fal_port_rxhdr_mode_set(dev_id, i, FAL_NO_HEADER_EN);
        fal_port_txhdr_mode_set(dev_id, i, FAL_NO_HEADER_EN);
        fal_port_3az_status_set(dev_id, i, A_FALSE);
        fal_port_flowctrl_forcemode_set(dev_id, i, A_TRUE);
        fal_port_flowctrl_set(dev_id, i, A_FALSE);

        if (i != 0 && i != 6) {
            fal_port_flowctrl_set(dev_id, i, A_TRUE);
            fal_port_flowctrl_forcemode_set(dev_id, i, A_FALSE);
        }
        fal_port_default_svid_set(dev_id, i, 0);
        fal_port_default_cvid_set(dev_id, i, 0);
        fal_port_1qmode_set(dev_id, i, FAL_1Q_DISABLE);
        fal_port_egvlanmode_set(dev_id, i, FAL_EG_UNMODIFIED);

        fal_fdb_port_learn_set(dev_id, i, A_TRUE);
        fal_stp_port_state_set(dev_id, 0, i, FAL_STP_FARWARDING);
        fal_port_vlan_propagation_set(dev_id, i, FAL_VLAN_PROPAGATION_REPLACE);

        fal_port_igmps_status_set(dev_id, i, A_FALSE);
        fal_port_igmp_mld_join_set(dev_id, i, A_FALSE);
        fal_port_igmp_mld_leave_set(dev_id, i, A_FALSE);
        fal_igmp_mld_entry_creat_set(dev_id, A_FALSE);
        fal_igmp_mld_entry_v3_set(dev_id, A_FALSE);

        /*make sure cpu port can communicate with
        the other ports normally*/
        if (i != 0)
        {
            fal_portvlan_member_add(dev_id, i, 0);
            fal_portvlan_member_add(dev_id, 0, i);
        }

        /* forward multicast and broadcast frames to CPU */
        fal_port_unk_uc_filter_set(dev_id, i, A_FALSE);
        fal_port_unk_mc_filter_set(dev_id, i, A_FALSE);
        fal_port_bc_filter_set(dev_id, i, A_FALSE);
        if ((SSDK_CURRENT_CHIP_TYPE == CHIP_SHIVA))  continue;
        /* Updating HOL registers and RGMII delay settings
	    with the values suggested by QCA switch team */
        if (i == 0 || i == 5 || i == 6)
        {
            nr = 240; /*30*8*/
            fal_qos_port_tx_buf_nr_set(dev_id, i, &nr);
            nr = 48; /*6*8*/
            fal_qos_port_rx_buf_nr_set(dev_id, i, &nr);
            fal_qos_port_red_en_set(dev_id, i, A_TRUE);
            if (SSDK_CURRENT_CHIP_TYPE == CHIP_ISISC)
            nr = 64; /*8*8*/
            else if (SSDK_CURRENT_CHIP_TYPE == CHIP_ISIS)
            nr = 60;
            fal_qos_queue_tx_buf_nr_set(dev_id, i, 5, &nr);
            nr = 48; /*6*8*/
            fal_qos_queue_tx_buf_nr_set(dev_id, i, 4, &nr);
            nr = 32; /*4*8*/
            fal_qos_queue_tx_buf_nr_set(dev_id, i, 3, &nr);
            nr = 32; /*4*8*/
            fal_qos_queue_tx_buf_nr_set(dev_id, i, 2, &nr);
            nr = 32; /*4*8*/
            fal_qos_queue_tx_buf_nr_set(dev_id, i, 1, &nr);
            nr = 24; /*3*8*/
            fal_qos_queue_tx_buf_nr_set(dev_id, i, 0, &nr);
        }
        else
        {
            nr = 200; /*25*8*/
            fal_qos_port_tx_buf_nr_set(dev_id, i, &nr);
            nr = 48; /*6*8*/
            fal_qos_port_rx_buf_nr_set(dev_id, i, &nr);
            fal_qos_port_red_en_set(dev_id, i, A_TRUE);
            if (SSDK_CURRENT_CHIP_TYPE == CHIP_ISISC)
            nr = 64; /*8*8*/
            else if (SSDK_CURRENT_CHIP_TYPE == CHIP_ISIS)
            nr = 60;
            fal_qos_queue_tx_buf_nr_set(dev_id, i, 3, &nr);
            nr = 48; /*6*8*/
            fal_qos_queue_tx_buf_nr_set(dev_id, i, 2, &nr);
            nr = 32; /*4*8*/
            fal_qos_queue_tx_buf_nr_set(dev_id, i, 1, &nr);
            nr = 24; /*3*8*/
            fal_qos_queue_tx_buf_nr_set(dev_id, i, 0, &nr);
        }
    }
    return SW_OK;
}


sw_error_t
ssdk_init(a_uint32_t dev_id, ssdk_init_cfg * cfg)
{
    sw_error_t rv;

#if (defined(KERNEL_MODULE) && defined(USER_MODE))
    rv = hsl_dev_init(dev_id, cfg);
#else
#ifdef HSL_STANDALONG
    rv = hsl_dev_init(dev_id, cfg);
#else
    rv = fal_init(dev_id, cfg);
#endif
#endif
    rv =  ssdk_switch_init(dev_id);

    return rv;
}

sw_error_t
ssdk_reduced_init(a_uint32_t dev_id, hsl_init_mode cpu_mode,
                  hsl_access_mode reg_mode)
{
    sw_error_t rv;

#if (defined(KERNEL_MODULE) && defined(USER_MODE))
    rv = hsl_dev_reduced_init(dev_id, cpu_mode, reg_mode);
#else
#ifdef HSL_STANDALONG
    rv = hsl_dev_reduced_init(dev_id, cpu_mode, reg_mode);
#else
    rv = fal_reduced_init(dev_id, cpu_mode, reg_mode);
#endif
#endif

    return rv;
}

sw_error_t
ssdk_cleanup(void)
{
    sw_error_t rv;

#if (defined(KERNEL_MODULE) && defined(USER_MODE))
    rv = hsl_dev_cleanup();
#else
#ifdef HSL_STANDALONG
    rv = hsl_dev_cleanup();
#else
    rv = fal_cleanup();
#endif
#endif

    return rv;
}

sw_error_t
ssdk_hsl_access_mode_set(a_uint32_t dev_id, hsl_access_mode reg_mode)
{
    sw_error_t rv;

    rv = hsl_access_mode_set(dev_id, reg_mode);
    return rv;
}

static inline void
split_addr(uint32_t regaddr, uint16_t *r1, uint16_t *r2, uint16_t *page)
{
	regaddr >>= 1;
	*r1 = regaddr & 0x1e;

	regaddr >>= 5;
	*r2 = regaddr & 0x7;

	regaddr >>= 3;
	*page = regaddr & 0x3ff;
}

uint32_t
qca_ar8216_mii_read(int reg)
{
	struct mii_bus *bus = miibus;
	uint16_t r1, r2, page;
	uint16_t lo, hi;
    static uint16_t irq_preepmt = 0;

	split_addr((uint32_t) reg, &r1, &r2, &page);
retry:
    irq_preepmt = 0;
    if(!in_interrupt()) {
	    mutex_lock(&bus->mdio_lock);
    } else {
        if(mutex_trylock(&bus->mdio_lock) == 0) {
            irq_preepmt = 1;
        }
    }

	bus->write(bus, switch_chip_id, switch_chip_reg, page);
	lo = bus->read(bus, 0x10 | r2, r1);
	hi = bus->read(bus, 0x10 | r2, r1 + 1);

	mutex_unlock(&bus->mdio_lock);
    if(!in_interrupt()) {
        if(irq_preepmt) {
            goto retry;
        }
    }

	return (hi << 16) | lo;
}

void
qca_ar8216_mii_write(int reg, uint32_t val)
{
	struct mii_bus *bus = miibus;
	uint16_t r1, r2, r3;
	uint16_t lo, hi;
    static uint16_t irq_preepmt = 0;

	split_addr((a_uint32_t) reg, &r1, &r2, &r3);
	lo = val & 0xffff;
	hi = (a_uint16_t) (val >> 16);

retry:
    irq_preepmt = 0;
    if(!in_interrupt()) {
	    mutex_lock(&bus->mdio_lock);
    } else {
        if(mutex_trylock(&bus->mdio_lock) == 0) {
            irq_preepmt = 1;
        }
    }

	bus->write(bus, switch_chip_id, switch_chip_reg, r3);
	if(SSDK_CURRENT_CHIP_TYPE != CHIP_SHIVA) {
	    bus->write(bus, 0x10 | r2, r1, lo);
	    bus->write(bus, 0x10 | r2, r1 + 1, hi);
	} else {
	    bus->write(bus, 0x10 | r2, r1 + 1, hi);
	    bus->write(bus, 0x10 | r2, r1, lo);
	}
	mutex_unlock(&bus->mdio_lock);
    if(!in_interrupt()) {
        if(irq_preepmt) {
            goto retry;
        }
    }
}

static uint32_t switch_chip_id_adjuest()
{
	uint32_t chip_version = 0;
	chip_version = (qca_ar8216_mii_read(0)&0xff00)>>8;
	if((chip_version !=0) && (chip_version !=0xff))
		return 0;

	switch_chip_id = SHIVA_CHIP_ID;
	switch_chip_reg = SHIVA_CHIP_REG;

	chip_version = (qca_ar8216_mii_read(0)&0xff00)>>8;
	printk("chip_version:0x%x\n", chip_version);
	return 1;
}

static sw_error_t
qca_ar8327_phy_read(a_uint32_t dev_id, a_uint32_t phy_addr,
                           a_uint32_t reg, a_uint16_t* data)
{
    struct mii_bus *bus = miibus;
    static uint16_t irq_preepmt = 0;

retry:
    irq_preepmt = 0;
    if(!in_interrupt()) {
        mutex_lock(&bus->mdio_lock);
    } else {
        if(mutex_trylock(&bus->mdio_lock) == 0) {
            irq_preepmt = 1;
        }
    }

    *data = bus->read(bus, phy_addr, reg);
    
	mutex_unlock(&bus->mdio_lock);
    if(!in_interrupt()) {
        if(irq_preepmt) {
            goto retry;
        }
    }

    return 0;
}

static sw_error_t
qca_ar8327_phy_write(a_uint32_t dev_id, a_uint32_t phy_addr,
                            a_uint32_t reg, a_uint16_t data)
{
    struct mii_bus *bus = miibus;
    static uint16_t irq_preepmt = 0;

retry:
    irq_preepmt = 0;
    if(!in_interrupt()) {
        mutex_lock(&bus->mdio_lock);
    } else {
        if(mutex_trylock(&bus->mdio_lock) == 0) {
            irq_preepmt = 1;
        }
    }

    bus->write(bus, phy_addr, reg, data);

	mutex_unlock(&bus->mdio_lock);
    if(!in_interrupt()) {
        if(irq_preepmt) {
            goto retry;
        }
    }

    return 0;
}

static void
qca_ar8327_phy_dbg_write(a_uint32_t dev_id, a_uint32_t phy_addr,
		                a_uint16_t dbg_addr, a_uint16_t dbg_data)
{
	struct mii_bus *bus = miibus;
    static uint16_t irq_preepmt = 0;
    
retry:
    irq_preepmt = 0;
    if(!in_interrupt()) {
        mutex_lock(&bus->mdio_lock);
    } else {
        if(mutex_trylock(&bus->mdio_lock) == 0) {
            irq_preepmt = 1;
        }
    }

	bus->write(bus, phy_addr, QCA_MII_DBG_ADDR, dbg_addr);
	bus->write(bus, phy_addr, QCA_MII_DBG_DATA, dbg_data);
    
	mutex_unlock(&bus->mdio_lock);
    if(!in_interrupt()) {
        if(irq_preepmt) {
            goto retry;
        }
    }
    
}

static void
qca_ar8327_mmd_write(a_uint32_t dev_id, a_uint32_t phy_addr,
                          a_uint16_t addr, a_uint16_t data)
{
	struct mii_bus *bus = miibus;
    static uint16_t irq_preepmt = 0;
    
retry:
    irq_preepmt = 0;
    if(!in_interrupt()) {
        mutex_lock(&bus->mdio_lock);
    } else {
        if(mutex_trylock(&bus->mdio_lock) == 0) {
            irq_preepmt = 1;
        }
    }

	bus->write(bus, phy_addr, QCA_MII_MMD_ADDR, addr);
	bus->write(bus, phy_addr, QCA_MII_MMD_DATA, data);
    
	mutex_unlock(&bus->mdio_lock);
    if(!in_interrupt()) {
        if(irq_preepmt) {
            goto retry;
        }
    }

}

static int miibus_get()
{
	struct device *miidev;
#ifdef BOARD_AR71XX
	struct ag71xx_mdio *am;
#endif
	uint8_t busid[MII_BUS_ID_SIZE];
#if defined(CONFIG_OF) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,0))
	const __be32 *prop = NULL;
	struct device_node *mdio_node = NULL;
	struct platform_device *mdio_plat = NULL;

	mdio_node = of_find_compatible_node(NULL, NULL, "virtual,mdio-gpio");
	if (!mdio_node) {
		printk("getting virtual,mdio-gpio failed\n");
		return 1;
	}

	mdio_plat = of_find_device_by_node(mdio_node);
	if (!mdio_plat) {
		printk("cannot find platform device from mdio node\n");
		return 1;
	}

	miibus = dev_get_drvdata(&mdio_plat->dev);
	if (!miibus) {
		printk("cannot get mii bus reference from device data\n");
		return 1;
	}

#else
	snprintf(busid, MII_BUS_ID_SIZE, "%s.%d",
		IPQ806X_MDIO_BUS_NAME, IPQ806X_MDIO_BUS_NUM);

	miidev = bus_find_device_by_name(&platform_bus_type, NULL, busid);
	if (!miidev) {
		printk("cannot get mii bus\n");
		return 1;
	}

#ifdef BOARD_AR71XX
	am = dev_get_drvdata(miidev);
	miibus = am->mii_bus;
#else
	miibus = dev_get_drvdata(miidev);
#endif

#ifdef BOARD_AR71XX
	if(switch_chip_id_adjuest()) {

		snprintf(busid, MII_BUS_ID_SIZE, "%s.%d",
		IPQ806X_MDIO_BUS_NAME, MDIO_BUS_1);

		miidev = bus_find_device_by_name(&platform_bus_type, NULL, busid);
		if (!miidev) {
			printk("cannot get mii bus\n");
			return 1;
		}

		am = dev_get_drvdata(miidev);
		miibus = am->mii_bus;
		printk("chip_version:0x%x\n", (qca_ar8216_mii_read(0)&0xff00)>>8);
	}
#endif

	if(!miidev){
		printk("mdio bus '%s' get FAIL\n", busid);
		return 1;
	}
#endif

	return 0;
}

static int __init
regi_init(void)
{
	ssdk_init_cfg cfg;
	int rv = 0;
	garuda_init_spec_cfg chip_spec_cfg;

	rv = ssdk_plat_init();
	if(rv)
		goto out;

	memset(&cfg, 0, sizeof(ssdk_init_cfg));
	memset(&chip_spec_cfg, 0, sizeof(garuda_init_spec_cfg));

	cfg.cpu_mode = HSL_CPU_1;
	cfg.reg_mode = HSL_MDIO;
	cfg.nl_prot = 30;

	cfg.chip_spec_cfg = &chip_spec_cfg;
	cfg.reg_func.mdio_set = qca_ar8327_phy_write;
	cfg.reg_func.mdio_get = qca_ar8327_phy_read;

    if((qca_ar8216_mii_read(0)&0xff00)>>8 == 0x02)
        cfg.chip_type = CHIP_SHIVA;
    else if((qca_ar8216_mii_read(0)&0xff00)>>8 == 0x13)
        cfg.chip_type = CHIP_ISISC;
    else if((qca_ar8216_mii_read(0)&0xff00)>>8 == 0x12)
        cfg.chip_type = CHIP_ISIS;
    else
	rv = 100;

    if(rv)
		goto out;

	rv = ssdk_init(0, &cfg);

out:
	if (rv == 0)
		printk("qca-ssdk module init succeeded!\n");
	else
		printk("qca-ssdk module init failed! (code: %d)\n", rv);

	return rv;
}

static void __exit
regi_exit(void)
{
    sw_error_t rv=ssdk_cleanup();

    if (rv == 0)
    	printk("qca-ssdk module exit  done!\n");
    else
        printk("qca-ssdk module exit failed! (code: %d)\n", rv);

    ssdk_plat_exit();
}

module_init(regi_init);
module_exit(regi_exit);

MODULE_DESCRIPTION("QCA SSDK Driver");
MODULE_AUTHOR("Qualcomm Atheros Inc");
MODULE_LICENSE("Dual BSD/GPL");

