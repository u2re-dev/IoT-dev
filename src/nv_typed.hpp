#pragma once

//#include <EEPROM.h>
#include <Preferences.h>

//
static Preferences nvs;

//
char const* _concat_(char const * one, char const * two) {
    char* result = new char[strlen(one)+strlen(two)];
    strcpy(result, one);
    strcat(result, two);
    return result;
}

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
        nvs.getBytes(this->key, &_tmp_, sizeof(T));
        return _tmp_;
    }

    nv_t& operator =(T const& _var_) {
        //nvs.put<T>(this->address, _var_);
        nvs.putBytes(this->key, &_var_, sizeof(T));
        return *this;
    }
};


//
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
        return nvs.getFloat(this->key);
    }

    nv_float& operator =(float _var_) {
        nvs.putFloat(this->key, _var_);
        return *this;
    }
};

//
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
        return nvs.getUChar(this->key);
    }

    nv_uint8_t& operator =(uint8_t _var_) {
        nvs.putUChar(this->key, _var_);
        return *this;
    }
};

//
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
        return nvs.getUInt(this->key);
    }

    nv_uint32_t& operator =(uint32_t _var_) {
        nvs.putUInt(this->key, _var_);
        return *this;
    }
};

//
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
        return nvs.getInt(this->key);
    }

    nv_int32_t& operator =(int32_t _var_) {
        nvs.putInt(this->key, _var_);
        return *this;
    }
};

//
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
        return nvs.getBool(this->key);
    }

    nv_bool& operator =(bool _var_) {
        nvs.putBool(this->key, _var_);
        return *this;
    }
};
