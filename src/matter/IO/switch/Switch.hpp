#ifndef HOST_IO_H
#define HOST_IO_H

#include <esp_err.h>

// Определение типа дескриптора для аппаратного ввода (например, кнопок)
typedef void *app_driver_handle_t;

class HostIO {
public:
    HostIO();
    ~HostIO();

    //
    esp_err_t init();
    esp_err_t registerIOCallbacks();
    app_driver_handle_t getSwitchHandle() const;

private:
    app_driver_handle_t mSwitchHandle;
    esp_err_t initSwitch();
};

#endif // HOST_IO_H
