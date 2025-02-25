#ifndef D193BBED_1DDC_4CC0_AD00_0C9E1A6017FB
#define D193BBED_1DDC_4CC0_AD00_0C9E1A6017FB

//
#include "../tlv.h"

//
namespace tlvcpp {
    //
    inline uint8_t octets(uint8_t bits) {
        switch (bits & 0x03) {
            case 0x00: return 1;
            case 0x01: return 2;
            case 0x02: return 4;
            case 0x03: return 8;
        }; return 1;
    }

    //
    inline bool writeOctets(writer_t& writer, tlv const& value, uint8_t const& type) {
        uint8_t octet = type&0b00000011;
        switch (octet) {
            case 0: writer.writeByte(value); break;
            case 1: writer.writeUInt16(value); break;
            case 2: writer.writeUInt32(value); break;
            case 3: writer.writeUInt64(value); break;
        }
        return true;
    }

    //
    inline bool readOctets(reader_t& reader, auto& value, uint8_t const& type) {
        uint8_t octet = type&0b00000011;
        value = uint64_t(0);
        switch (octet) {
            case 0: if (!reader.checkMemory(1)) return false; value = reader.readByte(); break;
            case 1: if (!reader.checkMemory(2)) return false; value = reader.readUInt16(); break;
            case 2: if (!reader.checkMemory(4)) return false; value = reader.readUInt32(); break;
            case 3: if (!reader.checkMemory(8)) return false; value = reader.readUInt64(); break;
        }
        return true;
    }

};

#endif
