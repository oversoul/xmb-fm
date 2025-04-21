#include "ribbon.h"
#include "ui.h"

RibbonState ribbonState = {};

static void check_shader_error(uint32_t shader, const char *type) {
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        fprintf(stderr, "Shader %s Compile Error: %s\n", type, infoLog);
    }
}

void init_ribbon() {
    glGenBuffers(1, &ribbonState.ribbon_vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, ribbonState.ribbon_vertex_buffer);

    static const char *ribbon_vertex_shader_text = GLSL( //
        uniform float time;                              //
        layout(location = 0) in vec2 VertexCoord;        //
        out vec3 vEC;                                    //

        float iqhash(float n) { return fract(sin(n) * 43758.5453); }

        float noise(vec3 x) {
            vec3 p = floor(x);
            vec3 f = fract(x);
            f = f * f * (3.0 - 2.0 * f);
            float n = p.x + p.y * 57.0 + 113.0 * p.z;
            return mix(mix(mix(iqhash(n), iqhash(n + 1.0), f.x), mix(iqhash(n + 57.0), iqhash(n + 58.0), f.x), f.y),
                       mix(mix(iqhash(n + 113.0), iqhash(n + 114.0), f.x),
                           mix(iqhash(n + 170.0), iqhash(n + 171.0), f.x), f.y),
                       f.z);
        }

        float noise2(vec3 x) { return cos(x.z * 4.0) * cos(x.z + time / 10.0 + x.x); }

        void main() {
            vec3 v = vec3(VertexCoord.x, 0.0, VertexCoord.y);
            vec3 v2 = v;
            vec3 v3 = v;

            float dt = time / 2.0;

            v.y = noise2(v2) / 8.0;

            v3.x -= dt / 5.0;
            v3.x /= 4.0;

            v3.z -= dt / 10.0;
            v3.y -= dt / 100.0;

            v.z -= noise(v3 * 7.0) / 15.0;
            v.y -= noise(v3 * 7.0) / 15.0 + cos(v.x * 2.0 - dt / 2.0) / 5.0 - 0.3;
            v.y = -v.y;

            vEC = v;
            gl_Position = vec4(v, 1.0);
        });

    static const char *ribbon_fragment_shader_text = GLSL( //

        uniform float time;

        in vec3 vEC; //
        out vec4 FragColor;

        void main() {
            const vec3 up = vec3(0.0, 0.0, 1.0);
            vec3 x = dFdx(vEC);
            vec3 y = dFdy(vEC);
            vec3 normal = normalize(cross(x, y));
            float c = 1.0 - dot(normal, up);
            c = (1.0 - cos(c * c)) / 3.0;
            FragColor = vec4(1.0, 1.0, 1.0, c);
        });

#define RIBBON_X_SEGMENTS 128
#define RIBBON_Y_SEGMENTS 32
#define RIBBON_VERTEX_COUNT ((RIBBON_X_SEGMENTS + 1) * (RIBBON_Y_SEGMENTS + 1))
#define RIBBON_INDEX_COUNT (RIBBON_X_SEGMENTS * RIBBON_Y_SEGMENTS * 6)

    float ribbon_vertices[RIBBON_VERTEX_COUNT * 2];
    size_t i = 0;
    for (int y = 0; y <= RIBBON_Y_SEGMENTS; ++y) {
        for (int x = 0; x <= RIBBON_X_SEGMENTS; ++x) {
            float u = (float)x / RIBBON_X_SEGMENTS;
            float v = (float)y / RIBBON_Y_SEGMENTS;
            float px = u * 2.0f - 1.0f;
            float py = v * 2.0f - 1.0f;
            ribbon_vertices[i++] = px;
            ribbon_vertices[i++] = py;
        }
    }

    glGenBuffers(1, &ribbonState.ribbon_vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, ribbonState.ribbon_vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * RIBBON_VERTEX_COUNT * 2, ribbon_vertices, GL_STATIC_DRAW);

    const GLuint ribbon_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(ribbon_vertex_shader, 1, &ribbon_vertex_shader_text, NULL);
    glCompileShader(ribbon_vertex_shader);
    check_shader_error(ribbon_vertex_shader, "vertex");

    const GLuint ribbon_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(ribbon_fragment_shader, 1, &ribbon_fragment_shader_text, NULL);
    glCompileShader(ribbon_fragment_shader);
    check_shader_error(ribbon_fragment_shader, "fragment");

    ribbonState.ribbon_program = glCreateProgram();
    glAttachShader(ribbonState.ribbon_program, ribbon_vertex_shader);
    glAttachShader(ribbonState.ribbon_program, ribbon_fragment_shader);
    glLinkProgram(ribbonState.ribbon_program);

    glDeleteShader(ribbon_vertex_shader);
    glDeleteShader(ribbon_fragment_shader);

    glGenVertexArrays(1, &ribbonState.ribbon_vertex_array);
    glBindVertexArray(ribbonState.ribbon_vertex_array);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);

    uint32_t *ribbon_indices = malloc(sizeof(GLuint) * RIBBON_INDEX_COUNT);
    uint32_t ribbon_offset = 0;
    for (int y = 0; y < RIBBON_Y_SEGMENTS; ++y) {
        for (int x = 0; x < RIBBON_X_SEGMENTS; ++x) {
            int i0 = y * (RIBBON_X_SEGMENTS + 1) + x;
            int i1 = i0 + 1;
            int i2 = i0 + (RIBBON_X_SEGMENTS + 1);
            int i3 = i2 + 1;

            // Triangle 1
            ribbon_indices[ribbon_offset++] = i0;
            ribbon_indices[ribbon_offset++] = i2;
            ribbon_indices[ribbon_offset++] = i1;

            // Triangle 2
            ribbon_indices[ribbon_offset++] = i1;
            ribbon_indices[ribbon_offset++] = i2;
            ribbon_indices[ribbon_offset++] = i3;
        }
    }

    GLuint ribbon_ebo;
    glGenBuffers(1, &ribbon_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ribbon_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * RIBBON_INDEX_COUNT, ribbon_indices, GL_STATIC_DRAW);

    free(ribbon_indices);
}

void draw_ribbon(float width, float height, float time) {
    glUseProgram(ribbonState.ribbon_program);
    glUniform1f(glGetUniformLocation(ribbonState.ribbon_program, "time"), time);

    glBindVertexArray(ribbonState.ribbon_vertex_array);
    glBindBuffer(GL_ARRAY_BUFFER, ribbonState.ribbon_vertex_buffer);

    glDrawElements(GL_TRIANGLES, RIBBON_INDEX_COUNT, GL_UNSIGNED_INT, 0);
}
