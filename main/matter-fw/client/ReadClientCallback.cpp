#include "./ReadClientCallback.hpp"

//
#ifdef ENABLE_MATTER
namespace Callback {

    //
    void ReadClient::OnAttributeData(const chip::app::ConcreteDataAttributePath &aPath, chip::TLV::TLVReader *aReader, const chip::app::StatusIB &aStatus) { ESP_LOGI(TAG, "Received attribute data for cluster: %" PRIu32, aPath.mClusterId); }
    void ReadClient::OnEventData(const chip::app::EventHeader &aEventHeader, chip::TLV::TLVReader *apData, const chip::app::StatusIB *aStatus) { ESP_LOGI(TAG, "Received event data"); }
    void ReadClient::OnError(CHIP_ERROR aError) { ESP_LOGI(TAG, "ReadClient error: %" CHIP_ERROR_FORMAT, aError.Format()); }
    void ReadClient::OnDone(chip::app::ReadClient *apReadClient) { ESP_LOGI(TAG, "ReadClient finished"); }

    //
    ReadClient readClient;

};
#endif
