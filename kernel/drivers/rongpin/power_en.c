#include <linux/module.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/kernel.h>
#include <linux/of_gpio.h>
#include <linux/of_platform.h>
#include <linux/delay.h>
#include <linux/wakelock.h>
#include <linux/proc_fs.h>
struct gpio_gpio {
	int gpio_num;
	int val;
};

struct power_en_gpio {
	struct gpio_gpio gpio_3v3en;
	struct gpio_gpio gpio_5ven;
	struct gpio_gpio hub_rst;
	struct gpio_gpio gpio_sdemmc;
	struct gpio_gpio gpio_fan;
	int sleep_flag;
	struct wake_lock rp_wake_lock;
};

static struct power_en_gpio *hub_data = NULL;

static int hub_rst_proc_write(struct file * file,const char __user* buffer,size_t count,loff_t *data)
{
			gpio_direction_output(hub_data->hub_rst.gpio_num, 0);
        	gpio_set_value(hub_data->hub_rst.gpio_num, 0);
			msleep(500);
			gpio_direction_output(hub_data->hub_rst.gpio_num, 1);
        	gpio_set_value(hub_data->hub_rst.gpio_num, 1);
			return count;
			
}

static const struct file_operations hub_rst_proc = {
	.owner		= THIS_MODULE,
	.write		= hub_rst_proc_write,
};

static struct proc_dir_entry *gpio_ctl_entry;

static int power_en_probe(struct platform_device *pdev)
{
        int ret = 0;
        struct device_node *np = pdev->dev.of_node;
	struct power_en_gpio *data;

	data = devm_kzalloc(&pdev->dev, sizeof(struct power_en_gpio),GFP_KERNEL);
	if (!data) {
                dev_err(&pdev->dev, "failed to allocate memory\n");
                return -ENOMEM;
        }
	memset(data, 0, sizeof(struct power_en_gpio));
	
		hub_data = data;
		
        data->gpio_3v3en.gpio_num = of_get_named_gpio_flags(np, "33ven_pin", 0, NULL);
        if (!gpio_is_valid(data->gpio_3v3en.gpio_num))
                data->gpio_3v3en.gpio_num = -1;

        data->gpio_5ven.gpio_num = of_get_named_gpio_flags(np, "5ven_pin", 0, NULL);
        if (!gpio_is_valid(data->gpio_5ven.gpio_num))
                data->gpio_5ven.gpio_num = -1;

        data->gpio_sdemmc.gpio_num = of_get_named_gpio_flags(np, "sdemmc_pin", 0, NULL);
        if (!gpio_is_valid(data->gpio_sdemmc.gpio_num))
                data->gpio_sdemmc.gpio_num = -1;
				
        data->hub_rst.gpio_num = of_get_named_gpio_flags(np, "hub_rst", 0, NULL);
        if (!gpio_is_valid(data->hub_rst.gpio_num))
                data->hub_rst.gpio_num = -1;

        data->gpio_fan.gpio_num = of_get_named_gpio_flags(np, "fan_pin", 0, NULL);
        if (!gpio_is_valid(data->gpio_fan.gpio_num))
                data->gpio_fan.gpio_num = -1;
	of_property_read_u32(np, "rp_not_deep_leep", &data->sleep_flag);

	platform_set_drvdata(pdev, data);

	if(data->gpio_5ven.gpio_num != -1){
		ret = gpio_request(data->gpio_5ven.gpio_num, "gpio_5ven");
        	if (ret < 0){
			printk("data->gpio_5ven request error\n");
//      	        return ret;
		}else{
			gpio_direction_output(data->gpio_5ven.gpio_num, 1);
        		gpio_set_value(data->gpio_5ven.gpio_num, 1);
		}
	}

	if(data->gpio_3v3en.gpio_num != -1){
		ret = gpio_request(data->gpio_3v3en.gpio_num, "gpio_3v3en");
        	if (ret < 0){
			printk("data->gpio_3v3en request error\n");
//      	        return ret;
		}else{
			gpio_direction_output(data->gpio_3v3en.gpio_num, 1);
        		gpio_set_value(data->gpio_3v3en.gpio_num, 1);
		}
	}

	if(data->hub_rst.gpio_num != -1){
		ret = gpio_request(data->hub_rst.gpio_num, "hub_rst");
        	if (ret < 0){
			printk("data->hub_rst request error\n");
//        	        return ret;
		}else{
			gpio_direction_output(data->hub_rst.gpio_num, 0);
        		gpio_set_value(data->hub_rst.gpio_num, 0);
			msleep(100);
			gpio_direction_output(data->hub_rst.gpio_num, 1);
        		gpio_set_value(data->hub_rst.gpio_num, 1);
		}
	}

	if(data->gpio_sdemmc.gpio_num != -1){
		ret = gpio_request(data->gpio_sdemmc.gpio_num, "gpio_sdemmc");
        	if (ret < 0){
			printk("data->gpio_sdemmc request error\n");
//      	        return ret;
		}else{
			gpio_direction_output(data->gpio_sdemmc.gpio_num, 0);
        		gpio_set_value(data->gpio_sdemmc.gpio_num, 0);
		}
	}

	if(data->gpio_fan.gpio_num != -1){
		ret = gpio_request(data->gpio_fan.gpio_num, "gpio_fan");
        	if (ret < 0){
			printk("data->gpio_fan request error\n");
//      	        return ret;
		}else{
			gpio_direction_output(data->gpio_fan.gpio_num, 1);
        		gpio_set_value(data->gpio_fan.gpio_num, 1);
		}
	}
	if(data->sleep_flag != 0){
		wake_lock_init(&data->rp_wake_lock,WAKE_LOCK_SUSPEND, "rpdzkj_no_deep_sleep");
		wake_lock(&data->rp_wake_lock);
	}

        return 0;
}

static int power_en_remove(struct platform_device *pdev)
{
        struct power_en_gpio *data = platform_get_drvdata(pdev);


	if(data->gpio_3v3en.gpio_num != -1){
		gpio_direction_output(data->gpio_3v3en.gpio_num, 0);
		gpio_free(data->gpio_3v3en.gpio_num);
	}
	if(data->gpio_5ven.gpio_num != -1){
		gpio_direction_output(data->gpio_5ven.gpio_num, 0);
		gpio_free(data->gpio_5ven.gpio_num);
	}
	if(data->hub_rst.gpio_num != -1){
		gpio_direction_output(data->hub_rst.gpio_num, 0);
		gpio_free(data->hub_rst.gpio_num);
	}
	if(data->gpio_sdemmc.gpio_num != -1){
		gpio_direction_output(data->gpio_sdemmc.gpio_num, 0);
		gpio_free(data->gpio_sdemmc.gpio_num);
	}
	if(data->gpio_fan.gpio_num != -1){
		gpio_direction_output(data->gpio_fan.gpio_num, 0);
		gpio_free(data->gpio_fan.gpio_num);
	}
	if(data->sleep_flag != 0){
		wake_unlock(&data->rp_wake_lock);
	}

        return 0;
}

static int power_en_suspend(struct platform_device *pdev, pm_message_t state) 
{ 
//	struct platform_device *pdev = to_platform_device(dev);
        struct power_en_gpio *data = platform_get_drvdata(pdev);
 
	if(data->gpio_3v3en.gpio_num != -1){
		gpio_direction_output(data->gpio_3v3en.gpio_num, 0);
		gpio_set_value(data->gpio_3v3en.gpio_num,0);
	}
	if(data->gpio_5ven.gpio_num != -1){
		gpio_direction_output(data->gpio_5ven.gpio_num, 0);
		gpio_set_value(data->gpio_5ven.gpio_num,0);
	}
	if(data->hub_rst.gpio_num != -1){
		gpio_direction_output(data->hub_rst.gpio_num, 0);
		gpio_set_value(data->hub_rst.gpio_num,0);
	}
	if(data->gpio_sdemmc.gpio_num != -1){
		gpio_direction_output(data->gpio_sdemmc.gpio_num, 0);
		gpio_set_value(data->gpio_sdemmc.gpio_num,0);
	}
	if(data->gpio_fan.gpio_num != -1){
		gpio_direction_output(data->gpio_fan.gpio_num, 0);
		gpio_set_value(data->gpio_fan.gpio_num,0);
	}
 
        return 0; 
} 
 
static int power_en_resume(struct platform_device *pdev) 
{ 
//	struct platform_device *pdev = to_platform_device(dev);
        struct power_en_gpio *data = platform_get_drvdata(pdev);
 
	if(data->gpio_3v3en.gpio_num != -1){
			gpio_direction_output(data->gpio_3v3en.gpio_num, 1);
        		gpio_set_value(data->gpio_3v3en.gpio_num, 1);
	}
      
	if(data->gpio_5ven.gpio_num != -1){
			gpio_direction_output(data->gpio_5ven.gpio_num, 1);
  			gpio_set_value(data->gpio_5ven.gpio_num, 1);
	}
  
	if(data->hub_rst.gpio_num != -1){
			gpio_direction_output(data->hub_rst.gpio_num, 0);
        	gpio_set_value(data->hub_rst.gpio_num, 0);
			msleep(100);
			gpio_direction_output(data->hub_rst.gpio_num, 1);
        	gpio_set_value(data->hub_rst.gpio_num, 1);
	}
	if(data->gpio_sdemmc.gpio_num != -1){
			gpio_direction_output(data->gpio_sdemmc.gpio_num, 1);
  			gpio_set_value(data->gpio_sdemmc.gpio_num, 1);
	}
  
	if(data->gpio_fan.gpio_num != -1){
			gpio_direction_output(data->gpio_fan.gpio_num, 1);
  			gpio_set_value(data->gpio_fan.gpio_num, 1);
	}
        return 0; 
} 


static const struct of_device_id power_en_of_match[] = {
        { .compatible = "33v_en" },
        { }
};

static struct platform_driver power_en_driver = {
        .probe = power_en_probe,
        .remove = power_en_remove,
        .driver = {
                .name           = "power_en",
                .of_match_table = of_match_ptr(power_en_of_match),
				.owner   = THIS_MODULE,

        },
	.suspend = power_en_suspend,
	.resume = power_en_resume,
};


static int __init rpdzkj_gpio_init(void)
{	
	gpio_ctl_entry = proc_mkdir("hub_rst_proc", NULL);
	proc_create("hub_rst_",0666,gpio_ctl_entry,&hub_rst_proc);
	return platform_driver_register(&power_en_driver);
}
subsys_initcall(rpdzkj_gpio_init);
//arch_initcall(rpdzkj_gpio_init);

static void __exit rpdzkj_gpio_exit(void)
{
	platform_driver_unregister(&power_en_driver);
}
module_exit(rpdzkj_gpio_exit);


MODULE_LICENSE("GPL");
