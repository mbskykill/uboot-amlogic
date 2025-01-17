/*
 * AMLOGIC lcd external driver.
 *
 * Communication protocol:
 * I2C
 *
 */

#include <common.h>
#include <asm/arch/io.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/gpio.h>
#include <asm/arch/lcd_reg.h>
#include <asm/arch/clock.h>
#include <asm/arch/timing.h>
#include <aml_i2c.h>
#include <amlogic/aml_lcd_extern.h>
#include <i2c.h>
#include <asm/arch/i2c.h>
#include "lcd_extern.h"

#define LCD_EXTERN_INDEX		0
#define LCD_EXTERN_NAME			"i2c_T5800Q"
#define LCD_EXTERN_TYPE			LCD_EXTERN_I2C

#define LCD_EXTERN_I2C_ADDR		(0x38 >> 1) //7bit address
#define LCD_EXTERN_I2C_BUS		AML_I2C_MASTER_A

static struct lcd_extern_config_t *ext_config;
extern int aml_i2c_xfer(struct i2c_msg *msgs, int num);
//extern int aml_i2c_xfer_slow(struct i2c_msg *msgs, int num);

#define INIT_LEN        7
static unsigned char i2c_init_table[][INIT_LEN] = {
	//QFHD 50/60Hz 1 division Video Mode ：
	{0x20, 0x01, 0x02, 0x00, 0x40, 0xFF, 0x00},
	{0x80, 0x02, 0x00, 0x40, 0x62, 0x51, 0x73},
	{0x61, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0xC1, 0x05, 0x0F, 0x00, 0x08, 0x70, 0x00},
	{0x13, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x3D, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00},	//Color Engine Bypass Enable
	{0xED, 0x0D, 0x01, 0x00, 0x00, 0x00, 0x00},	//Mute only when starting
	{0x23, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00},	//MEMC off
	{0xFF, 0xFF, 0x0A, 0xFF, 0xFF, 0xFF, 0xFF},
	{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}
};

static int lcd_extern_i2c_write(unsigned i2caddr, unsigned char *buff, unsigned len)
{
	int ret = 0;
#ifdef LCD_EXT_DEBUG_INFO
	int i;
#endif
	struct i2c_msg msg;


	msg.addr = 0x1c;//i2caddr;
	msg.flags = 0;
	msg.len = 7;//len;
	msg.buf = buff;

#ifdef LCD_EXT_DEBUG_INFO
	printf("%s:", __func__);
	for (i = 0; i < len; i++) {
		printf(" 0x%02x", buff[i]);
	}
	printf(" [addr 0x%02x]\n", i2caddr);
#endif

	ret = aml_i2c_xfer(&msg, 1);
	//ret = aml_i2c_xfer_slow(&msg, 1);
	if (ret < 0)
		LCD_EXT_PR("i2c write failed [addr 0x%02x]\n", i2caddr);

	return ret;
}

static int lcd_extern_reg_read(unsigned char reg, unsigned char *buf)
{
	int ret = 0;

	return ret;
}

static int lcd_extern_reg_write(unsigned char reg, unsigned char value)
{
	int ret = 0;

	return ret;
}

static int lcd_extern_i2c_init(void)
{
	int i = 0, ending_mark = 0;
	int ret = 0;

	while (ending_mark == 0) {
		if ((i2c_init_table[i][0] == 0xff) && (i2c_init_table[i][1] == 0xff)) { //special mark
			if (i2c_init_table[i][2] == 0xff) //ending mark
				ending_mark = 1;
			else //delay mark
				mdelay(i2c_init_table[i][2]);
		} else {
			ret = lcd_extern_i2c_write(ext_config->i2c_addr, i2c_init_table[i], INIT_LEN);
		}
		i++;
	}
	LCD_EXT_PR("%s: %s\n", __func__, ext_config->name);
	return ret;
}

static int lcd_extern_i2c_remove(void)
{
	int ret = 0;

	return ret;
}

static struct aml_lcd_extern_pinmux_t aml_lcd_extern_pinmux_set[] = {
	{.reg = 4, .mux = ((1 << 2) | (1 << 3)),},
};

static struct aml_lcd_extern_pinmux_t aml_lcd_extern_pinmux_clr[] = {
	{.reg = 4, .mux = ((1 << 6) | (1 << 7)),},
	{.reg = 1, .mux = ((1 << 18) | (1 << 19) | (1 << 23) | (1 << 26)),},
};

static int lcd_extern_port_init(void)
{
	int i;
	unsigned pinmux_reg, pinmux_data;

	for (i = 0; i < ARRAY_SIZE(aml_lcd_extern_pinmux_clr); i++) {
		pinmux_reg = PERIPHS_PIN_MUX_0+aml_lcd_extern_pinmux_clr[i].reg;
		pinmux_data = ~(aml_lcd_extern_pinmux_clr[i].mux);
		WRITE_LCD_CBUS_REG(pinmux_reg, READ_LCD_CBUS_REG(pinmux_reg) & pinmux_data);
	}
	for (i = 0; i < ARRAY_SIZE(aml_lcd_extern_pinmux_set); i++) {
		pinmux_reg = PERIPHS_PIN_MUX_0+aml_lcd_extern_pinmux_set[i].reg;
		pinmux_data = aml_lcd_extern_pinmux_set[i].mux;
		WRITE_LCD_CBUS_REG(pinmux_reg, READ_LCD_CBUS_REG(pinmux_reg) | pinmux_data);
	}

	return 0;
}

#if 0
static int lcd_extern_change_i2c_bus(unsigned aml_i2c_bus)
{
	int ret = 0;
	extern struct aml_i2c_platform g_aml_i2c_plat;

	g_aml_i2c_plat.master_no = aml_i2c_bus;
	ret = aml_i2c_init();

	return ret;
}
#endif

static int lcd_extern_power_on(void)
{
	int ret = 0;
	//extern struct aml_i2c_platform g_aml_i2c_plat;
	//aml_i2c_bus_tmp = g_aml_i2c_plat.master_no;

	lcd_extern_port_init();
	//lcd_extern_change_i2c_bus(lcd_ext_config.i2c_bus);
	//aml_i2c_init();
	ret = lcd_extern_i2c_init();
	//lcd_extern_change_i2c_bus(aml_i2c_bus_tmp);

	return ret;
}

static int lcd_extern_power_off(void)
{
	int ret = 0;
	//extern struct aml_i2c_platform g_aml_i2c_plat;

	//aml_i2c_bus_tmp = g_aml_i2c_plat.master_no;

	//lcd_extern_port_init();
	//lcd_extern_change_i2c_bus(lcd_ext_config.i2c_bus);
	ret = lcd_extern_i2c_remove();
	//lcd_extern_change_i2c_bus(aml_i2c_bus_tmp);

	return ret;
}

static int lcd_extern_driver_update(struct aml_lcd_extern_driver_t *ext_drv)
{
	if (ext_drv == NULL) {
		LCD_EXT_PR("%s driver is null\n", LCD_EXTERN_NAME);
		return -1;
	}

	if (ext_drv->config.type == LCD_EXTERN_MAX) { //default for no dt
		ext_drv->config.index = LCD_EXTERN_INDEX;
		ext_drv->config.type = LCD_EXTERN_TYPE;
		strcpy(ext_drv->config.name, LCD_EXTERN_NAME);
		ext_drv->config.i2c_addr = LCD_EXTERN_I2C_ADDR;
		ext_drv->config.i2c_bus = LCD_EXTERN_I2C_BUS;
	}
	ext_drv->reg_read  = lcd_extern_reg_read;
	ext_drv->reg_write = lcd_extern_reg_write;
	ext_drv->power_on  = lcd_extern_power_on;
	ext_drv->power_off = lcd_extern_power_off;

	return 0;
}

int aml_lcd_extern_i2c_T5800Q_get_default_index(void)
{
	return LCD_EXTERN_INDEX;
}

int aml_lcd_extern_i2c_T5800Q_probe(struct aml_lcd_extern_driver_t *ext_drv)
{
	int ret = 0;

	ext_config = &ext_drv->config;
	ret = lcd_extern_driver_update(ext_drv);

	DBG_PRINT("%s: %d\n", __func__, ret);
	return ret;
}

