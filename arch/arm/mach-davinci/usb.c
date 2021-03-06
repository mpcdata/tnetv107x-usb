/*
 * USB
 */
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/usb/musb.h>

#include <mach/common.h>
#include <mach/irqs.h>
#include <mach/cputype.h>
#include <mach/usb.h>

#define DAVINCI_USB_OTG_BASE	0x01c64000

#define DA8XX_USB0_BASE 	0x01e00000
#define DA8XX_USB1_BASE 	0x01e25000
#define TNETV107X_USB1_BASE	0x08120800

#if defined(CONFIG_USB_MUSB_HDRC) || defined(CONFIG_USB_MUSB_HDRC_MODULE)

#ifndef CONFIG_ARCH_DAVINCI_TNETV107X
static struct musb_hdrc_eps_bits musb_eps[] = {
	{ "ep1_tx", 8, },
	{ "ep1_rx", 8, },
	{ "ep2_tx", 8, },
	{ "ep2_rx", 8, },
	{ "ep3_tx", 5, },
	{ "ep3_rx", 5, },
	{ "ep4_tx", 5, },
	{ "ep4_rx", 5, },
};

static struct musb_hdrc_config musb_config = {
	.multipoint	= true,
	.dyn_fifo	= true,
	.soft_con	= true,
	.dma		= true,

	.num_eps	= 5,
	.dma_channels	= 8,
	.ram_bits	= 10,
	.eps_bits	= musb_eps,
};
#else

static struct musb_hdrc_config musb_config = {
	.multipoint	= 1,
	.dyn_fifo	= 1,
	.dma		= 0,
	.num_eps	= 16,
	.ram_bits	= 12,
};

#endif

static struct musb_hdrc_platform_data usb_data = {
#if defined(CONFIG_USB_MUSB_OTG)
	/* OTG requires a Mini-AB connector */
	.mode           = MUSB_OTG,
#elif defined(CONFIG_USB_MUSB_PERIPHERAL)
	.mode           = MUSB_PERIPHERAL,
#elif defined(CONFIG_USB_MUSB_HOST)
	.mode           = MUSB_HOST,
#endif
	.clock		= "usb",
	.config		= &musb_config,
};

static struct resource usb_resources[] = {
	{
		/* physical address */
		.start          = DAVINCI_USB_OTG_BASE,
		.end            = DAVINCI_USB_OTG_BASE + 0x5ff,
		.flags          = IORESOURCE_MEM,
	},
	{
		.start          = IRQ_USBINT,
		.flags          = IORESOURCE_IRQ,
	},
	{
		/* placeholder for the dedicated CPPI IRQ */
		.flags          = IORESOURCE_IRQ,
	},
};

static u64 usb_dmamask = DMA_BIT_MASK(32);

static struct platform_device usb_dev = {
	.name           = "musb_hdrc",
	.id             = -1,
	.dev = {
		.platform_data		= &usb_data,
		.dma_mask		= &usb_dmamask,
		.coherent_dma_mask      = DMA_BIT_MASK(32),
	},
	.resource       = usb_resources,
	.num_resources  = ARRAY_SIZE(usb_resources),
};

void __init davinci_setup_usb(unsigned mA, unsigned potpgt_ms)
{
	usb_data.power = mA > 510 ? 255 : mA / 2;
	usb_data.potpgt = (potpgt_ms + 1) / 2;

	if (cpu_is_davinci_dm646x()) {
		/* Override the defaults as DM6467 uses different IRQs. */
		usb_dev.resource[1].start = IRQ_DM646X_USBINT;
		usb_dev.resource[2].start = IRQ_DM646X_USBDMAINT;
	} else	/* other devices don't have dedicated CPPI IRQ */
		usb_dev.num_resources = 2;

	platform_device_register(&usb_dev);
}

#ifdef CONFIG_ARCH_DAVINCI_DA8XX
static struct resource da8xx_usb20_resources[] = {
	{
		.start		= DA8XX_USB0_BASE,
		.end		= DA8XX_USB0_BASE + SZ_64K - 1,
		.flags		= IORESOURCE_MEM,
	},
	{
		.start		= IRQ_DA8XX_USB_INT,
		.flags		= IORESOURCE_IRQ,
	},
};

int __init da8xx_register_usb20(unsigned mA, unsigned potpgt)
{
	usb_data.clock  = "usb20";
	usb_data.power	= mA > 510 ? 255 : mA / 2;
	usb_data.potpgt = (potpgt + 1) / 2;

	usb_dev.resource = da8xx_usb20_resources;
	usb_dev.num_resources = ARRAY_SIZE(da8xx_usb20_resources);

	return platform_device_register(&usb_dev);
}
#endif	/* CONFIG_DAVINCI_DA8XX */

#ifdef CONFIG_ARCH_DAVINCI_TNETV107X

static struct resource tnetv107x_usb20_resources[] = {
	{
		.start		= TNETV107X_USB1_BASE,
		.end		= TNETV107X_USB1_BASE + SZ_2K - 1,
		.flags		= IORESOURCE_MEM,
	},
	{
		.start		= IRQ_TNETV107X_USB1,
		.flags		= IORESOURCE_IRQ,
	},

};

int __init tnetv107x_register_usb20(void)
{
	usb_data.clock  = "clk_usb1";
	usb_data.power	= 500 / 2;
	usb_dev.id = 1; /* tnetv has 2xmusb controllers */
	usb_dev.resource = tnetv107x_usb20_resources;
	usb_dev.num_resources = ARRAY_SIZE(tnetv107x_usb20_resources);

	return platform_device_register(&usb_dev);
}

#endif	/* CONFIG_ARCH_DAVINCI_TNETV107X */

#else

void __init davinci_setup_usb(unsigned mA, unsigned potpgt_ms)
{
}

#ifdef CONFIG_ARCH_DAVINCI_DA8XX
int __init da8xx_register_usb20(unsigned mA, unsigned potpgt)
{
	return 0;
}
#endif

#ifdef CONFIG_ARCH_DAVINCI_TNETV107X
int __init tnetv107x_register_usb20()
{
	return 0;
}
#endif

#endif  /* CONFIG_USB_MUSB_HDRC */

#ifdef	CONFIG_ARCH_DAVINCI_DA8XX
static struct resource da8xx_usb11_resources[] = {
	[0] = {
		.start	= DA8XX_USB1_BASE,
		.end	= DA8XX_USB1_BASE + SZ_4K - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= IRQ_DA8XX_IRQN,
		.end	= IRQ_DA8XX_IRQN,
		.flags	= IORESOURCE_IRQ,
	},
};

static u64 da8xx_usb11_dma_mask = DMA_BIT_MASK(32);

static struct platform_device da8xx_usb11_device = {
	.name		= "ohci",
	.id		= 0,
	.dev = {
		.dma_mask		= &da8xx_usb11_dma_mask,
		.coherent_dma_mask	= DMA_BIT_MASK(32),
	},
	.num_resources	= ARRAY_SIZE(da8xx_usb11_resources),
	.resource	= da8xx_usb11_resources,
};

int __init da8xx_register_usb11(struct da8xx_ohci_root_hub *pdata)
{
	da8xx_usb11_device.dev.platform_data = pdata;
	return platform_device_register(&da8xx_usb11_device);
}
#endif	/* CONFIG_DAVINCI_DA8XX */
