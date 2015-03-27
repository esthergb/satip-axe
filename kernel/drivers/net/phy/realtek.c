/*
 * drivers/net/phy/realtek.c
 *
 * Driver for Realtek PHYs
 *
 * Author: Johnson Leung <r58129@freescale.com>
 *
 * Copyright (c) 2004 Freescale Semiconductor, Inc.
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */
#include <linux/phy.h>

#define RTL821x_PHYSR		0x11
#define RTL821x_PHYSR_DUPLEX	0x2000
#define RTL821x_PHYSR_SPEED	0xc000
#define RTL821x_INER		0x12
#define RTL821x_INER_INIT	0x6400
#define RTL821x_INSR		0x13
#define RTL821x_PHYCR   0x10

MODULE_DESCRIPTION("Realtek PHY driver");
MODULE_AUTHOR("Johnson Leung");
MODULE_LICENSE("GPL");

static int rtl821x_ack_interrupt(struct phy_device *phydev)
{
	int err;

	err = phy_read(phydev, RTL821x_INSR);

	return (err < 0) ? err : 0;
}

static int rtl821x_config_intr(struct phy_device *phydev)
{
	int err;

	if (phydev->interrupts == PHY_INTERRUPT_ENABLED)
		err = phy_write(phydev, RTL821x_INER,
				RTL821x_INER_INIT);
	else
		err = phy_write(phydev, RTL821x_INER, 0);

	return err;
}

int rtl821x_read_status(struct phy_device *phydev)
{
  int result = genphy_read_status(phydev);
  if (!result)
  {
    int value;

    value = phy_read(phydev, RTL821x_INSR);
    if (value&(1<<8))
    {
      /* false carrier detected - restarting auto negotiation */
      genphy_restart_aneg(phydev);
    }

    value = phy_read(phydev, RTL821x_PHYCR);

    if (phydev->speed == SPEED_1000)
    {
      if (value&(1<<4))
      {
        // switch on the clock
        value &= ~(1<<4);
        phy_write(phydev, RTL821x_PHYCR, value);
      }
    }
    else if ((value&(1<<4)) == 0)
    {
      // switch on the clock
      value |= (1<<4);
      phy_write(phydev, RTL821x_PHYCR, value);
    }
  }
  return result;
}

/* RTL8211B */
static struct phy_driver rtl821x_driver = {
	.phy_id		= 0x001cc910,
	.name		= "RTL821x Gigabit Ethernet",
	.phy_id_mask	= 0x001ffff0,
	.features	= PHY_GBIT_FEATURES,
	.flags		= PHY_HAS_INTERRUPT,
	.config_aneg	= &genphy_config_aneg,
	.read_status	= &rtl821x_read_status,
	.ack_interrupt	= &rtl821x_ack_interrupt,
	.config_intr	= &rtl821x_config_intr,
	.driver		= { .owner = THIS_MODULE,},
};

static int __init realtek_init(void)
{
	int ret;

	ret = phy_driver_register(&rtl821x_driver);

	return ret;
}

static void __exit realtek_exit(void)
{
	phy_driver_unregister(&rtl821x_driver);
}

module_init(realtek_init);
module_exit(realtek_exit);