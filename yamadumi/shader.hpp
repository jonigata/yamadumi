// 2013/04/29 Naoyuki Hirayama

#ifndef SHADER_HPP_
#define SHADER_HPP_

#include "gl.hpp"

class Shader {

};

class RoomShader : public Shader {
public:
    RoomShader() { build(); }

    void build() {
        GLuint v = create_vertex_shader();
        GLuint f = create_fragment_shader();

        /* Create and link the shader program */
        program_ = glCreateProgram();
        glAttachShader(program_, v);
        glAttachShader(program_, f);
        glBindAttribLocation(program_, 0, "Position");
        glBindAttribLocation(program_, 1, "SrcColor");

        glLinkProgram(program_);
        char msg[512];
        glGetProgramInfoLog(program_, sizeof msg, NULL, msg);
        printf("info: %s\n", msg);

        /* Get the locations of the uniforms so we can access them */
        ModelViewProjectionMatrix_location_ =
            glGetUniformLocation(program_, "ModelViewProjectionMatrix");
    }

    void attach() {
        /* Enable the shaders */
        glUseProgram(program_);
    }

    void bind(
        const Matrix& model_matrix,
        const Matrix& view_matrix,
        const Matrix& projection_matrix) {

        /* Translate and rotate the shape */
        Matrix model_view = view_matrix * model_matrix;

        /* Create and set the ModelViewProjectionMatrix */
        Matrix model_view_projection =
            projection_matrix * model_view;

        glUniformMatrix4fv(
            ModelViewProjectionMatrix_location_,
            1,
            GL_FALSE,
            model_view_projection.m);
    }

private:
    GLuint create_vertex_shader() {
        static const char vertex_shader_source[] = R"(
attribute vec3 Position;
attribute vec4 SrcColor;

uniform mat4 ModelViewProjectionMatrix;

varying vec4 Color;

void main(void)
{
    Color = SrcColor;
    gl_Position = ModelViewProjectionMatrix * vec4(Position, 1.0);
}
)";
        const char* p = vertex_shader_source;

        GLuint v = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(v, 1, &p, NULL);
        glCompileShader(v);

        char msg[512];
        glGetShaderInfoLog(v, sizeof msg, NULL, msg);
        printf("vertex shader info: %s\n", msg);

        return v;
    }

    GLuint create_fragment_shader() {
        static const char fragment_shader_source[] = R"(
precision mediump float;
varying vec4 Color;

void main(void)
{
    gl_FragColor = Color;
}
)";
        const char* p = fragment_shader_source;

        GLuint f = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(f, 1, &p, NULL);
        glCompileShader(f);

        char msg[512];
        glGetShaderInfoLog(f, sizeof msg, NULL, msg);
        printf("fragment shader info: %s\n", msg);

        return f;
    }

    GLuint program_;
    
    GLuint ModelViewProjectionMatrix_location_;

};

class FigureShader : public Shader {
public:
    FigureShader() { build(); }

    void build() {
        GLuint v = create_vertex_shader();
        GLuint f = create_fragment_shader();

        /* Create and link the shader program */
        program_ = glCreateProgram();
        glAttachShader(program_, v);
        glAttachShader(program_, f);
        glBindAttribLocation(program_, 0, "Position");
        glBindAttribLocation(program_, 1, "Normal");
        glBindAttribLocation(program_, 2, "SrcColor");
        glBindAttribLocation(program_, 3, "SrcUv");

        glLinkProgram(program_);
        char msg[512];
        glGetProgramInfoLog(program_, sizeof msg, NULL, msg);
        printf("info: %s\n", msg);

        /* Get the locations of the uniforms so we can access them */
        Sampler_location_ =
            glGetUniformLocation(program_, "Texture2D");            
        ModelViewProjectionMatrix_location_ =
            glGetUniformLocation(program_, "ModelViewProjectionMatrix");
        NormalMatrix_location_ =
            glGetUniformLocation(program_, "NormalMatrix");
        LightSourcePosition_location_ =
            glGetUniformLocation(program_, "LightSourcePosition");
        MaterialColor_location_ =
            glGetUniformLocation(program_, "MaterialColor");
    }

    void attach(const GLfloat light_source_position[4]) {
        /* Enable the shaders */
        glUseProgram(program_);

        /* Set the sampler */
        glUniform1i(Sampler_location_, 0);

        /* Set the LightSourcePosition uniform which is constant throught the program */
        glUniform4fv(LightSourcePosition_location_, 1, light_source_position);
    }

    void bind(
        const Color& material_color,
        const Matrix& model_matrix,
        const Matrix& view_matrix,
        const Matrix& projection_matrix) {
        /* Set the material color */
        glUniform4fv(MaterialColor_location_, 1, material_color.rgba);

        /* Translate and rotate the shape */
        Matrix model_view = view_matrix * model_matrix;

        /* Create and set the ModelViewProjectionMatrix */
        Matrix model_view_projection =
            projection_matrix * model_view;

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
    }

private:
    GLuint create_vertex_shader() {
        static const char vertex_shader_source[] = R"(
attribute vec3 Position;
attribute vec3 Normal;
attribute vec4 SrcColor;
attribute vec2 SrcUv;

uniform mat4 ModelViewProjectionMatrix;
uniform mat4 NormalMatrix;
uniform vec4 LightSourcePosition;
uniform vec4 MaterialColor;

varying vec4 Color;
varying vec2 Uv;

void main(void)
{
    vec3 N = normalize(vec3(NormalMatrix * vec4(Normal, 1.0)));
    vec3 L = normalize(LightSourcePosition.xyz);

    float diffuse = max(dot(N, L), 0.0);
    Color = diffuse * SrcColor;
    Uv = SrcUv;

    gl_Position = ModelViewProjectionMatrix * vec4(Position, 1.0);
}
)";
        const char* p = vertex_shader_source;

        GLuint v = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(v, 1, &p, NULL);
        glCompileShader(v);

        char msg[512];
        glGetShaderInfoLog(v, sizeof msg, NULL, msg);
        printf("vertex shader info: %s\n", msg);

        return v;
    }

    GLuint create_fragment_shader() {
        static const char fragment_shader_source[] = R"(
precision mediump float;
varying vec4 Color;
varying vec2 Uv;
uniform sampler2D Texture2D;

void main(void)
{
    //gl_FragColor = Color;
    gl_FragColor = texture2D(Texture2D, Uv);
}
)";
        const char* p = fragment_shader_source;

        GLuint f = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(f, 1, &p, NULL);
        glCompileShader(f);

        char msg[512];
        glGetShaderInfoLog(f, sizeof msg, NULL, msg);
        printf("fragment shader info: %s\n", msg);

        return f;
    }

    GLuint program_;
    
    GLuint Sampler_location_;
    GLuint ModelViewProjectionMatrix_location_;
    GLuint NormalMatrix_location_;
    GLuint LightSourcePosition_location_;
    GLuint MaterialColor_location_;

    
};

#endif // SHADER_HPP_
