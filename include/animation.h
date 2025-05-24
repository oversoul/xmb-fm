#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    DialogTag,
    OptionListTag,
    VerticalListTag,
    HorizontalListTag,
} AnimationTag;

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

void animation_push(float duration, float target, float *subject, AnimationTag tag);
void animation_update(float current_time);
void animation_clean();
void animation_remove_by_tag(AnimationTag tag);
