/*!
  @file     mouse_acceptor.hpp
  @brief    <ŠT—v>

  <à–¾>
  $Id: mouse_acceptor.hpp 10 2008-03-14 06:55:24Z Naoyuki.Hirayama $
*/
#ifndef MOUSE_ACCEPTOR_HPP
#define MOUSE_ACCEPTOR_HPP

#include <vector>

enum class MouseButton {
    left,
    middle,
    right,
};

struct Point {
    int x;
    int y;
    Point() {}
    Point(int ax, int ay) { x = ax; y = ay; }

    Point& operator-=(const Point& r) {
        x -= r.x;
        y -= r.y;
        return *this;
    }

    Point operator-(const Point& r) const {
        Point result;
        result.x = x - r.x;
        result.y = y - r.y;
        return result;
    }
};

class IMouseReceiver {
public:
    virtual ~IMouseReceiver(){}

    virtual void on_down(const Point&) = 0;
    virtual void on_drag(const Point&) = 0;
    virtual void on_up(const Point&) = 0;
};

class IMouaseAcceptor;

struct MouseAcceptInfo {
    typedef std::vector<class IMouseAcceptor*> ancestors_type;

    ancestors_type  ancestors;
    IMouseReceiver* receiver;

    MouseAcceptInfo() {
        receiver = NULL;
    }
    MouseAcceptInfo(
        const ancestors_type& as, IMouseAcceptor* a, IMouseReceiver* r) {
        ancestors = as;
        ancestors.push_back(a);
        receiver = r;
    }
};

class IMouseAcceptor {
public:
    virtual ~IMouseAcceptor(){}

    virtual Point   get_offset() = 0;

    virtual MouseAcceptInfo accept_mouse(
        const MouseAcceptInfo::ancestors_type&  ancestors,
        const Point&                            position,
        MouseButton                             button ) = 0;

    virtual void on_hover(const Point&)= 0;
        
};

inline
Point
calcurate_local_mouse_position(
    const MouseAcceptInfo&  info,
    Point                   o) {
    for (const auto& a: info.ancestors) {
        o -= a->get_offset();
    }
    return o;
}

inline
MouseAcceptInfo::ancestors_type
append_ancestor(
    const MouseAcceptInfo::ancestors_type& ancestors, IMouseAcceptor* a) {

    MouseAcceptInfo::ancestors_type new_ancestors(ancestors);
    new_ancestors.push_back(a);
    return new_ancestors;
}

#endif // MOUSE_ACCEPTOR_HPP
