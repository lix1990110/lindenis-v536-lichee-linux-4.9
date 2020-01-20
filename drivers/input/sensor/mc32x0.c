/*****************************************************************************
 *
 * Copyright (c) 2013 mCube, Inc.  All rights reserved.
 *
 * This source is subject to the mCube Software License.
 * This software is protected by Copyright and the information and source code
 * contained herein is confidential. The software including the source code
 * may not be copied and the information contained herein may not be used or
 * disclosed except with the written permission of mCube Inc.
 *
 * All other rights reserved.
 *
 * This code and information are provided "as is" without warranty of any
 * kind, either expressed or implied, including but not limited to the
 * implied warranties of merchantability and/or fitness for a
 * particular purpose.
 *
 * The following software/firmware and/or related documentation ("mCube Software")
 * have been modified by mCube Inc. All revisions are subject to any receiver's
 * applicable license agreements with mCube Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 *
 *****************************************************************************/ 
 
#include <linux/module.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/mutex.h>
//#include <linux/earlysuspend.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <linux/miscdevice.h>

#include "../init-input.h"
//#include <mach/hardware.h>
#include <linux/fs.h>

//=== CONFIGURATIONS ==========================================================
#define DOT_CALI
#define _MC32X0_DEBUG_ON_

//=============================================================================
#ifdef _MC32X0_DEBUG_ON_
    #define mcprintkreg(x...)     //printk(x)
    #define mcprintkfunc(x...)    //printk(x)
    #define GSE_ERR(x...) 	      //printk(x)
    #define GSE_LOG(x...) 	      //printk(x)
#else
    #define mcprintkreg(x...)
    #define mcprintkfunc(x...)
    #define GSE_ERR(x...)
    #define GSE_LOG(x...)
#endif

//=============================================================================
#define SENSOR_NAME 			"mc32x0"
#define SENSOR_DRIVER_VERSION    "2.0.0"
#define SENSOR_DATA_SIZE	3
#define AVG_NUM 16
/* Addresses to scan */
//static const unsigned short normal_i2c[2] = {0x00,I2C_CLIENT_END};

static __u32 twi_id = 0;
//volatile unsigned char mc32x0_on_off=0;
//static int mc32x0_pin_hd;
static char mc32x0_on_off_str[32];
#define G_0		ABS_X
#define G_1		ABS_Y
#define G_2		ABS_Z
#define G_0_REVERSE        1
#define G_1_REVERSE        1
#define G_2_REVERSE        1

#define GRAVITY_1G_VALUE    1000
#define ZERO_FIR            5
//#define CONFIG_HAS_LOW_PASS_FILTER

static unsigned char    s_bPCODE      = 0x00;


//=============================================================================
#define SENSOR_DMARD_IOCTL_BASE 		234

#define IOCTL_SENSOR_SET_DELAY_ACCEL   	_IO(SENSOR_DMARD_IOCTL_BASE, 100)
#define IOCTL_SENSOR_GET_DELAY_ACCEL   	_IO(SENSOR_DMARD_IOCTL_BASE, 101)
#define IOCTL_SENSOR_GET_STATE_ACCEL   	_IO(SENSOR_DMARD_IOCTL_BASE, 102)
#define IOCTL_SENSOR_SET_STATE_ACCEL		_IO(SENSOR_DMARD_IOCTL_BASE, 103)
#define IOCTL_SENSOR_GET_DATA_ACCEL		_IO(SENSOR_DMARD_IOCTL_BASE, 104)

#define IOCTL_MSENSOR_SET_DELAY_MAGNE   	_IO(SENSOR_DMARD_IOCTL_BASE, 200)
#define IOCTL_MSENSOR_GET_DATA_MAGNE		_IO(SENSOR_DMARD_IOCTL_BASE, 201)
#define IOCTL_MSENSOR_GET_STATE_MAGNE   	_IO(SENSOR_DMARD_IOCTL_BASE, 202)
#define IOCTL_MSENSOR_SET_STATE_MAGNE	_IO(SENSOR_DMARD_IOCTL_BASE, 203)

#define IOCTL_SENSOR_GET_NAME   _IO(SENSOR_DMARD_IOCTL_BASE, 301)
#define IOCTL_SENSOR_GET_VENDOR   _IO(SENSOR_DMARD_IOCTL_BASE, 302)

#define IOCTL_SENSOR_GET_CONVERT_PARA   _IO(SENSOR_DMARD_IOCTL_BASE, 401)

#define SENSOR_CALIBRATION   	_IOWR(SENSOR_DMARD_IOCTL_BASE,  402, int[SENSOR_DATA_SIZE])

//=============================================================================
#define mc32x0_CONVERT_PARAMETER    (1.5f * (9.80665f) / 256.0f)
#define mc32x0_DISPLAY_NAME         SENSOR_NAME
#define mc32x0_DIPLAY_VENDOR        "mCube"

#define X_OUT 					0x41
#define CONTROL_REGISTER		0x44
#define SW_RESET 				0x53
#define WHO_AM_I 				0x0f
#define WHO_AM_I_VALUE 		0x06

#define MC32X0_AXIS_X		   0
#define MC32X0_AXIS_Y		   1
#define MC32X0_AXIS_Z		   2
#define MC32X0_AXES_NUM 	   3
#define MC32X0_DATA_LEN 	   6

#define MC32X0_XOUT_REG						0x00
#define MC32X0_YOUT_REG						0x01
#define MC32X0_ZOUT_REG						0x02
#define MC32X0_Tilt_Status_REG				0x03
#define MC32X0_Sampling_Rate_Status_REG		0x04
#define MC32X0_Sleep_Count_REG				0x05
#define MC32X0_Interrupt_Enable_REG			0x06
#define MC32X0_Mode_Feature_REG				0x07
#define MC32X0_Sample_Rate_REG				0x08
#define MC32X0_Tap_Detection_Enable_REG		0x09
#define MC32X0_TAP_Dwell_Reject_REG			0x0a
#define MC32X0_DROP_Control_Register_REG	0x0b
#define MC32X0_SHAKE_Debounce_REG			0x0c
#define MC32X0_XOUT_EX_L_REG				0x0d
#define MC32X0_XOUT_EX_H_REG				0x0e
#define MC32X0_YOUT_EX_L_REG				0x0f
#define MC32X0_YOUT_EX_H_REG				0x10
#define MC32X0_ZOUT_EX_L_REG				0x11
#define MC32X0_ZOUT_EX_H_REG				0x12
#define MC32X0_CHIP_ID_REG					0x18
#define MC32X0_RANGE_Control_REG			0x20
#define MC32X0_SHAKE_Threshold_REG			0x2B
#define MC32X0_UD_Z_TH_REG					0x2C
#define MC32X0_UD_X_TH_REG					0x2D
#define MC32X0_RL_Z_TH_REG					0x2E
#define MC32X0_RL_Y_TH_REG					0x2F
#define MC32X0_FB_Z_TH_REG					0x30
#define MC32X0_DROP_Threshold_REG			0x31
#define MC32X0_TAP_Threshold_REG			0x32

#define MC32X0_HIGH_END    0x01
/*******MC3210/20 define this**********/
#define MCUBE_8G_14BIT     0x10

#define SUPPORT_VIRTUAL_Z_SENSOR
#define LOW_RESOLUTION 1
#define HIGH_RESOLUTION 2
#define RBM_RESOLUTION 3
#ifdef SUPPORT_VIRTUAL_Z_SENSOR
#define Low_Pos_Max 127
#define Low_Neg_Max -128
#define High_Pos_Max 8191
#define High_Neg_Max -8192
#define VIRTUAL_Z	1
static int Railed = 0;
#else
#define VIRTUAL_Z	0
#endif
#define MC32X0_LOW_END 		0x02
/*******mc32x0 define this**********/
#define MCUBE_1_5G_8BIT    0x20
#define MC32X0_MODE_DEF    0x43

#define MC32X0ADDRESS           0x4c

#define mc32x0_I2C_NAME            SENSOR_NAME
#define SENSOR_DEV_COUNT	        1
#define SENSOR_DURATION_MAX                        200
#define SENSOR_DURATION_MIN                        10
#define SENSOR_DURATION_DEFAULT    20 //100

#define MAX_RETRY				20
#define INPUT_FUZZ  0
#define INPUT_FLAT  0

#define AUTO_CALIBRATION 0


//=============================================================================
static unsigned char is_new_mc34x0 = 0;
static unsigned char is_mc3250 = 0;
static unsigned char is_mc35xx = 0;
static unsigned char IS_MC2234=0;
static unsigned char  McubeID = 0;

//=============================================================================
#ifdef DOT_CALI
#define CALIB_PATH				"/data/data/com.mcube.acc/files/mcube-calib.txt"
//MCUBE_BACKUP_FILE
#define BACKUP_CALIB_PATH		"/data/misc/sensors/mcube-calib.txt"
static char backup_buf[64];
//MCUBE_BACKUP_FILE
#define DATA_PATH			   "/sdcard/mcube-register-map.txt"

typedef struct {
	unsigned short	x;		/**< X axis */
	unsigned short	y;		/**< Y axis */
	unsigned short	z;		/**< Z axis */
} GSENSOR_VECTOR3D;

static GSENSOR_VECTOR3D gsensor_gain = { 0 };
static struct miscdevice mc32x0_device;

static struct file * fd_file = NULL;

static mm_segment_t oldfs = { 0 };
static unsigned char offset_buf[9] = { 0 };
static signed int offset_data[3] = { 0 };
s16 G_RAW_DATA[3] = { 0 };
static signed int gain_data[3] = { 0 };
static signed int enable_RBM_calibration = 0;

//static unsigned char    s_bMPOL       = 0x00;//add by sheen

#define GSENSOR                                0x95
#define GSENSOR_IOCTL_INIT                     _IO(GSENSOR,  0x01)
#define GSENSOR_IOCTL_READ_CHIPINFO            _IOR(GSENSOR, 0x02, int)
#define GSENSOR_IOCTL_READ_SENSORDATA          _IOR(GSENSOR, 0x03, int)
#define GSENSOR_IOCTL_READ_OFFSET              _IOR(GSENSOR, 0x04, GSENSOR_VECTOR3D)
#define GSENSOR_IOCTL_READ_GAIN                _IOR(GSENSOR, 0x05, GSENSOR_VECTOR3D)
#define GSENSOR_IOCTL_READ_RAW_DATA            _IOR(GSENSOR, 0x06, int)
//#define GSENSOR_IOCTL_SET_CALI                 _IOW(GSENSOR, 0x06, SENSOR_DATA)
#define GSENSOR_IOCTL_GET_CALI                 _IOW(GSENSOR, 0x07, SENSOR_DATA)
#define GSENSOR_IOCTL_CLR_CALI                 _IO(GSENSOR, 0x08)
#define GSENSOR_MCUBE_IOCTL_READ_RBM_DATA      _IOR(GSENSOR, 0x09, SENSOR_DATA)
#define GSENSOR_MCUBE_IOCTL_SET_RBM_MODE       _IO(GSENSOR, 0x0a)
#define GSENSOR_MCUBE_IOCTL_CLEAR_RBM_MODE     _IO(GSENSOR, 0x0b)
#define GSENSOR_MCUBE_IOCTL_SET_CALI           _IOW(GSENSOR, 0x0c, SENSOR_DATA)
#define GSENSOR_MCUBE_IOCTL_REGISTER_MAP       _IO(GSENSOR, 0x0d)
#define GSENSOR_IOCTL_SET_CALI_MODE            _IOW(GSENSOR, 0x0e,int)
#define GSENSOR_MCUBE_IOCTL_READ_PRODUCT_ID    _IOR(GSENSOR, 0x0f, int)
#define GSENSOR_MCUBE_IOCTL_READ_FILEPATH      _IOR(GSENSOR, 0x10, char[256])

static int MC32X0_ReadRegMap(struct i2c_client *client, u8 *pbUserBuf);


typedef struct{
	int x;
	int y;
	int z;
}SENSOR_DATA;

static int load_cali_flg = 0;
//MCUBE_BACKUP_FILE
static bool READ_FROM_BACKUP = false;
//MCUBE_BACKUP_FILE

#endif  // END OF #ifdef DOT_CALI

//=============================================================================
#define MC32X0_WAKE						1
#define MC32X0_SNIFF					2
#define MC32X0_STANDBY					3

struct dev_data {
	struct i2c_client *client;
};

static struct dev_data dev = { 0 };

/* Addresses to scan */
static const unsigned short normal_i2c[2] = {MC32X0ADDRESS, I2C_CLIENT_END};

struct acceleration {
	int x;
	int y;
	int z;
};

//void gsensor_write_offset_to_file(void);
//void gsensor_read_offset_from_file(void);
//char OffsetFileName[] = "/data/misc/dmt/offset.txt";
static bool z_flag_report = false;  //for gms

static struct sensor_config_info gsensor_info = {
	.input_type = GSENSOR_TYPE,
};

static u32 debug_mask = 0;
#define dprintk(level_mask, fmt, arg...)	if (unlikely(debug_mask & level_mask)) \
	printk(KERN_DEBUG fmt , ## arg)

module_param_named(debug_mask, debug_mask, int, 0644);


enum {
	DEBUG_INIT = 1U << 0,
	DEBUG_CONTROL_INFO = 1U << 1,
	DEBUG_DATA_INFO = 1U << 2,
	DEBUG_SUSPEND = 1U << 3,
};

struct mc32x0_data {
	struct mutex lock;
	struct i2c_client *client;
	struct work_struct  work;
	struct workqueue_struct *mc32x0_wq;
	struct hrtimer timer;
	struct device *device;
	struct input_dev *input_dev;
	int use_count;
	int enabled;
	volatile unsigned int duration;
	int use_irq; 
	int irq;
	unsigned long irqflags;
	int gpio;
	unsigned int map[3];
	int inv[3];
/*
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend;
#endif
*/
};

//=============================================================================
enum mc3xx0_orientation
{
    MC3XX0_TOP_LEFT_DOWN = 0,
    MC3XX0_TOP_RIGHT_DOWN,
    MC3XX0_TOP_RIGHT_UP,
    MC3XX0_TOP_LEFT_UP,
    MC3XX0_BOTTOM_LEFT_DOWN,
    MC3XX0_BOTTOM_RIGHT_DOWN,
    MC3XX0_BOTTOM_RIGHT_UP,
    MC3XX0_BOTTOM_LEFT_UP
};

enum mc3xx0_axis
{
    MC3XX0_AXIS_X = 0,
    MC3XX0_AXIS_Y,
    MC3XX0_AXIS_Z,
    MC3XX0_AXIS_NUM
};

struct mc3xx0_hwmsen_convert
{
    signed char sign[3];
    unsigned char map[3];
};

// Transformation matrix for chip mounting position
static const struct mc3xx0_hwmsen_convert mc3xx0_cvt[] =
{
    {{ 1,  1,  1}, {MC3XX0_AXIS_X, MC3XX0_AXIS_Y, MC3XX0_AXIS_Z}},    // 0: top   , left-down
    {{-1,  1,  1}, {MC3XX0_AXIS_Y, MC3XX0_AXIS_X, MC3XX0_AXIS_Z}},    // 1: top   , right-down
    {{-1, -1,  1}, {MC3XX0_AXIS_X, MC3XX0_AXIS_Y, MC3XX0_AXIS_Z}},    // 2: top   , right-up
    {{ 1, -1,  1}, {MC3XX0_AXIS_Y, MC3XX0_AXIS_X, MC3XX0_AXIS_Z}},    // 3: top   , left-up
    {{-1,  1, -1}, {MC3XX0_AXIS_X, MC3XX0_AXIS_Y, MC3XX0_AXIS_Z}},    // 4: bottom, left-down
    {{ 1,  1, -1}, {MC3XX0_AXIS_Y, MC3XX0_AXIS_X, MC3XX0_AXIS_Z}},    // 5: bottom, right-down
    {{ 1, -1, -1}, {MC3XX0_AXIS_X, MC3XX0_AXIS_Y, MC3XX0_AXIS_Z}},    // 6: bottom, right-up
    {{-1, -1, -1}, {MC3XX0_AXIS_Y, MC3XX0_AXIS_X, MC3XX0_AXIS_Z}},    // 7: bottom, left-up
};

static unsigned char mc3xx0_current_placement = MC3XX0_TOP_RIGHT_UP; // current soldered placement
static float convert_para = mc32x0_CONVERT_PARAMETER;

//=============================================================================
volatile static short sensor_duration = SENSOR_DURATION_DEFAULT;
volatile static short sensor_state_flag = 1;

//=============================================================================
static ssize_t mc32x0_map_show(struct device *dev, struct device_attribute *attr,char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct mc32x0_data *data = NULL;
	int i = 0;
	data = i2c_get_clientdata(client);
	for (i = 0; i< 3; i++)
	{
		if(data->inv[i] == 1)
		{
			switch(data->map[i])
			{
				case ABS_X:
					buf[i] = 'x';
					break;
				case ABS_Y:
					buf[i] = 'y';
					break;
				case ABS_Z:
					buf[i] = 'z';
					break;
				default:
					buf[i] = '_';
					break;
			}
		}
		else
		{
			switch(data->map[i])
			{
				case ABS_X:
					buf[i] = 'X';
					break;
				case ABS_Y:
					buf[i] = 'Y';
					break;
				case ABS_Z:
					buf[i] = 'Z';
					break;
				default:
					buf[i] = '-';
					break;
			}
		}
	}
	sprintf(buf+3,"\r\n");
	return 5;
}

//Function as i2c_master_send, and return 1 if operation is successful. 
static int i2c_write_bytes(struct i2c_client *client, uint8_t *data, uint16_t len)
{
	struct i2c_msg msg;
	int ret=-1;
	
	msg.flags = !I2C_M_RD;
	msg.addr = client->addr;
	msg.len = len;
	msg.buf = data;		
	
	ret=i2c_transfer(client->adapter, &msg,1);
	return ret;
}

static bool gsensor_i2c_test(struct i2c_client * client)
{
	int ret, retry;
	uint8_t test_data[1] = { 0 };	//only write a data address.
	
	for(retry=0; retry < 2; retry++)
	{
		ret =i2c_write_bytes(client, test_data, 1);	//Test i2c.
		if (ret == 1)
			break;
		msleep(5);
	}
	
	return ret==1 ? true : false;
}

/**
 * gsensor_detect - Device detection callback for automatic device creation
 * return value:  
 *                    = 0; success;
 *                    < 0; err
 */
static int gsensor_detect(struct i2c_client *client, struct i2c_board_info *info)
{
	struct i2c_adapter *adapter = client->adapter;
	int ret;
	
	dprintk(DEBUG_INIT, "%s enter \n", __func__);
	
	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA))
        return -ENODEV;
    
	if(twi_id == adapter->nr){
            pr_info("%s: addr= %x\n",__func__,client->addr);

            ret = gsensor_i2c_test(client);
        	if(!ret){
        		pr_info("%s:I2C connection might be something wrong or maybe the other gsensor equipment! \n",__func__);
        		return -ENODEV;
        	}else{           	    
            	pr_info("I2C connection sucess!\n");
            	strlcpy(info->type, SENSOR_NAME, I2C_NAME_SIZE);
    		    return 0;	
	             }

	}else{
		return -ENODEV;
	}
}

int mc32x0_set_image (struct i2c_client *client) 
{
	unsigned char data = 0;

	//add by sheen
	//data = i2c_smbus_read_byte_data(client, 0x2A);
	//s_bMPOL = (data & 0x03);
    //end
	data = i2c_smbus_read_byte_data(client, 0x3B);
	s_bPCODE =data;
	if((data == 0x19)||(data == 0x29))
		McubeID = 0x22;
	else if((data == 0x90)||(data == 0xA8)||(data == 0x88))
		McubeID = 0x11;
	else
		McubeID = 0;

    if (0x88 == data)
        is_mc3250 = 1;
    if (0x59 == data)
	{
		McubeID = 0x22;		
		IS_MC2234 = 1;
	}

	if (0x39 == data)
	{
		McubeID = 0x22;		
		is_new_mc34x0 = 1;
	}
	else if (0xB8 == data)
	{
		McubeID = 0x11;
		is_new_mc34x0 = 1;
	}
	if((0x40 == data) ||  (0x10 == (data&0XF1)))
	{
		McubeID = 0x11;
		is_mc35xx = 1;
	}
	if((0x30 == data) ||(0x6E == (data | 0x0E)))
	{
		McubeID = 0x22;
		is_mc35xx = 1;
	}
	if(McubeID & MCUBE_8G_14BIT)
	{
		data = MC32X0_MODE_DEF;
	  	i2c_smbus_write_byte_data(client, MC32X0_Mode_Feature_REG, data);
		data = 0x00;
	  	i2c_smbus_write_byte_data(client, MC32X0_Sleep_Count_REG, data);
		data = 0x00;
	  	i2c_smbus_write_byte_data(client, MC32X0_Sample_Rate_REG, data);
		data = 0x00;
	  	i2c_smbus_write_byte_data(client, MC32X0_Tap_Detection_Enable_REG, data);
		data = 0x3F;
    if (is_mc35xx)
    {	
    	data = 0xA5;
    }	
	  	i2c_smbus_write_byte_data(client, MC32X0_RANGE_Control_REG, data);
		data = 0x00;
		i2c_smbus_write_byte_data(client, MC32X0_Interrupt_Enable_REG, data);	

        #ifdef DOT_CALI
            gsensor_gain.x = gsensor_gain.y = gsensor_gain.z = 1024;
        #endif
	}
	else if(McubeID & MCUBE_1_5G_8BIT)
	{		
		data = MC32X0_MODE_DEF;
		i2c_smbus_write_byte_data(client, MC32X0_Mode_Feature_REG, data);
		data = 0x00;
		i2c_smbus_write_byte_data(client, MC32X0_Sleep_Count_REG, data);
		data = 0x00;
		i2c_smbus_write_byte_data(client, MC32X0_Sample_Rate_REG, data);
		data = 0x00;
		i2c_smbus_write_byte_data(client, MC32X0_Tap_Detection_Enable_REG, data);
		data = 0x02;
		i2c_smbus_write_byte_data(client, MC32X0_RANGE_Control_REG, data);
		data = 0x00;
		i2c_smbus_write_byte_data(client, MC32X0_Interrupt_Enable_REG, data);

        #ifdef DOT_CALI
            gsensor_gain.x = gsensor_gain.y = gsensor_gain.z = 86;
            if (is_mc35xx)
            {
            	gsensor_gain.x = gsensor_gain.y = gsensor_gain.z = 64;
            }
        #endif
	}

	data = 0x41;
	i2c_smbus_write_byte_data(client, MC32X0_Mode_Feature_REG, data);	

	return 0;
}

/*****************************************
 *** show_regiter_map
 *****************************************/
static ssize_t show_regiter_map(struct device *dev, struct device_attribute *attr, char *buf)
{
    u8         _bIndex = 0;
    u8         _baRegMap[64] = { 0 };
    ssize_t    _tLength = 0;

   struct i2c_client *client = to_i2c_client(dev);

    MC32X0_ReadRegMap(client, _baRegMap);

    for (_bIndex = 0; _bIndex < 64; _bIndex++)
        _tLength += snprintf((buf + _tLength), (PAGE_SIZE - _tLength), "Reg[0x%02X]: 0x%02X\n", _bIndex, _baRegMap[_bIndex]); 

    return (_tLength);
}

static ssize_t mc32x0_map_store(struct device *dev, struct device_attribute *attr,const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct mc32x0_data *data = NULL;
	int i = 0;
	data = i2c_get_clientdata(client);

	if(count < 3) return -EINVAL;

	for(i = 0; i< 3; i++)
	{
		switch(buf[i])
		{
			case 'x':
				data->map[i] = ABS_X;
				data->inv[i] = 1;
				break;
			case 'y':
				data->map[i] = ABS_Y;
				data->inv[i] = 1;
				break;
			case 'z':
				data->map[i] = ABS_Z;
				data->inv[i] = 1;
				break;
			case 'X':
				data->map[i] = ABS_X;
				data->inv[i] = -1;
				break;
			case 'Y':
				data->map[i] = ABS_Y;
				data->inv[i] = -1;
				break;
			case 'Z':
				data->map[i] = ABS_Z;
				data->inv[i] = -1;
				break;
			default:
				return -EINVAL;
		}
	}

	return count;
}

static int mc32x0_enable(struct mc32x0_data *data, int enable);

static ssize_t mc32x0_enable_show(struct device *dev,
                struct device_attribute *attr, char *buf)
{        
        struct i2c_client *client = container_of(mc32x0_device.parent, struct i2c_client, dev);

        struct mc32x0_data *mc32x0 = i2c_get_clientdata(client);

        return sprintf(buf, "%d\n", mc32x0->enabled);
}

static ssize_t mc32x0_enable_store(struct device *dev,
                struct device_attribute *attr,
                const char *buf, size_t count)
{
        bool new_enable;

        struct i2c_client *client = container_of(mc32x0_device.parent, struct i2c_client, dev);
        
        struct mc32x0_data *mc32x0 = i2c_get_clientdata(client);

        if (sysfs_streq(buf, "1"))
                new_enable = true;
        else if (sysfs_streq(buf, "0"))
                new_enable = false;
        else {
                pr_debug("%s: invalid value %d\n", __func__, *buf);
                return -EINVAL;
        }

        mc32x0_enable(mc32x0, new_enable);

        return count;
}

static ssize_t mc32x0_delay_show(struct device *dev,
                struct device_attribute *attr, char *buf)
{
        return sprintf(buf, "%d\n", sensor_duration);
}

static ssize_t mc32x0_delay_store(struct device *dev,
                struct device_attribute *attr,
                const char *buf, size_t count)
{
        unsigned long data;
        int error;

        data = simple_strtoul(buf, NULL, 10);
        //if (error)
        //        return error;
        if (data > SENSOR_DURATION_MAX)
                data = SENSOR_DURATION_MAX;
        if (data < SENSOR_DURATION_MIN)
                data = SENSOR_DURATION_MIN;
        sensor_duration = data;

        return count;
}

static DEVICE_ATTR(map, 0660, mc32x0_map_show, mc32x0_map_store);
static DEVICE_ATTR(enable, 0660, mc32x0_enable_show, mc32x0_enable_store);
static DEVICE_ATTR(delay, 0660, mc32x0_delay_show, mc32x0_delay_store);

static struct attribute* mc32x0_attrs[] =
{
        &dev_attr_map.attr,
        &dev_attr_enable.attr,
        &dev_attr_delay.attr,
        NULL
};

static const struct attribute_group mc32x0_group =
{
	.attrs = mc32x0_attrs,
};

//=============================================================================
static int mc32x0_chip_init(struct i2c_client *client)
{
	mc32x0_set_image(client);

	return 0;
}

int mc32x0_set_mode(struct i2c_client *client, unsigned char mode) 
{
	int comres = 0;
	unsigned char data = 0;

	if (mode < 4)
	{
		data = (0x40 | mode);
		comres = i2c_smbus_write_byte_data(client, MC32X0_Mode_Feature_REG, data);
	} 

	return comres;
}


#ifdef SUPPORT_VIRTUAL_Z_SENSOR
int Verify_Z_Railed(int AccData, int resolution)
{
	int status = 0;
	GSE_LOG("%s: AccData = %d",__func__, AccData);
	if(resolution == 1) // Low resolution
	{
		if((AccData >= Low_Pos_Max && AccData >=0)|| (AccData <= Low_Neg_Max && AccData < 0))
		{
			status = 1;
			GSE_LOG("%s: Railed at Low Resolution",__func__);
		}
	}
	else if (resolution == 2)	//High resolution
	{
		if((AccData >= High_Pos_Max && AccData >=0) || (AccData <= High_Neg_Max && AccData < 0))
		{
			status = 1;
			GSE_LOG("%s: Railed at High Resolution",__func__);
		}
	}
	else if (resolution == 3)	//High resolution
	{
		if((AccData >= Low_Pos_Max*3 && AccData >=0) || (AccData <= Low_Neg_Max*3 && AccData < 0))
		{
			status = 1;
			GSE_LOG("%s: Railed at High Resolution",__func__);
		}
	}
	else
		GSE_LOG("%s, Wrong resolution",__func__);

	return status;
}

int SquareRoot(int x) 
{

    if(x < 0) return -1;
    if(x == 0 || x == 1) return x;
    int lowerbound = 1;
    int upperbound = x;
    int root = lowerbound + (upperbound - lowerbound)/2;

    while(root > x/root || root+1 <= x/(root+1))
    {
        if(root > x/root)
        {
            upperbound = root;
        } 
        else 
        {
            lowerbound = root;
        }
        root = lowerbound + (upperbound - lowerbound)/2;
    }
    GSE_LOG("%s: Sqrt root is %d",__func__, root);
    return root;
}
#endif
#ifdef DOT_CALI
//=============================================================================
struct file *openFile(char *path,int flag,int mode) 
{ 
	struct file *fp = NULL; 
	 
	fp = filp_open(path, flag, mode); 

	if (IS_ERR(fp) || !fp->f_op) 
	{
		GSE_LOG("Calibration File filp_open return NULL\n");
		return NULL; 
	}

	return fp; 
} 
 
//=============================================================================
int readFile(struct file *fp,char *buf,int readlen) 
{ 
	if (fp->f_op && fp->f_op->read) 
		return fp->f_op->read(fp,buf,readlen, &fp->f_pos); 
	else 
		return -1; 
} 

//=============================================================================
int writeFile(struct file *fp,char *buf,int writelen) 
{ 
	if (fp->f_op && fp->f_op->write) 
		return fp->f_op->write(fp,buf,writelen, &fp->f_pos); 
	else 
		return -1; 
}
 
//=============================================================================
int closeFile(struct file *fp) 
{ 
	filp_close(fp,NULL); 

	return 0; 
} 

//=============================================================================
void initKernelEnv(void) 
{ 
	oldfs = get_fs(); 
	set_fs(KERNEL_DS);
	printk(KERN_INFO "initKernelEnv\n");
} 

//=============================================================================
 int MC32X0_WriteCalibration(struct i2c_client *client, int dat[MC32X0_AXES_NUM])
{
	int err = 0;
	u8 buf[9] = { 0 };
	s16 tmp = 0, x_gain = 0, y_gain = 0, z_gain = 0;
	s32 x_off = 0, y_off = 0, z_off = 0;
    int temp_cali_dat[MC32X0_AXES_NUM] = { 0 };
	const struct mc3xx0_hwmsen_convert *pCvt = NULL;
	
    u8  bMsbFilter       = 0x3F;
    s16 wSignBitMask     = 0x2000;
    s16 wSignPaddingBits = 0xC000;
    s32 dwRangePosLimit  = 0x1FFF;
    s32 dwRangeNegLimit  = -0x2000;
    
	pCvt = &mc3xx0_cvt[mc3xx0_current_placement];

    temp_cali_dat[pCvt->map[MC3XX0_AXIS_X]] = pCvt->sign[MC3XX0_AXIS_X] * dat[MC3XX0_AXIS_X];
    temp_cali_dat[pCvt->map[MC3XX0_AXIS_Y]] = pCvt->sign[MC3XX0_AXIS_Y] * dat[MC3XX0_AXIS_Y];
    temp_cali_dat[pCvt->map[MC3XX0_AXIS_Z]] = pCvt->sign[MC3XX0_AXIS_Z] * dat[MC3XX0_AXIS_Z];

    temp_cali_dat[MC3XX0_AXIS_X] = ((temp_cali_dat[MC3XX0_AXIS_X] * gsensor_gain.x) / GRAVITY_1G_VALUE);
    temp_cali_dat[MC3XX0_AXIS_Y] = ((temp_cali_dat[MC3XX0_AXIS_Y] * gsensor_gain.y) / GRAVITY_1G_VALUE);
    temp_cali_dat[MC3XX0_AXIS_Z] = ((temp_cali_dat[MC3XX0_AXIS_Z] * gsensor_gain.z) / GRAVITY_1G_VALUE);

    if ((is_new_mc34x0)||(is_mc35xx))
    {
        temp_cali_dat[MC3XX0_AXIS_X] = -temp_cali_dat[MC3XX0_AXIS_X];
        temp_cali_dat[MC3XX0_AXIS_Y] = -temp_cali_dat[MC3XX0_AXIS_Y];
    }
    else if (is_mc3250)
    {
        s16    temp = 0;

        temp = temp_cali_dat[MC3XX0_AXIS_X];

        temp_cali_dat[MC3XX0_AXIS_X] = -temp_cali_dat[MC3XX0_AXIS_Y];
        temp_cali_dat[MC3XX0_AXIS_Y] = temp;
    }

    dat[MC3XX0_AXIS_X] = temp_cali_dat[MC3XX0_AXIS_X];
    dat[MC3XX0_AXIS_Y] = temp_cali_dat[MC3XX0_AXIS_Y];
    dat[MC3XX0_AXIS_Z] = temp_cali_dat[MC3XX0_AXIS_Z];

	GSE_LOG("UPDATE dat: (%+3d %+3d %+3d)\n", 
	dat[MC32X0_AXIS_X], dat[MC32X0_AXIS_Y], dat[MC32X0_AXIS_Z]);

    // read register 0x21~0x29
	err  = i2c_smbus_read_i2c_block_data(client , 0x21 , 3 , &buf[0]);
	err |= i2c_smbus_read_i2c_block_data(client , 0x24 , 3 , &buf[3]);
	err |= i2c_smbus_read_i2c_block_data(client , 0x27 , 3 , &buf[6]);
	
	if (is_mc35xx)
    {
        bMsbFilter       = 0x7F;
        wSignBitMask     = 0x4000;
        wSignPaddingBits = 0x8000;
        dwRangePosLimit  = 0x3FFF;
        dwRangeNegLimit  = -0x4000;
    }
     
	// get x,y,z offset
	tmp = ((buf[1] & bMsbFilter) << 8) + buf[0];
		if (tmp & wSignBitMask)
			tmp |= wSignPaddingBits;
		x_off = tmp;
					
	tmp = ((buf[3] & bMsbFilter) << 8) + buf[2];
		if (tmp & wSignBitMask)
			tmp |= wSignPaddingBits;
		y_off = tmp;
					
	tmp = ((buf[5] & bMsbFilter) << 8) + buf[4];
		if (tmp & wSignBitMask)
			tmp |= wSignPaddingBits;
		z_off = tmp;
					
	// get x,y,z gain
	x_gain = ((buf[1] >> 7) << 8) + buf[6];
	y_gain = ((buf[3] >> 7) << 8) + buf[7];
	z_gain = ((buf[5] >> 7) << 8) + buf[8];
								
	// prepare new offset
	x_off = x_off + 16 * dat[MC32X0_AXIS_X] * 256 * 128 / 3 / gsensor_gain.x / (40 + x_gain);
	y_off = y_off + 16 * dat[MC32X0_AXIS_Y] * 256 * 128 / 3 / gsensor_gain.y / (40 + y_gain);
	z_off = z_off + 16 * dat[MC32X0_AXIS_Z] * 256 * 128 / 3 / gsensor_gain.z / (40 + z_gain);


	//add for over range 
	if( x_off > dwRangePosLimit) 
	{
		x_off = dwRangePosLimit;
	}
	else if( x_off < dwRangeNegLimit)
	{
		x_off = dwRangeNegLimit;
	}

	if( y_off > dwRangePosLimit) 
	{
		y_off = dwRangePosLimit;
	}
	else if( y_off < dwRangeNegLimit)
	{
		y_off = dwRangeNegLimit;
	}

	if( z_off > dwRangePosLimit) 
	{
		z_off = dwRangePosLimit;
	}
	else if( z_off < dwRangeNegLimit)
	{
		z_off = dwRangeNegLimit;
	}

	//storege the cerrunt offset data with DOT format
	offset_data[0] = x_off;
	offset_data[1] = y_off;
	offset_data[2] = z_off;

	//storege the cerrunt Gain data with GOT format
	gain_data[0] = 256*8*128/3/(40+x_gain);
	gain_data[1] = 256*8*128/3/(40+y_gain);
	gain_data[2] = 256*8*128/3/(40+z_gain);
	GSE_LOG("%d %d ======================\n\n ", gain_data[0], x_gain);

	buf[0] = 0x43;
	i2c_smbus_write_byte_data(client, 0x07, buf[0]);

	buf[0] = x_off & 0xff;
	buf[1] = ((x_off >> 8) & bMsbFilter) | (x_gain & 0x0100 ? 0x80 : 0);
	buf[2] = y_off & 0xff;
	buf[3] = ((y_off >> 8) & bMsbFilter) | (y_gain & 0x0100 ? 0x80 : 0);
	buf[4] = z_off & 0xff;
	buf[5] = ((z_off >> 8) & bMsbFilter) | (z_gain & 0x0100 ? 0x80 : 0);

	i2c_smbus_write_i2c_block_data(client, 0x21,   2, &buf[0]);
	i2c_smbus_write_i2c_block_data(client, 0x21+2, 2, &buf[2]);
	i2c_smbus_write_i2c_block_data(client, 0x21+4, 2, &buf[4]);
	
	buf[0] = 0x41;
	i2c_smbus_write_byte_data(client, 0x07,buf[0]);

    msleep(50);

    return err;

}

//=============================================================================
int mcube_read_cali_file(struct i2c_client *client)
{
	int cali_data[3] = { 0 };
	int err = 0;

				//MCUBE_BACKUP_FILE
	READ_FROM_BACKUP = false;
	//MCUBE_BACKUP_FILE
	initKernelEnv();
	fd_file = openFile(CALIB_PATH,O_RDONLY,0);
				//MCUBE_BACKUP_FILE
	if (fd_file == NULL) 
	{
		fd_file = openFile(BACKUP_CALIB_PATH, O_RDONLY, 0); 
    if(fd_file != NULL)
    {
    		READ_FROM_BACKUP = true;
    }
	}
	//MCUBE_BACKUP_FILE 
	if (fd_file == NULL) 
	{
		GSE_LOG("fail to open\n");
		cali_data[0] = 0;
		cali_data[1] = 0;
		cali_data[2] = 0;

		return -1;
	}
	else
	{
		memset(backup_buf,0,64); 
		if ((err = readFile(fd_file,backup_buf,128))>0) 
			GSE_LOG("buf:%s\n",backup_buf); 
		else 
			GSE_LOG("read file error %d\n",err); 

		set_fs(oldfs); 
		closeFile(fd_file); 

		sscanf(backup_buf, "%d %d %d",&cali_data[MC32X0_AXIS_X], &cali_data[MC32X0_AXIS_Y], &cali_data[MC32X0_AXIS_Z]);
		GSE_LOG("cali_data: %d %d %d\n", cali_data[MC32X0_AXIS_X], cali_data[MC32X0_AXIS_Y], cali_data[MC32X0_AXIS_Z]); 	
				
		MC32X0_WriteCalibration(client, cali_data);
	}

	return 0;
}


void MC32X0_rbm(struct i2c_client *client, int enable)
{
    char buf1[3] = { 0 };

	if(enable == 1 )
	{
        buf1[0] = 0x43; 
        i2c_smbus_write_byte_data(client, 0x07, buf1[0]);
        
        buf1[0] = 0x6D; 
        i2c_smbus_write_byte_data(client, 0x1B, buf1[0]);
      
        buf1[0] = 0x43; 
        i2c_smbus_write_byte_data(client, 0x1B, buf1[0]);
        
        buf1[0] = 0x00; 
        i2c_smbus_write_byte_data(client, 0x3B, buf1[0]);

        buf1[0] = 0x02; 
        i2c_smbus_write_byte_data(client, 0x14, buf1[0]);

        buf1[0] = 0x41; 
        i2c_smbus_write_byte_data(client, 0x07, buf1[0]);

		enable_RBM_calibration =1;
		
		GSE_LOG("set rbm!!\n");

        msleep(220);
	}
	else if(enable == 0 )  
	{
        buf1[0] = 0x43; 
        i2c_smbus_write_byte_data(client, 0x07, buf1[0]);

        buf1[0] = 0x00; 
        i2c_smbus_write_byte_data(client, 0x14, buf1[0]);
				GSE_LOG("set rbm!! %x @@@@\n",s_bPCODE);

        buf1[0] = s_bPCODE; 
        i2c_smbus_write_byte_data(client, 0x3B, buf1[0]);

        buf1[0] = 0x6D; 
        i2c_smbus_write_byte_data(client, 0x1B, buf1[0]);

        buf1[0] = 0x43; 
        i2c_smbus_write_byte_data(client, 0x1B, buf1[0]);

        buf1[0] = 0x41; 
        i2c_smbus_write_byte_data(client, 0x07, buf1[0]);

        enable_RBM_calibration = 0;

        GSE_LOG("clear rbm!!\n");

        msleep(220);
    }
}

/*----------------------------------------------------------------------------*/
 int MC32X0_ReadData_RBM(struct i2c_client *client,int data[MC32X0_AXES_NUM])
{   
	//u8 uData;
	u8 addr = 0x0d;
	u8 rbm_buf[MC32X0_DATA_LEN] = {0};
	int err = 0;

	
	//err = p_mc32x0->MC32X0_BUS_READ_FUNC(p_mc32x0->dev_addr, addr, &rbm_buf[0],6);
	err = i2c_smbus_read_i2c_block_data(client , addr , 6 , rbm_buf);
	//err = mc32x0_read_block(client, addr, rbm_buf, 0x06);

	data[MC32X0_AXIS_X] = (s16)((rbm_buf[0]) | (rbm_buf[1] << 8));
	data[MC32X0_AXIS_Y] = (s16)((rbm_buf[2]) | (rbm_buf[3] << 8));
	data[MC32X0_AXIS_Z] = (s16)((rbm_buf[4]) | (rbm_buf[5] << 8));

	GSE_LOG("rbm_buf<<<<<[%02x %02x %02x %02x %02x %02x]\n",rbm_buf[0], rbm_buf[2], rbm_buf[2], rbm_buf[3], rbm_buf[4], rbm_buf[5]);
	GSE_LOG("RBM<<<<<[%04x %04x %04x]\n", data[MC32X0_AXIS_X], data[MC32X0_AXIS_Y], data[MC32X0_AXIS_Z]);
	GSE_LOG("RBM<<<<<[%04d %04d %04d]\n", data[MC32X0_AXIS_X], data[MC32X0_AXIS_Y], data[MC32X0_AXIS_Z]);		
	return err;
}


 int MC32X0_ReadRBMData(struct i2c_client *client, char *buf)
{
	//struct mc32x0_data *mc32x0 = i2c_get_clientdata(client);
	int res = 0;
	int data[3];

	if (!buf)
	{
		return EINVAL;
	}
	
	mc32x0_set_mode(client,MC32X0_WAKE);
/*
	if(mc32x0->status == mc32x0_CLOSE)
	{
		res = mc32x0_start(client, 0);
		if(res)
		{
			GSE_ERR("Power on mc32x0 error %d!\n", res);
		}
	}
*/
	if(res = MC32X0_ReadData_RBM(client,data))
	{        
		GSE_ERR("%s I2C error: ret value=%d",__func__, res);
		return EIO;
	}
	else
	{
		sprintf(buf, "%04x %04x %04x", data[MC32X0_AXIS_X], 
			data[MC32X0_AXIS_Y], data[MC32X0_AXIS_Z]);
	
	}
	
	return 0;
}
 int MC32X0_ReadOffset(struct i2c_client *client,s16 ofs[MC32X0_AXES_NUM])
{    
	int err = 0;
	u8 off_data[6] = { 0 };

	if(McubeID & MCUBE_8G_14BIT)
	{
		err = i2c_smbus_read_i2c_block_data(client, MC32X0_XOUT_EX_L_REG, MC32X0_DATA_LEN, off_data);

		ofs[MC32X0_AXIS_X] = ((s16)(off_data[0]))|((s16)(off_data[1])<<8);
		ofs[MC32X0_AXIS_Y] = ((s16)(off_data[2]))|((s16)(off_data[3])<<8);
		ofs[MC32X0_AXIS_Z] = ((s16)(off_data[4]))|((s16)(off_data[5])<<8);
	}
	else if(McubeID & MCUBE_1_5G_8BIT) 
	{
		err = i2c_smbus_read_i2c_block_data(client, 0, 3, off_data);

		ofs[MC32X0_AXIS_X] = (s8)off_data[0];
		ofs[MC32X0_AXIS_Y] = (s8)off_data[1];
		ofs[MC32X0_AXIS_Z] = (s8)off_data[2];			
	}

	GSE_LOG("MC32X0_ReadOffset %d %d %d\n", ofs[MC32X0_AXIS_X], ofs[MC32X0_AXIS_Y], ofs[MC32X0_AXIS_Z]);

    return err;  
}

//=============================================================================
int MC32X0_ResetCalibration(struct i2c_client *client)
{
	u8 buf[6] = { 0 };
	s16 tmp = 0;
	int err = 0;

    u8  bMsbFilter       = 0x3F;
    s16 wSignBitMask     = 0x2000;
    s16 wSignPaddingBits = 0xC000;

	buf[0] = 0x43;
	err = i2c_smbus_write_byte_data(client, 0x07, buf[0]);
	if(err)
	{
		GSE_ERR("error 0x07: %d\n", err);
	}

	err = i2c_smbus_write_i2c_block_data(client, 0x21, 6, offset_buf);
	if(err)
	{
		GSE_ERR("error: %d\n", err);
	}
	
	buf[0] = 0x41;
	err = i2c_smbus_write_byte_data(client, 0x07, buf[0]);
	if(err)
	{
		GSE_ERR("error: %d\n", err);
	}

	msleep(20);


	if (is_mc35xx)
    {
        bMsbFilter       = 0x7F;
        wSignBitMask     = 0x4000;
        wSignPaddingBits = 0x8000;
    }


	tmp = ((offset_buf[1] & bMsbFilter) << 8) + offset_buf[0];
	if (tmp & wSignBitMask)
		tmp |= wSignPaddingBits;
	offset_data[0] = tmp;
					
	tmp = ((offset_buf[3] & bMsbFilter) << 8) + offset_buf[2];
	if (tmp & wSignBitMask)
			tmp |= wSignPaddingBits;
	offset_data[1] = tmp;
					
	tmp = ((offset_buf[5] & bMsbFilter) << 8) + offset_buf[4];
	if (tmp & wSignBitMask)
		tmp |= wSignPaddingBits;
	offset_data[2] = tmp;	

	return 0;  
}

//=============================================================================
 int MC32X0_ReadCalibration(struct i2c_client *client,int dat[MC32X0_AXES_NUM])
{
    signed short MC_offset[MC32X0_AXES_NUM + 1] = { 0 };    // +1: for 4-byte alignment
    int err = 0;

	memset(MC_offset, 0, sizeof(MC_offset));

	err = MC32X0_ReadOffset(client, MC_offset);

    if (err)
    {
        GSE_ERR("read offset fail, %d\n", err);
        return err;
    }    
    
    dat[MC32X0_AXIS_X] = MC_offset[MC32X0_AXIS_X];
    dat[MC32X0_AXIS_Y] = MC_offset[MC32X0_AXIS_Y];
    dat[MC32X0_AXIS_Z] = MC_offset[MC32X0_AXIS_Z];  
                                      
    return 0;
}

//=============================================================================
int MC32X0_ReadData(struct i2c_client *client, s16 buffer[MC32X0_AXES_NUM])
{
	unsigned char buf[6] = { 0 };
	signed char buf1[6] = { 0 };
	char rbm_buf[6] = { 0 };
	int ret = 0;


	int tempX=0;
	int tempY=0;
	int tempZ=0;


	if (enable_RBM_calibration == 0)
	{
		//err = hwmsen_read_block(client, addr, buf, 0x06);
	}
	else if (enable_RBM_calibration == 1)
	{		
		memset(rbm_buf, 0, 6);
        i2c_smbus_read_i2c_block_data(client, 0x0d  , 2, &rbm_buf[0]);
        i2c_smbus_read_i2c_block_data(client, 0x0d+2, 2, &rbm_buf[2]);
        i2c_smbus_read_i2c_block_data(client, 0x0d+4, 2, &rbm_buf[4]);
	}

	if (enable_RBM_calibration == 0)
	{
		if(McubeID & MC32X0_HIGH_END)
		{
			ret = i2c_smbus_read_i2c_block_data(client, MC32X0_XOUT_EX_L_REG, 6, buf);
			
			buffer[0] = (signed short)((buf[0])|(buf[1]<<8));
			buffer[1] = (signed short)((buf[2])|(buf[3]<<8));
			buffer[2] = (signed short)((buf[4])|(buf[5]<<8));
		}
		else if(McubeID & MC32X0_LOW_END)
		{
			ret = i2c_smbus_read_i2c_block_data(client, MC32X0_XOUT_REG, 3, buf1);
				
			buffer[0] = (signed short)buf1[0];
			buffer[1] = (signed short)buf1[1];
			buffer[2] = (signed short)buf1[2];
//add by sheen
#if 0
                    if(s_bMPOL ==0x03 ) 
					{
						 buffer[MC32X0_AXIS_X] = buffer[MC32X0_AXIS_Y];
						 buffer[MC32X0_AXIS_Y] = buffer[MC32X0_AXIS_X];					
					}
#endif					
//end			
			
			#ifdef SUPPORT_VIRTUAL_Z_SENSOR
			
			tempX = buffer[MC32X0_AXIS_X];
			tempY = buffer[MC32X0_AXIS_Y];
			tempZ = buffer[MC32X0_AXIS_Z];
			
			if(1 == Verify_Z_Railed((int)buffer[MC32X0_AXIS_Z], LOW_RESOLUTION)) // z-railed
			{
				Railed = 1;
				
				GSE_LOG("%s: Z railed", __func__);
				if (G_2_REVERSE == 1)
					buffer[MC32X0_AXIS_Z] = (s8) (  gsensor_gain.z - (abs(tempX) + abs(tempY)));
				else
					buffer[MC32X0_AXIS_Z] = (s8) -(  gsensor_gain.z - (abs(tempX) + abs(tempY)));
			}
						else
			{
				Railed = 0;	
			}
			#endif
}
			mcprintkreg("MC32X0_ReadData: %d %d %d\n", buffer[0], buffer[1], buffer[2]);
/*			if((buffer[0]==0)&&(buffer[1]==0)&&(buffer[2]==0))
			{
				i2c_smbus_write_byte_data(client, MC32X0_Mode_Feature_REG, 0x03);
				i2c_smbus_write_byte_data(client, MC32X0_RANGE_Control_REG, 0x02);
				i2c_smbus_write_byte_data(client, MC32X0_Mode_Feature_REG, 0x01);
			}*/
	}
	else if (enable_RBM_calibration == 1)
	{
		buffer[MC32X0_AXIS_X] = (s16)((rbm_buf[0]) | (rbm_buf[1] << 8));
		buffer[MC32X0_AXIS_Y] = (s16)((rbm_buf[2]) | (rbm_buf[3] << 8));
		buffer[MC32X0_AXIS_Z] = (s16)((rbm_buf[4]) | (rbm_buf[5] << 8));

		GSE_LOG("%s RBM<<<<<[%08d %08d %08d]\n", __func__, buffer[MC32X0_AXIS_X], buffer[MC32X0_AXIS_Y], buffer[MC32X0_AXIS_Z]);

		if(gain_data[0] == 0)
		{
			buffer[MC32X0_AXIS_X] = 0;
			buffer[MC32X0_AXIS_Y] = 0;
			buffer[MC32X0_AXIS_Z] = 0;

			return 0;
		}

		buffer[MC32X0_AXIS_X] = (buffer[MC32X0_AXIS_X] + offset_data[0]/2)*gsensor_gain.x/gain_data[0];
		buffer[MC32X0_AXIS_Y] = (buffer[MC32X0_AXIS_Y] + offset_data[1]/2)*gsensor_gain.y/gain_data[1];
		buffer[MC32X0_AXIS_Z] = (buffer[MC32X0_AXIS_Z] + offset_data[2]/2)*gsensor_gain.z/gain_data[2];
		#ifdef SUPPORT_VIRTUAL_Z_SENSOR
		
		tempX = buffer[MC32X0_AXIS_X];
		tempY = buffer[MC32X0_AXIS_Y];
		tempZ = buffer[MC32X0_AXIS_Z];
			
		GSE_LOG("Original RBM<<<<<[%08d %08d %08d]\n", buffer[MC32X0_AXIS_X], buffer[MC32X0_AXIS_Y], buffer[MC32X0_AXIS_Z]);
		
		if(((McubeID &MC32X0_LOW_END)&&(1 == Verify_Z_Railed(buffer[MC32X0_AXIS_Z], RBM_RESOLUTION)))||((McubeID &MC32X0_HIGH_END)&&(1 == Verify_Z_Railed(buffer[MC32X0_AXIS_Z], HIGH_RESOLUTION)))) // z-railed
		{
			GSE_LOG("%s: Z Railed in RBM mode",__func__);
			if (G_2_REVERSE == 1)
				buffer[MC32X0_AXIS_Z] = (s16) (  gsensor_gain.z - (abs(tempX) + abs(tempY)));
			else
				buffer[MC32X0_AXIS_Z] = (s16) -(  gsensor_gain.z - (abs(tempX) + abs(tempY)));
		}
		GSE_LOG("RBM<<<<<[%08d %08d %08d]\n", buffer[MC32X0_AXIS_X], buffer[MC32X0_AXIS_Y], buffer[MC32X0_AXIS_Z]);
		#endif		
		GSE_LOG("%s offset_data <<<<<[%d %d %d]\n", __func__,offset_data[0], offset_data[1], offset_data[2]);

		GSE_LOG("%s gsensor_gain <<<<<[%d %d %d]\n", __func__,gsensor_gain.x, gsensor_gain.y, gsensor_gain.z);
		
		GSE_LOG("%s gain_data <<<<<[%d %d %d]\n", __func__,gain_data[0], gain_data[1], gain_data[2]);

		GSE_LOG("%s RBM->RAW <<<<<[%d %d %d]\n", __func__,buffer[MC32X0_AXIS_X], buffer[MC32X0_AXIS_Y], buffer[MC32X0_AXIS_Z]);
	}
	
	return 0;
}

//=============================================================================
int MC32X0_ReadRawData(struct i2c_client *client,  char * buf)
{
	int res = 0;
	s16 raw_buf[3] = { 0 };

	if (!buf || !client)
	{
		return -EINVAL;
	}
	
	mc32x0_set_mode(client, MC32X0_WAKE);
	res = MC32X0_ReadData(client, &raw_buf[0]);
	if(res)
	{     
		GSE_ERR("I2C error: ret value=%d", res);
		return -EIO;
	}
	else
	{
    	const struct mc3xx0_hwmsen_convert *pCvt = &mc3xx0_cvt[mc3xx0_current_placement];

		GSE_LOG("UPDATE dat: (%+3d %+3d %+3d)\n", 
		raw_buf[MC32X0_AXIS_X], raw_buf[MC32X0_AXIS_Y], raw_buf[MC32X0_AXIS_Z]);

		//G_RAW_DATA[MC32X0_AXIS_X] = raw_buf[0];
		//G_RAW_DATA[MC32X0_AXIS_Y] = raw_buf[1];
		//G_RAW_DATA[MC32X0_AXIS_Z] = raw_buf[2];
		//G_RAW_DATA[MC32X0_AXIS_Z] = G_RAW_DATA[MC32X0_AXIS_Z] + gsensor_gain.z;

        raw_buf[MC3XX0_AXIS_X] = ((raw_buf[MC3XX0_AXIS_X] * GRAVITY_1G_VALUE) / gsensor_gain.x);
        raw_buf[MC3XX0_AXIS_Y] = ((raw_buf[MC3XX0_AXIS_Y] * GRAVITY_1G_VALUE) / gsensor_gain.y);
        raw_buf[MC3XX0_AXIS_Z] = ((raw_buf[MC3XX0_AXIS_Z] * GRAVITY_1G_VALUE) / gsensor_gain.z);

        if ((is_new_mc34x0)||(is_mc35xx))
        {
            raw_buf[MC3XX0_AXIS_X] = -raw_buf[MC3XX0_AXIS_X];
            raw_buf[MC3XX0_AXIS_Y] = -raw_buf[MC3XX0_AXIS_Y];
        }
        else if (is_mc3250)
        {
            s16    temp = 0;

            temp = raw_buf[MC3XX0_AXIS_X];

            raw_buf[MC3XX0_AXIS_X] = raw_buf[MC3XX0_AXIS_Y];
            raw_buf[MC3XX0_AXIS_Y] = -temp;
        }

        G_RAW_DATA[MC3XX0_AXIS_X] = pCvt->sign[MC3XX0_AXIS_X] * raw_buf[pCvt->map[MC3XX0_AXIS_X]];
        G_RAW_DATA[MC3XX0_AXIS_Y] = pCvt->sign[MC3XX0_AXIS_Y] * raw_buf[pCvt->map[MC3XX0_AXIS_Y]];
        G_RAW_DATA[MC3XX0_AXIS_Z] = pCvt->sign[MC3XX0_AXIS_Z] * raw_buf[pCvt->map[MC3XX0_AXIS_Z]];

		G_RAW_DATA[MC32X0_AXIS_Z]+=GRAVITY_1G_VALUE;


		sprintf(buf, "%04x %04x %04x", G_RAW_DATA[MC32X0_AXIS_X], 
				G_RAW_DATA[MC32X0_AXIS_Y], G_RAW_DATA[MC32X0_AXIS_Z]);

		GSE_LOG("G_RAW_DATA: (%+3d %+3d %+3d)\n", 
		G_RAW_DATA[MC32X0_AXIS_X], G_RAW_DATA[MC32X0_AXIS_Y], G_RAW_DATA[MC32X0_AXIS_Z]);
	}

	return 0;
}

//=============================================================================
static int MC32X0_ReadRegMap(struct i2c_client *client, u8 *pbUserBuf)
{
	u8 data[128] = {0};
	u8 addr = 0x00;
	int err = 0;
	int i = 0;

	if(NULL == client)
	{
		err = -EINVAL;
		return err;
	}


    for(i = 0; i < 64; i++)
    {
        data[i] = i2c_smbus_read_byte_data(client, i);
        printk(KERN_INFO "mcube register map Register[%x] = 0x%x\n", i ,data[i]);
    }

	//msleep(50);	
	
	//mcube_write_log_data(client, data);

	msleep(50);
   
    if (NULL != pbUserBuf)
    {
        printk(KERN_INFO "copy to user buffer\n");
        memcpy(pbUserBuf, data, 64);
    }
      	
	return err;
}

//=============================================================================
static int MC3XX0_DetectPcode(struct i2c_client *client)
{
    unsigned char product_code = 0;
    static    unsigned short        mc3xxx_i2c_auto_probe_addr[] = { 0x4C,0x6C};
    int              _nProbeAddrCount = (sizeof(mc3xxx_i2c_auto_probe_addr) / sizeof(mc3xxx_i2c_auto_probe_addr[0]));
    int              _nCount = 0;
    
    for (_nCount = 0; _nCount < _nProbeAddrCount; _nCount++)
    {
		client->addr = mc3xxx_i2c_auto_probe_addr[_nCount];
		product_code = i2c_smbus_read_byte_data(client, 0x3B);

		GSE_LOG("%s: 0x%x\n", __func__, product_code);

		if ((0x90 == product_code) || (0x19 == product_code) ||
		(0xA8 == product_code) || (0x29 == product_code) ||
		(0xB8 == product_code) || (0x39 == product_code) ||
		(0x88 == product_code) || (0x40 == product_code) ||
		(0x30 == product_code) || (0x10 == (product_code&0XF1)) ||
		(0x6E == (product_code | 0x0e)) || (0x00 == product_code)
		|| (0x59 == product_code))
		{
			return product_code;
		}

	}
    return -1;
}

//=============================================================================
void MC32X0_Reset(struct i2c_client *client) 
{
	s16 tmp = 0, x_gain = 0, y_gain = 0, z_gain = 0;
	s32 x_off = 0, y_off = 0, z_off = 0;
	u8 buf[3] = { 0 };
    int err = 0;

	buf[0] = 0x43;
  	i2c_smbus_write_byte_data(client, 0x07, buf[0]);

	buf[0] = 0x6d;
  	i2c_smbus_write_byte_data(client, 0x1b, buf[0]);
  	
	buf[0] = 0x43;
  	i2c_smbus_write_byte_data(client, 0x1b, buf[0]);

	msleep(5);
	
	buf[0] = 0x43;
  	i2c_smbus_write_byte_data(client, 0x07, buf[0]);

	buf[0] = 0x80;
  	i2c_smbus_write_byte_data(client, 0x1c, buf[0]);

	buf[0] = 0x80;
  	i2c_smbus_write_byte_data(client, 0x17, buf[0]);

	msleep(5);

	buf[0] = 0x00;
  	i2c_smbus_write_byte_data(client, 0x1c, buf[0]);

	buf[0] = 0x00;
  	i2c_smbus_write_byte_data(client, 0x17, buf[0]);

	msleep(5);

	msleep(100);
	err = MC3XX0_DetectPcode(client);

	if (err < 0)
	{
	    GSE_ERR("[%s] not mCube g-sensor!\n", __func__);

	    return err;
	}
	
	memset(offset_buf, 0, 9);

	err = i2c_smbus_read_i2c_block_data(client, 0x21, 9, offset_buf);

    tmp = ((offset_buf[1] & 0x3f) << 8) + offset_buf[0];
    if (tmp & 0x2000)
        tmp |= 0xc000;
    x_off = tmp;

    tmp = ((offset_buf[3] & 0x3f) << 8) + offset_buf[2];
    if (tmp & 0x2000)
        tmp |= 0xc000;
    y_off = tmp;

    tmp = ((offset_buf[5] & 0x3f) << 8) + offset_buf[4];
    if (tmp & 0x2000)
        tmp |= 0xc000;
    z_off = tmp;
					
	// get x,y,z gain
	x_gain = ((offset_buf[1] >> 7) << 8) + offset_buf[6];
	y_gain = ((offset_buf[3] >> 7) << 8) + offset_buf[7];
	z_gain = ((offset_buf[5] >> 7) << 8) + offset_buf[8];
							
	//storege the cerrunt offset data with DOT format
	offset_data[0] = x_off;
	offset_data[1] = y_off;
	offset_data[2] = z_off;

	//storege the cerrunt Gain data with GOT format
	gain_data[0] = 256*8*128/3/(40+x_gain);
	gain_data[1] = 256*8*128/3/(40+y_gain);
	gain_data[2] = 256*8*128/3/(40+z_gain);
	printk("offser gain = %d %d %d %d %d %d======================\n\n ",
		gain_data[0],gain_data[1],gain_data[2],offset_data[0],offset_data[1],offset_data[2]);

	return 0;
}


#endif

int mc32x0_read_accel_xyz(struct i2c_client *client, s16 * acc)
{
	int comres = 0;
	s16 raw_data[MC3XX0_AXIS_NUM] = { 0 };
	const struct mc3xx0_hwmsen_convert *pCvt = &mc3xx0_cvt[mc3xx0_current_placement];

#ifdef DOT_CALI
        s16 raw_buf[6] = { 0 };
        
        comres = MC32X0_ReadData(client, &raw_buf[0]);
        
        acc[0] = raw_buf[0];
        acc[1] = raw_buf[1];
        acc[2] = raw_buf[2];
#else
        unsigned char raw_buf[6] = { 0 };
        signed char raw_buf1[3] = { 0 };

        if(McubeID & MC32X0_HIGH_END)
        {
            comres = i2c_smbus_read_i2c_block_data(client, MC32X0_XOUT_EX_L_REG, 6, raw_buf);
            
            acc[0] = (signed short)((raw_buf[0])|(raw_buf[1]<<8));
            acc[1] = (signed short)((raw_buf[2])|(raw_buf[3]<<8));
            acc[2] = (signed short)((raw_buf[4])|(raw_buf[5]<<8));
        }
        else if(McubeID & MC32X0_LOW_END)
        {
            comres = i2c_smbus_read_i2c_block_data(client, MC32X0_XOUT_REG, 3, raw_buf1);
            
            acc[0] = (signed short)raw_buf1[0];
            acc[1] = (signed short)raw_buf1[1];
            acc[2] = (signed short)raw_buf1[2];
        }
#endif

    raw_data[MC3XX0_AXIS_X] = acc[MC3XX0_AXIS_X];
    raw_data[MC3XX0_AXIS_Y] = acc[MC3XX0_AXIS_Y];
    raw_data[MC3XX0_AXIS_Z] = acc[MC3XX0_AXIS_Z];

#ifdef CONFIG_HAS_LOW_PASS_FILTER
	if(abs (raw_data[MC3XX0_AXIS_X])<=ZERO_FIR)
		raw_data[MC3XX0_AXIS_X]=0;

	if(abs (raw_data[MC3XX0_AXIS_Y])<=ZERO_FIR)
		raw_data[MC3XX0_AXIS_Y]=0;
#endif

    raw_data[MC3XX0_AXIS_X] = ((raw_data[MC3XX0_AXIS_X] * GRAVITY_1G_VALUE) / gsensor_gain.x);
    raw_data[MC3XX0_AXIS_Y] = ((raw_data[MC3XX0_AXIS_Y] * GRAVITY_1G_VALUE) / gsensor_gain.y);
    raw_data[MC3XX0_AXIS_Z] = ((raw_data[MC3XX0_AXIS_Z] * GRAVITY_1G_VALUE) / gsensor_gain.z);

    if ((is_new_mc34x0)||(is_mc35xx))
    {
        raw_data[MC3XX0_AXIS_X] = -raw_data[MC3XX0_AXIS_X];
        raw_data[MC3XX0_AXIS_Y] = -raw_data[MC3XX0_AXIS_Y];
    }
    else if (is_mc3250)
    {
        s16    temp = 0;

        temp = raw_data[MC3XX0_AXIS_X];

        raw_data[MC3XX0_AXIS_X] = raw_data[MC3XX0_AXIS_Y];
        raw_data[MC3XX0_AXIS_Y] = -temp;
    }

    acc[MC3XX0_AXIS_X] = pCvt->sign[MC3XX0_AXIS_X] * raw_data[pCvt->map[MC3XX0_AXIS_X]];
    acc[MC3XX0_AXIS_Y] = pCvt->sign[MC3XX0_AXIS_Y] * raw_data[pCvt->map[MC3XX0_AXIS_Y]];
    acc[MC3XX0_AXIS_Z] = pCvt->sign[MC3XX0_AXIS_Z] * raw_data[pCvt->map[MC3XX0_AXIS_Z]];

	return comres;
}

static int mc32x0_measure(struct i2c_client *client, struct acceleration *accel)
{
	s16 raw[3] = { 0 };
	
    #ifdef DOT_CALI
        int ret = 0;

        if( load_cali_flg > 0)
        {
            ret = mcube_read_cali_file(client);

            if(ret == 0)
                load_cali_flg = ret;
            else 
                load_cali_flg--;

            GSE_LOG("load_cali %d\n",ret); 
        }  
    #endif

	mc32x0_read_accel_xyz(client, &raw[0]);

	accel->x = raw[0] ;
	accel->y = raw[1] ;
	accel->z = raw[2] ;
	
	return 0;
}

//=============================================================================
static void mc32x0_work_func(struct work_struct *work)
{
	struct mc32x0_data *data = container_of(work, struct mc32x0_data, work);
	struct acceleration accel = { 0 };

	mc32x0_measure(data->client, &accel);
	if(z_flag_report)
	  accel.z -= 1;
	else
	  accel.z += 1;
	z_flag_report =! z_flag_report;

	mcprintkreg("mc32x0_work_func: %d %d %d\n", accel.x, accel.y, accel.z);
	input_report_abs(data->input_dev, ABS_X, accel.x);
	input_report_abs(data->input_dev, ABS_Y, accel.y);
	input_report_abs(data->input_dev, ABS_Z, accel.z);
	input_sync(data->input_dev);

}

//=============================================================================
static enum hrtimer_restart mc32x0_timer_func(struct hrtimer *timer)
{
	struct mc32x0_data *data = container_of(timer, struct mc32x0_data, timer);

	queue_work(data->mc32x0_wq, &data->work);

	hrtimer_start(&data->timer, ktime_set(0, sensor_duration*1000000), HRTIMER_MODE_REL);

	return HRTIMER_NORESTART;
}

static int mc32x0_enable(struct mc32x0_data *data, int enable)
{
	if(enable){
		msleep(10);
		mc32x0_chip_init(data->client);
		hrtimer_start(&data->timer, ktime_set(0, sensor_duration*1000000), HRTIMER_MODE_REL);
                 data->enabled = true;
	}else{
		hrtimer_cancel(&data->timer);
                 data->enabled = false;
	}
	return 0;
}

//MCUBE_BACKUP_FILE
static void mcube_copy_file(const char *dstFilePath)
{

	int err =0;
	initKernelEnv();

	fd_file = openFile(dstFilePath,O_RDWR,0); 
	if (fd_file == NULL) 
	{
		GSE_LOG("open %s fail\n",dstFilePath);  
		return;
	}

		if ((err = writeFile(fd_file,backup_buf,64))>0) 
			GSE_LOG("buf:%s\n",backup_buf); 
		else 
			GSE_LOG("write file error %d\n",err);

		set_fs(oldfs); ; 
		closeFile(fd_file); 

}
//MCUBE_BACKUP_FILE

static long mc32x0_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	//int intBuf[SENSOR_DATA_SIZE];
	int ret = 0;
	int temp = 0;
#ifdef DOT_CALI
	void __user *data1;
	char strbuf[256];
	int cali[3];
	SENSOR_DATA sensor_data;
	struct i2c_client *client = container_of(mc32x0_device.parent, struct i2c_client, dev);
    //struct mc32x0_data* this = (struct mc32x0_data *)i2c_get_clientdata(client);  /* 设备数据实例的指针. */
#endif

	switch (cmd) {
		case IOCTL_SENSOR_SET_DELAY_ACCEL:
			if(copy_from_user((void *)&sensor_duration, (void __user *) arg, sizeof(short))!=0){
				printk("copy from error in %s.\n",__func__);
			}

			break;

		case IOCTL_SENSOR_GET_DELAY_ACCEL:
			if(copy_to_user((void __user *) arg, (const void *)&sensor_duration, sizeof(short))!=0){
				printk("copy to error in %s.\n",__func__);
			} 

			break;

		case IOCTL_SENSOR_GET_STATE_ACCEL:
			if(copy_to_user((void __user *) arg, (const void *)&sensor_state_flag, sizeof(short))!=0){
				printk("copy to error in %s.\n",__func__);
			}

			break;

		case IOCTL_SENSOR_SET_STATE_ACCEL:
			if(copy_from_user((void *)&sensor_state_flag, (void __user *) arg, sizeof(short))!=0){
				printk("copy from error in %s.\n",__func__);
			}     

			break;
		case IOCTL_SENSOR_GET_NAME:
			if(copy_to_user((void __user *) arg,(const void *)mc32x0_DISPLAY_NAME, sizeof(mc32x0_DISPLAY_NAME))!=0){
				printk("copy to error in %s.\n",__func__);
			}     			
			break;		

		case IOCTL_SENSOR_GET_VENDOR:
			if(copy_to_user((void __user *) arg,(const void *)mc32x0_DIPLAY_VENDOR, sizeof(mc32x0_DIPLAY_VENDOR))!=0){
				printk("copy to error in %s.\n",__func__);
			}     			
			break;

		case IOCTL_SENSOR_GET_CONVERT_PARA:
			if(copy_to_user((void __user *) arg,(const void *)&convert_para,sizeof(float))!=0){
				printk("copy to error in %s.\n",__func__);
			}     			
			break;

        #ifdef DOT_CALI		
            case GSENSOR_IOCTL_READ_SENSORDATA:	
            case GSENSOR_IOCTL_READ_RAW_DATA:
            case GSENSOR_MCUBE_IOCTL_READ_RBM_DATA:
                GSE_LOG("fwq GSENSOR_IOCTL_READ_RAW_DATA\n");
			MC32X0_ReadRawData(client,strbuf);
			if (copy_to_user((void __user *) arg, &strbuf, strlen(strbuf)+1)) {
			printk("failed to copy sense data to user space.");
			return -EFAULT;
		}
		break;

	case GSENSOR_MCUBE_IOCTL_SET_CALI:
			GSE_LOG("fwq GSENSOR_MCUBE_IOCTL_SET_CALI!!\n");
			data1 = (void __user *)arg;

			if(data1 == NULL)
			{
				ret = -EINVAL;
				break;	  
			}
			if(copy_from_user(&sensor_data, data1, sizeof(sensor_data)))
			{
				ret = -EFAULT;
				break;	  
			}
			else
			{
				cali[MC32X0_AXIS_X] = sensor_data.x;
				cali[MC32X0_AXIS_Y] = sensor_data.y;
				cali[MC32X0_AXIS_Z] = sensor_data.z;	

			  	GSE_LOG("GSENSOR_MCUBE_IOCTL_SET_CALI %d  %d  %d  %d  %d  %d!!\n", cali[MC32X0_AXIS_X], cali[MC32X0_AXIS_Y],cali[MC32X0_AXIS_Z] ,sensor_data.x, sensor_data.y ,sensor_data.z);
				
				ret = MC32X0_WriteCalibration(client, cali);			 
			}
				
			break;
		
		case GSENSOR_IOCTL_CLR_CALI:
			GSE_LOG("fwq GSENSOR_IOCTL_CLR_CALI!!\n");
			ret = MC32X0_ResetCalibration(client);
			break;

		case GSENSOR_IOCTL_GET_CALI:
			GSE_LOG("fwq mc32x0 GSENSOR_IOCTL_GET_CALI\n");
			
			data1 = (unsigned char*)arg;
			
			if(data1 == NULL)
			{
				ret = -EINVAL;
				break;	  
			}
			
			if((ret = MC32X0_ReadCalibration(client,cali)))
			{
				GSE_LOG("fwq mc32x0 MC32X0_ReadCalibration error!!!!\n");
				break;
			}

                sensor_data.x = cali[MC32X0_AXIS_X];
                sensor_data.y = cali[MC32X0_AXIS_Y];
                sensor_data.z = cali[MC32X0_AXIS_Z];
                
			if(copy_to_user(data1, &sensor_data, sizeof(sensor_data)))
			{
				ret = -EFAULT;
				break;
			}		
			break;	
		// add by liang ****
		//add in Sensors_io.h
		//#define GSENSOR_IOCTL_SET_CALI_MODE   _IOW(GSENSOR, 0x0e, int)
		case GSENSOR_IOCTL_SET_CALI_MODE:
			GSE_LOG("fwq mc32x0 GSENSOR_IOCTL_SET_CALI_MODE\n");
			break;


		case GSENSOR_MCUBE_IOCTL_SET_RBM_MODE:
			GSE_LOG("fwq GSENSOR_MCUBE_IOCTL_SET_RBM_MODE\n");
			//MCUBE_BACKUP_FILE
			if(READ_FROM_BACKUP==true)
			{
			
				mcube_copy_file(CALIB_PATH);
			
				READ_FROM_BACKUP = false;
			}
			//MCUBE_BACKUP_FILE
			MC32X0_rbm(client, 1);

			break;

		case GSENSOR_MCUBE_IOCTL_CLEAR_RBM_MODE:
			GSE_LOG("fwq GSENSOR_MCUBE_IOCTL_CLEAR_RBM_MODE\n");

			MC32X0_rbm(client, 0);

			break;

		case GSENSOR_MCUBE_IOCTL_REGISTER_MAP:
                GSE_LOG("fwq GSENSOR_MCUBE_IOCTL_REGISTER_MAP\n");
                MC32X0_ReadRegMap(client, NULL);
                break;
            case GSENSOR_MCUBE_IOCTL_READ_PRODUCT_ID:
                GSE_LOG("fwq GSENSOR_MCUBE_IOCTL_READ_PRODUCT_ID\n");
                data1 = (void __user *) arg;
                if(data1 == NULL)
                {
                    ret = -EINVAL;
                    break;	  
                }
                
                temp = MC3XX0_DetectPcode(client);
                if (temp > 0)
                    temp = 0;
                else
                    temp = -1;
                if(copy_to_user(data1, &temp, sizeof(temp)))
                {
                    GSE_LOG("%s: read pcode fail to copy!\n", __func__);
                    return -EFAULT;
                }
                break;
#endif
		
		
		
		default:
			ret = -EINVAL;
			break;
	}

	return ret;
}

//=============================================================================
static int mc32x0_open(struct inode *inode, struct file *filp)
{
	return nonseekable_open(inode, filp);
}

//=============================================================================
static int mc32x0_release(struct inode *inode, struct file *filp)
{
	return 0;
}

//=============================================================================
static struct file_operations sensor_fops =
{
	.owner          = THIS_MODULE,
	.open       	= mc32x0_open,
	.release    	= mc32x0_release,
	.unlocked_ioctl = mc32x0_ioctl,
};

/*#ifdef CONFIG_HAS_EARLYSUSPEND
//=============================================================================
static void mc32x0_early_suspend(struct early_suspend *handler)
{
	struct mc32x0_data *data = NULL;

	data = container_of(handler, struct mc32x0_data, early_suspend);

	hrtimer_cancel(&data->timer);
	
	mc32x0_set_mode(data->client,MC32X0_STANDBY);
}

//=============================================================================
static void mc32x0_early_resume(struct early_suspend *handler)
{
	struct mc32x0_data *data = NULL;

	data = container_of(handler, struct mc32x0_data, early_suspend);
	
	//Add 20130722 
	mc32x0_chip_init(data->client); 
	MC32X0_ResetCalibration(data->client); 
	mcube_read_cali_file(data->client);
	//before
	
	mc32x0_set_mode(data->client,MC32X0_WAKE);


	hrtimer_start(&data->timer, ktime_set(1, 0), HRTIMER_MODE_REL);
}
#endif
*/
//=============================================================================

static void mc32x0_suspend(struct i2c_client *client)
{
	struct mc32x0_data *data = i2c_get_clientdata(client);

	hrtimer_cancel(&data->timer);
	
	mc32x0_set_mode(data->client,MC32X0_STANDBY);
}

//=============================================================================
static void mc32x0_resume(struct i2c_client *client)
{
	struct mc32x0_data *data = i2c_get_clientdata(client);

	//Add 20130722 
	mc32x0_chip_init(data->client); 
	MC32X0_ResetCalibration(data->client); 
	mcube_read_cali_file(data->client);
	//before
	
	mc32x0_set_mode(data->client,MC32X0_WAKE);

	hrtimer_start(&data->timer, ktime_set(1, 0), HRTIMER_MODE_REL);
}


static struct miscdevice mc32x0_device =
{
	.minor = MISC_DYNAMIC_MINOR,
	.name  = SENSOR_NAME,
	.fops  = &sensor_fops,
};



static int mc32x0_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	int ret = 0;
	int product_code = 0;
	struct mc32x0_data *data = NULL;
	
#ifdef DOT_CALI
		load_cali_flg = 30;
#endif

    product_code = MC3XX0_DetectPcode(client);

    if (product_code < 0)
    {
        GSE_ERR("[%s] not mCube g-sensor!\n", __func__);

        return -ENODEV;
    }

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) 
	{
		ret = -ENODEV;
		goto err_check_functionality_failed;
	}

	data = kzalloc(sizeof(struct mc32x0_data), GFP_KERNEL);
	if(data == NULL)
	{
		ret = -ENOMEM;
		goto err_alloc_data_failed;
	}

	data->mc32x0_wq = create_singlethread_workqueue("mc32x0_wq");
	if (!data->mc32x0_wq )
	{
		ret = -ENOMEM;
		goto err_create_workqueue_failed;
	}
	INIT_WORK(&data->work, mc32x0_work_func);
	mutex_init(&data->lock);

	sensor_duration = SENSOR_DURATION_DEFAULT;
	sensor_state_flag = 1;

	data->client = client;
	dev.client=client;

	i2c_set_clientdata(client, data);	

	data->input_dev = input_allocate_device();
	if (!data->input_dev) {
		ret = -ENOMEM;
		goto exit_input_dev_alloc_failed;
	}

    #ifdef DOT_CALI
        MC32X0_Reset(client);
    #endif

	ret = mc32x0_chip_init(client);
	if (ret < 0) {
		goto err_chip_init_failed;
	}

	set_bit(EV_ABS, data->input_dev->evbit);
	data->map[0] = G_0;
	data->map[1] = G_1;
	data->map[2] = G_2;
	data->inv[0] = G_0_REVERSE;
	data->inv[1] = G_1_REVERSE;
	data->inv[2] = G_2_REVERSE;

	input_set_abs_params(data->input_dev, ABS_X, -32*8, 32*8, INPUT_FUZZ, INPUT_FLAT);
	input_set_abs_params(data->input_dev, ABS_Y, -32*8, 32*8, INPUT_FUZZ, INPUT_FLAT);
	input_set_abs_params(data->input_dev, ABS_Z, -32*8, 32*8, INPUT_FUZZ, INPUT_FLAT);

	data->input_dev->name = SENSOR_NAME;

	ret = input_register_device(data->input_dev);
	if (ret) {
		goto exit_input_register_device_failed;
	}
	
    mc32x0_device.parent = &client->dev;
 
	ret = misc_register(&mc32x0_device);
	if (ret) {
		goto exit_misc_device_register_failed;
	}

	ret = sysfs_create_group(&client->dev.kobj, &mc32x0_group);

	if (!data->use_irq){
		hrtimer_init(&data->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
		data->timer.function = mc32x0_timer_func;
		hrtimer_start(&data->timer, ktime_set(1, 0), HRTIMER_MODE_REL);
	}

/*
#ifdef CONFIG_HAS_EARLYSUSPEND
	data->early_suspend.suspend = mc32x0_early_suspend;
	data->early_suspend.resume = mc32x0_early_resume;
	register_early_suspend(&data->early_suspend);
#endif
*/
	data->enabled = true;
	strcpy(mc32x0_on_off_str,"gsensor_int2");
	dprintk(DEBUG_INIT,"mc32x0 probe ok \n");

	return 0;
exit_misc_device_register_failed:
exit_input_register_device_failed:
	input_free_device(data->input_dev);
err_chip_init_failed:
exit_input_dev_alloc_failed:
	destroy_workqueue(data->mc32x0_wq);	
err_create_workqueue_failed:
	kfree(data);	
err_alloc_data_failed:
err_check_functionality_failed:
	printk("mc32x0 probe failed \n");

	return ret;
}

//=============================================================================
static int mc32x0_remove(struct i2c_client *client)
{
	struct mc32x0_data *data = i2c_get_clientdata(client);

	hrtimer_cancel(&data->timer);
	input_unregister_device(data->input_dev);	
	//gpio_release(mc32x0_pin_hd, 2);
	misc_deregister(&mc32x0_device);
	sysfs_remove_group(&client->dev.kobj, &mc32x0_group);
	kfree(data);

	return 0;
}

//=============================================================================
static void mc32x0_shutdown(struct i2c_client *client)
{
	struct mc32x0_data *data = i2c_get_clientdata(client);

	if(data->enabled)
		mc32x0_enable(data, 0);
}

//=============================================================================
static const struct i2c_device_id mc32x0_id[] =
{
	{ SENSOR_NAME, 0 },
	{ }
};

MODULE_DEVICE_TABLE(i2c, mc32x0_id);

static const struct of_device_id mc32x0_of_match[] = {
       {.compatible = "allwinner,sun8i-gsensor-para"},
       {},
};

static struct i2c_driver mc32x0_driver =
{
	.class = I2C_CLASS_HWMON,
	.driver = {
		.owner	= THIS_MODULE,
		.name	= SENSOR_NAME,
		.of_match_table = mc32x0_of_match,
	},
	.id_table	= mc32x0_id,
	.probe		= mc32x0_probe,
	.remove		= mc32x0_remove,
#if 0//sheen mask it for M	 kernel3.10
	.suspend	= mc32x0_suspend,
	.resume 	= mc32x0_resume,
	.shutdown	= mc32x0_shutdown,	
#endif	
	.detect = gsensor_detect,
	.address_list	= normal_i2c,
};

static int __init mc32x0_init(void)
{
	int ret = -1;
	dprintk(DEBUG_INIT, "======%s=========. \n", __func__);

	if(input_sensor_startup(&(gsensor_info.input_type))){
		printk("%s: err.\n", __func__);
		return -1;
	}else{
		ret = input_sensor_init(&(gsensor_info.input_type));
		if (0 != ret){
			printk("%s:ctp_ops.init_platform_resource err. \n", __func__);
		}
	}
		twi_id = gsensor_info.twi_id;

	dprintk(DEBUG_INIT,"%s: after fetch_sysconfig_para:  normal_i2c: 0x%hx.  \n", \
			__func__, normal_i2c[0]);
    
	input_set_power_enable(&(gsensor_info.input_type),1);//sheen add for M kernel3.10
	
	ret = i2c_add_driver(&mc32x0_driver);

	return ret;
}
static void __exit mc32x0_exit(void)
{
	i2c_del_driver(&mc32x0_driver);
	input_set_power_enable(&(gsensor_info.input_type),0);//sheen add for M kernel3.10
	input_sensor_free(&(gsensor_info.input_type));
}

//=============================================================================
module_init(mc32x0_init);
module_exit(mc32x0_exit);

MODULE_DESCRIPTION("mc32x0 accelerometer driver");
MODULE_AUTHOR("mCube-inc");
MODULE_LICENSE("GPL");
MODULE_VERSION(SENSOR_DRIVER_VERSION);

