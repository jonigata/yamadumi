// $Id: camera.cpp 32 2008-10-25 12:19:56Z Naoyuki.Hirayama $

#include "camera.hpp"
#include <cstdio>

const float PI = 3.14159265358979323846264;

/*===========================================================================*/
/*!
 * @class CameraRotater
 * @brief 
 *
 * 
 */
/*==========================================================================*/

class CameraRotator : public IMouseReceiver {
public:
    CameraRotator() { reset(); }

    void reset() {
        pan_            = PI;
        pan_temporary_  = pan_;
        tilt_           = PI/10;
        tilt_temporary_ = tilt_;
    }

    void on_down(const Point& pos) {
        start_ = pos;
    }
        
    void on_drag(const Point& pos) {
        Point d = pos - start_;

        const float coefficient_x = -0.01f;
        const float coefficient_y = 0.01f;

        pan_temporary_ = fmodf(pan_ + d.x * coefficient_x, PI*2);
        tilt_temporary_ = float(tilt_ + d.y * coefficient_y); 
        if (tilt_temporary_ < PI * -0.4999f) {
            tilt_temporary_ = PI * -0.4999f;
        }
        if (PI * 0.4999f < tilt_temporary_) {
            tilt_temporary_ = PI * 0.4999f;
        }
    }
        
    void on_up(const Point&) {
        pan_ = pan_temporary_;
        tilt_ = tilt_temporary_;
        printf("rotate\n");
    }

    float get_pan() { return pan_temporary_; }
    float get_tilt() { return tilt_temporary_; }
    
    void set_pan(float pan) { pan_ = pan_temporary_ = pan; }
    void set_tilt(float tilt) { tilt_ = tilt_temporary_ = tilt; }

private:
    float   tilt_;
    float   tilt_temporary_;
    float   pan_;
    float   pan_temporary_;

    Point   start_;
        
};

/*===========================================================================*/
/*!
 * @class CameraZoomer
 * @brief 
 *
 * 
 */
/*==========================================================================*/

class CameraZoomer : public IMouseReceiver {
public:
    CameraZoomer() { reset(); }

    void reset() {
        distance_ = 25.0f;
    }

    void on_down(const Point& pos) {
        previous_ = pos;
    }
        
    void on_drag(const Point& pos) {
        int zdelta = pos.y - previous_.y;

        distance_ += zdelta * 0.05f;
        previous_ = pos;
    }
        
    void on_up(const Point&) {
        printf("zoom\n");
    }

    float get_distance() { return distance_; }

    void set_distance( float distance ) { distance_ = distance; }

private:
    float   distance_;
    Point   previous_;
        
};

/*===========================================================================*/
/*!
 * @class CameraShifter
 * @brief 
 *
 * 
 */
/*==========================================================================*/

class CameraShifter : public IMouseReceiver {
public:
    CameraShifter() {
        reset();
    }

    void reset() {
        offset_ = Vector(0, 10, 0);
    }

    void set_angle(float pan, float tilt) {
        pan_ = pan;
        tilt_ = tilt;
    }

    void on_down(const Point& pos) {
        previous_ = pos;
    }
        
    void on_drag(const Point& pos) {
        int xdelta = pos.x - previous_.x;
        int ydelta = pos.y - previous_.y;

        offset_ += foo(xdelta * 0.05f, ydelta * 0.05f);

        previous_ = pos;
    }
        
    void on_up(const Point&) {
        printf("shift\n");
    }

    Vector get_offset() {
        return offset_;
    }

    void set_offset(const Vector& v) {
        offset_ = v;
    }
        
private:
    Vector foo(float dx, float dy) {
        float pan = -pan_;
        float tilt = PI/2 - tilt_;

        Vector eye;
        eye.x = sinf(tilt)* cosf(pan);
        eye.y = cosf(tilt);
        eye.z = sinf(tilt)* sinf(pan);
        eye *= -1;

        Vector c(0.0f, 1.0f, 0.0f);
        Vector xaxis = cross(eye, c);
        Vector yaxis = cross(xaxis, eye);;

        return xaxis * dx + yaxis * dy;
    }

private:
    float   pan_;
    float   tilt_;
    Vector  offset_;
    Point   previous_;
        
};

/*============================================================================
 *
 * class Camera 
 *
 * 
 *
 *==========================================================================*/
//<<<<<<<<<< Camera

//****************************************************************
// constructor
Camera::Camera() {
    rotator_.reset(new CameraRotator);
    zoomer_.reset(new CameraZoomer);
    shifter_.reset(new CameraShifter);
}

//****************************************************************
// reset
void Camera::reset() {
    rotator_->reset();
    zoomer_->reset();
    shifter_->reset();
}

//****************************************************************
// destructor
Camera::~Camera() {
}

//****************************************************************
// make_view_point
Vector Camera::make_view_point() {
    Matrix mat;

    float pan = -rotator_->get_pan();
    float tilt = PI/2 - rotator_->get_tilt();
    float distance = zoomer_->get_distance();

    Vector eye;
    eye.x = sinf(tilt)* cosf(pan);
    eye.y = cosf(tilt);
    eye.z = sinf(tilt)* sinf(pan);
    eye *= distance;

    return eye + shifter_->get_offset();
}

//****************************************************************
// make_focal_point
Vector Camera::make_focal_point() {
    return shifter_->get_offset();
}

//****************************************************************
// move_view_point
void Camera::move_view_point(const Vector& view_point) {
    Vector focal_point = shifter_->get_offset();
    Vector v = focal_point - view_point;
    float distance = length(v);
    float pan = atan2(v.z, v.x);
    float tilt = -atan2(v.y, sqrtf(v.x * v.x + v.z * v.z));
    rotator_->set_pan(pan);
    rotator_->set_tilt(tilt);
    zoomer_->set_distance(distance);
}

//****************************************************************
// move_focal_point
void Camera::move_focal_point(const Vector& focal_point) {
    shifter_->set_offset(focal_point);
}

//----------------------------------------------------------------
// get_offset
Point Camera::get_offset() {
    return Point(0, 0);
}

//----------------------------------------------------------------
// accept_mouse
MouseAcceptInfo Camera::accept_mouse(
    const MouseAcceptInfo::ancestors_type&  ancestors,
    const Point&,
    MouseButton                             button )
{
    MouseAcceptInfo info( ancestors, this, NULL );

    if( button == MouseButton::left  ) { info.receiver = &*zoomer_; }
    if( button == MouseButton::right ) { info.receiver = &*rotator_; }
    if( button == MouseButton::middle ) {
        shifter_->set_angle(
            rotator_->get_pan(),
            rotator_->get_tilt());
        info.receiver = &*shifter_;
    }

    return info;
}

//----------------------------------------------------------------
// on_hover
void Camera::on_hover(const Point&) {
}

//>>>>>>>>>> Camera

