/*!
  @file     mouse_dispatcher.hpp
  @brief    <ŠT—v>

  <à–¾>
  $Id: mouse_dispatcher.hpp 10 2008-03-14 06:55:24Z Naoyuki.Hirayama $
*/
#ifndef MOUSE_DISPATCHER_HPP
#define MOUSE_DISPATCHER_HPP

#include "mouse_acceptor.hpp"
#include <memory>

struct MouseState {
    bool    lbutton;
    bool    mbutton;
    bool    rbutton;
    Point   position;

    MouseState() {
        lbutton = false;
        mbutton = false;
        rbutton = false;
        position = Point(0, 0);
    }
};

class MouseDispatcher {
public:
    MouseDispatcher();
    ~MouseDispatcher();

    void add_acceptor(IMouseAcceptor*, int priority);
    void remove_acceptor(IMouseAcceptor*);

    void on_mouse_message(const MouseState& m);

private:
    std::unique_ptr<class MouseDispatcherImp> pimpl;

};

#endif // MOUSE_DISPATCHER_HPP
