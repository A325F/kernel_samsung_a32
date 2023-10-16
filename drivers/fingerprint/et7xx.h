/*
 * Copyright (C) 2016 Samsung Electronics. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */

#ifndef _ET7XX_LINUX_DRIVER_H_
#define _ET7XX_LINUX_DRIVER_H_

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/of.h>
#ifdef ENABLE_SENSORS_FPRINT_SECURE
#include <linux/clk.h>
#include <linux/pm_runtime.h>
#include <linux/spi/spidev.h>
#include <linux/of_device.h>
#include <linux/of_dma.h>
#endif /* ENABLE_SENSORS_FPRINT_SECURE */

#include <linux/regulator/consumer.h>
#include <linux/pinctrl/consumer.h>
#include "../pinctrl/core.h"
#include "fingerprint_common.h"

#define DISABLED_GPIO_PROTECTION

#define VENDOR							"EGISTEC"
#define CHIP_ID							"ET7XX"

/* assigned */
#define ET7XX_MAJOR						152
/* ... up to 256 */
#define N_SPI_MINORS					32
static DECLARE_BITMAP(minors, N_SPI_MINORS);
static LIST_HEAD(device_list);
static DEFINE_MUTEX(device_list_lock);

static unsigned int bufsiz = 1024;

/* spi communication opcode */
#define OP_REG_R						0x20
#define OP_REG_R_S						0x22
#define OP_REG_R_S_BW					0x23
#define OP_REG_W						0x24
#define OP_REG_W_S						0x26
#define OP_REG_W_S_BW					0x27
#define OP_EF_R							0x40
#define OP_EF_W							0x42
#define OP_FB_R							0x50
#define OP_FB_W							0x52
#define OP_ZAVG_R						0x60
#define OP_HSTG_R						0x62
#define OP_CIS_ADDR_R					0x71
#define OP_CIS_REG_W					0x70
#define OP_GET_FRAME					0x61
#define OP_PRE_CAPTURE					0x62

/* ioctl opcode for spi */
#ifndef ENABLE_SENSORS_FPRINT_SECURE
#define FP_REGISTER_READ				0x01
#define FP_REGISTER_BREAD				0x20
#define FP_REGISTER_BREAD_BACKWARD		0x24
#define FP_REGISTER_WRITE				0x02
#define FP_REGISTER_BWRITE				0x21
#define FP_REGISTER_BWRITE_BACKWARD		0x25
#define FP_EFUSE_READ					0x10
#define FP_EFUSE_WRITE					0x11
#define FP_GET_IMG						0x12
#define FP_WRITE_IMG					0x13
#define FP_GET_ZAVG						0x14
#define FP_GET_HSTG						0x15
#define FP_CIS_REGISTER_READ			0x08
#define FP_CIS_REGISTER_WRITE			0x09
#define FP_CIS_PRE_CAPTURE				0x0A
#define FP_GET_CIS_FRAME				0x03
#define FP_TRANSFER_COMMAND				0x0D

#define FP_EEPROM_READ					0x80
#define FP_EEPROM_HIGH_SPEED_READ		0x81
#define FP_EEPROM_WRITE					0x82
#define FP_EEPROM_CHIP_ERASE			0x83
#define FP_EEPROM_SECTOR_ERASE			0x84
#define FP_EEPROM_BLOCK_ERASE			0x85
#define FP_EEPROM_WREN					0x86
#define FP_EEPROM_WRDI					0x87
#define FP_EEPROM_RSDR					0x88
#define FP_EEPROM_WRITE_IN_NON_TZ		0x8A
#endif

/* ioctl opcode for other request */
#define FP_SENSOR_RESET					0x04
#define FP_POWER_CONTROL				0x05
#define FP_SET_SPI_CLOCK				0x06
#define FP_RESET_CONTROL				0x07
#define FP_DISABLE_SPI_CLOCK			0x10
#ifdef ENABLE_SENSORS_FPRINT_SECURE
#define FP_CPU_SPEEDUP					0x11
#define FP_SET_SENSOR_TYPE				0x14
/* Do not use ioctl number 0x15 */
#endif
#define FP_SPI_VALUE					0x1a
#define FP_MODEL_INFO					0x1f
#define INT_TRIGGER_READ				0xa6
#define INT_TRIGGER_POLLING				0xa7
#define FP_IOCTL_RESERVED_01			0x1b
#define FP_IOCTL_RESERVED_02			0x1c
#define FP_IOCTL_RESERVED_03			0xa4
#define FP_IOCTL_RESERVED_04			0xa5
#define FP_IOCTL_RESERVED_05			0xa8
#define FP_IOCTL_RESERVED_06			0x16
#define FP_IOCTL_RESERVED_07			0x17
#define FP_IOCTL_RESERVED_08			0x19

#define SLOW_BAUD_RATE					20000000
#define DIVISION_OF_IMAGE 4
#define LARGE_SPI_TRANSFER_BUFFER	64
#define DETECT_ADM 1

struct egis_ioc_transfer {
	u8 *tx_buf;
	u8 *rx_buf;
	__u32 len;
	__u32 speed_hz;
	__u16 delay_usecs;
	__u8 bits_per_word;
	__u8 cs_change;
	__u8 opcode;
	__u8 pad[3];
};

#define EGIS_IOC_MAGIC			'k'
#define EGIS_MSGSIZE(N) \
	((((N)*(sizeof(struct egis_ioc_transfer))) < (1 << _IOC_SIZEBITS)) \
		? ((N)*(sizeof(struct egis_ioc_transfer))) : 0)
#define EGIS_IOC_MESSAGE(N) _IOW(EGIS_IOC_MAGIC, 0, char[EGIS_MSGSIZE(N)])

struct et7xx_data {
	dev_t devt;
	spinlock_t spi_lock;
	struct spi_device *spi;
	struct list_head device_entry;

	/* buffer is NULL unless this device is open (users > 0) */
	struct mutex buf_lock;
	unsigned int users;
	u8 *buf;/* tx buffer for sensor register read/write */
	unsigned int bufsiz; /* MAX size of tx and rx buffer */
	unsigned int sleepPin;	/* Sleep GPIO pin number */
	unsigned int ldo_pin;	/* Ldo GPIO pin number */
	const char *rb;
	bool ldo_onoff;

	int sensortype;
	u32 spi_value;
	struct device *dev;
	struct device *fp_device;
	bool tz_mode;
	const char *chipid;
	const char *model_info;
	const char *btp_vdd;
	const char *sensor_position;
	struct regulator *regulator_3p3;
	struct pinctrl *p;
	struct pinctrl_state *pins_poweron;
	struct pinctrl_state *pins_poweroff;
	bool ldo_enabled;
	struct spi_clk_setting *clk_setting;
	struct boosting_config *boosting;
	struct debug_logger *logger;
};

int et7xx_io_read_cis_register(struct et7xx_data *etspi, u8 *addr, u8 *buf);
int et7xx_io_write_cis_register(struct et7xx_data *etspi, u8 *buf);
int et7xx_io_pre_capture(struct et7xx_data *etspi);
int et7xx_io_get_cis_frame(struct et7xx_data *etspi, u8 *fr, u32 size);

int et7xx_io_read_register(struct et7xx_data *etspi, u8 *addr, u8 *buf);
int et7xx_io_burst_read_register(struct et7xx_data *etspi,
									struct egis_ioc_transfer *ioc);
int et7xx_io_burst_read_register_backward(struct et7xx_data *etspi,
											struct egis_ioc_transfer *ioc);
int et7xx_io_write_register(struct et7xx_data *etspi, u8 *buf);
int et7xx_io_burst_write_register(struct et7xx_data *etspi,
									struct egis_ioc_transfer *ioc);
int et7xx_io_burst_write_register_backward(struct et7xx_data *etspi,
											struct egis_ioc_transfer *ioc);
int et7xx_io_read_efuse(struct et7xx_data *etspi,
							struct egis_ioc_transfer *ioc);
int et7xx_io_write_efuse(struct et7xx_data *etspi,
							struct egis_ioc_transfer *ioc);
int et7xx_io_get_frame(struct et7xx_data *etspi, u8 *fr, u32 size);
int et7xx_io_write_frame(struct et7xx_data *etspi, u8 *fr, u32 size);
int et7xx_io_get_zone_average(struct et7xx_data *etspi, u8 *fr, u32 size);
int et7xx_io_get_histogram(struct et7xx_data *etspi, u8 *fr, u32 size);
int et7xx_io_transfer_command(struct et7xx_data *etspi, u8 *tx, u8 *rx,
								u32 size);

int et7xx_read_register(struct et7xx_data *etspi, u8 addr, u8 *buf);
int et7xx_write_register(struct et7xx_data *etspi, u8 addr, u8 buf);

int et7xx_eeprom_rdsr(struct et7xx_data *etspi, u8 *value);
int et7xx_eeprom_read(struct et7xx_data *etspi, struct egis_ioc_transfer *ioc);
int et7xx_eeprom_high_speed_read(struct et7xx_data *etspi,
									struct egis_ioc_transfer *ioc);
int et7xx_eeprom_write(struct et7xx_data *etspi, struct egis_ioc_transfer *ioc);
int et7xx_eeprom_chip_erase(struct et7xx_data *etspi);
int et7xx_eeprom_sector_erase(struct et7xx_data *etspi,
								struct egis_ioc_transfer *ioc);
int et7xx_eeprom_block_erase(struct et7xx_data *etspi,
								struct egis_ioc_transfer *ioc);
int et7xx_eeprom_write_controller(struct et7xx_data *etspi, int enable);
int et7xx_eeprom_write_in_non_tz(struct et7xx_data *etspi,
									struct egis_ioc_transfer *ioc);
#endif
