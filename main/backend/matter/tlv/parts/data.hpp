#ifndef D174CAC2_38E2_46ED_8DC4_68114F306E9A
#define D174CAC2_38E2_46ED_8DC4_68114F306E9A

//
#include <cassert>
#include <cstdint>
#include <cstring>
#include <stdexcept>

//
namespace tlvcpp
{
    struct data_writer {
        uint8_t* memory  = nullptr;
        uintptr_t offset = 0;
        size_t capacity  = 0;

        //
        bool checkMemory(size_t size = 1) const { return ((offset + size) < capacity); }

        //
        data_writer(uint8_t* data = nullptr, size_t capacity = 0) : memory(data), offset(0), capacity(capacity) {};

        //
        data_writer& writeByte(uint8_t const& a) { *(uint8_t*)memory = a; offset += 1; return *this; };
        data_writer& writeShort(uint16_t const& a) { *(uint16_t*)memory = a; offset += 2; return *this; };
        data_writer& writeInt(uint32_t const& a) { *(uint32_t*)memory = a; offset += 4; return *this; };

        //
        data_writer& writeBytes(uint8_t const* a, size_t count) { memcpy(memory, a, count); offset += count; return *this; };
    };

    //
    struct data_reader {
        intptr_t reamin_size = 0;
        size_t offset = 0;
        uint8_t const* memory = nullptr;

        //
        data_reader(data_reader const& reader) : reamin_size(reader.reamin_size), offset(reader.offset), memory(reader.memory) {}
        data_reader(uint8_t const* data = nullptr, size_t size = 0) : reamin_size(size), offset(0), memory(data) {}

        //
        bool checkMemory(size_t size = 1) const {
            return (reamin_size >= intptr_t(size));
        }

        //
        int32_t readI32() { return *(int32_t*)readBytes(4); }
        int16_t readI16() { return *(int16_t*)readBytes(2); }
        int8_t readI8 () { return *(int8_t *)readBytes(1); }

        //
        uint32_t readU32() { return *(uint32_t*)readBytes(4); }
        uint16_t readU16() { return *(uint16_t*)readBytes(2); }
        uint8_t readU8 () { return *(uint8_t *)readBytes(1); }

        //
        uint8_t const* readBytes(size_t size) {
            if (reamin_size < intptr_t(size)) {
                throw std::runtime_error("Remain memory exceeded");
                return nullptr;
            }
            reamin_size -= size;
            auto ptr = memory + offset;
            offset += size;
            return ptr;
        }
    };
};

#endif /* D174CAC2_38E2_46ED_8DC4_68114F306E9A */
