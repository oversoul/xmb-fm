#include "ui.h"
#include <math.h>
#include <string.h>

#define GL_GLEXT_PROTOTYPES
#include <GLFW/glfw3.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <ft2build.h>
#include FT_FREETYPE_H

RenderState renderState = {
    .vert_count = 0,
    .index_count = 0,
};

// static const uint32_t indices[BATCHES * 6] = {
//     0, 1, 2, // tri 1
//     2, 3, 0, // tri 2
// };

static void check_shader_error(uint32_t shader, const char *type) {
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        fprintf(stderr, "Shader %s Compile Error: %s\n", type, infoLog);
    }
}

void add_vert(float x, float y, float w, float h, Color color, float px, float py, float t0, float t1, float is_text,
              Radius rad) {
    renderState.verts[renderState.vert_count++] = (Vertex){
        .pos = {x, y},
        .size = {w, h},
        .color = color,
        .pos_px = {px, py},
        .texcoord = {t0, t1},
        .is_text = is_text,
        .corner_radius = rad,
    };
}

void create_font_atlas() {
    FontAtlas *atlas = (FontAtlas *)malloc(sizeof(FontAtlas));
    if (!atlas) {
        fprintf(stderr, "Error: Could not initialize font.\n");
        exit(1);
    }

    // Initialize FreeType
    FT_Error error = FT_Init_FreeType(&atlas->ft_library);
    if (error) {
        fprintf(stderr, "Error: Could not initialize FreeType library\n");
        free(atlas);
        exit(1);
    }

    // Initialize atlas
    atlas->width = ATLAS_WIDTH;
    atlas->height = ATLAS_HEIGHT;
    atlas->pixels = (unsigned char *)calloc(atlas->width * atlas->height, 1);
    atlas->font_count = 0;

    // Initialize glyph storage
    atlas->glyph_capacity = 1024; // Start with space for 1024 glyphs
    atlas->glyphs = (GlyphInfo *)malloc(atlas->glyph_capacity * sizeof(GlyphInfo));
    atlas->glyph_count = 0;

    // Initialize positioning
    atlas->current_x = 0;
    atlas->current_y = 0;
    atlas->current_line_height = 0;

    // Mark atlas as not needing update yet
    atlas->dirty = false;

    // Create OpenGL texture
    glGenTextures(1, &atlas->texture_id);
    glBindTexture(GL_TEXTURE_2D, atlas->texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, atlas->width, atlas->height, 0, GL_RED, GL_UNSIGNED_BYTE, atlas->pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    renderState.font_atlas = atlas;
}

void use_font(const char *name) {
    int font_id = -1;
    for (size_t i = 0; i < renderState.font_atlas->font_count; ++i) {
        if (strcmp(name, renderState.font_atlas->font_names[i]) == 0) {
            font_id = i;
            break;
        }
    }

    if (font_id == -1) {
        fprintf(stderr, "Font %s not declared.\n", name);
        return;
    }

    renderState.current_font = font_id;
}

// Load font from file and register it with the atlas
int register_font(const char *name, const char *filename) {
    FontAtlas *atlas = renderState.font_atlas;
    if (!atlas) {
        fprintf(stderr, "Font atlas not created.\n");
        return -1;
    }

    if (atlas->font_count >= MAX_FONTS) {
        fprintf(stderr, "Error: Maximum number of fonts reached\n");
        return -1;
    }

    // Load font using FreeType
    FontInfo *font = &atlas->fonts[atlas->font_count];
    FT_Error error = FT_New_Face(atlas->ft_library, filename, 0, &font->face);

    if (error == FT_Err_Unknown_File_Format) {
        fprintf(stderr, "Error: Unsupported font format: %s\n", filename);
        return -1;
    } else if (error) {
        fprintf(stderr, "Error: Could not open font file: %s\n", filename);
        return -1;
    }

    // Store font information
    strncpy(font->name, filename, 63);
    font->name[63] = '\0';
    font->size_count = 0;

    size_t len = strlen(name);
    if (len > 59) {
        len = 59;
        printf("INFO: use a shorter name for font.\n");
    }

    strncpy(atlas->font_names[atlas->font_count], name, len);
    atlas->font_names[atlas->font_count][len] = '\0';

    return atlas->font_count++; // Return font ID and increment count
}

// Find glyph in atlas or return -1 if not found
int find_glyph(int font_id, float size, int codepoint) {
    FontAtlas *atlas = renderState.font_atlas;
    if (!atlas) {
        fprintf(stderr, "Font atlas not created.\n");
        return -1;
    }

    for (int i = 0; i < atlas->glyph_count; i++) {
        GlyphInfo *glyph = &atlas->glyphs[i];
        if (glyph->used && glyph->font_id == font_id && glyph->size == size && glyph->codepoint == codepoint) {
            return i;
        }
    }
    return -1;
}

// Add a new line in the atlas when current line is full
bool add_new_line(FontAtlas *atlas) {
    atlas->current_x = 0;
    atlas->current_y += atlas->current_line_height;
    atlas->current_line_height = 0;

    // Check if we've run out of atlas space
    if (atlas->current_y >= atlas->height) {
        // Could implement atlas expansion or paging here
        fprintf(stderr, "Error: Font atlas is full\n");
        return false;
    }

    return true;
}

int add_glyph_to_atlas(FontAtlas *atlas, int font_id, float size, int codepoint) {
    // Check if font_id is valid
    if (font_id < 0 || font_id >= atlas->font_count) {
        fprintf(stderr, "Error: Invalid font ID: %d\n", font_id);
        return -1;
    }

    // Check if already in atlas
    int existing = find_glyph(font_id, size, codepoint);
    if (existing >= 0) {
        return existing;
    }

    // Get font info
    FontInfo *font = &atlas->fonts[font_id];

    // Check if we've seen this size before
    bool size_found = false;
    for (int i = 0; i < font->size_count; i++) {
        if (font->sizes[i] == size) {
            size_found = true;
            break;
        }
    }

    // Add new size if needed
    if (!size_found && font->size_count < MAX_FONT_SIZES) {
        font->sizes[font->size_count++] = size;
    } else if (!size_found) {
        fprintf(stderr, "Warning: Maximum number of font sizes reached\n");
    }

    // Set font size (FreeType uses 26.6 fixed-point format, so multiply by 64)
    FT_Error error = FT_Set_Char_Size(font->face, 0, (FT_F26Dot6)(size * 64.0), 0, 0);
    if (error) {
        fprintf(stderr, "Error: Could not set font size\n");
        return -1;
    }

    // Load glyph
    FT_UInt glyph_index = FT_Get_Char_Index(font->face, codepoint);
    error = FT_Load_Glyph(font->face, glyph_index, FT_LOAD_DEFAULT | FT_LOAD_RENDER);
    if (error) {
        fprintf(stderr, "Error: Could not load glyph\n");
        return -1;
    }

    if (font->face == NULL)
        return -1;

    // Get bitmap and metrics
    FT_Bitmap bitmap = font->face->glyph->bitmap;
    FT_GlyphSlot slot = font->face->glyph;

    int width = bitmap.width;
    int height = bitmap.rows;

    int effective_width = width;
    if (slot->bitmap_left < 0) {
        effective_width += -slot->bitmap_left; // Extend width to the left
    }

    float xadvance = (float)slot->advance.x / 64.0f;
    if (xadvance > effective_width) {
        effective_width = (int)ceil(xadvance);
    }

    // Get the font's global metrics for this size
    float ascender = font->face->size->metrics.ascender / 64.0f;
    float descender = font->face->size->metrics.descender / 64.0f;

    float padding = 1.0f;
    int total_height = (int)ceil(ascender - descender) + (int)padding;
    int final_height = height > total_height ? height : total_height;

    if (atlas->glyph_count >= atlas->glyph_capacity) {
        atlas->glyph_capacity *= 2;
        atlas->glyphs = (GlyphInfo *)realloc(atlas->glyphs, atlas->glyph_capacity * sizeof(GlyphInfo));
    }

    if (atlas->current_x + width > atlas->width) {
        if (!add_new_line(atlas)) {
            return -1;
        }
    }

    if (final_height > atlas->current_line_height) {
        atlas->current_line_height = final_height;
    }

    // Add glyph info
    GlyphInfo *glyph = &atlas->glyphs[atlas->glyph_count];
    glyph->x = atlas->current_x;
    glyph->y = atlas->current_y;
    glyph->width = effective_width;
    glyph->height = final_height;
    glyph->xoff = (float)slot->bitmap_left;
    glyph->yoff = (float)-slot->bitmap_top;
    glyph->xadvance = xadvance;
    glyph->codepoint = codepoint;
    glyph->font_id = font_id;
    glyph->size = size;
    glyph->used = true;

    // Copy bitmap to atlas
    for (int j = 0; j < height; ++j) {
        for (int i = 0; i < width; ++i) {
            int atlas_x = glyph->x + i + (slot->bitmap_left < 0 ? -slot->bitmap_left : 0);
            int atlas_y = glyph->y + j;
            if (atlas_x < atlas->width && atlas_y < atlas->height) { // Safety check
                atlas->pixels[atlas_y * atlas->width + atlas_x] = bitmap.buffer[j * bitmap.pitch + i];
            }
        }
    }

    atlas->current_x += effective_width + padding;
    atlas->dirty = true;
    return atlas->glyph_count++;
}

void update_atlas_texture(FontAtlas *atlas) {
    if (!atlas->dirty)
        return;

    glBindTexture(GL_TEXTURE_2D, atlas->texture_id);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, atlas->width, atlas->height, GL_RED, GL_UNSIGNED_BYTE, atlas->pixels);

    atlas->dirty = false;
}

unsigned int decode_utf8(const char **text) {
    unsigned int c = (unsigned char)**text;

    if (c < 0x80) { // ASCII
        (*text)++;
        return c;
    } else if (c < 0xE0) { // 2-byte
        unsigned int c2 = (unsigned char)*((*text) + 1);
        (*text) += 2;
        return ((c & 0x1F) << 6) | (c2 & 0x3F);
    } else if (c < 0xF0) { // 3-byte
        unsigned int c2 = (unsigned char)*((*text) + 1);
        unsigned int c3 = (unsigned char)*((*text) + 2);
        (*text) += 3;
        return ((c & 0x0F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F);
    } else { // 4-byte (or invalid)
        unsigned int c2 = (unsigned char)*((*text) + 1);
        unsigned int c3 = (unsigned char)*((*text) + 2);
        unsigned int c4 = (unsigned char)*((*text) + 3);
        (*text) += 4;
        return ((c & 0x07) << 18) | ((c2 & 0x3F) << 12) | ((c3 & 0x3F) << 6) | (c4 & 0x3F);
    }
}

void draw_glyph(float cursor_x, float cursor_y, Color color, GlyphInfo *glyph, float aw, float ah) {
    // Calculate vertices
    float x0 = cursor_x + glyph->xoff;
    float y0 = cursor_y + glyph->yoff;
    float w = glyph->width;
    float h = glyph->height;

    // Calculate texture coordinates
    float u0 = (float)glyph->x / aw;
    float v0 = (float)glyph->y / ah;
    float u1 = (float)(glyph->x + glyph->width) / aw;
    float v1 = (float)(glyph->y + glyph->height) / ah;

    // Draw quad
    Radius rad = {0, 0, 0, 0};

    float pos[8] = {
        -1.0, -1.0, // Vertex 0: Bottom-left
        1.0,  -1.0, // Vertex 1: Bottom-right
        1.0,  1.0,  // Vertex 2: Top-right
        -1.0, 1.0   // Vertex 3: Top-left
    };

    add_vert(pos[0], pos[1], w, h, color, x0, y0, u0, v0, 1, rad);
    add_vert(pos[2], pos[3], w, h, color, x0, y0, u1, v0, 1, rad);
    add_vert(pos[4], pos[5], w, h, color, x0, y0, u1, v1, 1, rad);
    add_vert(pos[6], pos[7], w, h, color, x0, y0, u0, v1, 1, rad);
    renderState.index_count += 6;
}

// Get glyph positions for a string, adding glyphs to atlas as needed
void get_string_glyphs(FontAtlas *atlas, int font_id, float size, const char *text, GlyphInfo **glyphs, int *count) {
    if (!text || !glyphs || !count)
        return;

    // Count UTF-8 characters
    *count = 0;
    const char *p = text;
    while (*p) {
        decode_utf8(&p);
        (*count)++;
    }

    // Allocate glyph array
    *glyphs = (GlyphInfo *)malloc(*count * sizeof(GlyphInfo));

    // Process each character
    p = text;
    int idx = 0;

    while (*p) {
        int codepoint = decode_utf8(&p);

        // Get or add glyph
        int glyph_idx = add_glyph_to_atlas(atlas, font_id, size, codepoint);
        if (glyph_idx >= 0) {
            (*glyphs)[idx++] = atlas->glyphs[glyph_idx];
        }
    }

    *count = idx;
    update_atlas_texture(atlas);
}

// Calculate text bounds
void get_text_bounds(float size, const char *text, float *width, float *height, float *start_x, float *bearing_y) {
    GlyphInfo *glyphs;
    int glyph_count;

    FontAtlas *atlas = renderState.font_atlas;
    if (!atlas) {
        fprintf(stderr, "Font atlas not created.\n");
        return;
    }

    int font_id = renderState.current_font;
    get_string_glyphs(atlas, font_id, size, text, &glyphs, &glyph_count);

    float min_x = 0, min_y = 0, max_x = 0, max_y = 0;
    float cursor_x = 0;

    for (int i = 0; i < glyph_count; i++) {
        GlyphInfo *glyph = &glyphs[i];

        float x0 = cursor_x + glyph->xoff;
        float y0 = glyph->yoff;
        float x1 = x0 + glyph->width;
        float y1 = y0 + glyph->height;

        min_x = (i == 0) ? x0 : (x0 < min_x ? x0 : min_x);
        min_y = (i == 0) ? y0 : (y0 < min_y ? y0 : min_y);
        max_x = (x1 > max_x) ? x1 : max_x;
        max_y = (y1 > max_y) ? y1 : max_y;

        cursor_x += glyph->xadvance;
    }

    // For empty strings
    if (glyph_count == 0) {
        min_x = min_y = max_x = max_y = 0;
    }

    *width = max_x - min_x;
    *height = max_y - min_y;

    if (start_x)
        *start_x = min_x;
    if (bearing_y)
        *bearing_y = -min_y;

    free(glyphs);
}

static int render_line(FontAtlas *atlas, float x, float y, Color color, GlyphInfo *glyphs, int glyph_count,
                       float max_width, int *glyphs_consumed, float *rendered_width) {
    int end = 0;
    float width = 0;
    int last_break = -1;
    int atlas_w = atlas->width;
    int atlas_h = atlas->height;

    while (end < glyph_count) {
        GlyphInfo *glyph = &glyphs[end];

        if (glyph->codepoint == '\n') {
            end++; // Consume the newline but don't render it
            break;
        }

        if (glyph->codepoint == ' ' || glyph->codepoint == '\t') {
            last_break = end;
        }

        // Check if adding this glyph would exceed max_width
        if (width + glyph->xadvance > max_width && max_width > 0) {
            if (last_break > 0) {
                end = last_break; // Break at last space
            }
            break;
        }

        width += glyph->xadvance;
        end++;
    }

    // If we couldn't fit even one glyph, force at least one
    if (end == 0 && glyph_count > 0) {
        end = 1;
        width = glyphs[0].xadvance;
    }

    // Render the line
    float current_x = x;
    for (int i = 0; i < end; i++) {
        if (glyphs[i].codepoint == ' ' || glyphs[i].codepoint == '\n' || glyphs[i].codepoint == '\t') {
            continue;
        }
        draw_glyph(current_x, y, color, &glyphs[i], atlas_w, atlas_h);
        current_x += glyphs[i].xadvance;
    }

    *glyphs_consumed = end;
    *rendered_width = width;
    return end;
}

float draw_wrapped_text(float size, float x, float y, const char *text, Color color, float max_width) {
    if (!text || !text[0] || !renderState.font_atlas)
        return 0;

    GlyphInfo *glyphs;
    int glyph_count;

    FontAtlas *atlas = renderState.font_atlas;
    if (!atlas) {
        fprintf(stderr, "Font atlas not created.\n");
        return 0;
    }

    int font_id = renderState.current_font;
    // Get glyphs for the entire string
    get_string_glyphs(atlas, font_id, size, text, &glyphs, &glyph_count);

    // Line tracking variables
    float line_height = size * 1.5f; // Adjust line spacing as needed

    int start = 0;
    float total_height = 0;
    while (start < glyph_count) {
        int consumed;
        float line_width;

        render_line(atlas, x, y + total_height, color, &glyphs[start], glyph_count - start, max_width, &consumed,
                    &line_width);

        start += consumed;
        total_height += line_height;
    }

    free(glyphs);

    return total_height;
}

void draw_single_line(float font_size, float x, float y, const char *text, Color color, float max_width) {
    GlyphInfo *glyphs;
    int glyph_count;
    get_string_glyphs(renderState.font_atlas, renderState.current_font, font_size, text, &glyphs, &glyph_count);

    int consumed;
    float width;
    render_line(renderState.font_atlas, x, y, color, glyphs, glyph_count, max_width, &consumed, &width);

    free(glyphs);
}

void draw_text(float size, float x, float y, const char *text, Color color) {
    // Get glyphs for the string
    GlyphInfo *glyphs;
    int glyph_count;

    FontAtlas *atlas = renderState.font_atlas;
    if (!atlas) {
        fprintf(stderr, "Font atlas not created.\n");
        return;
    }

    int font_id = renderState.current_font;

    get_string_glyphs(atlas, font_id, size, text, &glyphs, &glyph_count);

    // Enable texture
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, atlas->texture_id);

    float cursor_x = x;
    float cursor_y = y;

    FontInfo *font = &atlas->fonts[font_id];
    for (int i = 0; i < glyph_count; i++) {
        GlyphInfo *glyph = &glyphs[i];
        if (glyph->codepoint == ' ') {
            cursor_x += glyph->xadvance;
            continue;
        }

        draw_glyph(cursor_x, cursor_y, color, glyph, atlas->width, atlas->height);

        cursor_x += glyph->xadvance;

        if (i < glyph_count - 1) {
            FT_Vector kerning;
            FT_UInt current = FT_Get_Char_Index(font->face, glyph->codepoint);
            FT_UInt next = FT_Get_Char_Index(font->face, glyphs[i + 1].codepoint);

            if (FT_HAS_KERNING(font->face)) {
                FT_Get_Kerning(font->face, current, next, FT_KERNING_DEFAULT, &kerning);
                cursor_x += kerning.x / 64.0f; // Convert from 26.6 format
            }
        }
    }

    free(glyphs);
}

// Clean up resources
void destroy_font_atlas() {
    FontAtlas *atlas = renderState.font_atlas;
    if (!atlas)
        return;

    free(atlas->pixels);

    for (int i = 0; i < atlas->font_count; i++) {
        FT_Done_Face(atlas->fonts[i].face);
    }

    FT_Done_FreeType(atlas->ft_library);

    free(atlas->glyphs);

    glDeleteTextures(1, &atlas->texture_id);

    renderState.current_font = -1;
    free(atlas);
}

void begin_rect(float x, float y) {
    if (renderState.shape_in_progress)
        return;

    float pos[8] = {
        -1.0f, -1.0f, // Vertex 0: Bottom-left
        1.0f,  -1.0f, // Vertex 1: Bottom-right
        1.0f,  1.0f,  // Vertex 2: Top-right
        -1.0f, 1.0f   // Vertex 3: Top-left
    };

    renderState.shape_in_progress = true;
    int base_vert = renderState.vert_count;
    for (size_t i = 0; i < 4; ++i) {
        renderState.verts[base_vert + i] = (Vertex){
            .pos = {pos[i * 2], pos[i * 2 + 1]},
            .pos_px = {x, y},
        };
    }

    renderState.vert_count += 4;
}

void rect_size(float w, float h) {
    if (!renderState.shape_in_progress)
        return;

    for (size_t i = 4; i > 0; i--) {
        renderState.verts[renderState.vert_count - i].size[0] = w;
        renderState.verts[renderState.vert_count - i].size[1] = h;
    }
}

void rect_color(float r, float g, float b, float a) {
    if (!renderState.shape_in_progress)
        return;
    for (size_t i = 4; i > 0; i--)
        renderState.verts[renderState.vert_count - i].color = (Color){r, g, b, a};
}

void rect_gradient_topdown(Color top, Color bottom) {
    if (!renderState.shape_in_progress)
        return;

    size_t vc = renderState.vert_count;
    renderState.verts[vc - 4].color = top;    // top-left
    renderState.verts[vc - 3].color = top;    // top-right
    renderState.verts[vc - 2].color = bottom; // bottom-right
    renderState.verts[vc - 1].color = bottom; // bottom-left
}

void rect_gradient_sides(Color left, Color right) {
    if (!renderState.shape_in_progress)
        return;

    size_t vc = renderState.vert_count;
    renderState.verts[vc - 4].color = left;  // top-left
    renderState.verts[vc - 3].color = right; // top-right
    renderState.verts[vc - 2].color = right; // bottom-right
    renderState.verts[vc - 1].color = left;  // bottom-left
}

void rect_gradient4(Color tl, Color tr, Color br, Color bl) {
    if (!renderState.shape_in_progress)
        return;

    size_t vc = renderState.vert_count;
    renderState.verts[vc - 4].color = tl; // top-left
    renderState.verts[vc - 3].color = tr; // top-right
    renderState.verts[vc - 2].color = br; // bottom-right
    renderState.verts[vc - 1].color = bl; // bottom-left
}

void rect_radius(float tl, float tr, float br, float bl) {
    if (!renderState.shape_in_progress)
        return;
    for (size_t i = 4; i > 0; i--)
        renderState.verts[renderState.vert_count - i].corner_radius = (Radius){tr, br, tl, bl};
}

void rect_radius_all(float v) { rect_radius(v, v, v, v); }

void end_rect() {
    if (!renderState.shape_in_progress)
        return;

    renderState.shape_in_progress = false;
    renderState.index_count += 6;
}

void add_rect(float x, float y, float w, float h, Color color[4]) {
    Radius rad = {0, 0, 0, 0};
    float pos[8] = {
        -1.0f, -1.0f, // Vertex 0: Bottom-left
        1.0f,  -1.0f, // Vertex 1: Bottom-right
        1.0f,  1.0f,  // Vertex 2: Top-right
        -1.0f, 1.0f   // Vertex 3: Top-left
    };
    add_vert(pos[0], pos[1], w, h, color[0], x, y, 0, 0, 0, rad);
    add_vert(pos[2], pos[3], w, h, color[1], x, y, 0, 0, 0, rad);
    add_vert(pos[4], pos[5], w, h, color[2], x, y, 0, 0, 0, rad);
    add_vert(pos[6], pos[7], w, h, color[3], x, y, 0, 0, 0, rad);
    renderState.index_count += 6;
}

void ui_create() {
    static const char *vertex_shader_text = GLSL(   //
        layout(location = 0) in vec2 vPos;          //
        layout(location = 1) in vec4 vCol;          //
        layout(location = 2) in vec2 vSize;         //
        layout(location = 3) in vec2 aPos;          //
        layout(location = 4) in vec4 aCornerRadius; //
        layout(location = 5) in vec2 vTexCoord;     //
        layout(location = 6) in float vIsText;      //

        out vec2 texCoord;            //
        flat out vec4 cornerRadius;   //
        flat out float isText;        //
        out vec4 color;               //
        out vec2 frag_local_pos;      // this will be used to calculate UV
        out vec2 frag_rect_pos;       // Global fragment position in screen space
        flat out vec2 frag_rect_size; //
        uniform vec2 u_resolution;    //

        void main() {
            vec2 screenPos = aPos + (vPos * 0.5 + 0.5) * vSize;
            // Convert to NDC
            vec2 ndc = (screenPos / u_resolution) * 2.0 - 1.0;
            ndc.y = -ndc.y; // Flip Y
            gl_Position = vec4(ndc, 0.0, 1.0);

            frag_local_pos = screenPos;
            frag_rect_pos = aPos;
            frag_rect_size = vSize;
            color = vCol;
            cornerRadius = aCornerRadius;
            isText = vIsText;
            texCoord = vTexCoord;
        });

    static const char *fragment_shader_text = GLSL( //
        in vec4 color;                              //
        in vec2 frag_local_pos;                     //
        in vec2 frag_rect_pos;                      //
        flat in vec2 frag_rect_size;                //
        flat in vec4 cornerRadius;                  //
                                                    //
        out vec4 fragment;                          //
        uniform vec2 u_resolution;                  //

        in vec2 texCoord;                  //
        flat in float isText;              //
        uniform sampler2D u_glyph_texture; //

        float sdroundedbox(vec2 p, vec2 b, vec4 r) { //
            r.xy = (p.x > 0.0) ? r.xy : r.zw;
            r.x = (p.y > 0.0) ? r.x : r.y;
            vec2 q = abs(p) - b + r.x;
            return min(max(q.x, q.y), 0.0) + length(max(q, 0.0)) - r.x;
        }

        float roundedBox(vec2 uv, float radius) {
            vec2 halfSize = vec2(0.5) - radius;
            vec2 d = abs(uv - 0.5) - halfSize;
            return length(max(d, 0.0)) - radius;
        }

        void main() {
            // Check if we're in the rectangle area
            if (frag_local_pos.x < frag_rect_pos.x || frag_local_pos.x > frag_rect_pos.x + frag_rect_size.x ||
                frag_local_pos.y < frag_rect_pos.y || frag_local_pos.y > frag_rect_pos.y + frag_rect_size.y) {
                discard; // Early out for pixels outside the rectangle
                return;
            }

            if (isText > 0.5) {
                float glyph_alpha = texture(u_glyph_texture, texCoord).r; // grayscale
                fragment = vec4(color.rgb, color.a * glyph_alpha);
                return;
            }

            // // if circle
            // vec2 uv = (frag_local_pos - frag_rect_pos) / frag_rect_size;
            // float dist = length(uv - 0.5);
            // float alpha = 1.0 - smoothstep(0.49, 0.5, dist);
            // fragment = vec4(color.rgb, color.a * alpha);
            // return;

            vec2 center = frag_rect_pos + frag_rect_size * 0.5;            // Center of rectangle
            vec2 p = frag_local_pos - center;                              // Vector from center to current fragment
            float d = sdroundedbox(p, frag_rect_size * 0.5, cornerRadius); //
            float alpha = 1.0 - smoothstep(-0.5, 0.5, d);                  // Smooth edge
            fragment = vec4(color.rgb, color.a * alpha);
        });

    glGenBuffers(1, &renderState.vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, renderState.vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * BATCHES * 4, renderState.verts, GL_DYNAMIC_DRAW);

    const GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
    glCompileShader(vertex_shader);
    check_shader_error(vertex_shader, "vertex");

    const GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
    glCompileShader(fragment_shader);
    check_shader_error(fragment_shader, "fragment");

    renderState.program = glCreateProgram();
    glAttachShader(renderState.program, vertex_shader);
    glAttachShader(renderState.program, fragment_shader);
    glLinkProgram(renderState.program);

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    glGenVertexArrays(1, &renderState.vertex_array);
    glBindVertexArray(renderState.vertex_array);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, pos));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, color));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, size));

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, pos_px));

    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, corner_radius));

    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, texcoord));

    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, is_text));

    uint32_t *indices = malloc(sizeof(uint32_t) * BATCHES * 6);
    uint32_t offset = 0;
    for (uint32_t i = 0; i < BATCHES * 6; i += 6) {
        // tri 1
        indices[i + 0] = offset + 0;
        indices[i + 1] = offset + 1;
        indices[i + 2] = offset + 2;

        // tri 2
        indices[i + 3] = offset + 2;
        indices[i + 4] = offset + 3;
        indices[i + 5] = offset + 0;
        offset += 4;
    }

    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * BATCHES * sizeof(uint32_t), indices, GL_STATIC_DRAW);

    free(indices);

    // Create the font atlas
    create_font_atlas();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void start_frame(float width, float height) {
    glUseProgram(renderState.program);
    glUniform2f(glGetUniformLocation(renderState.program, "u_resolution"), (float)width, (float)height);
}

void ui_delete() {
    destroy_font_atlas();
    glDeleteProgram(renderState.program);
}

void end_frame() {
    // printf("vert count: %d\n", renderState.vert_count);
    // printf("/////////////////\n");
    // for (int i = 0; i < renderState.vert_count; i++) {
    //     Vertex v = renderState.verts[i];
    //     // printf("Vertex %d: pos = (%.2f, %.2f)\n", i, v.pos[0], v.pos[1]);
    //     printf("Vertex %d: pos_px = (%.2f, %.2f)\n", i, v.pos_px[0], v.pos_px[1]);
    // }
    // printf("/////////////////\n");

    glBindVertexArray(renderState.vertex_array);
    glBindBuffer(GL_ARRAY_BUFFER, renderState.vertex_buffer);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(renderState.verts), renderState.verts);

    glDrawElements(GL_TRIANGLES, renderState.index_count, GL_UNSIGNED_INT, 0);

    renderState.vert_count = 0;
    renderState.index_count = 0;
}
