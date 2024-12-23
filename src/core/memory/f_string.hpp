#pragma once

//
#include "../imports/imports.hpp"

//
inline String cString(char const* data, size_t length) {
    String _str_(data);
    _str_.reserve(std::min(_str_.length(), length));
    return _str_.substring(0, std::min(_str_.length(), length));
}

//
inline String cString(uint8_t const* data, size_t length) {
    String _str_((char const*)data);
    _str_.reserve(std::min(_str_.length(), length));
    return _str_.substring(0, std::min(_str_.length(), length));
}


//
class _StringWrite_ {
    //
    std::atomic<char *> _chars_;
    std::atomic<size_t> _length_;

public: 
    //_String_() {
        //bzero((void*)_chars_, MAX_STRING_LENGTH);
        //_length_ = 0u;
    //}

    _StringWrite_() = delete;
    //_StringWrite_(_StringWrite_ const&) = delete;

    explicit _StringWrite_(_StringWrite_ const& _str_) {
        _chars_ = (char*)_str_.c_str();
        _length_ = _str_.length();
    }

    _StringWrite_(char* _str_) {
        _chars_ = _str_;
        _length_ = strlen(_str_);
    }

    _StringWrite_(char* _str_, size_t _len_) {
        _chars_ = _str_;
        _length_ = std::min(strlen(_str_), _len_);
    }

    _StringWrite_& operator =(char const* _str_) {
        memcpy(_chars_.load(), _str_, _length_ = strlen(_str_));
        return *this;
    }

    _StringWrite_& operator =(String const& _str_) {
        memcpy(_chars_.load(), _str_.c_str(), _length_ = _str_.length());
        return *this;
    }

    _StringWrite_& atEnd(String const& _str_) {
        memcpy(_chars_.load()+(_length_-_str_.length()), _str_.c_str(), _str_.length());
        return *this;
    }

    String toString() const {
        return cString((char const*)_chars_, _length_);
    }

    char const& operator[] (int index) const {
        return ((char const*)_chars_)[index];
    }

    operator char const*() const {
        return _chars_;
    }

    explicit operator String() const {
        return cString(_chars_, _length_);
    }

    /*char volatile* c_str() volatile {
        return _chars_;
    }*/

    char const* c_str() const {
        return _chars_;
    }

    char* c_str() {
        return _chars_;
    }

    size_t length() const {
        return _length_;
    }
};


//
class _StringView_ {
    //
    std::atomic<char const*> _chars_;
    std::atomic<size_t> _length_;

public: 
    //_String_() {
        //bzero((void*)_chars_, MAX_STRING_LENGTH);
        //_length_ = 0u;
    //}

    _StringView_(_StringView_ const& _str_) {
        _chars_ = _str_.c_str();
        _length_ = _str_.length();
    }

    _StringView_(String const& _str_) {
        _chars_ = _str_.c_str();
        _length_ = std::min(_str_.length(), strlen(_chars_));
    }

    _StringView_(char const* _str_) {
        _chars_ = _str_;
        _length_ = strlen(_str_);
    }

    _StringView_(char const* _str_, size_t _len_) {
        _chars_ = _str_;
        _length_ = std::min(strlen(_str_), _len_);
    }

    _StringView_& operator =(_StringView_ _str_) {
        _chars_ = _str_.c_str();
        _length_ = _str_.length();
        return *this;
    }

    _StringView_& operator =(String const& _str_) {
        _chars_ = _str_.c_str();
        _length_ = _str_.length();
        return *this;
    }

    String toString() const {
        return cString((char const*)_chars_, _length_);
    }

    char const& operator[] (int index) const {
        return ((char const*)_chars_)[index];
    }

    operator char const*() const {
        return _chars_;
    }

    explicit operator String() const {
        return cString(_chars_, _length_);
    }

    /*char volatile* c_str() volatile {
        return _chars_;
    }*/

    char const* c_str() const {
        return _chars_;
    }

    size_t length() const {
        return _length_;
    }
};

//
template<size_t MAX_STRING_LENGTH=32>
class _String_ {
    //
    char _chars_[MAX_STRING_LENGTH];
    std::atomic<size_t> _length_;

public: 
    _String_() {
        bzero((void*)_chars_, MAX_STRING_LENGTH);
        _length_ = 0u;
    }

    _String_(_String_ const& _str_) {
        bzero((void*)_chars_, MAX_STRING_LENGTH);
        memcpy((void*)_chars_, _str_.c_str(), _length_ = std::min(_str_.length(), MAX_STRING_LENGTH));
    }
    
    _String_(char const* _str_) {
        bzero((void*)_chars_, MAX_STRING_LENGTH);
        memcpy((void*)_chars_, _str_, _length_ = std::min(strlen(_str_), MAX_STRING_LENGTH));
    }

    _String_(char const* _str_, size_t length) {
        bzero((void*)_chars_, MAX_STRING_LENGTH);
        memcpy((void*)_chars_, _str_, _length_ = std::min(std::min(strlen(_str_), length), MAX_STRING_LENGTH));
    }

    _String_(_StringView_ _str_) {
        bzero((void*)_chars_, MAX_STRING_LENGTH);
        memcpy((void*)_chars_, _str_.c_str(), _length_ = std::min(_str_.length(), MAX_STRING_LENGTH));
    }

    _String_(String const& _str_) {
        bzero((void*)_chars_, MAX_STRING_LENGTH);
        memcpy((void*)_chars_, _str_.c_str(), _length_ = std::min(_str_.length(), MAX_STRING_LENGTH));
    }

    _String_& operator =(_StringView_ _str_) {
        bzero((void*)_chars_, MAX_STRING_LENGTH);
        memcpy((void*)_chars_, _str_.c_str(), _length_ = std::min(_str_.length(), MAX_STRING_LENGTH));
        return *this;
    }

    _String_& operator =(String const& _str_) {
        bzero((void*)_chars_, MAX_STRING_LENGTH);
        memcpy((void*)_chars_, _str_.c_str(), _length_ = std::min(_str_.length(), MAX_STRING_LENGTH));
        return *this;
    }

    _String_& operator =(char const* _str_) {
        bzero((void*)_chars_, MAX_STRING_LENGTH);
        memcpy((void*)_chars_, _str_, _length_ = std::min(strlen(_str_), MAX_STRING_LENGTH));
        return *this;
    }

    String toString() const {
        return cString((char const*)_chars_, _length_);
    }

    char const& operator[] (int index) const {
        return ((char*)_chars_)[index];
    }

    char& operator[] (int index) {
        return ((char*)_chars_)[index];
    }

    operator _StringView_() const {
        return _StringView_((char const*)_chars_, _length_);
    }

    operator char const*() const {
        return (char const*)_chars_;
    }

    explicit operator String() const {
        return cString((char const*)_chars_, _length_);
    }

    /*char volatile* c_str() volatile {
        return _chars_;
    }*/

    char const* c_str() const {
        return (char const*)_chars_;
    }

    char* c_str() {
        return (char*)_chars_;
    }

    size_t length() const {
        return _length_;
    }
};
