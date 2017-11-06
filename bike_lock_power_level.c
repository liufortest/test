/**
 *****************************************************************************
 * @file     bike_lock_adc_sample.c
 * @author   liujinshuai
 * @version  V1.0.0
 * @date     17-10-2017
 * @brief    
 *****************************************************************************
 * @attention
 *
 *
 */
#include "system_config.h"
#include "bike_lock_config.h"
#include "app_ble.h"

extern device_state_t           device_state;

uint16                   curr_voltage = 3600;   //unit : mv
uint8                    curr_power_level = 100;


extern uint16 read_charger_out_front_end_vol(void);
extern uint16 read_charger_out_behind_end_vol(void);

extern void charger_ic_bq25601_en(uint8 chr_en);
extern uint8 get_bq25601_status(void);

/*******************************************
get charge status
*******************************************/
uint8 power_get_bat_charge_status(void)
{
    uint8 status=0;

    status = get_bq25601_status();
    status &= 0x18;
    if((status == 0x08)||(status == 0x10))  //Detection of charger status
    {
        device_state.battery_is_charging = 1;
        return 1;

    }
    else
    {
        device_state.battery_is_charging = 0;
        return 0;
    }

}

/*******************************************
battery adc value Convert to voltage
*******************************************/
uint16 power_get_bat_vol(void)
{
    uint16 bat_vol=0,min=0,max=0,sum=0;
    uint8 i;
    charger_ic_bq25601_en(0);
    for (i = 0; i < 22; i++)
    {
        bat_vol = read_charger_out_behind_end_vol();
        min = (min == 0)? bat_vol:((bat_vol < min )? bat_vol:min);
        max = (max == 0)? bat_vol:((bat_vol> max )? bat_vol:max);
        sum += bat_vol;
        APP_DBG(("bat_adc[%d]: %d\r\n", i,bat_vol));
    }
    charger_ic_bq25601_en(1);
    bat_vol = (sum - max - min)/20;
    APP_DBG(("vbat vol is: %d\n", bat_vol));

    //bat_vol=((bat_vol*1000*1200*2.8)/(1023*200));
    bat_vol=((bat_vol*1000*6*2.8)/(1023));
    curr_voltage = bat_vol;
    return bat_vol;
}

/*******************************************
battery percentage
*******************************************/
uint8 power_get_battery_percentage(void)
{
    uint32 vbat = 0;
    uint8 current_percentage = 0;
    vbat = power_get_bat_vol();
    //vbat = 3711;
    APP_DBG(("BQ25601: vbat vol is: %d\n", vbat));
    if(vbat > 4100)
    {
        current_percentage = 100;
    }
    else if (vbat >4030)
    {
        current_percentage = 90 + (vbat - 4030)/7;
    }
    else if(vbat > 3920)
    {
        current_percentage = 80 + (vbat - 3920)/11;
    }
    else if(vbat > 3810)
    {
        current_percentage = 70 + (vbat - 3810)/11;
    }
    else if(vbat > 3710)
    {
        current_percentage = 60 + (vbat - 3710)/10;
    }
    else if(vbat > 3650)
    {
        current_percentage = 50 + (vbat - 3650)/6;
    }
    else if(vbat > 3610)
    {
        current_percentage = 40 + (vbat - 3610)/4;
    }
    else if(vbat > 3580)
    {
        current_percentage = 30 + (vbat - 3580)/3;
    }
    else if(vbat > 3540)
    {
        current_percentage = 20 + (vbat - 3540)/4;
    }
    else if(vbat > 3450)
    {
        current_percentage = 10 + (vbat - 3450)/9;
    }
    else if(vbat > 2750)
    {
        current_percentage = 0 + (vbat - 2750)/70;
    }
    else
    {
        current_percentage = 0;
    }

    if(current_percentage >= 10)
        current_percentage = (current_percentage - 10) * 10 / 9;
    else
        current_percentage = 0;
    //return current_percentage;
    curr_power_level = current_percentage;
    APP_DBG(("BQ25601: current_percentage: %d\n", current_percentage));
    return current_percentage;
}
/*******************************************
power_timer_timeout_handler
*******************************************/
void power_timer_timeout_handler(void)
{
    static uint8 times_cnt = 0;
    static uint8 times_second = 0;
    static uint8 times_minute = 0;
    times_cnt++;
    if(times_cnt < (1000/TIMER_TIMEOUT_INTERVAL))
    {
        return;
    }
    times_cnt = 0;
    times_second++;
    if(times_second<60)
    {
        return;
    }
    times_second = 0;
    times_minute++;
    if(times_minute<5)
    {
        return;
    }
    times_minute = 0;
    ble_peri_sendMsgToApp(MSG_BIKE_LOCK, MSG_BIKELOCK_EXECUTE_TASK, MSG_BIKE_LOCK_POWER, 0, NULL);
}
/*******************************************
power_bat_vol_handler
*******************************************/
void power_bat_vol_handler(void)
{
    power_get_battery_percentage();
    APP_DBG(("MSG_ADC_BATVOL %d\r\n",MSG_BIKE_LOCK_POWER));
}
/*******************************************
power_calc_charge_current
*******************************************/
uint16 power_calc_charge_current(void)
{
    uint16 current=0;
    uint16 charge_behind_adc=5;
    uint16 charge_front_adc=10;
    uint8 status=0;
    status = get_bq25601_status();
    status &= 0x18;
    if((status == 0x08)||(status == 0x10))  //Detection of charger status
    {
        charge_front_adc = read_charger_out_front_end_vol();
        charge_behind_adc = read_charger_out_behind_end_vol();
        APP_DBG(("MSG_ADC_CURRENT %d charge_current_adc_status %d\r\n",MSG_BIKE_LOCK_CHARGE,status));
        if(charge_behind_adc >= charge_front_adc)
        {
            APP_DBG(("charge_behind_adc >= charge_front_adc"));
            APP_DBG(("charge_behind_adc :%d charge_front_adc:%d",charge_behind_adc,charge_front_adc));
            return 0;
        }

        //r_vol=(((charge_front_adc-charge_behind_adc)*1000*1000*1200*2.8)/(1023*200*20));
        current=(((charge_front_adc - charge_behind_adc)*1000*100*6*2.8)/(1023*2));
        APP_DBG(("power_calc_charge_current %d\r\n",current));

        return current;
    }
    else
    {
      return 0;
    }
}
