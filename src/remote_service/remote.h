#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>


#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/hci.h>

/*@brief UUID of the remote service*/
#define BT_UUID_REMOTE_SERV_VAL BT_UUID_128_ENCODE(0xe9ea0001, 0xe19b, 0x482d, 0x9293, 0xc7907585fc48)
#define BT_UUID_REMOTE_SERVICE BT_UUID_DECLARE_128(BT_UUID_REMOTE_SERV_VAL)





int bluetooth_init(void);
