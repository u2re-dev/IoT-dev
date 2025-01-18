#pragma once

//
#include <std/std.hpp>
#include <IPAddress.h>

//
void binary_hex(uint8_t const* pin, char* pout, size_t blen);
uint8_t tallymarker_hextobin(const char * str, uint8_t * bytes, size_t blen);
uint32_t bswap32(uint32_t const& num);
void store32(uint32_t* ptr, uint32_t _a_);
void store32(uint8_t* ptr, uint32_t _a_);

//
inline bool compareIP(uint8_t const* a, uint8_t const* b) { return a[0]==b[0]&&a[1]==b[1]&&a[2]==b[2]&&a[3]==b[3]; }
inline bool compareIP(IPAddress const& a, uint8_t const* b) { return a[0]==b[0]&&a[1]==b[1]&&a[2]==b[2]&&a[3]==b[3]; }
inline bool compareIP(IPAddress const& a, IPAddress const& b) { return a[0]==b[0]&&a[1]==b[1]&&a[2]==b[2]&&a[3]==b[3]; }

//
static const uint64_t unix_shift = 946684800;
int64_t getTimestamp(int year, int mon, int mday, int hour, int min, int sec);
