#include <mach/lm3528.h>

static int	lm3528_read_byte(struct lm3528_private_data* pdata, int reg)
{
	int		ret;

	mutex_lock(&pdata->update_lock);
	ret	=	i2c_smbus_read_byte_data(pdata->client, reg);
	mutex_unlock(&pdata->update_lock);

	return	ret;
}

static int	lm3528_write_byte(struct lm3528_private_data* pdata, int reg, int value)
{
	int		ret;

	mutex_lock(&pdata->update_lock);
	ret	=	i2c_smbus_write_byte_data(pdata->client, reg, value);
	mutex_unlock(&pdata->update_lock);

	return	ret;
}

static void	lm3528_store(struct lm3528_private_data* pdata)
{
	lm3528_write_byte(pdata, LM3528_REG_GP, pdata->reg_gp);
	lm3528_write_byte(pdata, LM3528_REG_BMAIN, pdata->reg_bmain);
	lm3528_write_byte(pdata, LM3528_REG_BSUB, pdata->reg_bsub);
	lm3528_write_byte(pdata, LM3528_REG_HPG, pdata->reg_hpg);
	lm3528_write_byte(pdata, LM3528_REG_GPIO, pdata->reg_gpio);
	lm3528_write_byte(pdata, LM3528_REG_PGEN0, pdata->reg_pgen0);
	lm3528_write_byte(pdata, LM3528_REG_PGEN1, pdata->reg_pgen1);
	lm3528_write_byte(pdata, LM3528_REG_PGEN2, pdata->reg_pgen2);
	lm3528_write_byte(pdata, LM3528_REG_PGEN3, pdata->reg_pgen3);
}

static void	lm3528_load(struct lm3528_private_data* pdata)
{
	pdata->reg_gp		=	lm3528_read_byte(pdata, LM3528_REG_GP);
	pdata->reg_bmain	=	lm3528_read_byte(pdata, LM3528_REG_BMAIN);
	pdata->reg_bsub		=	lm3528_read_byte(pdata, LM3528_REG_BSUB);
	pdata->reg_hpg		=	lm3528_read_byte(pdata, LM3528_REG_HPG);
	pdata->reg_gpio		=	lm3528_read_byte(pdata, LM3528_REG_GPIO);
	pdata->reg_pgen0	=	lm3528_read_byte(pdata, LM3528_REG_PGEN0);
	pdata->reg_pgen1	=	lm3528_read_byte(pdata, LM3528_REG_PGEN1);
	pdata->reg_pgen2	=	lm3528_read_byte(pdata, LM3528_REG_PGEN2);
	pdata->reg_pgen3	=	lm3528_read_byte(pdata, LM3528_REG_PGEN3);
}

int	lm3528_set_hwen(struct lm3528_private_data* pdata, int gpio, int status)
{
	if (status == 0) {
		lm3528_load(pdata);
		gpio_set_value(gpio, 0);
		return	0;
	}

	gpio_set_value(gpio, 1);
	lm3528_store(pdata);

	return	1;
}

int	lm3528_get_hwen(struct lm3528_private_data* pdata, int gpio)
{
	return	gpio_get_value(gpio);
}

int	lm3528_set_bmain(struct lm3528_private_data* pdata, int val)
{
	if ((val < 0) || (val > 127))
		return	-EINVAL;
	return	lm3528_write_byte(pdata, LM3528_REG_BMAIN, val);
}

int	lm3528_get_bmain(struct lm3528_private_data* pdata)
{
	int		val;

	mutex_lock(&pdata->update_lock);
	val	=	lm3528_read_byte(pdata, LM3528_REG_BMAIN);
	mutex_unlock(&pdata->update_lock);

	if (val < 0)
		return	val;

	return	(val & LM3528_BMASK);
}

int	lm3528_init(struct lm3528_private_data* pdata, struct i2c_client* client)
{
	mutex_init(&pdata->update_lock);
	pdata->client	=	client;

	lm3528_load(pdata);
}

EXPORT_SYMBOL(lm3528_init);
EXPORT_SYMBOL(lm3528_set_hwen);
EXPORT_SYMBOL(lm3528_get_hwen);
EXPORT_SYMBOL(lm3528_set_bmain);
EXPORT_SYMBOL(lm3528_get_bmain);

MODULE_AUTHOR("LG Electronics (dongjin73.kim@lge.com)");
MODULE_DESCRIPTION("Multi Display LED driver");
MODULE_LICENSE("GPL");
