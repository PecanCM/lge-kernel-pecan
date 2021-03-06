/* arch/arm/mach-msm/board-pecan-input.c
 * Copyright (C) 2009 LGE, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */



#include <linux/types.h>
#include <linux/list.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/i2c-gpio.h>
#include <linux/gpio_event.h>
#include <linux/keyreset.h>
#include <mach/gpio.h>
#include <mach/vreg.h>
#include <mach/board.h>
#include <mach/lge/board_lge.h>
#include <mach/rpc_server_handset.h>
#include <mach/pmic.h>
#include "proc_comm.h"
#include "../board-msm7627-regulator.h"

#include "board-pecan.h"

/* LGE_S [ynj.kim@lge.com] 2010-05-15 : atcmd virtual device */
static unsigned short atcmd_virtual_keycode[ATCMD_VIRTUAL_KEYPAD_ROW][ATCMD_VIRTUAL_KEYPAD_COL] = {
	{KEY_1, 		KEY_8, 				KEY_Q,  	 KEY_I,          KEY_D,      	KEY_HOME,	KEY_B,          KEY_UP},
	{KEY_2, 		KEY_9, 		  		KEY_W,		 KEY_O,       	 KEY_F,		 	KEY_RIGHTSHIFT, 	KEY_N,			KEY_DOWN},
	{KEY_3, 		KEY_0, 		  		KEY_E,		 KEY_P,          KEY_G,      	KEY_Z,        	KEY_M, 			KEY_UNKNOWN},
	{KEY_4, 		KEY_BACK,  			KEY_R,		 KEY_SEARCH,     KEY_H,			KEY_X,    		KEY_LEFTSHIFT,	KEY_UNKNOWN},
	{KEY_5, 		KEY_BACKSPACE, 		KEY_T,		 KEY_LEFTALT,    KEY_J,      	KEY_C,     		KEY_REPLY,    KEY_CAMERA},
	{KEY_6, 		KEY_ENTER,  		KEY_Y,  	 KEY_A,		     KEY_K,			KEY_V,  	    KEY_RIGHT,     	KEY_CAMERAFOCUS},
	{KEY_7, 		KEY_MENU,	KEY_U,  	 KEY_S,    		 KEY_L, 	    KEY_SPACE,      KEY_LEFT,     	KEY_SEND},
	{KEY_UNKNOWN, 	KEY_UNKNOWN,  		KEY_UNKNOWN, KEY_UNKNOWN, 	 KEY_UNKNOWN,	KEY_UNKNOWN,    KEY_FOLDER_MENU,      	KEY_FOLDER_HOME},

};

static struct atcmd_virtual_platform_data atcmd_virtual_pdata = {
	.keypad_row = ATCMD_VIRTUAL_KEYPAD_ROW,
	.keypad_col = ATCMD_VIRTUAL_KEYPAD_COL,
	.keycode = (unsigned char *)atcmd_virtual_keycode,
};

static struct platform_device atcmd_virtual_device = {
	.name = "atcmd_virtual_kbd",
	.id = -1,
	.dev = {
		.platform_data = &atcmd_virtual_pdata,
	},
};
/* LGE_E [ynj.kim@lge.com] 2010-05-15 : atcmd virtual device */

/* head set device */
static struct msm_handset_platform_data hs_platform_data = {
	.hs_name = "7k_handset",
	.pwr_key_delay_ms = 500, /* 0 will disable end key */
};

static struct platform_device hs_device = {
	.name   = "msm-handset",
	.id     = -1,
	.dev    = {
		.platform_data = &hs_platform_data,
	},
};

static unsigned int keypad_row_gpios[] = {
	38, 37
};

static unsigned int keypad_col_gpios[] = {32, 33};

#define KEYMAP_INDEX(col, row) ((col)*ARRAY_SIZE(keypad_row_gpios) + (row))

static const unsigned short keypad_keymap_pecan[] = {
/* pecan keymap by bongkyu.kim */
/* rev_c schematic reversed send, end key */
	[KEYMAP_INDEX(0, 0)] = KEY_END,
	[KEYMAP_INDEX(0, 1)] = KEY_VOLUMEUP,
	[KEYMAP_INDEX(1, 0)] = KEY_SEND,
	[KEYMAP_INDEX(1, 1)] = KEY_VOLUMEDOWN,

};

int pecan_matrix_info_wrapper(struct gpio_event_input_devs *input_dev, struct gpio_event_info *info, void **data, int func)
{
        int ret ;
		if (func == GPIO_EVENT_FUNC_RESUME) {
			gpio_tlmm_config(GPIO_CFG(keypad_row_gpios[0], 0,
						GPIO_CFG_INPUT, GPIO_CFG_PULL_UP,GPIO_CFG_2MA), GPIO_CFG_ENABLE);
			gpio_tlmm_config(GPIO_CFG(keypad_row_gpios[1], 0,
						GPIO_CFG_INPUT, GPIO_CFG_PULL_UP,GPIO_CFG_2MA), GPIO_CFG_ENABLE);
		}

		ret = gpio_event_matrix_func(input_dev, info, data, func);
        return ret ;
}

static int pecan_gpio_matrix_power(
                const struct gpio_event_platform_data *pdata, bool on)
{
	/* this is dummy function to make gpio_event driver register suspend function
	 * 2010-01-29, cleaneye.kim@lge.com
	 * copy from ALOHA code
	 * 2010-04-22 younchan.kim@lge.com
	 */

	return 0;
}

static struct gpio_event_matrix_info pecan_keypad_matrix_info = {
	.info.func	= pecan_matrix_info_wrapper,
	.keymap		= NULL,
	.output_gpios	= keypad_col_gpios,
	.input_gpios	= keypad_row_gpios,
	.noutputs	= ARRAY_SIZE(keypad_col_gpios),
	.ninputs	= ARRAY_SIZE(keypad_row_gpios),
	.settle_time.tv_nsec = 40 * NSEC_PER_USEC,
	.poll_time.tv_nsec = 20 * NSEC_PER_MSEC,
	.flags		= GPIOKPF_LEVEL_TRIGGERED_IRQ | GPIOKPF_PRINT_UNMAPPED_KEYS | GPIOKPF_DRIVE_INACTIVE
};

static void __init pecan_select_keymap(void)
{
	pecan_keypad_matrix_info.keymap = keypad_keymap_pecan;

	return;
}

static struct gpio_event_info *pecan_keypad_info[] = {
	&pecan_keypad_matrix_info.info
};

static struct gpio_event_platform_data pecan_keypad_data = {
	.name		= "pecan_keypad",
	.info		= pecan_keypad_info,
	.info_count	= ARRAY_SIZE(pecan_keypad_info),
	.power          = pecan_gpio_matrix_power,
};

struct platform_device keypad_device_pecan= {
	.name	= GPIO_EVENT_DEV_NAME,
	.id	= -1,
	.dev	= {
		.platform_data	= &pecan_keypad_data,
	},
};

/* keyreset platform device */
static int pecan_reset_keys_up[] = {
	KEY_HOME,
	0
};

static struct keyreset_platform_data pecan_reset_keys_pdata = {
	.keys_up = pecan_reset_keys_up,
	.keys_down = {
		//KEY_BACK,
		KEY_END,
		KEY_VOLUMEDOWN,
		0
	},
};

struct platform_device pecan_reset_keys_device = {
	.name = KEYRESET_NAME,
	.dev.platform_data = &pecan_reset_keys_pdata,
};

/* input platform device */
static struct platform_device *pecan_input_devices[] __initdata = {
	&hs_device,
	&keypad_device_pecan,
	//&pecan_reset_keys_device,
	&atcmd_virtual_device,
};

/* MCS6000 Touch */
static struct gpio_i2c_pin ts_i2c_pin[] = {
	[0] = {
		.sda_pin	= TS_GPIO_I2C_SDA,
		.scl_pin	= TS_GPIO_I2C_SCL,
		.reset_pin	= 0,
		.irq_pin	= TS_GPIO_IRQ,
	},
};

static struct i2c_gpio_platform_data ts_i2c_pdata = {
	.sda_is_open_drain	= 0,
	.scl_is_open_drain	= 0,
	.udelay			= 1,
};

static struct platform_device ts_i2c_device = {
	.name	= "i2c-gpio",
	.dev.platform_data = &ts_i2c_pdata,
};

static int ts_power_save_on;
static int ts_set_vreg(unsigned char onoff)
{
	int rc = 0;
	static struct regulator *vreg_touch;

	if (!ts_power_save_on) {
	  vreg_touch = regulator_get(0, "synt");
	  if (IS_ERR_OR_NULL(vreg_touch)) {
	    pr_err("could not get vreg_touch, rc = %ld\n",
		PTR_ERR(vreg_touch));
	    return -ENODEV;
	  }

	  rc = regulator_set_voltage(vreg_touch, 3050000, 3050000);
	  if (rc) {
	    pr_err("set_voltage vreg_touch failed, rc=%d\n", rc);
	    regulator_put(vreg_touch);
	    return -EINVAL;
	  }

	  ts_power_save_on = true;
	}

	printk("[Touch] %s() onoff:%d\n", __FUNCTION__, onoff);

	if (onoff) {
	  rc = regulator_enable(vreg_touch);
	  if (rc) {
	    pr_err("enable vreg_touch failed, rc=%d\n", rc);
	    return -ENODEV;
	  }
	} else {
	  rc = regulator_disable(vreg_touch);
	  if (rc) {
	    pr_err("disable vreg_touch failed, rc=%d\n", rc);
	    return -ENODEV;
	  }
	}
	return rc;
}

static struct touch_platform_data ts_pdata = {
	.ts_x_min = TS_X_MIN,
	.ts_x_max = TS_X_MAX,
	.ts_y_min = TS_Y_MIN,
	.ts_y_max = TS_Y_MAX,
	.power 	  = ts_set_vreg,
	.irq 	  = TS_GPIO_IRQ,
	.scl      = TS_GPIO_I2C_SCL,
	.sda      = TS_GPIO_I2C_SDA,
};

static struct i2c_board_info ts_i2c_bdinfo[] = {
	[0] = {
		I2C_BOARD_INFO("touch_mcs7000", TS_I2C_SLAVE_ADDR),
		.type = "touch_mcs7000",
		.platform_data = &ts_pdata,
	},
};

/* this routine should be checked for nessarry */
static int init_gpio_i2c_pin_touch(struct i2c_gpio_platform_data *i2c_adap_pdata,
		struct gpio_i2c_pin gpio_i2c_pin, struct i2c_board_info *i2c_board_info_data)
{
	i2c_adap_pdata->sda_pin = gpio_i2c_pin.sda_pin;
	i2c_adap_pdata->scl_pin = gpio_i2c_pin.scl_pin;

	gpio_tlmm_config(GPIO_CFG(gpio_i2c_pin.sda_pin, 0, GPIO_CFG_OUTPUT,
				GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	gpio_tlmm_config(GPIO_CFG(gpio_i2c_pin.scl_pin, 0, GPIO_CFG_OUTPUT,
				GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	gpio_set_value(gpio_i2c_pin.sda_pin, 1);
	gpio_set_value(gpio_i2c_pin.scl_pin, 1);

	if (gpio_i2c_pin.reset_pin) {
		gpio_tlmm_config(GPIO_CFG(gpio_i2c_pin.reset_pin, 0, GPIO_CFG_OUTPUT,
					GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
		gpio_set_value(gpio_i2c_pin.reset_pin, 1);
	}

	if (gpio_i2c_pin.irq_pin) {
		gpio_tlmm_config(GPIO_CFG(gpio_i2c_pin.irq_pin, 0, GPIO_CFG_INPUT,
					GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
		i2c_board_info_data->irq =
			MSM_GPIO_TO_INT(gpio_i2c_pin.irq_pin);
	}

  if (lge_bd_rev <= LGE_REV_C)
    gpio_tlmm_config(GPIO_CFG(28, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA), GPIO_CFG_ENABLE);

	return 0;
}


static void __init pecan_init_i2c_touch(int bus_num)
{
	ts_i2c_device.id = bus_num;

	init_gpio_i2c_pin_touch(&ts_i2c_pdata, ts_i2c_pin[0], &ts_i2c_bdinfo[0]);
	i2c_register_board_info(bus_num, &ts_i2c_bdinfo[0], 1);
	platform_device_register(&ts_i2c_device);
}

/* acceleration */
static int kr3dh_config_gpio(int config)
{
	if (config) {	/* for wake state */
		gpio_tlmm_config(GPIO_CFG(ACCEL_GPIO_I2C_SCL, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
		gpio_tlmm_config(GPIO_CFG(ACCEL_GPIO_I2C_SDA, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	} else {		/* for sleep state */
		gpio_tlmm_config(GPIO_CFG(ACCEL_GPIO_I2C_SCL, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
		gpio_tlmm_config(GPIO_CFG(ACCEL_GPIO_I2C_SDA, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	}

	return 0;
}

static int kr_init(void)
{
	return 0;
}

static void kr_exit(void)
{
}

static struct regulator *gp3_vreg;
static int Accel_Proximity_Ecompass_power_save_on;
static int Accel_Proximity_Ecompass_power_save(void)
{
	int rc = 0;

	if (!Accel_Proximity_Ecompass_power_save_on) {
		gp3_vreg = regulator_get(0, "gp3");
		if (IS_ERR_OR_NULL(gp3_vreg)) {
			pr_err("could not get gp3_vreg, rc = %ld\n",
				PTR_ERR(gp3_vreg));
			return -ENODEV;
		}

		rc = regulator_set_voltage(gp3_vreg, 3000000, 3000000);
		if (rc) {
			pr_err("set_voltage gp3_vreg failed, rc=%d\n", rc);
			regulator_put(gp3_vreg);
			return -EINVAL;
		}

		Accel_Proximity_Ecompass_power_save_on = true;
	}

	return rc;
}

static int accel_power_on(void)
{
	int rc = 0;
	Accel_Proximity_Ecompass_power_save();
	rc = regulator_enable(gp3_vreg);

	printk("[Accelerometer] %s() onoff:%d\n", __FUNCTION__, 1);

	if (rc) {
	  pr_err("enable gp3_vreg failed, rc=%d\n", rc);
	  return -ENODEV;
	}
	return rc;
}

static int accel_power_off(void)
{
	int rc = 0;
	Accel_Proximity_Ecompass_power_save();
	rc = regulator_disable(gp3_vreg);

	printk("[Accelerometer] %s() onoff:%d\n", __FUNCTION__, 0);

	if (rc) {
	  pr_err("disable gp3_vreg failed, rc=%d\n", rc);
	  return -ENODEV;
	}
	return rc;
}

struct kr3dh_platform_data kr3dh_data = {
	.poll_interval = 100,
	.min_interval = 0,
	.g_range = 0x00,
	.axis_map_x = 0,
	.axis_map_y = 1,
	.axis_map_z = 2,

	.negate_x = 0,
	.negate_y = 0,
	.negate_z = 0,

	.irq_pin = ACCEL_GPIO_INT,

	.power_on = accel_power_on,
	.power_off = accel_power_off,
	.kr_init = kr_init,
	.kr_exit = kr_exit,
	.gpio_config = kr3dh_config_gpio,
};

static struct gpio_i2c_pin accel_i2c_pin[] = {
	[0] = {
		.sda_pin	= ACCEL_GPIO_I2C_SDA,
		.scl_pin	= ACCEL_GPIO_I2C_SCL,
		.reset_pin	= 0,
		.irq_pin	= ACCEL_GPIO_INT,
	},
};

static struct i2c_gpio_platform_data accel_i2c_pdata = {
	.sda_is_open_drain = 0,
	.scl_is_open_drain = 0,
	.udelay = 2,
};

static struct platform_device accel_i2c_device = {
	.name = "i2c-gpio",
	.dev.platform_data = &accel_i2c_pdata,
};

static struct i2c_board_info accel_i2c_bdinfo[] = {
	[1] = {
		I2C_BOARD_INFO("KR3DH", ACCEL_I2C_ADDRESS_H),
		.type = "KR3DH",
		.platform_data = &kr3dh_data,
	},
	[0] = {
		I2C_BOARD_INFO("KR3DM", ACCEL_I2C_ADDRESS),
		.type = "KR3DM",
		.platform_data = &kr3dh_data,
	},
};

static void __init pecan_init_i2c_acceleration(int bus_num)
{
	accel_i2c_device.id = bus_num;

	init_gpio_i2c_pin(&accel_i2c_pdata, accel_i2c_pin[0], &accel_i2c_bdinfo[0]);

//	if(lge_bd_rev >= LGE_REV_11)
		i2c_register_board_info(bus_num, &accel_i2c_bdinfo[1], 1);	/* KR3DH */
//	else
//		i2c_register_board_info(bus_num, &accel_i2c_bdinfo[0], 1);	/* KR3DM */

	platform_device_register(&accel_i2c_device);
}

/* proximity & ecompass */
static int ecom_power_set(unsigned char onoff)
{
	int rc = 0;
	Accel_Proximity_Ecompass_power_save();

	printk("[Ecompass] %s() onoff:%d\n", __FUNCTION__, 1);

	if (onoff) {
	  rc = regulator_enable(gp3_vreg);
	  if (rc) {
	    pr_err("enable gp3_vreg failed, rc=%d\n", rc);
	    return -ENODEV;
	  }
	}
	else {
	  rc = regulator_disable(gp3_vreg);
	  if (rc) {
	    pr_err("disable gp3_vreg failed, rc=%d\n", rc);
	    return -ENODEV;
	  }
	}

	return rc;
}

static struct ecom_platform_data ecom_pdata = {
	.pin_int        	= ECOM_GPIO_INT,
	.pin_rst		= 0,
	.power          	= ecom_power_set,
};

static struct i2c_board_info prox_ecom_i2c_bdinfo[] = {
	[0] = {
		I2C_BOARD_INFO("ami304_sensor", ECOM_I2C_ADDRESS),
		.type = "ami304_sensor",
		.platform_data = &ecom_pdata,
	},
};

static struct gpio_i2c_pin proxi_ecom_i2c_pin[] = {
	[0] = {
		.sda_pin	= ECOM_GPIO_I2C_SDA,
		.scl_pin	= ECOM_GPIO_I2C_SCL,
		.reset_pin	= 0,
		.irq_pin	= ECOM_GPIO_INT,
	},
};

static struct i2c_gpio_platform_data proxi_ecom_i2c_pdata = {
	.sda_is_open_drain = 0,
	.scl_is_open_drain = 0,
	.udelay = 2,
};

static struct platform_device proxi_ecom_i2c_device = {
        .name = "i2c-gpio",
        .dev.platform_data = &proxi_ecom_i2c_pdata,
};


static void __init pecan_init_i2c_prox_ecom(int bus_num)
{
	proxi_ecom_i2c_device.id = bus_num;

	init_gpio_i2c_pin(&proxi_ecom_i2c_pdata, proxi_ecom_i2c_pin[0], &prox_ecom_i2c_bdinfo[0]);

	i2c_register_board_info(bus_num, &prox_ecom_i2c_bdinfo[0], 1);
	platform_device_register(&proxi_ecom_i2c_device);
}

/* common function */
void __init lge_add_input_devices(void)
{
	pecan_select_keymap();
	platform_add_devices(pecan_input_devices, ARRAY_SIZE(pecan_input_devices));

	lge_add_gpio_i2c_device(pecan_init_i2c_touch);
	lge_add_gpio_i2c_device(pecan_init_i2c_prox_ecom);
	lge_add_gpio_i2c_device(pecan_init_i2c_acceleration);
}
