#include "animation.h"
#include <math.h>
#include <stdio.h>

animation_t anim_st = {0};

void gfx_animation_push(AnimatedProperty *entry, AnimationTag tag) {
    animation_t *p_anim = &anim_st;

    tween_t t = {
        .tag = tag,
        .subject = entry->subject,
        .duration = entry->duration,
        .target_value = entry->target,
        .initial_value = *entry->subject,
        .start_time = entry->start_time,
        .running_since = 0,
    };

    // init animation list
    if (p_anim->list == NULL) {
        if (p_anim->capacity)
            p_anim->capacity = 10;

        p_anim->list = malloc(p_anim->capacity * sizeof(tween_t));
        p_anim->size = 0;
    }

    // add animation
    if (p_anim->size >= p_anim->capacity) {
        p_anim->capacity = p_anim->capacity > 0 ? p_anim->capacity * 2 : 1;
        p_anim->list = realloc(p_anim->list, p_anim->capacity * sizeof(tween_t));
    }

    p_anim->list[p_anim->size++] = t;
}

float easeOutBounce(float x) {
    float n1 = 7.5625;
    float d1 = 2.75;
    if (x < 1 / d1) {
        return n1 * x * x;
    } else if (x < 2 / d1) {
        float v = x - 1.5 / d1;
        return n1 * v * x + 0.75;
    } else if (x < 2.5 / d1) {
        float v = x - 2.25 / d1;
        return n1 * v * x + 0.9375;
    } else {
        float v = x - 2.625 / d1;
        return n1 * v * x + 0.984375;
    }
}

float easeInOutExpo(float x) {
    return x == 0 ? 0 : x == 1 ? 1 : x < 0.5 ? powf(2.0, 20.0 * x - 10) / 2.0 : (2 - powf(2, -20 * x + 10)) / 2.0;
}

float ease(float x) { return x * x * (3 - 2 * x); }

void gfx_remove_animation(animation_t *p_anim, size_t i) {
    if (i < 0 || i >= p_anim->size)
        return;

    for (int idx = i; idx < p_anim->size - 1; idx++) {
        p_anim->list[idx] = p_anim->list[idx + 1]; // Shift elements left
    }

    (p_anim->size)--;
}

void gfx_animation_update(float current_time) {
    animation_t *p_anim = &anim_st;

    // printf("animations: %zu\n", p_anim->size);
    for (int i = 0; i < p_anim->size; ++i) {
        tween_t *tween = &p_anim->list[i];

        float elapsed = current_time - tween->start_time;
        tween->running_since = elapsed;

        float t = tween->running_since / tween->duration;
        if (t >= 1.0f) {
            *tween->subject = tween->target_value;
            gfx_remove_animation(p_anim, i);
            i--;
        } else {
            float eased = ease(t);
            // float eased = easeInOutExpo(t);
            *tween->subject = tween->initial_value + (tween->target_value - tween->initial_value) * eased;
        }
    }
}

void gfx_animation_remove_by_tag(AnimationTag tag) {
    animation_t *p_anim = &anim_st;

    for (int i = 0; i < p_anim->size; ++i) {
        tween_t *tween = &p_anim->list[i];

        if (tween->tag == tag) {
            gfx_remove_animation(p_anim, i);
            i--;
        }
    }
}

void gfx_animation_clean() {
    animation_t *p_anim = &anim_st;
    if (!p_anim)
        return;
    // clean animation
    free(p_anim->list);
    memset(p_anim, 0, sizeof(*p_anim));
}
