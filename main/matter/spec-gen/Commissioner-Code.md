# Matter Comission Code

**Enums (перечисления):**

* **Custom Flow (Раздел 5.1.1.3):**
  * 0: Standard commissioning flow
  * 1: User-intent commissioning flow
  * 2: Custom commissioning flow
  * 3: Reserved

**C/C++ структуры:**

* **Packed Binary Data Structure for Onboarding Payload (Таблица 39):**

```c
struct OnboardingPayload {
  /* here is 11 bytes bitfield flow, aka 88-bit */
  uint88_t version                        : 3;
  uint88_t vendor_id                      : 16;
  uint88_t product_id                     : 16;
  uint88_t custom_flow                    : 2;
  uint88_t discovery_capabilities_bitmask : 8;
  uint88_t discriminator                  : 12;
  uint88_t passcode                       : 27;
  uint88_t padding                        : 4;
  
  uint8_t tlv_data[]; // переменная длина
};
```

```c

// если нужно просто
struct OnboardingPayloadHeader {
  uint8_t   version;
  uint16_t  vendor_id;
  uint16_t  product_id;
  uint8_t   custom_flow;
  uint8_t   discovery_capabilities_bitmask;
  uint16_t  discriminator;
  uint32_t  passcode;
  uint8_t   padding;
};

// только внутри программмы, сохраняет компактность в памяти
// но передавать такое некорректно
struct OnboardingPayloadHeaderRepack {
  uint32_t version     : 3;
  uint32_t passcode    : 27;
  uint32_t custom_flow : 2;
  
  uint16_t vendor_id ; //: 16;
  uint16_t product_id; //: 16;
  
  uint16_t discriminator : 12;
  uint16_t padding       : 4;
  
  uint8_t discovery_capabilities_bitmask : 8;
};
```

Чтение таких битов.

```cpp
#include <cstdint>
#include <iostream>

struct OnboardingPayload {
    uint8_t data[11]; // 88 бит = 11 байт

    // Извлечение полей из массива данных
    uint8_t getVersion() const {
        return data[0] & 0b111; // Первые три бита
    }

    uint16_t getVendorId() const {
        return ((data[0] >> 3) | (data[1] << 5)) & 0xFFFF; // Следующие 16 бит
    }

    uint16_t getProductId() const {
        return ((data[2]) | (data[3] << 8)) & 0xFFFF; // Следующие 16 бит
    }

    uint8_t getCustomFlow() const {
        return (data[4] & 0b11); // Первые два бита пятого байта
    }

    uint8_t getDiscoveryCapabilitiesBitmask() const {
        return (data[4] >> 2) & 0xFF; // Следующие восемь бит
    }

    uint16_t getDiscriminator() const {
        return ((data[5] >> 6) | (data[6] << 2) | ((data[7] & 0b1111) << 10)) & 0xFFF; // Следующие двенадцать бит
    }

    uint32_t getPasscode() const {
        return ((data[7] >> 4) | (data[8] << 4) | (data[9] << 12) | ((uint32_t)(data[10]) << 20)) & ((1u << 27) - 1); // Следующие двадцать семь бит
    }

    uint8_t getPadding() const {
        return data[10] >> (27 - (20 + sizeof(uint32_t)));
    }
}
```

* **IdentificationDeclaration-struct (Раздел 5.3.5):**

    ```c
    struct IdentificationDeclaration {
      uint16_t vendor_id; // optional
      uint16_t product_id; // optional
      char device_name[33]; // optional, string length 0..32 + null terminator
      uint32_t device_type; // optional
      char pairing_instruction[33]; // optional, string length 0..32 + null terminator
      uint32_t pairing_hint; // optional
      char rotating_device_id[101]; // optional, string length 0..100 + null terminator
      uint16_t port; // optional
      // TargetAppList: ARRAY OF { TargetApp } // optional
      bool no_passcode; // optional
      bool cd_upon_passcode_dialog; // optional
      bool commissioner_passcode; // optional
      bool commissioner_passcode_ready; // optional
      bool cancel_passcode; // optional
    };
    ```

* **CommissionerDeclaration-struct (Раздел 5.3.6):**

    ```c
    struct CommissionerDeclaration {
      uint16_t error_code; // optional
      bool needs_passcode; // optional
      bool no_apps_found; // optional
      bool passcode_dialog_displayed; // optional
      bool commissioner_passcode; // optional
      bool qr_code_displayed; // optional
      bool cancel_passcode; // optional
    };
    ```

**Данные из спецификации (ключевые моменты):**

* **Назначение:** В документе описывается процесс ввода в эксплуатацию устройств в сети Matter, включая содержимое полезной нагрузки для ввода в эксплуатацию, представление в виде QR-кода, ручного кода и NFC-метки.
* **Onboarding Payload (полезная нагрузка для ввода в эксплуатацию):** Содержит версию, Vendor ID, Product ID, Custom Flow, Discovery Capabilities Bitmask, Discriminator, Passcode и TLV Data.
* **QR Code:** Полезная нагрузка кодируется в Base-38 и включается в QR-код.
* **Manual Pairing Code:** Состоит из Version, VID\_PID\_PRESENT, Discriminator, PASSCODE и Check Digit, закодированных в 11- или 21-значную строку.
* **TLV Content:** Дополнительные данные в формате Tag-Length-Value, включающие Manufacturer-specific Elements и Matter-common Elements.
* **User Directed Commissioning (UDC):** Протокол для инициирования ввода в эксплуатацию устройством (Commissionee) с отправкой сообщения Commissioner'у.
* **Device Discovery:** Описывает, как устройства объявляют о своей готовности к вводу в эксплуатацию с использованием BLE, Wi-Fi Public Action Frame или IP-сети.
* **Commissioning Flows:** Описывает Concurrent connection commissioning flow и Non-concurrent connection commissioning flow.
* **Administrator Assisted Commissioning Flows:** Описывает Basic Commissioning Method (BCM) и Enhanced Commissioning Method (ECM).
* **Device Commissioning Flows:** Описывает Standard Commissioning Flow, User-Intent Commissioning Flow и Custom Commissioning Flow.
* **Enhanced Setup Flow (ESF):** Описывает способ для устройства Custom Commissioning Flow указать в стандартизированном виде набор дополнительных шагов ввода в эксплуатацию, которые требуются устройству.
* **Commissioning Fallback Mechanism:** Описывает механизм, который позволяет Commissioner'у направить пользователя к предоставленному производителем механизму, где пользователь может получить рекомендации по решению проблем.
* **Manual Pairing Code and QR Code Inclusion:** Описывает требования и рекомендации относительно QR Code и Manual Pairing code для устройств Standard и User Intent Commissioning Flow.
* **In-field Upgrade to Matter:** Описывает случай предустановленного устройства Matter, которое получает обновление программного обеспечения для поддержки Matter.
