#include "ribbon.h"
#include "ui.h"

RibbonState ribbonState = {};

static Color gradient_golden[4] = {
    {174 / 255.0, 123 / 255.0, 44 / 255.0, 1.0},
    {205 / 255.0, 174 / 255.0, 84 / 255.0, 1.0},
    {58 / 255.0, 43 / 255.0, 24 / 255.0, 1.0},
    {58 / 255.0, 43 / 255.0, 24 / 255.0, 1.0},
};

static Color gradient_legacy_red[4] = {
    {171 / 255.0, 70 / 255.0, 59 / 255.0, 1.0},
    {171 / 255.0, 70 / 255.0, 59 / 255.0, 1.0},
    {190 / 255.0, 80 / 255.0, 69 / 255.0, 1.0},
    {190 / 255.0, 80 / 255.0, 69 / 255.0, 1.0},
};

static Color gradient_electric_blue[4] = {
    {1 / 255.0, 2 / 255.0, 67 / 255.0, 1.0},
    {1 / 255.0, 73 / 255.0, 183 / 255.0, 1.0},
    {1 / 255.0, 93 / 255.0, 194 / 255.0, 1.0},
    {3 / 255.0, 162 / 255.0, 254 / 255.0, 1.0},
};

static Color gradient_dark_purple[4] = {
    {20 / 255.0, 13 / 255.0, 20 / 255.0, 1.0},
    {20 / 255.0, 13 / 255.0, 20 / 255.0, 1.0},
    {92 / 255.0, 44 / 255.0, 92 / 255.0, 1.0},
    {148 / 255.0, 90 / 255.0, 148 / 255.0, 1.0},
};

static Color gradient_midnight_blue[4] = {
    {44 / 255.0, 62 / 255.0, 80 / 255.0, 1.0},
    {44 / 255.0, 62 / 255.0, 80 / 255.0, 1.0},
    {44 / 255.0, 62 / 255.0, 80 / 255.0, 1.0},
    {44 / 255.0, 62 / 255.0, 80 / 255.0, 1.0},
};

static Color gradient_apple_green[4] = {
    {102 / 255.0, 134 / 255.0, 58 / 255.0, 1.0},
    {122 / 255.0, 131 / 255.0, 52 / 255.0, 1.0},
    {82 / 255.0, 101 / 255.0, 35 / 255.0, 1.0},
    {63 / 255.0, 95 / 255.0, 30 / 255.0, 1.0},
};

static Color gradient_undersea[4] = {
    {23 / 255.0, 18 / 255.0, 41 / 255.0, 1.0},
    {30 / 255.0, 72 / 255.0, 114 / 255.0, 1.0},
    {52 / 255.0, 88 / 255.0, 110 / 255.0, 1.0},
    {69 / 255.0, 125 / 255.0, 140 / 255.0, 1.0},
};

static Color gradient_morning_blue[4] = {
    {221 / 255.0, 241 / 255.0, 254 / 255.0, 1.0},
    {135 / 255.0, 206 / 255.0, 250 / 255.0, 1.0},
    {0.7, 0.7, 0.7, 1.0},
    {170 / 255.0, 200 / 255.0, 252 / 255.0, 1.0},
};

static Color gradient_sunbeam[4] = {
    {20 / 255.0, 13 / 255.0, 20 / 255.0, 1.0},
    {30 / 255.0, 72 / 255.0, 114 / 255.0, 1.0},
    {0.7, 0.7, 0.7, 1.0},
    {0.1, 0.0, 0.1, 1.0},
};

static Color gradient_lime_green[4] = {
    {209 / 255.0, 255 / 255.0, 82 / 255.0, 1.0},
    {146 / 255.0, 232 / 255.0, 66 / 255.0, 1.0},
    {82 / 255.0, 101 / 255.0, 35 / 255.0, 1.0},
    {63 / 255.0, 95 / 255.0, 30 / 255.0, 1.0},
};

static Color gradient_pikachu_yellow[4] = {
    {63 / 255.0, 63 / 255.0, 1 / 255.0, 1.0},
    {174 / 255.0, 174 / 255.0, 1 / 255.0, 1.0},
    {191 / 255.0, 194 / 255.0, 1 / 255.0, 1.0},
    {254 / 255.0, 221 / 255.0, 3 / 255.0, 1.0},
};

static Color gradient_gamecube_purple[4] = {
    {40 / 255.0, 20 / 255.0, 91 / 255.0, 1.0},
    {160 / 255.0, 140 / 255.0, 211 / 255.0, 1.0},
    {107 / 255.0, 92 / 255.0, 177 / 255.0, 1.0},
    {84 / 255.0, 71 / 255.0, 132 / 255.0, 1.0},
};

static Color gradient_famicom_red[4] = {
    {255 / 255.0, 191 / 255.0, 171 / 255.0, 1.0},
    {119 / 255.0, 49 / 255.0, 28 / 255.0, 1.0},
    {148 / 255.0, 10 / 255.0, 36 / 255.0, 1.0},
    {206 / 255.0, 126 / 255.0, 110 / 255.0, 1.0},
};

static Color gradient_flaming_hot[4] = {
    {231 / 255.0, 53 / 255.0, 53 / 255.0, 1.0},
    {242 / 255.0, 138 / 255.0, 97 / 255.0, 1.0},
    {236 / 255.0, 97 / 255.0, 76 / 255.0, 1.0},
    {255 / 255.0, 125 / 255.0, 3 / 255.0, 1.0},
};

static Color gradient_ice_cold[4] = {
    {66 / 255.0, 183 / 255.0, 229 / 255.0, 1.0},
    {29 / 255.0, 164 / 255.0, 255 / 255.0, 1.0},
    {176 / 255.0, 255 / 255.0, 247 / 255.0, 1.0},
    {174 / 255.0, 240 / 255.0, 255 / 255.0, 1.0},
};

static Color gradient_midgar[4] = {
    {255 / 255.0, 0 / 255.0, 0 / 255.0, 1.0},
    {0 / 255.0, 0 / 255.0, 255 / 255.0, 1.0},
    {0 / 255.0, 255 / 255.0, 0 / 255.0, 1.0},
    {32 / 255.0, 32 / 255.0, 32 / 255.0, 1.0},
};

static Color gradient_volcanic_red[4] = {
    {1.0, 0.0, 0.1, 1.0},
    {1.0, 0.1, 0.0, 1.0},
    {0.1, 0.0, 0.1, 1.0},
    {0.1, 0.0, 0.1, 1.0},
};

static Color gradient_dark[4] = {
    {0.05, 0.05, 0.05, 1.0},
    {0.05, 0.05, 0.05, 1.0},
    {0.05, 0.05, 0.05, 1.0},
    {0.05, 0.05, 0.05, 1.0},
};

static Color gradient_light[4] = {
    {0.50, 0.50, 0.50, 1.0},
    {0.50, 0.50, 0.50, 1.0},
    {0.50, 0.50, 0.50, 1.0},
    {0.50, 0.50, 0.50, 1.0},
};

static Color gradient_gray_dark[4] = {
    {16 / 255.0, 16 / 255.0, 16 / 255.0, 1.0},
    {16 / 255.0, 16 / 255.0, 16 / 255.0, 1.0},
    {16 / 255.0, 16 / 255.0, 16 / 255.0, 1.0},
    {16 / 255.0, 16 / 255.0, 16 / 255.0, 1.0},
};

static Color gradient_gray_light[4] = {
    {32 / 255.0, 32 / 255.0, 32 / 255.0, 1.0},
    {32 / 255.0, 32 / 255.0, 32 / 255.0, 1.0},
    {32 / 255.0, 32 / 255.0, 32 / 255.0, 1.0},
    {32 / 255.0, 32 / 255.0, 32 / 255.0, 1.0},
};

Color *themes[] = {
    gradient_golden,          //
    gradient_legacy_red,      //
    gradient_electric_blue,   //
    gradient_dark_purple,     //
    gradient_midnight_blue,   //
    gradient_apple_green,     //
    gradient_undersea,        //
    gradient_morning_blue,    //
    gradient_sunbeam,         //
    gradient_lime_green,      //
    gradient_pikachu_yellow,  //
    gradient_gamecube_purple, //
    gradient_famicom_red,     //
    gradient_flaming_hot,     //
    gradient_ice_cold,        //
    gradient_midgar,          //
    gradient_volcanic_red,    //
    gradient_dark,            //
    gradient_light,           //
    gradient_gray_dark,       //
    gradient_gray_light,      //
};

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
    float fs_vertices[8] = {-1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f};

    glGenVertexArrays(1, &ribbonState.bg_vao);
    glGenBuffers(1, &ribbonState.bg_vbo);

    glBindVertexArray(ribbonState.bg_vao);
    glBindBuffer(GL_ARRAY_BUFFER, ribbonState.bg_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(fs_vertices), fs_vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);

    static const char *bg_vertex_shader_text = GLSL(         //
        layout(location = 0) in vec2 pos;                    //
        void main() { gl_Position = vec4(pos, 0.0, 1.0); }); //

    static const char *bg_fragment_shader_text = GLSL( //
        out vec4 FragColor;                            //
        uniform vec2 u_resolution;                     //

        uniform vec4 color0; // bottom‑left
        uniform vec4 color1; // top‑left
        uniform vec4 color2; // bottom‑right
        uniform vec4 color3; // top‑right

        void main() {
            vec2 uv = gl_FragCoord.xy / u_resolution;
            float t = uv.y;
            vec4 top = mix(color0, color2, t);
            vec4 bottom = mix(color1, color3, t);
            float s = uv.x;
            FragColor = mix(top, bottom, s);
        });

    const GLuint bg_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(bg_vertex_shader, 1, &bg_vertex_shader_text, NULL);
    glCompileShader(bg_vertex_shader);
    check_shader_error(bg_vertex_shader, "vertex");

    const GLuint bg_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(bg_fragment_shader, 1, &bg_fragment_shader_text, NULL);
    glCompileShader(bg_fragment_shader);
    check_shader_error(bg_fragment_shader, "fragment");

    ribbonState.bg_program = glCreateProgram();
    glAttachShader(ribbonState.bg_program, bg_vertex_shader);
    glAttachShader(ribbonState.bg_program, bg_fragment_shader);
    glLinkProgram(ribbonState.bg_program);

    glDeleteShader(bg_vertex_shader);
    glDeleteShader(bg_fragment_shader);

    // ribbon

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

void draw_ribbon(float width, float height, float time, int theme) {
    glUseProgram(ribbonState.bg_program);
    glBindVertexArray(ribbonState.bg_vao);

    Color *colors = themes[theme];

    GLint locColors[4] = {
        glGetUniformLocation(ribbonState.bg_program, "color0"),
        glGetUniformLocation(ribbonState.bg_program, "color1"),
        glGetUniformLocation(ribbonState.bg_program, "color2"),
        glGetUniformLocation(ribbonState.bg_program, "color3"),
    };

    glUniform2f(glGetUniformLocation(ribbonState.bg_program, "u_resolution"), width, height);

    for (size_t i = 0; i < 4; ++i) {
        glUniform4f(locColors[i], colors[i].r, colors[i].g, colors[i].b, colors[i].a);
    }

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glUseProgram(ribbonState.ribbon_program);
    glUniform1f(glGetUniformLocation(ribbonState.ribbon_program, "time"), time);

    glBindVertexArray(ribbonState.ribbon_vertex_array);
    glBindBuffer(GL_ARRAY_BUFFER, ribbonState.ribbon_vertex_buffer);

    glDrawElements(GL_TRIANGLES, RIBBON_INDEX_COUNT, GL_UNSIGNED_INT, 0);
}
