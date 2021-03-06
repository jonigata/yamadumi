// 2013/04/27 Naoyuki Hirayama

#include "screen.hpp"

#include <cstdio>
#include <cmath>
#include <cassert>
#include <vector>

#include "gl.hpp"
#include "geometry.hpp"
#include "shader.hpp"

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
        view_rot_ { 20.0, 30.0, 0.0 }
    {
        init(argc, argv, title);
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
        Matrix transform = Matrix::identity();

        glClearColor(0.0, 0.0, 0.0, 0.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        /* Translate and rotate the view */
        transform *= Matrix::translate(0, -10, -40);
        transform *= Matrix::rotate(2 * M_PI * view_rot_[0] / 360.0, 1, 0, 0);
        transform *= Matrix::rotate(2 * M_PI * view_rot_[1] / 360.0, 0, 1, 0);
        transform *= Matrix::rotate(2 * M_PI * view_rot_[2] / 360.0, 0, 0, 1);

        /* Draw the shapes */
        for(shape_ptr& p : shapes_) {
            draw_shape(p, transform);
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

        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);

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

    void draw_shape(const shape_ptr& shape, const Matrix& transform)
    {
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
    Matrix ProjectionMatrix_;
    const GLfloat LightSourcePosition_[4];
    GLfloat view_rot_[3];

    GLuint ModelViewProjectionMatrix_location_;
    GLuint NormalMatrix_location_;
    GLuint LightSourcePosition_location_;
    GLuint MaterialColor_location_;

    GLuint sampler_;

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

