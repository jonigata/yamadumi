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
    Vector make_view_point();
    void on_slider(std::function<void (int, float)>);
    void on_keyboard(std::function<void (int)>);
    void on_special(std::function<void (int,int,int)>);
    void on_idle(std::function<void (float)>);
    void do_main_loop();

};

#endif // SCREEN_HPP_
