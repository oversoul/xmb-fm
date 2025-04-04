#pragma once

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#define MAX(a, b) ((a > b) ? a : b)
#define ARR__HDR(b) (((struct rbuf__hdr *)(b)) - 1)

#define ARR_LEN(b) ((b) ? ARR__HDR(b)->len : 0)
#define ARR_CAP(b) ((b) ? ARR__HDR(b)->cap : 0)
#define ARR_END(b) ((b) + ARR_LEN(b))
#define ARR_SIZEOF(b) ((b) ? ARR_LEN(b) * sizeof(*b) : 0)

#define ARR_FREE(b) ((b) ? (free(ARR__HDR(b)), (b) = NULL) : 0)
#define ARR_FIT(b, n) ((size_t)(n) <= ARR_CAP(b) ? 0 : (*(void **)(&(b)) = rbuf__grow((b), (n), sizeof(*(b)))))
#define ARR_PUSH(b, val) (ARR_FIT((b), 1 + ARR_LEN(b)), (b)[ARR__HDR(b)->len++] = (val))
#define ARR_POP(b) (b)[--ARR__HDR(b)->len]
#define ARR_RESIZE(b, sz) (ARR_FIT((b), (sz)), ((b) ? ARR__HDR(b)->len = (sz) : 0))
#define ARR_CLEAR(b) ((b) ? ARR__HDR(b)->len = 0 : 0)
#define ARR_TRYFIT(b, n) (ARR_FIT((b), (n)), (((b) && ARR_CAP(b) >= (size_t)(n)) || !(n)))
#define ARR_REMOVE(b, idx) memmove((b) + (idx), (b) + (idx) + 1, (--ARR__HDR(b)->len - (idx)) * sizeof(*(b)))

struct rbuf__hdr {
    size_t len;
    size_t cap;
};

static void *rbuf__grow(void *buf, size_t new_len, size_t elem_size) {
    struct rbuf__hdr *new_hdr;
    size_t new_cap = MAX(2 * ARR_CAP(buf), MAX(new_len, 16));
    size_t new_size = sizeof(struct rbuf__hdr) + new_cap * elem_size;
    if (buf) {
        new_hdr = (struct rbuf__hdr *)realloc(ARR__HDR(buf), new_size);
        if (!new_hdr)
            return buf; /* out of memory, return unchanged */
    } else {
        new_hdr = (struct rbuf__hdr *)malloc(new_size);
        if (!new_hdr)
            return NULL; /* out of memory */
        new_hdr->len = 0;
    }
    new_hdr->cap = new_cap;
    return new_hdr + 1;
}

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

    float start_time;
    float running_since;
};

struct animations {
    struct tween *list;
};

typedef struct tween tween_t;
typedef struct animations animation_t;

bool gfx_animation_push(AnimatedProperty *entry);
void gfx_animation_update(float current_time);
void gfx_animation_clean();
