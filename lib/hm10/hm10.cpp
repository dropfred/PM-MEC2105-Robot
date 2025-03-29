#include <hm10.h>

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include <esp_log.h>

// BLECharacteristic::setValue() takes non const data (wtf?)
#ifndef HM10_YOLO_CONST_CAST_DATA
  #include <array>
  #include <algorithm>
  #include <cstring>
  #ifndef HM10_COPY_DATA_BUFFER_SIZE
    #define HM10_COPY_DATA_BUFFER_SIZE ESP_GATT_MAX_ATTR_LEN
  #endif
#endif

namespace
{
    char const * TAG = "HM10";

    BLEUUID const HM10_SERVICE        {"0000FFE0-0000-1000-8000-00805F9B34FB"};
    BLEUUID const HM10_CHARACTERISTIC {"0000FFE1-0000-1000-8000-00805F9B34FB"};

    HM10::ConnectCb connect_cb {};
    HM10::WriteCb write_cb {};

    struct : public BLEServerCallbacks
    {
        virtual void onConnect(BLEServer * server) override
        {
            ESP_LOGD(TAG, "onConnect");
            if (connect_cb)
            {
                connect_cb(true);
            }
        }

        virtual void onConnect(BLEServer * server, esp_ble_gatts_cb_param_t * param) override
        {
            ESP_LOGD(TAG, "onConnect (param)");
        }

        virtual void onDisconnect(BLEServer * server) override
        {
            ESP_LOGD(TAG, "onDisconnect");
            if (connect_cb)
            {
                connect_cb(false);
            }
        }

        virtual void onDisconnect(BLEServer * server, esp_ble_gatts_cb_param_t * param) override
        {
            ESP_LOGD(TAG, "onDisconnect (param)");
        }

        virtual void onMtuChanged(BLEServer * server, esp_ble_gatts_cb_param_t * param) override
        {
            ESP_LOGD(TAG, "onMtuChanged");
        }

    } server_callback;

    struct : public BLECharacteristicCallbacks
    {
        virtual void onRead(BLECharacteristic * c) override
        {
            ESP_LOGD(TAG, "onRead");
        };
        
        virtual void onWrite(BLECharacteristic * c) override
        {
            uint8_t const * data = c->getData();
            size_t size = c->getLength();
            ESP_LOGD(TAG, "onWrite: %zu", size);
            if (write_cb)
            {
                write_cb(data, size);
            }
        };

        virtual void onNotify(BLECharacteristic * c) override
        {
            ESP_LOGD(TAG, "onNotify");
        }

        virtual void onStatus(BLECharacteristic * c, Status s, uint32_t code) override
        {
            ESP_LOGD(TAG, "onStatus");
        }
    } characteristic_callback {};

    BLE2902 descriptor {};
    
    BLEServer * server = nullptr;
    BLEService * service = nullptr;
    BLECharacteristic * characteristic = nullptr;
    BLEAdvertising * advertising = nullptr;
}

bool HM10::start(std::string const & name)
{
    bool ok = (server == nullptr);
    if (ok)
    {
        ESP_LOGD(TAG, "BLE start");

        BLEDevice::init(name);

        server = BLEDevice::createServer();
        server->setCallbacks(&server_callback);

        service = server->createService(HM10_SERVICE);

        descriptor.setNotifications(true);

        characteristic = service->createCharacteristic
        (
            HM10_CHARACTERISTIC,
            BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE_NR |
            BLECharacteristic::PROPERTY_NOTIFY
        );
        characteristic->setCallbacks(&characteristic_callback);
        characteristic->addDescriptor(&descriptor);

        service->start();

        advertising = server->getAdvertising();
        advertising->addServiceUUID(service->getUUID());
        advertising->start();

        ESP_LOGD(TAG, "BLE started");
    }
    return ok;
}

void HM10::stop()
{
    // TODO
}

bool HM10::set(uint8_t const * data, size_t size)
{
    bool ok = server != nullptr;
    if (ok)
    {
#ifdef HM10_YOLO_CONST_CAST_DATA
        // YOLO, LET'S GO UB!
        characteristic->setValue(const_cast<uint8_t *>(data), size);
#else
  #ifdef HM10_COPY_DATA_BUFFER_STATIC
        static
  #endif
        std::array<uint8_t, HM10_COPY_DATA_BUFFER_SIZE> buffer {};
        size_t s = std::min(size, buffer.size());
        std::memcpy(buffer.data(), data, s);
        characteristic->setValue(buffer.data(), s);
#endif
        characteristic->notify(true);
    }
    return ok;
}

bool HM10::set(std::string const & data)
{
    return set(reinterpret_cast<uint8_t const * >(data.c_str()), data.size());
}

void HM10::onConnect(ConnectCb cb)
{
    connect_cb = cb;
}

void HM10::onWrite(WriteCb cb)
{
    write_cb = cb;
}
