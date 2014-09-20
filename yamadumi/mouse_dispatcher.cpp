// $Id: mouse_dispatcher.cpp 10 2008-03-14 06:55:24Z Naoyuki.Hirayama $

#include "mouse_dispatcher.hpp"
#include "mouse_acceptor.hpp"
//#include "zw/dprintf.hpp"
#include <map>
#include <cstdio>

/*===========================================================================*/
/*!
 * @class MouseDispatcherImp
 * @brief 
 *
 * 
 */
/*==========================================================================*/

class MouseDispatcherImp {
public:
    typedef std::multimap<int, IMouseAcceptor*> dic_type;

public:
    MouseDispatcherImp() : down_(false) {
        ldown_ = false;
        l_accept_info_.receiver = NULL;

        rdown_ = false;
        r_accept_info_.receiver = NULL;

        mdown_ = false;
        m_accept_info_.receiver = NULL;
    }

    void add_acceptor(IMouseAcceptor* acceptor, int priority) {
        dic_.insert(dic_type::value_type(priority, acceptor));
    }

    void remove_acceptor(IMouseAcceptor* acceptor) {
        dic_.erase(
            std::find_if (
                dic_.begin(),
                dic_.end(),
                [=](const dic_type::value_type& p) {
                    return p.second == acceptor;
                }));
    }

    void on_mouse_message(const MouseState& m) {
        bool down = m.lbutton || m.mbutton || m.rbutton;

        if (!down_ && !down) {
            for (const auto& pair: dic_) {
                pair.second->on_hover(m.position);
            }
            return;
        }

        down_ = down;

        dispatch(ldown_, m.lbutton, l_accept_info_, m.position,
                 MouseButton::left);
        dispatch(rdown_, m.rbutton, r_accept_info_, m.position,
                 MouseButton::right);
        dispatch(mdown_, m.mbutton, m_accept_info_, m.position,
                 MouseButton::middle);
    }

    void dispatch(
        bool&               old_down,
        bool                new_down,
        MouseAcceptInfo&    accept_info,
        const Point&        position,
        MouseButton         button) {
        if (!old_down) {
            if (new_down) {
                // start
                MouseAcceptInfo a = try_accept(position, button);
                if (a.receiver) { // accept
                    accept_info = a;
                    a.receiver->on_down(
                        calcurate_local_mouse_position(accept_info, position));
                }
                old_down = true;
            } else { /* do nothing */ }
        } else {
            if (accept_info.receiver) {
                if (new_down) {
                    // drag
                    accept_info.receiver->on_drag(
                        calcurate_local_mouse_position(accept_info, position));
                } else {
                    // end
                    accept_info.receiver->on_up(
                        calcurate_local_mouse_position(accept_info, position));
                }
            } else { /* do nothing */ }
            old_down = new_down;
        }
    }

    MouseAcceptInfo try_accept(
        const Point&    position,
        MouseButton     button) {
        for (const auto& pair: dic_) {
            MouseAcceptInfo::ancestors_type ancestors;
            MouseAcceptInfo a = pair.second->accept_mouse(
                ancestors, position, button);
            if (a.receiver) { return a; }
        }

        MouseAcceptInfo a;
        return a;
    }

private:
    bool            down_;
    dic_type        dic_;

    bool            ldown_;
    MouseAcceptInfo l_accept_info_;

    bool            rdown_;
    MouseAcceptInfo r_accept_info_;

    bool            mdown_;
    MouseAcceptInfo m_accept_info_;

};

/*============================================================================
 *
 * class MouseDispatcher 
 *
 * 
 *
 *==========================================================================*/
//<<<<<<<<<< MouseDispatcher

//****************************************************************
// constructor
MouseDispatcher::MouseDispatcher() : pimpl(new MouseDispatcherImp) {
}

//****************************************************************
// destructor
MouseDispatcher::~MouseDispatcher() {
}

//****************************************************************
// add_acceptor
void MouseDispatcher::add_acceptor(IMouseAcceptor* acceptor, int priority) {
    pimpl->add_acceptor(acceptor, priority);
}

//****************************************************************
// remove_acceptor
void MouseDispatcher::remove_acceptor(IMouseAcceptor* acceptor) {
    pimpl->remove_acceptor(acceptor);
}

//****************************************************************
// on_mouse_message
void MouseDispatcher::on_mouse_message(const MouseState& m) {
    pimpl->on_mouse_message(m);
}

//>>>>>>>>>> MouseDispatcher

