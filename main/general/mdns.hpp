#ifndef E38E8496_E60A_4D6C_B956_5E8E97701982
#define E38E8496_E60A_4D6C_B956_5E8E97701982

//
#include <iostream>
#include <avahi-client/client.h>
#include <avahi-client/publish.h>
#include <avahi-common/error.h>
#include <avahi-common/malloc.h>
#include <avahi-common/thread-watch.h>

//
inline void entry_group_callback(AvahiEntryGroup *g, AvahiEntryGroupState state, void *userdata) {
    if (state == AVAHI_ENTRY_GROUP_ESTABLISHED) { std::cout << "Service successfully established." << std::endl; } else
    if (state == AVAHI_ENTRY_GROUP_COLLISION)   { std::cerr << "Service name collision detected!"  << std::endl; } else
    if (state == AVAHI_ENTRY_GROUP_FAILURE)     { std::cerr << "Failed to establish service: "     << avahi_strerror(avahi_client_errno(avahi_entry_group_get_client(g))) << std::endl; }
}

//
inline void client_callback(AvahiClient *c, AvahiClientState state, void *userdata) {
    if (state == AVAHI_CLIENT_FAILURE) { std::cerr << "Avahi client failure: " << avahi_strerror(avahi_client_errno(c)) << std::endl; }
}

//
class MDNS {
private:
    const int port = 5540;
    const char *subtype1 = "_S3._sub._matterc._udp";
    const char *subtype2 = "_CM._sub._matterc._udp";
    const char *subtype3 = "_L840._sub._matterc._udp";
    const char *subtype4 = "_T261._sub._matterc._udp";
    const char *service_name = "DD200C20D25AE5F7";
    const char *service_type = "_matterc._udp";

    //
    const char *TXT_discr = "D=840";
    const char *TXT_com = "CM=1";
    const char *TXT_device_type = "DT=261";

public:

    //
    ~MDNS() {
        avahi_threaded_poll_stop(poll);
        avahi_entry_group_free(group);
        avahi_client_free(client);
        avahi_threaded_poll_free(poll);
    }

    //
    inline int init() {
        poll = avahi_threaded_poll_new(); if (!poll) { std::cerr << "Failed to create Avahi threaded poll object." << std::endl; return 1; }
        int error = 0; client = avahi_client_new(avahi_threaded_poll_get(poll), AVAHI_CLIENT_NO_FAIL, client_callback, nullptr, &error);
        if (!client) { std::cerr << "Failed to create Avahi client: " << avahi_strerror(error) << std::endl; avahi_threaded_poll_free(poll); return 1; }

        //
        group = avahi_entry_group_new(client, entry_group_callback, nullptr);
        if (!group) { std::cerr << "Failed to create Avahi entry group: " << avahi_strerror(avahi_client_errno(client)) << std::endl; avahi_client_free(client); avahi_threaded_poll_free(poll); return 1; }

        //
        return 0;
    }

    //
    inline int commit () {
        //
        if (avahi_entry_group_commit(group) < 0) {
            std::cerr << "Failed to commit service: " << avahi_strerror(avahi_client_errno(client)) << std::endl;
            avahi_entry_group_free(group);
            avahi_client_free(client);
            avahi_threaded_poll_free(poll);
            return 1;
        }

        //
        std::cout << "Service published. Listening for UDP messages on port " << port << "..." << std::endl;
        avahi_threaded_poll_start(poll);
        return 0;
    }

    //
    inline int service() {
        if (avahi_entry_group_add_service(
            group,
            AVAHI_IF_UNSPEC,  // Любой интерфейс
            AVAHI_PROTO_UNSPEC,  // Любой протокол (IPv4/IPv6)
            AVAHI_PUBLISH_USE_MULTICAST, //static_cast<AvahiPublishFlags>(0),
            service_name,
            service_type,
            nullptr,  // Домен (по умолчанию "local")
            nullptr,  // Хост (по умолчанию текущий хост)
            port,

            // TXT-records
            TXT_discr, TXT_com, TXT_device_type,
            nullptr
        ) < 0)
        {
            std::cerr << "Failed to add service: " << avahi_strerror(avahi_client_errno(client)) << std::endl;
            avahi_entry_group_free(group);
            avahi_client_free(client);
            avahi_threaded_poll_free(poll);
            return 1;
        }

        //
        if (avahi_entry_group_add_service_subtype(group, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, (AvahiPublishFlags)0,  service_name, service_type, nullptr, subtype1) < 0 ||
            avahi_entry_group_add_service_subtype(group, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, (AvahiPublishFlags)0,  service_name, service_type, nullptr, subtype2) < 0 ||
            avahi_entry_group_add_service_subtype(group, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, (AvahiPublishFlags)0,  service_name, service_type, nullptr, subtype3) < 0 ||
            avahi_entry_group_add_service_subtype(group, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, (AvahiPublishFlags)0,  service_name, service_type, nullptr, subtype4) < 0) {

            //
            std::cerr << "Failed to add service subtype: " << avahi_strerror(avahi_client_errno(client)) << std::endl;
            avahi_entry_group_free(group);
            avahi_client_free(client);
            avahi_threaded_poll_free(poll);
            return 1;
        }

        //
        return 0;
    }

private:
    AvahiClient *client = nullptr;
    AvahiThreadedPoll *poll = nullptr;
    AvahiEntryGroup *group = nullptr;
};

//
#endif
