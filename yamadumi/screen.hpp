// 2013/04/27 Naoyuki Hirayama

#ifndef SCREEN_HPP_
#define SCREEN_HPP_

#include <functional>
#include "shape.hpp"

class ScreenImp;

class Screen {
public:
    Screen(int argc, const char** argv, const char* title);
    ~Screen();

    void add_shape(shape_ptr);
    void on_idle(std::function<void (float)>);
    void do_main_loop();

};

#endif // SCREEN_HPP_
