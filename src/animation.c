#include "animation.h"

animation_t anim_st = {0};

bool gfx_animation_push(AnimatedProperty *entry) {
    animation_t *p_anim = &anim_st;
    struct tween t = {
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

float smoothStep(float current, float target, float dt, float speed) {
    float direction = (target > current) ? 1.0f : -1.0f;
    current += direction * speed * dt;
    return direction * current > direction * target ? target : current;
}

void gfx_animation_update(float current_time) {
    animation_t *p_anim = &anim_st;

    for (int i = 0; i < ARR_LEN(p_anim->list); ++i) {
        struct tween *tween = &p_anim->list[i];

        float elapsed_time = current_time - tween->start_time;
        tween->running_since += elapsed_time;

        float t = elapsed_time / tween->duration;
        *tween->subject = smoothStep(*tween->subject, tween->target_value, t, 800);

        if (tween->running_since >= tween->duration) {
            *tween->subject = tween->target_value;

            ARR_REMOVE(p_anim->list, i);
            i--;
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
