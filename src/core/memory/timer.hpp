#pragma once

//
#include "../imports/imports.hpp"

//
namespace timer {

    //
    template<size_t how_long = 1000>
    class OnceFn {
    protected: 
        //
        std::function<void()> _fn_;
        //std::atomic<size_t> how_long;
        std::atomic<size_t> last_time;

    public: 
        OnceFn(std::function<void()> _fn_ = nullptr) : /*how_long(how_long),*/ _fn_(_fn_) {
            
        }

        //
        size_t since() const { return last_time; };
        size_t millisSince() const { return (millis() - last_time); };

        //
        void setFn( std::function<void()> _fn_ ) {
            this->_fn_ = _fn_;
        }

        //
        void handle(  ) {
            if ((millis() - last_time) >= how_long) {
                if (_fn_) {
                    last_time = millis();
                    auto $fn = _fn_; _fn_ = nullptr;
                    $fn();
                };
            }
        }

        OnceFn& operator()() {
            this->handle();
            return *this;
        }

        //
        OnceFn& operator =(std::function<void()> _fn_) {
            this->_fn_ = _fn_;
            return *this;
        }

        //
        operator std::function<void()>&() {
            return _fn_;
        }

        //
        operator std::function<void()> const&() const {
            return _fn_;
        }
    };

    //
    template<size_t how_long = 1000>
    class IntervalFn {
    protected: 
        //
        std::function<void()> _fn_;
        //std::atomic<size_t> how_long;
        std::atomic<size_t> last_time;

    public: 
        IntervalFn(std::function<void()> _fn_ = nullptr) : /*how_long(how_long),*/ _fn_(_fn_) {
            
        }

        //
        size_t since() const { return last_time; };
        size_t millisSince() const { return (millis() - last_time); };

        //
        void setFn( std::function<void()> _fn_ ) {
            this->_fn_ = _fn_;
        }

        //
        void handle(  ) {
            if ((millis() - last_time) >= how_long) {
                if (_fn_) {
                    last_time = millis();
                    _fn_();
                };
            }
        }

        IntervalFn& operator()() {
            this->handle();
            return *this;
        }

        //
        IntervalFn& operator =(std::function<void()> _fn_) {
            this->_fn_ = _fn_;
            return *this;
        }

        //
        operator std::function<void()>&() {
            return _fn_;
        }

        //
        operator std::function<void()> const&() const {
            return _fn_;
        }
    };
}