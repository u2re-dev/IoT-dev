#pragma once

//
#include "./nv_typed.hpp"

//
#include "../std/string.hpp"

//
namespace nv {

    //
    template<size_t MAX_STRING_LENGTH=16>
    class _NvString_ {
        nv_uint8_t _length_;
        char const* _chars_ = 0;
        volatile char* _code_ = 0;

    public: 
        _NvString_() {
            _code_ = new char[MAX_STRING_LENGTH];
            bzero((char*)_code_, MAX_STRING_LENGTH);
        }

        explicit _NvString_(char const* key) : _chars_(_concat_(key, "_str")) {
            _code_ = new char[MAX_STRING_LENGTH];
            bzero((char*)_code_, MAX_STRING_LENGTH);
            this->_length_.setKey(_concat_(key, "_len"));
        }

        void setKey(char const* key) {
            this->_length_.setKey(_concat_(key, "_len"));
            this->_chars_ = _concat_(key, "_str");
        }

        char const* getKey() const {
            return this->_chars_;
        }

        _NvString_& operator =(_StringView_ _str_) {
            storage.putBytes(_chars_, _str_.bytes(), _length_ = min(_str_.length(), MAX_STRING_LENGTH));
            return *this;
        }

        _NvString_& operator =(char const* _str_) {
            storage.putBytes(_chars_, _str_, _length_ = min(strlen(_str_), MAX_STRING_LENGTH));
            return *this;
        }

        String toString() const {
            uint8_t _code_[uint8_t(_length_)];
            storage.getBytes(_chars_, _code_, _length_); if (_length_ < MAX_STRING_LENGTH) { _code_[_length_] = 0u; };
            return cString(_code_, _length_);
        }

        operator char const*() const {
            storage.getBytes(_chars_, (char*)_code_, _length_); if (_length_ < MAX_STRING_LENGTH) { _code_[_length_] = 0u; };
            return (char*)_code_;
        }

        char const* bytes() const {
            storage.getBytes(_chars_, (char*)_code_, _length_); if (_length_ < MAX_STRING_LENGTH) { _code_[_length_] = 0u; };
            return (char*)_code_;
        }

        uint8_t length() const {
            return _length_;
        }


#ifdef ENABLE_ARDUINO_STRING
        _NvString_& operator =(String const& _str_) {
            storage.putBytes(_chars_, _str_.bytes(), _length_ = min(_str_.length(), MAX_STRING_LENGTH));
            return *this;
        }

        explicit operator String() const {
            uint8_t _code_[uint8_t(_length_)];
            storage.getBytes(_chars_, _code_, _length_); if (_length_ < MAX_STRING_LENGTH) { _code_[_length_] = 0u; };
            return cString(_code_, _length_);
        }
#endif
    };

};
