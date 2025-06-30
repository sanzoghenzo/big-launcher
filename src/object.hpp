#pragma once

namespace BL {
    class Object {
    protected:
        Object() = default;
    public:
        ~Object() = default;
        virtual float get_x() const = 0;
        virtual void set_x(float x) = 0;
        virtual float get_y() const = 0;
        virtual void set_y(float y) = 0;
        virtual float get_w() const = 0;
        virtual float get_h() const = 0;
        virtual void inc_x(float x) = 0;
        virtual void dec_x(float x) = 0;
        virtual void inc_y(float y) = 0;
        virtual void dec_y(float y) = 0;
    };
}