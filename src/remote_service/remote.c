#include "remote.h"

#define LOG_MODULE_NAME remote
LOG_MODULE_REGISTER(LOG_MODULE_NAME);

static K_SEM_DEFINE(bt_init_ok, 1, 1);

#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME)-1)


/*adv data 
 only BLE not classic bluetooth - BT_LE_AD_NO_BREDR
*/
static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN)

};

static uint8_t button_value = 0;
static struct bt_remote_srv_cb remote_service_callbacks;
enum bt_button_notifications_enabled notifications_enabled;
/*scan response data*/
static const struct bt_data sd[] = {
    BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_REMOTE_SERV_VAL)
};


/*declarations*/
static ssize_t read_button_characteristic_cb(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset);
void button_chrc_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value);

BT_GATT_SERVICE_DEFINE(remote_srv, 
    BT_GATT_PRIMARY_SERVICE(BT_UUID_REMOTE_SERVICE),
        BT_GATT_CHARACTERISTIC(BT_UUID_REMOTE_BUTTON_CHRC, BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY, BT_GATT_PERM_READ, read_button_characteristic_cb, NULL, NULL),
        BT_GATT_CCC(button_chrc_ccc_cfg_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),

);


/*callbacks*/
void button_chrc_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value){
    bool notif_enabled = (value == BT_GATT_CCC_NOTIFY);
    LOG_INF("Notifications = %s", notif_enabled ? "enabled":"disabled");

    notifications_enabled = notif_enabled ? BT_BUTTON_NOTIFICATIONS_ENABLED:BT_BUTTON_NOTIFICATIONS_DISABLED;

    if(remote_service_callbacks.notif_changed){
        remote_service_callbacks.notif_changed(notifications_enabled);
    }  
}


static ssize_t read_button_characteristic_cb(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset){
    return bt_gatt_attr_read(conn, attr, buf, len, offset, &button_value, sizeof(button_value));
}

void on_sent(struct bt_conn *conn, void *user_data){
    ARG_UNUSED(user_data);
    LOG_INF("Notification sent on connection %p", (void*) conn);
}




void bt_ready(int err){
    if(err){
        LOG_ERR("bt_enable returned %d", err);
    }
    k_sem_give(&bt_init_ok);
}

int send_button_notification(struct bt_conn *conn, uint8_t value, uint16_t length){
    int err = 0;

    struct bt_gatt_notify_params params = {0};
    const struct bt_gatt_attr *attr = &remote_srv.attrs[2];

    params.attr = attr;
    params.data = &value;
    params.len = length;
    params.func = on_sent;

    err = bt_gatt_notify_cb(conn, &params);

    return err;
}





void set_button_value(uint8_t btn_value){
    button_value = btn_value;
}

int bluetooth_init(struct bt_conn_cb *bt_cb, struct bt_remote_srv_cb *remote_cb){
    int err;
    LOG_INF("Initializing bluetooth");

    if(bt_cb == NULL || remote_cb == NULL){
        return -NRFX_ERROR_NULL;
    }

    bt_conn_cb_register(bt_cb);
    remote_service_callbacks.notif_changed = remote_cb->notif_changed;




    err = bt_enable(bt_ready);
    if(err){
        LOG_ERR("bt_enable returned %d", err);
        return err;
    }    
    k_sem_take(&bt_init_ok, K_FOREVER);
    err = bt_le_adv_start(BT_LE_ADV_CONN, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
    if(err){
        LOG_ERR("couldn`t start advertising err = %d", err);
        return err;
    }

    return err;
}


