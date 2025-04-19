#include "ui.h"
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

// Render a glyph to the atlas using FreeType
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
    error = FT_Load_Glyph(font->face, glyph_index, FT_LOAD_DEFAULT);
    if (error) {
        fprintf(stderr, "Error: Could not load glyph\n");
        return -1;
    }

    // Render glyph to bitmap
    error = FT_Render_Glyph(font->face->glyph, FT_RENDER_MODE_NORMAL);
    if (error) {
        fprintf(stderr, "Error: Could not render glyph\n");
        return -1;
    }

    // Get bitmap and metrics
    FT_Bitmap bitmap = font->face->glyph->bitmap;
    FT_GlyphSlot slot = font->face->glyph;

    int width = bitmap.width;
    int height = bitmap.rows;

    // Ensure we have space in the glyph array
    if (atlas->glyph_count >= atlas->glyph_capacity) {
        atlas->glyph_capacity *= 2;
        atlas->glyphs = (GlyphInfo *)realloc(atlas->glyphs, atlas->glyph_capacity * sizeof(GlyphInfo));
    }

    // Check if this glyph fits on current line
    if (atlas->current_x + width > atlas->width) {
        if (!add_new_line(atlas)) {
            return -1;
        }
    }

    // Adjust line height if needed
    if (height > atlas->current_line_height) {
        atlas->current_line_height = height;
    }

    // Add glyph info
    GlyphInfo *glyph = &atlas->glyphs[atlas->glyph_count];
    glyph->x = atlas->current_x;
    glyph->y = atlas->current_y;

    glyph->width = width;
    glyph->height = bitmap.rows + 2; // hack: text appears cropped
    glyph->xoff = (float)slot->bitmap_left;
    glyph->yoff = (float)-slot->bitmap_top;           // Negative because FreeType's Y is up
    glyph->xadvance = (float)slot->advance.x / 64.0f; // Convert from 26.6 format
    glyph->codepoint = codepoint;
    glyph->font_id = font_id;
    glyph->size = size;
    glyph->used = true;

    // Copy bitmap to atlas, handling pitch (row stride)
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            atlas->pixels[(glyph->y + j) * atlas->width + (glyph->x + i)] = bitmap.buffer[j * bitmap.pitch + i];
        }
    }

    // Update atlas position
    atlas->current_x += width + 1; // +1 for padding

    // Mark atlas as needing update
    atlas->dirty = true;

    // Return the glyph index
    return atlas->glyph_count++;
}

// Update the OpenGL texture if the atlas is dirty
void update_atlas_texture(FontAtlas *atlas) {
    if (!atlas->dirty)
        return;

    glBindTexture(GL_TEXTURE_2D, atlas->texture_id);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, atlas->width, atlas->height, GL_RED, GL_UNSIGNED_BYTE, atlas->pixels);

    atlas->dirty = false;
}

// Decode a UTF-8 character and advance pointer
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

    // Update actual count (in case some glyphs failed)
    *count = idx;

    // Update texture if needed
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

void draw_wrapped_text(float size, float x, float y, const char *text, Color color, float max_width) {
    GlyphInfo *glyphs;
    int glyph_count;

    FontAtlas *atlas = renderState.font_atlas;
    if (!atlas) {
        fprintf(stderr, "Font atlas not created.\n");
        return;
    }

    int font_id = renderState.current_font;
    // Get glyphs for the entire string
    get_string_glyphs(atlas, font_id, size, text, &glyphs, &glyph_count);

    // Line tracking variables
    int line_start = 0;
    int last_space = -1;
    float cursor_x = 0;
    float line_y = y;
    float line_height = size * 1.5f; // Adjust line spacing as needed

    // Process each glyph
    for (int i = 0; i <= glyph_count; i++) {
        // Check if we're at the end or have a space
        bool is_space = false;
        bool is_linebreak = false;
        bool is_end = (i == glyph_count);

        if (!is_end) {
            GlyphInfo *glyph = &glyphs[i];
            is_space = (glyph->codepoint == ' ' || glyph->codepoint == '\t');
            is_linebreak = (glyph->codepoint == '\n');

            // Update the cursor position
            if (!is_linebreak) {
                cursor_x += glyph->xadvance;
            }

            // Remember position of the last space
            if (is_space) {
                last_space = i;
            }
        }

        // Check if we need to wrap (exceeded width or reached end)
        if (is_end || is_linebreak || cursor_x > max_width) {
            int line_end;

            // Determine where to break the line
            if (is_linebreak) {
                // Break exactly at the newline character
                line_end = i + 1;
            } else if (cursor_x > max_width && last_space > line_start) {
                // Break at the last space
                line_end = last_space;
            } else if (is_end) {
                // End of text, include all remaining glyphs
                line_end = glyph_count;
            } else {
                // No space found, force break at current position
                line_end = i;
            }

            // Draw this line
            float line_x = x;
            for (int j = line_start; j < line_end; j++) {
                GlyphInfo *glyph = &glyphs[j];

                // Draw this glyph
                draw_glyph(line_x, line_y, color, glyph, atlas->width, atlas->height);

                // Advance cursor for this line
                line_x += glyph->xadvance;
            }

            // Start new line after the break
            if (cursor_x > max_width && last_space > line_start) {
                // Skip the space that caused the break
                line_start = last_space + 1;
            } else {
                line_start = line_end;
            }

            // Reset tracking variables
            cursor_x = 0;
            last_space = -1;
            line_y += line_height;

            // Recompute cursor position for the new line
            for (int j = line_start; j < i; j++) {
                cursor_x += glyphs[j].xadvance;
            }

            // If we just processed the end, we're done
            if (is_end)
                break;
        }
    }

    // Free the glyphs
    free(glyphs);
}

// Draw text using the atlas
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

    // Current position
    float cursor_x = x;
    float cursor_y = y;

    for (int i = 0; i < glyph_count; i++) {
        GlyphInfo *glyph = &glyphs[i];

        draw_glyph(cursor_x, cursor_y, color, glyph, atlas->width, atlas->height);

        // Move cursor
        cursor_x += glyph->xadvance;

        // Kerning could be added here
        if (i < glyph_count - 1) {
            FontInfo *font = &atlas->fonts[font_id];
            FT_Vector kerning;
            FT_UInt current = FT_Get_Char_Index(font->face, glyph->codepoint);
            FT_UInt next = FT_Get_Char_Index(font->face, glyphs[i + 1].codepoint);

            if (FT_HAS_KERNING(font->face)) {
                FT_Get_Kerning(font->face, current, next, FT_KERNING_DEFAULT, &kerning);
                cursor_x += kerning.x / 64.0f; // Convert from 26.6 format
            }
        }
    }

    // Free temporary glyph array
    free(glyphs);
}

// Clean up resources
void destroy_font_atlas() {
    FontAtlas *atlas = renderState.font_atlas;
    if (!atlas)
        return;

    // Free pixel data
    free(atlas->pixels);

    // Free FreeType resources
    for (int i = 0; i < atlas->font_count; i++) {
        FT_Done_Face(atlas->fonts[i].face);
    }
    FT_Done_FreeType(atlas->ft_library);

    // Free glyph array
    free(atlas->glyphs);

    // Delete OpenGL texture
    glDeleteTextures(1, &atlas->texture_id);

    renderState.current_font = -1;

    // Free the atlas struct
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

            // Pass through data to fragment shader
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

    // ribbon
    glGenBuffers(1, &renderState.ribbon_vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, renderState.ribbon_vertex_buffer);

    //

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

        float xmb_noise2(vec3 x) { return cos(x.z * 4.0) * cos(x.z + time / 10.0 + x.x); }

        void main() {
            vec3 v = vec3(VertexCoord.x, 0.0, VertexCoord.y);
            vec3 v2 = v;
            vec3 v3 = v;

            v.y = xmb_noise2(v2) / 8.0;

            v3.x -= time / 5.0;
            v3.x /= 4.0;

            v3.z -= time / 10.0;
            v3.y -= time / 100.0;

            v.z -= noise(v3 * 7.0) / 15.0;
            v.y -= noise(v3 * 7.0) / 15.0 + cos(v.x * 2.0 - time / 2.0) / 5.0 - 0.3;
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
            FragColor = vec4(1.0, 1.0, 1.0, c * 1.5);
        });

#define X_SEGMENTS 128
#define Y_SEGMENTS 32
#define VERTEX_COUNT ((X_SEGMENTS + 1) * (Y_SEGMENTS + 1))
#define INDEX_COUNT (X_SEGMENTS * Y_SEGMENTS * 6)

    float ribbon_vertices[VERTEX_COUNT * 2];
    size_t i = 0;
    for (int y = 0; y <= Y_SEGMENTS; ++y) {
        for (int x = 0; x <= X_SEGMENTS; ++x) {
            float u = (float)x / X_SEGMENTS;
            float v = (float)y / Y_SEGMENTS;
            float px = u * 2.0f - 1.0f;
            float py = v * 2.0f - 1.0f;
            ribbon_vertices[i++] = px;
            ribbon_vertices[i++] = py;
        }
    }

    glGenBuffers(1, &renderState.ribbon_vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, renderState.ribbon_vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * VERTEX_COUNT * 2, ribbon_vertices, GL_STATIC_DRAW);

    const GLuint ribbon_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(ribbon_vertex_shader, 1, &ribbon_vertex_shader_text, NULL);
    glCompileShader(ribbon_vertex_shader);
    check_shader_error(ribbon_vertex_shader, "vertex");

    const GLuint ribbon_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(ribbon_fragment_shader, 1, &ribbon_fragment_shader_text, NULL);
    glCompileShader(ribbon_fragment_shader);
    check_shader_error(ribbon_fragment_shader, "fragment");

    renderState.ribbon_program = glCreateProgram();
    glAttachShader(renderState.ribbon_program, ribbon_vertex_shader);
    glAttachShader(renderState.ribbon_program, ribbon_fragment_shader);
    glLinkProgram(renderState.ribbon_program);

    glDeleteShader(ribbon_vertex_shader);
    glDeleteShader(ribbon_fragment_shader);

    glGenVertexArrays(1, &renderState.ribbon_vertex_array);
    glBindVertexArray(renderState.ribbon_vertex_array);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);

    uint32_t *ribbon_indices = malloc(sizeof(GLuint) * INDEX_COUNT);
    uint32_t ribbon_offset = 0;
    for (int y = 0; y < Y_SEGMENTS; ++y) {
        for (int x = 0; x < X_SEGMENTS; ++x) {
            int i0 = y * (X_SEGMENTS + 1) + x;
            int i1 = i0 + 1;
            int i2 = i0 + (X_SEGMENTS + 1);
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
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * INDEX_COUNT, ribbon_indices, GL_STATIC_DRAW);

    free(ribbon_indices);

    // Create the font atlas
    create_font_atlas();
}

void render_ribbon(float width, float height, float time) {
    glUseProgram(renderState.ribbon_program);
    glUniform1f(glGetUniformLocation(renderState.ribbon_program, "time"), time);

    glBindVertexArray(renderState.ribbon_vertex_array);
    glBindBuffer(GL_ARRAY_BUFFER, renderState.ribbon_vertex_buffer);
    // glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(renderState.ribbon_verts), renderState.ribbon_verts);

    glDrawElements(GL_TRIANGLES, INDEX_COUNT, GL_UNSIGNED_INT, 0);
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
