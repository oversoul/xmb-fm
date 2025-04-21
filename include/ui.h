#pragma once

#define GL_GLEXT_PROTOTYPES
#include <GLFW/glfw3.h>

#include <stdint.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#define GLSL(src) "#version 430\n" #src

typedef struct {
    float r, g, b, a;
} Color;
typedef struct {
    float tl, tr, br, bl;
} Radius;

typedef struct Vertex {
    float pos[2];
    float size[2];
    Color color;
    float pos_px[2];
    Radius corner_radius;
    float texcoord[2]; // for glyphs
    float is_text;     // 1.0 = text, 0.0 = shape
} Vertex;

#define BATCHES 10000

#define FIRST_CHAR 32
#define LAST_CHAR 126
#define MAX_CHARS (LAST_CHAR - FIRST_CHAR + 1)

// Atlas dimensions
#define ATLAS_WIDTH 1024
#define ATLAS_HEIGHT 1024

// Maximum number of fonts that can be registered
#define MAX_FONTS 8

// Maximum number of font sizes per font
#define MAX_FONT_SIZES 16

typedef struct {
    int x, y;          // Position in atlas
    int width, height; // Glyph dimensions
    float xoff, yoff;  // Offset for positioning
    float xadvance;    // Horizontal advance
    int codepoint;     // Unicode codepoint
    int font_id;       // Font identifier
    float size;        // Font size
    bool used;         // Is this slot used?
} GlyphInfo;

typedef struct {
    FT_Face face;
    char name[64];
    float sizes[MAX_FONT_SIZES];
    int size_count;
} FontInfo;

typedef struct {
    unsigned char *pixels;
    int width, height;
    FT_Library ft_library;
    FontInfo fonts[MAX_FONTS];
    char font_names[MAX_FONTS][60];
    int font_count;
    GlyphInfo *glyphs;
    int glyph_capacity;
    int glyph_count;
    int current_x, current_y;
    int current_line_height;
    GLuint texture_id;
    bool dirty;
} FontAtlas;

typedef struct {
    Vertex verts[BATCHES * 4];
    uint32_t vert_count;
    uint32_t index_count;
    bool shape_in_progress;

    uint32_t program;
    uint32_t vertex_buffer;
    uint32_t vertex_array;

    FontAtlas *font_atlas;
    int current_font;
} RenderState;

// static const uint32_t indices[BATCHES * 6] = {
//     0, 1, 2, // tri 1
//     2, 3, 0, // tri 2
// };

void begin_rect(float x, float y);
void rect_size(float w, float h);
void rect_color(float r, float g, float b, float a);
void rect_gradient4(Color tl, Color tr, Color br, Color bl);

void rect_radius(float tl, float tr, float br, float bl);
void rect_radius_all(float value);
void end_rect();
void ui_create();
void start_frame(float width, float height);
void render_ribbon(float width, float height, float current_time);
void ui_delete();
void end_frame();

void add_rect(float x, float y, float w, float h, Color color[4]);

void create_font_atlas();
void use_font(const char *name);
int register_font(const char *name, const char *filename);
void get_text_bounds(float size, const char *text, float *width, float *height, float *start_x, float *bearing_y);
void draw_text(float size, float x, float y, const char *text, Color color);
float draw_wrapped_text(float size, float x, float y, const char *text, Color color, float max_width);
void destroy_font_atlas();
