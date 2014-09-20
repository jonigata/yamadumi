/*!
  @file     camera.hpp
  @brief    <ŠT—v>

  <à–¾>
  $Id: camera.hpp 32 2008-10-25 12:19:56Z Naoyuki.Hirayama $
*/
#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "geometry.hpp"
#include "mouse_acceptor.hpp"
#include <memory>

class Camera : public IMouseAcceptor {
public:
    Camera();
    ~Camera();

    void reset();
    Vector make_view_point(); 
    Vector make_focal_point(); 
    void move_view_point(const Vector& view_point);
    void move_focal_point(const Vector& focal_point);

private:
    Point   get_offset();
        
    MouseAcceptInfo accept_mouse(
        const MouseAcceptInfo::ancestors_type&  ancestors,
        const Point&                            position,
        MouseButton                             button);

    void on_hover(const Point&);

private:                
    std::unique_ptr<class CameraRotator>  rotator_;
    std::unique_ptr<class CameraZoomer>   zoomer_;
    std::unique_ptr<class CameraShifter>  shifter_;

};

#endif // CAMERA_HPP
