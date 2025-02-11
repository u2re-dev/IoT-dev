#pragma once

//
#include "../../std/std.hpp"
#include "../hal/arduino.hpp"

//
namespace nv {

    //
    inline char const* _concat_(char const * one, char const * two) {
        char* result = new char[strlen(one)+strlen(two)];
        strcpy(result, one);
        strcat(result, two);
        return result;
    }

#ifdef USE_ARDUINO
    static Preferences storage;

    //
    template<class T>
    class nv_t {
    protected:
        char const* key = 0;

    public:
        nv_t() {

        }

        explicit nv_t(char const* key) : key(key) {

        }

        void setKey(char const* key) {
            this->key = key;
        }

        char const* getKey() const {
            return this->key;
        }

        operator T() const {
            T _tmp_;
            storage.getBytes(this->key, &_tmp_, sizeof(T));
            return _tmp_;
        }

        nv_t& operator =(T const& _var_) {
            //storage.put<T>(this->address, _var_);
            storage.putBytes(this->key, &_var_, sizeof(T));
            return *this;
        }
    };
#endif


#ifdef USE_ARDUINO
    class nv_float {
    protected:
        char const* key = 0;

    public:
        nv_float() {

        }

        explicit nv_float(char const* key) : key(key) {

        }

        void setKey(char const* key) {
            this->key = key;
        }

        char const* getKey() const {
            return this->key;
        }

        operator float() const {
            return storage.getFloat(this->key);
        }

        nv_float& operator =(float _var_) {
            storage.putFloat(this->key, _var_);
            return *this;
        }
    };
#else
    using nv_float = float;
#endif

    //
#ifdef USE_ARDUINO
    class nv_uint8_t {
    protected:
        char const* key = 0;

    public:
        nv_uint8_t() {

        }

        explicit nv_uint8_t(char const* key) : key(key) {

        }

        void setKey(char const* key) {
            this->key = key;
        }

        char const* getKey() const {
            return this->key;
        }

        operator uint8_t() const {
            return storage.getUChar(this->key);
        }

        nv_uint8_t& operator =(uint8_t _var_) {
            storage.putUChar(this->key, _var_);
            return *this;
        }
    };
#else
    using nv_uint8_t = uint8_t;
#endif

    //
#ifdef USE_ARDUINO
    class nv_uint32_t {
    protected:
        char const* key = 0;

    public:
        nv_uint32_t() {

        }

        explicit nv_uint32_t(char const* key) : key(key) {

        }

        void setKey(char const* key) {
            this->key = key;
        }

        char const* getKey() const {
            return this->key;
        }

        operator uint32_t() const {
            return storage.getUInt(this->key);
        }

        nv_uint32_t& operator =(uint32_t _var_) {
            storage.putUInt(this->key, _var_);
            return *this;
        }
    };
#else
    using nv_uint32_t = uint32_t;
#endif

    //
#ifdef USE_ARDUINO
    class nv_int32_t {
    protected:
        char const* key = 0;

    public:
        nv_int32_t() {

        }

        explicit nv_int32_t(char const* key) : key(key) {

        }

        void setKey(char const* key) {
            this->key = key;
        }

        char const* getKey() const {
            return this->key;
        }

        operator int32_t() const {
            return storage.getInt(this->key);
        }

        nv_int32_t& operator =(int32_t _var_) {
            storage.putInt(this->key, _var_);
            return *this;
        }
    };
#else
    using nv_int32_t = int32_t;
#endif

    //
#ifdef USE_ARDUINO
    class nv_bool {
    protected:
        char const* key = 0;

    public:
        nv_bool() {

        }

        explicit nv_bool(char const* key) : key(key) {

        }

        void setKey(char const* key) {
            this->key = key;
        }

        char const* getKey() const {
            return this->key;
        }

        operator bool() const {
            return storage.getBool(this->key);
        }

        nv_bool& operator =(bool _var_) {
            storage.putBool(this->key, _var_);
            return *this;
        }
    };
#else
    using nv_bool = bool;
#endif

};
