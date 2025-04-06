#include "animation.h"
#include <math.h>
#include <stdio.h>

animation_t anim_st = {0};

bool gfx_animation_push(AnimatedProperty *entry) {
    animation_t *p_anim = &anim_st;

    tween_t t = {
        .subject = entry->subject,
        .duration = entry->duration,
        .target_value = entry->target,
        .initial_value = *entry->subject,
        .start_time = entry->start_time,
        .running_since = 0,
    };

    ARR_PUSH(p_anim->list, t);
    return true;
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

void gfx_animation_update(float current_time) {
    animation_t *p_anim = &anim_st;

    printf("animations: %zu\n", ARR_LEN(p_anim->list));
    for (int i = 0; i < ARR_LEN(p_anim->list); ++i) {
        tween_t *tween = &p_anim->list[i];

        float elapsed = current_time - tween->start_time;
        tween->running_since = elapsed;

        float t = tween->running_since / tween->duration;
        if (t >= 1.0f) {
            *tween->subject = tween->target_value;
            ARR_REMOVE(p_anim->list, i);
            i--;
        } else {
            float eased = ease(t);
            // float eased = easeInOutExpo(t);
            *tween->subject = tween->initial_value + (tween->target_value - tween->initial_value) * eased;
        }
    }
}

void gfx_animation_clean() {
    animation_t *p_anim = &anim_st;
    if (!p_anim)
        return;
    ARR_FREE(p_anim->list);
    memset(p_anim, 0, sizeof(*p_anim));
}
