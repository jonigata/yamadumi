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

#if 0
        BGMatrix_ =
            Matrix::translate(-2.0, 0, 0) *
            Matrix::scale(width, height, 1);
#else
        BGMatrix_ =
            Matrix::scale(2.0, 2.0, 1.0) * 
            Matrix::translate(-0.5, -0.5, 0);
#endif

        /* Set the viewport */
        glViewport(0, 0, (GLint) width, (GLint) height);
    }

    void draw() {
        glClearColor(0.0, 0.0, 0.0, 0.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        draw_bg();

        // view matrix
        Vector view_point = camera_.make_view_point();
        Vector focal_point = camera_.make_focal_point();
        Matrix view = look_at_rh(
            view_point,
            focal_point,
            Vector(0, 1, 0));
        Matrix transform = Matrix::identity();
        transform *= Matrix::translate(0, -10, -40);
        transform *= Matrix::rotate(2 * M_PI * view_rot_[0] / 360.0, 1, 0, 0);
        transform *= Matrix::rotate(2 * M_PI * view_rot_[1] / 360.0, 0, 1, 0);
        transform *= Matrix::rotate(2 * M_PI * view_rot_[2] / 360.0, 0, 0, 1);

        //printf("%f, %f, %f\n", view_point.x, view_point.y, view_point.z);
        //printf("%f, %f, %f\n", focal_point.x, focal_point.y, focal_point.z);

        //transform *= Matrix::translate(0, -10, -40);

        /* Draw the shapes */
        for(shape_ptr& p : shapes_) {
            //draw_shape(p, transform);
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

        GLuint v = create_vertex_shader();
        GLuint f = create_fragment_shader();

        /* Create and link the shader program */
        GLuint program = glCreateProgram();
        glAttachShader(program, v);
        glAttachShader(program, f);
        glBindAttribLocation(program, 0, "Position");
        glBindAttribLocation(program, 1, "Normal");
        glBindAttribLocation(program, 2, "SrcColor");
        glBindAttribLocation(program, 3, "SrcUv");

        glLinkProgram(program);
        char msg[512];
        glGetProgramInfoLog(program, sizeof msg, NULL, msg);
        printf("info: %s\n", msg);

        /* Enable the shaders */
        glUseProgram(program);

        /* sampler */
        glUniform1i(glGetUniformLocation(program, "Texture2D"), 0);

        /* Get the locations of the uniforms so we can access them */
        ModelViewProjectionMatrix_location_ =
            glGetUniformLocation(program, "ModelViewProjectionMatrix");
        NormalMatrix_location_ =
            glGetUniformLocation(program, "NormalMatrix");
        LightSourcePosition_location_ =
            glGetUniformLocation(program, "LightSourcePosition");
        MaterialColor_location_ =
            glGetUniformLocation(program, "MaterialColor");

        /* Set the LightSourcePosition uniform which is constant throught the program */
        glUniform4fv(LightSourcePosition_location_, 1, LightSourcePosition_);
    }

    void draw_bg() {
        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);

        glUniformMatrix4fv(
            ModelViewProjectionMatrix_location_,
            1,
            GL_FALSE,
            BGMatrix_.m);
        bg_->render();
    }

    void draw_shape(const shape_ptr& shape, const Matrix& transform)
    {
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);

        /* Set the material color */
        glUniform4fv(
            MaterialColor_location_, 1, shape->get_material_color().rgba);

        /* Translate and rotate the shape */
        Matrix model_view = transform * shape->get_transform();

        /* Create and set the ModelViewProjectionMatrix */
        Matrix model_view_projection = ProjectionMatrix_ * model_view;
        glUniformMatrix4fv(
            ModelViewProjectionMatrix_location_,
            1,
            GL_FALSE,
            model_view_projection.m);

        /*
         * Create and set the NormalMatrix. It's the inverse transpose of the
         * ModelView matrix.
         */
        Matrix normal_matrix = model_view;
        normal_matrix.invert();
        normal_matrix.transpose();
        glUniformMatrix4fv(
            NormalMatrix_location_,
            1,
            GL_FALSE,
            normal_matrix.m);

        shape->render();
    }

private:
    MouseDispatcher mouse_dispatcher_;
    Camera          camera_;
    MouseState      mouse_state_;

    Matrix BGMatrix_;
    Matrix ProjectionMatrix_;
    const GLfloat LightSourcePosition_[4];
    GLfloat view_rot_[3];

    GLuint ModelViewProjectionMatrix_location_;
    GLuint NormalMatrix_location_;
    GLuint LightSourcePosition_location_;
    GLuint MaterialColor_location_;

    GLuint sampler_;

    std::shared_ptr<BG> bg_;
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

//>>>>>>>>>> Screen

extern "C" {
void addMouseEvent(int which, int kind, int x, int y) {
    printf("%d, %d: %d, %d\n", which, kind, x, y);
    imp_->on_mouse(which, kind, x, y);
}
}

