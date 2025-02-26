# Matter Commission Code

**Enums:**

* **Custom Flow (Chapter 5.1.1.3):**
  * 0: Standard commissioning flow
  * 1: User-intent commissioning flow
  * 2: Custom commissioning flow
  * 3: Reserved

**C/C++ structs:**

* **Packed Binary Data Structure for Onboarding Payload (Table 39):**

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
  std::vector<uint8_t> tlv_data;
};
```

```c
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

//
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

How to reads those bit-fields?

```cpp
#include <cstdint>
#include <iostream>

struct OnboardingPayload {
    uint8_t data[11];
    uint8_t getVersion() const {
        return data[0] & 0b111;
    }

    uint16_t getVendorId() const {
        return ((data[0] >> 3) | (data[1] << 5)) & 0xFFFF;
    }

    uint16_t getProductId() const {
        return ((data[2]) | (data[3] << 8)) & 0xFFFF;
    }

    uint8_t getCustomFlow() const {
        return (data[4] & 0b11);
    }

    uint8_t getDiscoveryCapabilitiesBitmask() const {
        return (data[4] >> 2) & 0xFF;
    }

    uint16_t getDiscriminator() const {
        return ((data[5] >> 6) | (data[6] << 2) | ((data[7] & 0b1111) << 10)) & 0xFFF;
    }

    uint32_t getPasscode() const {
        return ((data[7] >> 4) | (data[8] << 4) | (data[9] << 12) | ((uint32_t)(data[10]) << 20)) & ((1u << 27) - 1);
    }

    uint8_t getPadding() const {
        return data[10] >> (27 - (20 + sizeof(uint32_t)));
    }
}
```

* **IdentificationDeclaration-struct (Chapter 5.3.5):**

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

* **CommissionerDeclaration-struct (Chapter 5.3.6):**

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
