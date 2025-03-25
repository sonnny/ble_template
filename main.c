
#define BTSTACK_FILE__ "gatt_counter.c"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Pico headers
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

// BT stack headers
#include "btstack.h"
#include "btstack_run_loop.h"
#include "ble/gatt-service/battery_service_server.h"

// App headers
#include "pico_demo_gatt_service.h"
#include "gap_config.h"

static int le_notification_enabled;
static btstack_packet_callback_registration_t hci_event_callback_registration;
static hci_con_handle_t con_handle;

#ifdef ENABLE_GATT_OVER_CLASSIC
static uint8_t gatt_service_buffer[70];
#endif

/*
 * function att_write_callback()
 */
static int att_write_callback(hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size){
switch (att_handle){
  case ATT_CHARACTERISTIC_0000FF11_0000_1000_8000_00805F9B34FB_01_CLIENT_CONFIGURATION_HANDLE:
    le_notification_enabled = little_endian_read_16(buffer, 0) == GATT_CLIENT_CHARACTERISTICS_CONFIGURATION_NOTIFICATION;
    con_handle = connection_handle;
    break;
  case ATT_CHARACTERISTIC_0000FF11_0000_1000_8000_00805F9B34FB_01_VALUE_HANDLE:
    printf_hexdump(buffer, buffer_size);
    break;
  default:
    break;} return 0;}

/*
 * function setup_gatt_service()
 */
static void setup_gatt_service(void){
    l2cap_init();
    sm_init();
#ifdef ENABLE_GATT_OVER_CLASSIC
    // init SDP, create record for GATT and register with SDP
    sdp_init();
    memset(gatt_service_buffer, 0, sizeof(gatt_service_buffer));
    gatt_create_sdp_record(gatt_service_buffer, sdp_create_service_record_handle(), ATT_SERVICE_GATT_SERVICE_START_HANDLE, ATT_SERVICE_GATT_SERVICE_END_HANDLE);
    btstack_assert(de_get_len( gatt_service_buffer) <= sizeof(gatt_service_buffer));
    sdp_register_service(gatt_service_buffer);

    // configure Classic GAP
    gap_set_local_name("GATT Counter BR/EDR 00:00:00:00:00:00");
    gap_ssp_set_io_capability(SSP_IO_CAPABILITY_DISPLAY_YES_NO);
    gap_discoverable_control(1);
#endif
    // no read just write callback
    att_server_init(profile_data, NULL, att_write_callback);
    hci_event_callback_registration.callback = &packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);
    att_server_register_packet_handler(packet_handler);}

int main(){
  stdio_init_all();
  cyw43_arch_init();
  sleep_ms(1000);
  setup_gatt_service();
  hci_power_control(HCI_POWER_ON);
  btstack_run_loop_execute();}
