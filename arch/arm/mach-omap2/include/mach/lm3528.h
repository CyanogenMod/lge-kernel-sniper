#ifndef __LM3528_H
#define __LM3528_H

#include <linux/kernel.h>
#include <asm/gpio.h>
#include <linux/i2c.h>

#define LM3528_I2C_NAME		"lm3528"
#define LM3528_I2C_ADDR		0x36

struct lm3528_private_data {
	unsigned char	reg_gp;
	unsigned char	reg_bmain;
	unsigned char	reg_bsub;
	unsigned char	reg_hpg;
	unsigned char	reg_gpio;
	unsigned char	reg_pgen0;
	unsigned char	reg_pgen1;
	unsigned char	reg_pgen2;
	unsigned char	reg_pgen3;

	struct i2c_client*	client;
	struct mutex	update_lock;
};

struct lm3528_platform_data {
	int		gpio_hwen;
	struct lm3528_private_data	private;
};

#define	LM3528_REG_GP		0x10
#define	LM3528_REG_BMAIN	0xa0
#define	LM3528_REG_BSUB		0xb0
#define	LM3528_REG_HPG		0x80
#define	LM3528_REG_GPIO		0x81
#define	LM3528_REG_PGEN0	0x90
#define	LM3528_REG_PGEN1	0x91
#define	LM3528_REG_PGEN2	0x92
#define	LM3528_REG_PGEN3	0x93

#define	LM3528_BMASK		0x7f	// Brightness Mask

int	lm3528_get_bmain(struct lm3528_private_data* pdata);
int	lm3528_set_bmain(struct lm3528_private_data* pdata, int val);
int	lm3528_get_hwen(struct lm3528_private_data* pdata, int gpio);
int	lm3528_set_hwen(struct lm3528_private_data* pdata, int gpio, int status);
int	lm3528_init(struct lm3528_private_data* pdata, struct i2c_client* client);

#endif
