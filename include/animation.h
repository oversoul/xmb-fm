#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    OptionListTag,
    VerticalListTag,
    HorizontalListTag,
} AnimationTag;

typedef struct {
    float target;
    float start_time;
    float duration;
    float *subject;
} AnimatedProperty;

struct tween {
    float duration;
    float initial_value;
    float target_value;
    float *subject;
    AnimationTag tag;

    float start_time;
    float running_since;
};

struct animations {
    struct tween *list;
    size_t size;
    size_t capacity;
};

typedef struct tween tween_t;
typedef struct animations animation_t;

void gfx_animation_push(AnimatedProperty *entry, AnimationTag tag);
void gfx_animation_update(float current_time);
void gfx_animation_clean();
void gfx_animation_remove_by_tag(AnimationTag tag);
