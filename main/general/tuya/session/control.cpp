#include "../session.hpp"
#include <chrono>

//
namespace th {
#ifdef USE_ARDUINO_JSON
    bytespan_t TuyaSession::setDPS(ArduinoJson::JsonObject const &dps)
#else
    bytespan_t TuyaSession::setDPS(json const &dps)
#endif
    {
        sending["protocol"] = 5;
        //sending["t"] = uint64_t(getUnixTime()) * 1000ull;
        sending["t"] = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();;

        //
        data["dps"] = dps;

        //
#ifdef USE_ARDUINO_JSON
        sending["data"] = data.as<ArduinoJson::JsonObject>();
#else
        sending["data"] = data;
#endif

        // protocol 3.3
        // sending["devId"] = device_id;
        // sending["uid"] = device_uid;
        // sending["dps"] = dps;
        // sendJSON(0x7, sending);

        // protocol 3.4
        return encodeJSON(0xd, sending);
    }
}
