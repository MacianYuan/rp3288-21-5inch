/*
 *  max17047_battery.c
 *
 *  based on max17047-fuelgauge.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/mutex.h>
#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/power_supply.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/power/max17047_battery.h>	
#include <linux/wakelock.h>
#include <linux/of_gpio.h>	/* of_get_named_gpio_flags */
#include <linux/of.h>	/* of_mach_ptr */

//3.4
//#define SOC_VALUE_ONE_EQ_PER 20
//#define SOC_SURPLUS_VALUE 16
//3.5
#define SOC_VALUE_ONE_EQ_PER 12
#define SOC_SURPLUS_VALUE 70
//3.6
//#define SOC_VALUE_ONE_EQ_PER 8
//#define SOC_SURPLUS_VALUE 136
//3.7
//#define SOC_VALUE_ONE_EQ_PER 0
//#define SOC_SURPLUS_VALUE 256

static void max17047_get_soc(struct i2c_client *client);




static struct max17047_platform_data max17047_pdata = {
	.enable_gauging_temperature = 0,
	
};
/*
static int charge_detect_gpio = -1;
static int battery_full_detect_gpio = -1;

#define CHARGE_DETECT charge_detect_gpio
#define BATTERY_FULL_DETECT battery_full_detect_gpio
*/
//extern	bool is_charging(void);
//extern	bool battery_is_full(void);

//extern void read_pad();

#undef dev_info
//#define DEBUG_MAX17047
#ifdef DEBUG_MAX17047
#define dev_info(dev, fmt, arg...) _dev_info(dev, fmt, ##arg)
#else
#define dev_info(dev, fmt, arg...) 
#endif


static int max17047_read_reg(struct i2c_client *client, int reg, u8 * buf);
//static int max17047_write_reg(struct i2c_client *client, int reg, u8 * buf);
//static void max17047_write_reg_array(struct i2c_client *client, struct max17047_reg_data *data, int size);

struct max17047 {
	struct i2c_client *client;
	struct delayed_work work;
	struct power_supply *battery;
	struct power_supply *ac;
	struct max17047_platform_data *pdata;
	int charge_detect_gpio;
	int battery_full_detect_gpio;
	int vcell;			/* battery voltage */

	int soc;			/* battery capacity */

	int temperature;	/* temperature */

};

static bool is_charging(struct max17047 *max17047)
{
	return gpio_get_value(max17047->charge_detect_gpio) ? 0 : 1;
}

static bool battery_is_full(struct max17047 *max17047)
{
	return gpio_get_value(max17047->battery_full_detect_gpio) ? 1 : 0;
}

static int max17047_get_property(struct power_supply *psy,
			    enum power_supply_property psp,
			    union power_supply_propval *val)
{
	struct max17047 *max17047 = power_supply_get_drvdata(psy);
//	u8 data[2];
//	s32 temper = 0;

	switch (psp) {
	case POWER_SUPPLY_PROP_PRESENT:
		val->intval = 1;
		break;
/*		
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		switch (val->intval) {
		case 0:
			val->intval = max17047->vcell;
			break;
		default:
			val->intval = max17047->vcell;
			break;
		}
		break;
*/
		
	case POWER_SUPPLY_PROP_CAPACITY:
		switch (val->intval) {
		case 0:	/*normal soc */
			val->intval = max17047->soc;
			break;
		default:
			val->intval = max17047->soc;
			
		}
		break;
		
//	case POWER_SUPPLY_PROP_TEMP:
//		if (max17047_read_reg(max17047->client, MAX17047_REG_TEMPERATURE, data) < 0)
//			return EIO;
//		
//		/* data[] store 2's compliment format number */
//		if (data[1] & (0x1 << 7)) {
//			/* Negative */
//			temper = ((~(data[1])) & 0xFF) + 1;
//			temper *= (-1000);
//		} else {
//			temper = data[1] & 0x7F;
//			temper *= 1000;
//			temper += data[0] * 39 / 10;
//		}
//		val->intval = temper / 100;
//		break;
//
//	case POWER_SUPPLY_PROP_CURRENT_NOW:
//		val->intval = 1000 * 1000;
//		break;
//        case POWER_SUPPLY_PROP_HEALTH:
//		val->intval = POWER_SUPPLY_HEALTH_GOOD;
//		break;
	case POWER_SUPPLY_PROP_STATUS:
		if(is_charging(max17047)){
			if(battery_is_full(max17047)){
				val->intval = POWER_SUPPLY_STATUS_FULL;
				//POWER_SUPPLY_STATUS_FULL    display   POWER_SUPPLY_STATUS_NOT_CHARGING
				//val->intval = POWER_SUPPLY_STATUS_NOT_CHARGING;	
				break;
			}
			val->intval = POWER_SUPPLY_STATUS_CHARGING;	
		}else 
			val->intval = POWER_SUPPLY_STATUS_NOT_CHARGING;		
		break;
		
		break;
		
	default:
		return -EINVAL;
	}
	
	return 0;
}

static int max17047_ac_get_property(struct power_supply *psy,
			    enum power_supply_property psp,
			    union power_supply_propval *val)
{
	struct max17047 *max17047 = power_supply_get_drvdata(psy);

	switch (psp) {
	case POWER_SUPPLY_PROP_STATUS:
		if(is_charging(max17047)){
			if(battery_is_full(max17047)){
				val->intval = POWER_SUPPLY_STATUS_FULL;
				//POWER_SUPPLY_STATUS_FULL    display   POWER_SUPPLY_STATUS_NOT_CHARGING
				//val->intval = POWER_SUPPLY_STATUS_NOT_CHARGING;	
				break;
			}
			val->intval = POWER_SUPPLY_STATUS_CHARGING;	
		}else 
			val->intval = POWER_SUPPLY_STATUS_NOT_CHARGING;		
		break;
//	case POWER_SUPPLY_PROP_PRESENT:
//		val->intval = 0;
//		break;
		
	case POWER_SUPPLY_PROP_ONLINE:
		if(is_charging(max17047)){
			/*if(battery_is_full(max17047)){
				val->intval = 0;
				break;
			}*/
			val->intval = 1;	
		}else 
			val->intval = 0;		
		break;

	case POWER_SUPPLY_PROP_MODEL_NAME:
		val->strval = max17047->ac->desc->name;
		break;
		
	default:
		return -EINVAL;
	}
	
	return 0;	
}

static int max17047_read_reg(struct i2c_client *client, int reg, u8 * buf)
{
	int ret;

	ret = i2c_smbus_read_i2c_block_data(client, reg, 2, buf);
	if (ret < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, ret);

	return ret;
}

/*
static int max17047_write_reg(struct i2c_client *client, int reg, u8 * buf)
{
	int ret;

	ret = i2c_smbus_write_i2c_block_data(client, reg, 2, buf);
	if (ret < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, ret);

	return ret;
}

static void max17047_write_reg_array(struct i2c_client *client, struct max17047_reg_data *data, int size)
{
	int i;
	for (i = 0; i < size; i += 3)
		max17047_write_reg(client, (data + i)->reg_addr,
				   ((u8 *) (data + i)) + 1);
}
static void max17047_alert_init(struct i2c_client *client)
{
	struct max17047_platform_data *pdata = client->dev.platform_data;
	dev_info(&client->dev, "%s\n", __func__);
	max17047_write_reg_array(client, pdata->alert_init,
		pdata->alert_init_size);
}

static int max17047_read_vfsoc(struct i2c_client *client)
{
	struct max17047 *max17047 = i2c_get_clientdata(client);

	max17047_get_soc(client);

	return max17047->soc;
}

static void max17047_reset_soc(struct i2c_client *client)
{
	u8 data[2];

	if (max17047_read_reg(client, MAX17047_REG_MISCCFG, data) < 0)
		return;

	data[1] |= (0x1 << 2);
	max17047_write_reg(client, MAX17047_REG_MISCCFG, data);

	msleep(500);

	return;
}
*/

static void max17047_get_vcell(struct i2c_client *client)
{
	struct max17047 *max17047 = i2c_get_clientdata(client);
	u8 data[2];

	if (max17047_read_reg(client, MAX17047_REG_VCELL, data) < 0)
		return;

	max17047->vcell = ((data[0] >> 3) + (data[1] << 5)) * 625;

//	if (max17047_read_reg(client, MAX17047_REG_AVGVCELL, data) < 0)
//		return;

//	max17047->avgvcell = ((data[0] >> 3) + (data[1] << 5)) * 625;
}
static void max17047_get_soc(struct i2c_client *client)
{
	struct max17047 *max17047 = i2c_get_clientdata(client);
	u8 data[2];
	int soc;
//	int diff = 0;

	if (max17047_read_reg(client, MAX17047_REG_SOC_VF, data) < 0)
		return;

	soc = (data[1] * 100) + (data[0] * 100 / 256);
	//printk("data[1]     =     %d    data[0]     ===  %d\n",data[1],data[0]);

	
	dev_info(&max17047->client->dev, "%s : soc(0x%02X%02X))\n", __func__, data[1], data[0]);

//bill   data[0] < 16 + 240/SOC_VALUE_ONE_EQ_PER  soc = 0;
	soc = (soc >= 100) ? SOC_VALUE_ONE_EQ_PER + (soc -100)*(100 - SOC_VALUE_ONE_EQ_PER)/((90 - 1)*100) : \
		(data[0] - SOC_SURPLUS_VALUE)/((256 - SOC_SURPLUS_VALUE)/SOC_VALUE_ONE_EQ_PER);// + min((data[0] - 16)%((256 - 16)/SOC_VALUE_ONE_EQ_PER) , 1 );
	soc = min(soc, 100);
	soc = max(soc, 0);

//	if(is_charging()){
//		if(battery_is_full()){
//			soc = 100;
//		}else if(soc == 100){
//			soc = soc - 1;
//		}	
//	}
	dev_info(&max17047->client->dev, "%s : soc(%d%%)\n", __func__, soc);
	//printk("soc(%d%%)\n" , soc);

	max17047->soc = soc;
	
}

static void max17047_get_temperature(struct i2c_client *client)
{
	struct max17047 *max17047 = i2c_get_clientdata(client);
	u8 data[2];
	s32 temper = 0;

	if (max17047_read_reg(client, MAX17047_REG_TEMPERATURE, data) < 0)
		return;

	/* data[] store 2's compliment format number */
	if (data[1] & (0x1 << 7)) {
		/* Negative */
		temper = ((~(data[1])) & 0xFF) + 1;
		temper *= (-1000);
	} else {
		temper = data[1] & 0x7F;
		temper *= 1000;
		temper += data[0] * 39 / 10;
	}

	max17047->temperature = temper;
}

//static int max17047_ac_set_property(struct power_supply *psy,
//			    enum power_supply_property psp,
//			    const union power_supply_propval *val)
//{
//	return 0;
//}
static void max17047_work(struct work_struct *work)
{
	struct max17047 *max17047;
//	u8 data[2];
//	u8 flag = 0;
//	int value = 0;
	
	max17047 = container_of(work, struct max17047, work.work);
	max17047_get_vcell(max17047->client);
	max17047_get_soc(max17047->client);

	if (max17047->pdata->enable_gauging_temperature)
		max17047_get_temperature(max17047->client);


	
	//printk("-------------- rpdzkj ivy soc = %d --------------\n", max17047->soc);
		power_supply_changed(max17047->ac);
		power_supply_changed(max17047->battery);
	schedule_delayed_work(&max17047->work, MAX17047_LONG_DELAY);
}

static enum power_supply_property max17047_battery_props[] = {
	POWER_SUPPLY_PROP_PRESENT,
//	POWER_SUPPLY_PROP_TEMP,
//	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_STATUS,
//	POWER_SUPPLY_PROP_CURRENT_NOW,
//	POWER_SUPPLY_PROP_HEALTH
};

static enum power_supply_property max17047_ac_props[] = {
	POWER_SUPPLY_PROP_STATUS,	
	POWER_SUPPLY_PROP_ONLINE,
//	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_MODEL_NAME,
};

//static enum power_supply_property max17047_ac_props[] = {
//	POWER_SUPPLY_PROP_STATUS,	
//	POWER_SUPPLY_PROP_ONLINE,
//};
static const struct power_supply_desc max17047_battery_desc = {
	.name				= "max17047_battery",
	.type				= POWER_SUPPLY_TYPE_BATTERY,
	.get_property		= max17047_get_property,
	.properties			= max17047_battery_props,
	.num_properties		= ARRAY_SIZE(max17047_battery_props),
};
static const struct power_supply_desc max17047_ac_desc = {
	.name				= "max17047_ac",
	.type 				= POWER_SUPPLY_TYPE_MAINS,
	.get_property		= max17047_ac_get_property,
//	.set_property		= max17047_ac_set_property,
	.properties			= max17047_ac_props,
	.num_properties 	= ARRAY_SIZE(max17047_ac_props),
};

static int max17047_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
//	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	struct max17047 *max17047;
	struct power_supply_config psy_cfg = {};
	u8 data[2];
	struct device_node *np = client->dev.of_node;
//	enum of_gpio_flags flag;
	int i, ret;
	
//	printk("%s start\n", __func__);

	max17047 = kzalloc(sizeof(struct max17047), GFP_KERNEL);
	if (!max17047)
		return -ENOMEM;
	
	client->dev.platform_data = (void *)&max17047_pdata;
	if (!client->dev.platform_data) {
		dev_err(&client->dev, "%s : No platform data supplied\n", __func__);
		ret = -EINVAL;
		goto err_pdata;
	}
	//i2c check 17047 
	for(i = 0; i < 5 ; i++){
		if (max17047_read_reg(client, MAX17047_REG_MISCCFG, data) < 0){
			if( i == 4 ){
				printk("max17047 i2c failed\n");
				ret = -1;
				goto err_pdata;
			}else{
				continue;
			}
		}
	}

	
	max17047->charge_detect_gpio = of_get_named_gpio_flags(np, "charge-detect-gpio", 0, NULL);
	if (!gpio_is_valid(max17047->charge_detect_gpio)){
		max17047->charge_detect_gpio = -1;
	}else{
		gpio_request(max17047->charge_detect_gpio, "charge-detect-gpio");
		gpio_direction_input(max17047->charge_detect_gpio);
	}
	max17047->battery_full_detect_gpio = of_get_named_gpio_flags(np, "battery-full-detect-gpio", 0, NULL);
	if (!gpio_is_valid(max17047->battery_full_detect_gpio)){
		max17047->battery_full_detect_gpio = -1;
	}else{
		gpio_request(max17047->battery_full_detect_gpio, "battery-full-detect-gpio");
		gpio_direction_input(max17047->battery_full_detect_gpio);
	}
		
	max17047->client = client;
	max17047->pdata = client->dev.platform_data;

	i2c_set_clientdata(client, max17047);

//	printk("%s:name = %s\n",__func__, adapter->name);

	max17047->battery = power_supply_register(&client->dev, &max17047_battery_desc, &psy_cfg);
	if (IS_ERR(max17047->battery)) {
		printk("%s %d line: power supply register\n", __func__, __LINE__);
		kfree(max17047);
		return PTR_ERR(max17047->battery);
	}
	max17047->battery->drv_data = max17047;
	
	max17047->ac = power_supply_register(&client->dev, &max17047_ac_desc, &psy_cfg);
	if (IS_ERR(max17047->ac)) {
		printk("%s %d line: power supply register\n", __func__, __LINE__);
		kfree(max17047);
		return PTR_ERR(max17047->ac);
	}
	max17047->ac->drv_data = max17047;
	
	INIT_DELAYED_WORK(&max17047->work, max17047_work);
	schedule_delayed_work(&max17047->work, MAX17047_NO_DELAY);

	printk("%s end\n", __func__);

	return 0;

err_pdata:
	kfree(max17047);
	i2c_set_clientdata(client, NULL);

	return ret;
}

static int max17047_remove(struct i2c_client *client)
{
	struct max17047 *max17047 = i2c_get_clientdata(client);

	power_supply_unregister(max17047->battery);
	power_supply_unregister(max17047->ac);
	cancel_delayed_work(&max17047->work);
	kfree(max17047);
	return 0;
}

#ifdef CONFIG_PM
static int max17047_suspend(struct device *dev)
{
//	struct i2c_client *client = to_i2c_client(dev);
//	struct max17047 *max17047 = i2c_get_clientdata(client);

//	cancel_delayed_work(&max17047->work);//rpdzkj jeff
	return 0;
}

static int max17047_resume(struct device *dev)
{	
//	struct i2c_client *client = to_i2c_client(dev);
//	struct max17047 *max17047 = i2c_get_clientdata(client);


//	schedule_delayed_work(&max17047->work, MAX17047_RESUME_DELAY);//rpdzkj jeff
//	schedule_delayed_work(&max17047->work, MAX17047_SHORT_DELAY);
	return 0;
}
static const struct dev_pm_ops max17047_pm_ops = {
		.suspend	= max17047_suspend,
		.resume		= max17047_resume,
};

#else

#define max17047_suspend NULL
#define max17047_resume NULL

#endif

static const struct i2c_device_id max17047_id[] = {
	{DEV_NAME, 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, max17047_id);

static struct of_device_id max17047_dt_id[] = {
	{.compatible = "maxim,max17047"},
	{}
};

static struct i2c_driver max17047_i2c_driver = {
	.driver	= {
		.name	= DEV_NAME,
		.of_match_table = of_match_ptr(max17047_dt_id),
#ifdef CONFIG_PM
		.pm     = &max17047_pm_ops,
#endif
	},
	.probe		= max17047_probe,
	.remove		= max17047_remove,
	.id_table	= max17047_id,
};

static int __init max17047_init(void)
{
	return i2c_add_driver(&max17047_i2c_driver);
}

static void __exit max17047_exit(void)
{
	i2c_del_driver(&max17047_i2c_driver);
}

module_init(max17047_init);
module_exit(max17047_exit);

MODULE_AUTHOR("Gyungoh Yoo<jack.yoo@maxim-ic.com>");
MODULE_DESCRIPTION("max17047 battery driver");
MODULE_LICENSE("GPL");
