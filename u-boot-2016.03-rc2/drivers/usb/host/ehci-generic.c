/*
 * Copyright (C) 2015 Alexey Brodkin <abrodkin@synopsys.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include "ehci.h"

/*
 * Even though here we don't explicitly use "struct ehci_ctrl"
 * ehci_register() expects it to be the first thing that resides in
 * device's private data.
 */
struct generic_ehci {
	struct ehci_ctrl ctrl;
};

static int ehci_usb_probe(struct udevice *dev)
{
	struct ehci_hccr *hccr = (struct ehci_hccr *)dev_get_addr(dev);
	struct ehci_hcor *hcor;
	int i;

	for (i = 0; ; i++) {
		struct udevice *clk_dev;
		int clk_id;

		clk_id = clk_get_by_index(dev, i, &clk_dev);
		if (clk_id < 0)
			break;
		if (clk_enable(clk_dev, clk_id))
			printf("failed to enable clock (dev=%s, id=%d)\n",
			       clk_dev->name, clk_id);
	}

	hcor = (struct ehci_hcor *)((uintptr_t)hccr +
				    HC_LENGTH(ehci_readl(&hccr->cr_capbase)));

	return ehci_register(dev, hccr, hcor, NULL, 0, USB_INIT_HOST);
}

static int ehci_usb_remove(struct udevice *dev)
{
	return ehci_deregister(dev);
}

static const struct udevice_id ehci_usb_ids[] = {
	{ .compatible = "generic-ehci" },
	{ }
};

U_BOOT_DRIVER(ehci_generic) = {
	.name	= "ehci_generic",
	.id	= UCLASS_USB,
	.of_match = ehci_usb_ids,
	.probe = ehci_usb_probe,
	.remove = ehci_usb_remove,
	.ops	= &ehci_usb_ops,
	.priv_auto_alloc_size = sizeof(struct generic_ehci),
	.flags	= DM_FLAG_ALLOC_PRIV_DMA,
};
