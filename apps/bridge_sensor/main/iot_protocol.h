#ifndef _IOT_PROTOCOL_H_
#define _IOT_PROTOCOL_H_


typedef struct {
    uint16_t    magic;
    uint16_t    chksum;
    uint32_t    seq_id;
    uint16_t    options;
    uint16_t    payload_len;
} iot_packet_header_t;

typedef struct {
    
typedef enum {
    IOT_CMD_DISCOVERY
} iot_command_id_t;

} iot_command_t;
    iot_command_id_t    id;
typedef struct {

} iot_data_t;

Iot Protocol Over EpsNow
IPROVES
IotProtocolEspNow
IPEN 
CIpenDevice -> CIpenGateway
            -> CIpenNode    -> CIpenSleepNode

typedef union {
    iot_command_t   cmd;
    iot_data_t      data;
} iot_packet_info_t;

typedef struct {
    iot_packet_header_t header;
    iot_packet_info_t   info;
} iot_packet_t;


typedef void* iot_packet_handle_t;
iot_packet_handle_t iot_packet_create();
iot_err_t iot_packet_delete(iot_packet_handle_t iot_packet);

iot_device_addr = 0x0500;

SEND
    CLEAR SEND bit
    CLEAR RECV bit
    espnow_send_cb ( BIT );
    wait_for_send_bit();
    wait_for_recv_bit();




#endif // _IOT_PROTOCOL_H_