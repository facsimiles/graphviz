#pragma once

#include <stddef.h>
#include <util/api.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct thread_pool thread_pool_t;

UTIL_API thread_pool_t *
gv_thread_pool_new(size_t threads, int (*entry)(size_t thread_id, void *state),
                   void *state);

UTIL_API void gv_thread_pool_start(thread_pool_t *pool);

UTIL_API int gv_thread_pool_join(thread_pool_t *pool);

UTIL_API void gv_thread_pool_free(thread_pool_t *pool);

#ifdef __cplusplus
}
#endif
