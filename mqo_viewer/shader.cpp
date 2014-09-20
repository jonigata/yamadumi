// 2013/04/29 Naoyuki Hirayama

#include <cstdio>
#include "gl.hpp"

const char* vertex_shader() {
    static const char vertex_shader_source[] =
        "attribute vec3 Position;\n"
        "attribute vec3 Normal;\n"
        "attribute vec4 SrcColor;\n"
        "attribute vec2 SrcUv;\n"
        "\n"
        "uniform mat4 ModelViewProjectionMatrix;\n"
        "uniform mat4 NormalMatrix;\n"
        "uniform vec4 LightSourcePosition;\n"
        "uniform vec4 MaterialColor;\n"
        "\n"
        "varying vec4 Color;\n"
        "varying vec2 Uv;\n"
        "\n"
        "void main(void)\n"
        "{\n"
        "    // Transform the normal to eye coordinates\n"
        "    vec3 N = normalize(vec3(NormalMatrix * vec4(Normal, 1.0)));\n"
        "\n"
        "    // The LightSourcePosition is actually its direction for directional light\n"
        "    vec3 L = normalize(LightSourcePosition.xyz);\n"
        "\n"
        "    // Multiply the diffuse value by the vertex color (which is fixed in this case)\n"
        "    // to get the actual color that we will use to draw this vertex with\n"
        "    float diffuse = max(dot(N, L), 0.0);\n"
        "    Color = diffuse * SrcColor;\n"
        "    Uv = SrcUv;\n"
        "\n"
        "    // Transform the position to clip coordinates\n"
        "    gl_Position = ModelViewProjectionMatrix * vec4(Position, 1.0);\n"
        "}";

    return vertex_shader_source;
}

const char* fragment_shader() {
    static const char fragment_shader_source[] =
        "#ifdef GL_ES\n"
        "precision mediump float;\n"
        "#endif\n"
        "varying vec4 Color;\n"
        "varying vec2 Uv;\n"
        "uniform sampler2D Texture2D;\n"
        "\n"
        "void main(void)\n"
        "{\n"
//        "    gl_FragColor = Color;\n"
        "    gl_FragColor = texture2D(Texture2D, Uv);\n"
        "}";

    return fragment_shader_source;
}

GLuint create_vertex_shader() {
    const char* p = vertex_shader();
    GLuint v = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(v, 1, &p, NULL);
    glCompileShader(v);

    char msg[512];
    glGetShaderInfoLog(v, sizeof msg, NULL, msg);
    printf("vertex shader info: %s\n", msg);

    return v;
}

GLuint create_fragment_shader() {
    const char* p = fragment_shader();
    GLuint f = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(f, 1, &p, NULL);
    glCompileShader(f);

    char msg[512];
    glGetShaderInfoLog(f, sizeof msg, NULL, msg);
    printf("fragment shader info: %s\n", msg);

    return f;
}
