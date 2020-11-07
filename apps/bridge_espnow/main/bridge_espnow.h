#ifndef BRIDGE_ESPNOW_H
#define BRIDGE_ESPNOW_H

#define ESPNOW_QUEUE_SIZE   20

#define ESPNOW_WIFI_MODE WIFI_MODE_AP
#define ESPNOW_WIFI_IF   ESP_IF_WIFI_AP

typedef enum {
    BRIDGE_ESPNOW_SEND_CB,
    BRIDGE_ESPNOW_RECV_CB,
} bridge_espnow_event_id_t;

typedef struct {
    uint8_t mac_addr[ESP_NOW_ETH_ALEN];
    esp_now_send_status_t status;
} bridge_espnow_event_send_cb_t;

typedef struct {
    uint8_t mac_addr[ESP_NOW_ETH_ALEN];
    uint8_t *data;
    int data_len;
} bridge_espnow_event_recv_cb_t;

typedef union {
    bridge_espnow_event_send_cb_t send_cb;
    bridge_espnow_event_recv_cb_t recv_cb;
} bridge_espnow_event_info_t;

typedef struct {
    bridge_espnow_event_id_t    id;
    bridge_espnow_event_info_t  info;
} bridge_espnow_event_t;

typedef struct {
    uint16_t    id;
    uint16_t    crc;
} pattern_t;

#endif // BRIDGE_ESPNOW_H