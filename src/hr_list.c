#include "hr_list.h"
#include "animation.h"

void init_horizontal_list(HorizontalList *hr_list) {}

void update_horizontal_list(HorizontalList *hr_list, float current_time) {
    float offset = 150;

    AnimatedProperty anim;
    anim.duration = 100;
    anim.start_time = current_time;
    anim.target = hr_list->selected * offset;
    if (hr_list->depth > 0) {
        anim.target += 50;
    }

    anim.subject = &hr_list->scroll;

    gfx_animation_push(&anim);
}
