// 2013/04/27 Naoyuki Hirayama

#include "screen.hpp"

#include <cstdio>
#include <cmath>
#include <cassert>
#include <vector>

#include "gl.hpp"
#include "geometry.hpp"
#include "shader.hpp"
#include "bg.hpp"
#include "room.hpp"
#include "mouse_dispatcher.hpp"
#include "camera.hpp"

static ScreenImp* imp_ = nullptr;

void sincos(float angle, float* s, float* c) {
    *s = sin(angle);
    *c = cos(angle);
}

class ScreenImp {
public:
    ////////////////////////////////////////////////////////////////
    // public interface
    ScreenImp(int argc, const char** argv, const char* title)
        :
        LightSourcePosition_ {5.0, 5.0, 10.0, 1.0},
        view_rot_ { 20.0, 30.0, 0.0 } {

        init(argc, argv, title);
        bg_ = std::make_shared<BG>();
        room_ = std::make_shared<Room>();

        mouse_dispatcher_.add_acceptor(&camera_, 0);
    }

    ~ScreenImp() {
        mouse_dispatcher_.remove_acceptor(&camera_);
    }

    void on_mouse(int which, int kind, int x, int y) {
        bool* b = nullptr;
        switch(which) {
            case 1: b = &mouse_state_.lbutton; break;
            case 2: b = &mouse_state_.mbutton; break;
            case 3: b = &mouse_state_.rbutton; break;
        }

        switch(kind) {
            case 0:
                if (b) {
                    *b = true;
                }
                break;
            case 2:
                if (b) {
                    *b = false;
                }
                break;
        }
        mouse_state_.position = Point(x, y);
        mouse_dispatcher_.on_mouse_message(mouse_state_);
    }

    void on_idle(std::function<void (float)> f) {
        printf("setting on_idle\n");
        on_idle_ = f;
    }

    void add_shape(shape_ptr shape) {
        shapes_.push_back(shape);
    }

    void do_main_loop() {
        glutMainLoop();
    }

    ////////////////////////////////////////////////////////////////
    // glut callback
    void idle() {
        static int frames = 0;
        static double tRot0 = -1.0, tRate0 = -1.0;
        double dt, t = glutGet(GLUT_ELAPSED_TIME) / 1000.0;

        if (tRot0 < 0.0)
            tRot0 = t;
        dt = t - tRot0;
        tRot0 = t;

        /* advance rotation for next frame */
        if (on_idle_) {
            on_idle_(dt);
        }

        glutPostRedisplay();
        frames++;

        if (tRate0 < 0.0)
            tRate0 = t;
        if (t - tRate0 >= 5.0) {
            GLfloat seconds = t - tRate0;
            GLfloat fps = frames / seconds;
            printf("%d frames in %3.1f seconds = %6.3f FPS\n", frames, seconds,
                   fps);
            tRate0 = t;
            frames = 0;
        }
    }

    void reshape(int width, int height) {
        /* Update the projection matrix */
        //perspective(ProjectionMatrix_, 60.0, width / (float)height, 1.0, 1024.0);
        ProjectionMatrix_ =
            perspective_fov_rh(
                DEG2RAD(60.0), width / (float)height, 1.0, 1024.0);

        /* Set the viewport */
        glViewport(0, 0, (GLint) width, (GLint) height);
    }

    void draw() {
        glClearColor(0.0, 0.0, 0.0, 0.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        figure_shader_->attach(LightSourcePosition_);
        draw_bg();

        // view matrix
        Vector view_point = camera_.make_view_point();
        Vector focal_point = camera_.make_focal_point();
        Matrix view = look_at_rh(
            view_point,
            focal_point,
            Vector(0, 1, 0));

        room_shader_->attach();
        draw_room(view);

        /* Draw the shapes */
        figure_shader_->attach(LightSourcePosition_);
        for(shape_ptr& p : shapes_) {
            draw_shape(p, view);
        }
        
        glutSwapBuffers();
    }

    void special(int special, int crap, int morecrap) {
        switch (special) {
            case GLUT_KEY_LEFT:
                view_rot_[1] += 5.0;
                break;
            case GLUT_KEY_RIGHT:
                view_rot_[1] -= 5.0;
                break;
            case GLUT_KEY_UP:
                view_rot_[0] += 5.0;
                break;
            case GLUT_KEY_DOWN:
                view_rot_[0] -= 5.0;
                break;
            case GLUT_KEY_F11:
                glutFullScreen();
                break;
        }
    }

    Vector make_view_point() {
        return camera_.make_view_point();
    }

private:
    void init(int argc, const char** argv, const char* title) {
        glutInit(&argc, const_cast<char**>(argv));
        glutInitWindowSize(640, 480);
        glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

        glutCreateWindow(title);

        /* Set up glut callback functions */
        glutIdleFunc ([](){ imp_->idle(); });
        glutReshapeFunc([](int w, int h) { imp_->reshape(w, h); });
        glutDisplayFunc([](){ imp_->draw(); });
        glutSpecialFunc([](int s, int c, int m){ imp_->special(s, c, m); });

        figure_shader_.reset(new FigureShader);
        room_shader_.reset(new RoomShader);
    }

    void draw_bg() {
        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);

        Matrix bg_matrix =
            Matrix::scale(2.0, 2.0, 1.0) * 
            Matrix::translate(-0.5, -0.5, 0);

        Color c {{1.0, 1.0, 1.0, 1.0}};
        
        figure_shader_->bind(
            c,
            Matrix::identity(),
            Matrix::identity(),
            bg_matrix);
        bg_->render();
    }

    void draw_shape(const shape_ptr& shape, const Matrix& view_matrix)
    {
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);

        figure_shader_->bind(
            shape->get_material_color(),
            shape->get_transform(),
            view_matrix,
            ProjectionMatrix_);
        shape->render();
    }

    void draw_room(const Matrix& view_matrix) {
        glEnable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);

        room_shader_->bind(
            Matrix::identity(),
            view_matrix,
            ProjectionMatrix_);
        room_->render();
    }

private:
    MouseDispatcher mouse_dispatcher_;
    Camera          camera_;
    MouseState      mouse_state_;

    Matrix ProjectionMatrix_;
    const GLfloat LightSourcePosition_[4];
    GLfloat view_rot_[3];

    std::unique_ptr<FigureShader> figure_shader_;
    std::unique_ptr<RoomShader> room_shader_;

    std::shared_ptr<BG> bg_;
    std::shared_ptr<Room> room_;
    std::vector<shape_ptr> shapes_;
    std::function<void (float)> on_idle_;

};

/*============================================================================
 *
 * class Screen 
 *
 * 
 *
 *==========================================================================*/
//<<<<<<<<<< Screen

//****************************************************************
// constructor
Screen::Screen(int argc, const char** argv, const char* title){
    assert(imp_ == nullptr);
    printf("Screen::Screen1\n");
    imp_ = new ScreenImp(argc, argv, title);
    printf("Screen::Screen2\n");
}

//****************************************************************
// destructor
Screen::~Screen() {
    delete imp_;
}

//****************************************************************
// add_shape
void Screen::add_shape(shape_ptr shape) {
    printf("Screen::add_shape\n");
    imp_->add_shape(shape);
}

//****************************************************************
// do_main_loop
void Screen::do_main_loop() {
    printf("Screen::do_main_loop\n");
    imp_->do_main_loop();
}

//****************************************************************
// on_idle
void Screen::on_idle(std::function<void (float)> f) {
    printf("Screen::on_idle\n");
    imp_->on_idle(f);
}

//****************************************************************
// make_view_point
Vector Screen::make_view_point() {
    return imp_->make_view_point();
}

//>>>>>>>>>> Screen

extern "C" {
void addMouseEvent(int which, int kind, int x, int y) {
    //printf("%d, %d: %d, %d\n", which, kind, x, y);
    imp_->on_mouse(which, kind, x, y);
}
}

