#pragma once

#include <stddef.h>
#include <util/api.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mt_stack mt_stack_t;

UTIL_API mt_stack_t *gv_mt_stack_new(size_t threads);

UTIL_API void gv_mt_stack_push(mt_stack_t *stack, size_t thread_id, void *item);

UTIL_API void *gv_mt_stack_pop(mt_stack_t *stack, size_t thread_id);

UTIL_API void gv_mt_stack_free(mt_stack_t *stack);

#ifdef __cplusplus
}
#endif
