/*
 * \file        cmd_detect_sys_recovery_key.c
 * \brief       Detect whether user want to enter sys_recovery
 *
 * \version     1.0.0
 * \date        16/09/08
 * \author      Sam.Wu <yihui.wu@amlgic.com>
 *
 * Copyright (c) 2014 Amlogic. All Rights Reserved.
 *
 */
#include <config.h>
#include <common.h>
#include <asm/arch/io.h>
#include <command.h>
#include <malloc.h>
#include <amlogic/gpio.h>

#define debugP(fmt...) //printf("L%d:", __LINE__),printf(fmt)
#define errorP(fmt...) printf("[ERR]L%d:", __LINE__),printf(fmt)
#define MsgP(fmt...)   printf("[msg]"fmt)

static int store_key_open(const char* keyName, const char* keyType)
{
        int pin = -1;

        pin=gpioname_to_pin(keyName);
        if(pin<0){
                errorP("wrong gpio name %s\n",keyName);
                return -1;
        }
        udelay(100);

        return pin;
}

static int is_sys_recovery_key_pressed(int hKey)
{
        int val = -1;

        val=amlogic_get_value(hKey);
        
        return val == 1;
}

static int assert_key_is_pressed_in_a_period(unsigned nMillSeconds, const char* keyName, const char* keyType)
{
        unsigned start = 0;
        int hKey = -1;

        hKey = store_key_open(keyName, keyType);
        if(hKey < 0){
                errorP("Fail to init key for aml_sysrecovery, hKey=%d\n", hKey);
                return 1;
        }

        if(!is_sys_recovery_key_pressed(hKey)){
                return 1;
        }

        MsgP("pin=%d\n",hKey);
        start = get_timer(0);
        while(is_sys_recovery_key_pressed(hKey))
        {
                const unsigned pressTime = (unsigned)get_timer(start) ;
                if(pressTime > nMillSeconds){
                        MsgP("store key pressed time %d[ms]\n", pressTime);
                        return 0;
                }
        }

        MsgP("key released in time %u[ms]\n", (unsigned)get_timer(start));

        return 1;
}

//test If the recovery_key pressed time >= @nMillSeconds
int do_sys_rec_key(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int rcode = 0;
    unsigned nMillSeconds = 0;
    const char* keyName =  NULL;
    const char* keyType = "gpio";

    if(2 > argc) {
        cmd_usage(cmdtp);
        return __LINE__;
    }

    nMillSeconds = simple_strtoul(argv[1], NULL, 0);
    keyName = argc > 2 ? argv[2] :"GPIOAO_13";
    rcode = assert_key_is_pressed_in_a_period(nMillSeconds, keyName, keyType);

    return rcode;
}


U_BOOT_CMD(
   get_gpio_key,      //command name
   5,               //maxargs
   0,               //repeatable
   do_sys_rec_key,   //command function
   "check if user press sys_recovery key",           //description
   "Usage: sys_recovery nMillSeconds [GPIOAO_13] [key_type]\n"   //usage
);
