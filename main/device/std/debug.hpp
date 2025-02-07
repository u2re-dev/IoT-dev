#pragma once

//
#include <string>

//
std::string DebugLog(std::string message);
std::string DebugLine(std::string message);
std::string DebugLineWithInterval(std::string message, unsigned long interval = 100);
std::string DebugCode(uint8_t const* code, size_t length = 0);
