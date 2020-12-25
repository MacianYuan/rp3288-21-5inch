/*
 * GPIO driver for RICOH583 power management chip.
 *
 * Copyright (c) 2012, NVIDIA CORPORATION.  All rights reserved.
 * Author: Laxman dewangan <ldewangan@nvidia.com>
 *
 * Based on code
 *	Copyright (C) 2011 RICOH COMPANY,LTD
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include <linux/module.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/kernel.h>
#include <linux/of_gpio.h>
#include <linux/of_platform.h>
#include <linux/delay.h>

struct rp_gpio{
	int gpio;
	int enable;
};

struct led_data{
	struct rp_gpio led_gpio;
};

struct led_data *led;
static struct timer_list mytimer;
static int led_recovery_flag  = 0;

static ssize_t recovery_store(struct device* dev, struct device_attribute* attr, const char* buf, size_t count){

	led_recovery_flag = buf[0] - 0x30;
	printk("led_flash_status = %d\n",led_recovery_flag);

	if(led_recovery_flag == 2){
		gpio_direction_output(led->led_gpio.gpio,1);
		led->led_gpio.enable = 1;
	}else if(led_recovery_flag == 1){
		gpio_direction_output(led->led_gpio.gpio,0);
		led->led_gpio.enable = 0;
	}
	return count;
}
static ssize_t recovery_show(struct device* dev, struct device_attribute* attr, char* buf){
	printk("led_flash_status = %d\n",led_recovery_flag);
	return 0;
}
static DEVICE_ATTR_RW(recovery);

static struct attribute *recovery_led_attrs[] = {
	&dev_attr_recovery.attr,
	NULL,
};
ATTRIBUTE_GROUPS(recovery_led);

void function(unsigned long data){
	if(led_recovery_flag == 2){
		gpio_direction_output(led->led_gpio.gpio,1);
		led->led_gpio.enable = 1;
        	mod_timer(&mytimer, jiffies + msecs_to_jiffies(2000));
		return;
	}else if(led_recovery_flag == 1){
		gpio_direction_output(led->led_gpio.gpio,0);
		led->led_gpio.enable = 0;
        	mod_timer(&mytimer, jiffies + msecs_to_jiffies(2000));
		return;
	}
        led->led_gpio.enable ^= 1;
        gpio_direction_output(led->led_gpio.gpio,led->led_gpio.enable);
        mod_timer(&mytimer, jiffies + msecs_to_jiffies(2000));
        return;
}

static int led_gpio_probe(struct platform_device *pdev)
{
	
	struct class *led_class;
#ifdef CONFIG_OF
	enum of_gpio_flags flags;
	struct device_node *node = pdev->dev.of_node;
#endif
	
	if(!led){
		led = devm_kzalloc(&pdev->dev,sizeof(struct led_data), GFP_KERNEL);
		if (!led)
			return -ENOMEM;
			memset(led, 0, sizeof(struct led_data));
	}
	if(!node){
		if(!led)
			devm_kfree(&pdev->dev,led);
		return 0;
	}
#ifdef CONFIG_OF
	led->led_gpio.gpio = of_get_named_gpio_flags(node, "led_gpio", 0, &flags);
	if (gpio_is_valid(led->led_gpio.gpio)){
		led->led_gpio.enable = (flags == 1)? 1:0;
	}else{
		printk("led_gpio invalid gpio: %d\n",led->led_gpio.gpio);
		gpio_free(led->led_gpio.gpio);
	}
#endif

	gpio_request(led->led_gpio.gpio, "led_gpio");
	gpio_direction_output(led->led_gpio.gpio,led->led_gpio.enable);
	
	init_timer(&mytimer);
	mytimer.expires = jiffies + jiffies_to_msecs(2);
	mytimer.function = function;
	mytimer.data = 0;
	add_timer(&mytimer);

	led_class = class_create(THIS_MODULE, "recovery_led");
	if (IS_ERR(led_class)) {
		pr_err("%s: couldn't create led_class\n", __FILE__);
		return PTR_ERR(led_class);
	}
	led_class->dev_groups = recovery_led_groups;

	return 0;
}

static int led_gpio_remove(struct platform_device *pdev)
{
	del_timer(&mytimer);
	gpio_free(led->led_gpio.gpio);
	if(!led){
		devm_kfree(&pdev->dev,led);
	}
	return 0;
}
static int led_gpio_suspend(struct platform_device *pdev, pm_message_t state)
{
	del_timer(&mytimer);
	if(led->led_gpio.enable == 1){
		gpio_direction_output(led->led_gpio.gpio,1);
	}
	return 0;
}
static int led_gpio_resume(struct platform_device *pdev)
{
	gpio_direction_output(led->led_gpio.gpio,0);
	led->led_gpio.enable = 1;
	add_timer(&mytimer);
	return 0;
}

#ifdef CONFIG_OF
static struct of_device_id gpio_dt_ids[] = {
	{ .compatible = "led_gpio" },
	{}
};
MODULE_DEVICE_TABLE(of, gpio_dt_ids);
#endif
static struct platform_driver led_gpio_driver = {
	.driver = {
		.name    = "led_gpio",
		.owner   = THIS_MODULE,
#ifdef CONFIG_OF
		.of_match_table = of_match_ptr(gpio_dt_ids),
#endif
	},
	.suspend = led_gpio_suspend,
	.resume = led_gpio_resume,
	.probe		= led_gpio_probe,
	.remove		= led_gpio_remove,
};

static int __init led_gpio_init(void)
{
	return platform_driver_register(&led_gpio_driver);
}
subsys_initcall(led_gpio_init);

static void __exit led_gpio_exit(void)
{
	platform_driver_unregister(&led_gpio_driver);
}
module_exit(led_gpio_exit);

MODULE_LICENSE("GPL");

