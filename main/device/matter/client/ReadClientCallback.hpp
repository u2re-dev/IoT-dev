#pragma once

//
#ifdef ENABLE_MATTER
#include <esp_err.h>
#include <esp_matter.h>

//
#include <client/CHIPClient.h
#include <chip/CHIPError.h>

//
namespace Callback {

    // Callback‑класс для ReadClient (при подписке на атрибуты)
    class ReadClient : public chip::app::ReadClient::Callback {
    public:
        void OnAttributeData(const chip::app::ConcreteDataAttributePath &aPath, chip::TLV::TLVReader *aReader, const chip::app::StatusIB &aStatus) override;
        void OnEventData(const chip::app::EventHeader &aEventHeader, chip::TLV::TLVReader *apData, const chip::app::StatusIB *aStatus) override;
        void OnError(CHIP_ERROR aError) override;
        void OnDone(chip::app::ReadClient *apReadClient) override;
    };

    //
    extern ReadClient readClient;
};
#endif
