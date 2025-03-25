#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "thread/pt_cornell_rp2040_v1_3.h"
#include "ble/ble.h"

char ble_data[80];

static PT_THREAD (ble_thread(struct pt *pt)){
  PT_BEGIN(pt);
  while(1){
    PT_SEM_SAFE_WAIT(pt, &BLUETOOTH_READY);
    printf("data from ble: %s\n", ble_data);
  }
  PT_END(pt);}
  
static PT_THREAD (blink_thread(struct pt *pt)){
  PT_BEGIN(pt);
  static bool led_state = false;
  gpio_init(15);
  gpio_set_dir(15, GPIO_OUT);
  PT_INTERVAL_INIT();
  while(1){
    led_state = !led_state;
    gpio_put(15, led_state);
    PT_YIELD_INTERVAL(1000000);}
  PT_END(pt);}

void main(void){

stdio_init_all();
sleep_ms(500);
multicore_launch_core1(bt_main);
pt_add_thread(ble_thread);
pt_add_thread(blink_thread);
pt_sched_method = SCHED_ROUND_ROBIN;
pt_schedule_start;}

