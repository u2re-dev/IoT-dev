
//  mgream 2024
//  - significantly refactored

//  Copyright (C) 2010 Georg Kaindl
//  http://gkaindl.com
//
//  This file is part of Arduino EthernetBonjour.
//
//  EthernetBonjour is free software: you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public License
//  as published by the Free Software Foundation, either version 3 of
//  the License, or (at your option) any later version.
//
//  EthernetBonjour is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with EthernetBonjour. If not, see
//  <http://www.gnu.org/licenses/>.
//

#if ! defined(__MDNS_H__)
#define __MDNS_H__ 1

// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------

#include <Arduino.h>

#include <vector>
#include <set>

// #define DEBUG_MDNS
// #define DEBUG_MDNS_UDP_READ
// #define DEBUG_MDNS_UDP_WRITE

#if ! defined(DEBUG_PRINTF)
#ifdef DEBUG_MDNS
#define DEBUG_PRINTF Serial.printf
#else
#define DEBUG_PRINTF(...) \
    do {                  \
    } while (0)
#endif
#endif

// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------

class MDNSTXTBuilder;
class MDNSTXT {

public:
    static constexpr size_t KEY_LENGTH_MAX = 9;        // RFC recommendation
    static constexpr size_t TOTAL_LENGTH_MAX = 255;    // Per TXT string

    struct _Entry {
        String key;
        std::vector<uint8_t> value;
        bool binary;
    };
    using Entries = std::vector<_Entry>;

    using Builder = MDNSTXTBuilder;

private:
    Entries _entries;
    mutable uint16_t cached_length { 0 };
    mutable bool length_valid { false };
    bool validate (const String &key) const;

public:
    bool insert (const String &key, const void *value, const size_t length, const bool is_binary);

    inline const Entries &entries () const {
        return _entries;
    }
    inline size_t size () const {
        return _entries.size ();
    }
    uint16_t length () const;
    String toString () const;
};

// -----------------------------------------------------------------------------------------------

class MDNSServiceBuilder;
struct MDNSService {
    using TXT = MDNSTXT;

    struct Config {
        uint16_t priority = 0x0000;
        uint16_t weight = 0x0000;
        // std::vector<String> subtypes;
    };
    enum class Protocol {
        TCP,
        UDP
    };
    static String toString (const Protocol protocol) {
        if (protocol == Protocol::TCP)
            return "TCP";
        else if (protocol == Protocol::UDP)
            return "UDP";
        else
            return "Unknown";
    }

    uint16_t port;
    Protocol proto;
    String name;
    Config config {};
    TXT text {};
    String _serv {}, _fqsn {};

    using Builder = MDNSServiceBuilder;
};

// -----------------------------------------------------------------------------------------------

class MDNS {

public:
    struct TTLConfig {
        uint32_t announce = 120;     // Default announcement TTL
        uint32_t probe = 0;          // Probe TTL always 0
        uint32_t goodbye = 0;        // Goodbye/release TTL always 0
        uint32_t shared_max = 10;    // Maximum TTL for shared records per RFC
    };

    enum class Status {
        TryLater = 2,
        Success = 1,
        Failure = 0,
        InvalidArgument = -1,
        OutOfMemory = -2,
        ServerError = -3,
        PacketBad = -4,
        NameConflict = -5,
    };
    // clang-format off
    static String toString(const Status status) {
        switch (status) {
            case Status::TryLater: return "TryLater";
            case Status::Success: return "Success";
            case Status::Failure: return "Failure";
            case Status::InvalidArgument: return "InvalidArgument";
            case Status::OutOfMemory: return "OutOfMemory";
            case Status::ServerError: return "ServerError";
            case Status::PacketBad: return "PacketBad";
            case Status::NameConflict: return "NameConflict";
            default: return "Unknown";
        }
    }
    // clang-format on 

    using Service = MDNSService;
    using Services = std::vector<Service>;
    using ServiceTypes = std::set<String>;

private:
    UDP* _udp;
    IPAddress _addr;
    String _name, _fqhn, _arpa;
    TTLConfig _ttls;
    bool _enabled{ false };

    Status _messageRecv(void);
    Status _messageSend(const uint16_t xid, const int type, const Service* service = nullptr);

    unsigned long _announced{ 0 };
    Status _announce(void);
    Status _conflicted(void);

    Services _services;
    ServiceTypes _serviceTypes;
    void _writeAddressRecord(const uint32_t ttl) const;
    void _writeReverseRecord(const uint32_t ttl) const;
    void _writeServiceRecord(const Service& service, const uint32_t ttl) const;
    void _writeCompleteRecord(const uint32_t ttl) const;
    void _writeProbeRecord(const uint32_t ttl) const;
    void _writeNextSecureRecord(const String& name, const std::initializer_list<uint8_t>& types, const uint32_t ttl, const bool includeAdditional = false) const;

    inline uint32_t _announceTime() const {
        return ((_ttls.announce / 2) + (_ttls.announce / 4)) * static_cast<uint32_t>(1000);
    }

    Status serviceRecordInsert(const Service::Protocol proto, const uint16_t port, const String& name, const Service::Config& config = Service::Config(), const Service::TXT& text = Service::TXT());    // deprecated
    Status serviceRecordRemove(const Service::Protocol proto, const uint16_t port, const String& name);                                                                                                  // deprecated
    Status serviceRecordRemove(const String& name);                                                                                                                                                      // deprecated
    Status serviceRecordClear(void);                                                                                                                                                                     // deprecated

public:
    explicit MDNS(UDP& udp);
    virtual ~MDNS();

    Status begin(void);
    Status start(const IPAddress& addr, const String& name = String(), const bool checkForConflicts = false);
    Status process(void);
    Status stop(void);

    inline Status serviceInsert(const Service& service) {
        return serviceRecordInsert(service.proto, service.port, service.name, service.config, service.text);
    }
    inline Status serviceRemove(const Service& service) {
        return serviceRecordRemove(service.proto, service.port, service.name);
    }
    inline Status serviceRemove(const String& name) {
        return name.isEmpty () ? Status::InvalidArgument : serviceRecordRemove(name);
    }
    inline Status serviceClear(void) {
        return serviceRecordClear();
    }
};

// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------

#include <stdexcept>

class MDNSTXTBuilder {
private:
    MDNSTXT _record;

    inline MDNSTXTBuilder& _add(const String& key, const void* value, const size_t length, const bool is_binary) {
        if (!_record.insert(key, value, length, is_binary)) throw std::runtime_error("TXT insert failed");
        return *this;
    }

public:
    MDNSTXTBuilder()
        : _record{} {}

    inline MDNSTXT build() const {
        return _record;
    }
    inline operator MDNSTXT() const {
        return build();
    }

    inline MDNSTXTBuilder& add(const String& key) {
        return _add(key, nullptr, 0, false);
    }
    inline MDNSTXTBuilder& add(const String& key, const String& value) {
        return _add(key, reinterpret_cast<const uint8_t*>(value.c_str()), value.length(), false);
    }
    inline MDNSTXTBuilder& add(const String& key, const char* value) {
        return add(key, String(value));
    }
    inline MDNSTXTBuilder& add(const String& key, const bool value) {
        return add(key, String(value ? "true" : "false"));
    }
    inline MDNSTXTBuilder& add(const String& key, const int value) {
        return add(key, String(value));
    }
    inline MDNSTXTBuilder& add(const String& key, const uint8_t* value, const size_t length) {
        return _add(key, value, length, true);
    }
};

// -----------------------------------------------------------------------------------------------

class MDNSServiceBuilder {
private:
    using Service = MDNS::Service;

    Service _service;

    bool _hasName{ false }, _hasPort{ false }, _hasProtocol{ false };
    inline bool validate() const {
        return (_hasName && _hasPort && _hasProtocol);
    }

public:
    MDNSServiceBuilder()
        : _service{} {}

    MDNS::Service build() const {
        if (!validate())
            throw std::runtime_error("Invalid service configuration: missing required fields");
        return _service;
    }
    operator MDNS::Service() const {
        return build();
    }

    MDNSServiceBuilder& withName(const String& name) {
        _service.name = name;
        _hasName = true;
        return *this;
    }
    MDNSServiceBuilder& withPort(uint16_t port) {
        _service.port = port;
        _hasPort = true;
        return *this;
    }
    MDNSServiceBuilder& withProtocol(Service::Protocol proto) {
        _service.proto = proto;
        _hasProtocol = true;
        return *this;
    }
    MDNSServiceBuilder& withConfig(const Service::Config& config) {
        _service.config = config;
        return *this;
    }
    MDNSServiceBuilder& withPriority(uint16_t priority) {
        _service.config.priority = priority;
        return *this;
    }
    MDNSServiceBuilder& withWeight(uint16_t weight) {
        _service.config.weight = weight;
        return *this;
    }
    MDNSServiceBuilder& withTXT(const Service::TXT& txt) {
        _service.text = txt;
        return *this;
    }
};

// -----------------------------------------------------------------------------------------------

#endif    // __MDNS_H__
