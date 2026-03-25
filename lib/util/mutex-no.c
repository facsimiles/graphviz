/// @file
/// @brief No-op implementation of mutex.h for single-threaded environments

#include "config.h"

#include <util/mutex.h>
#include <util/unused.h>

/// a phantom type to pretend our mutex is not nothing
struct mutex {
  int unused;
};

/// a placeholder we pass around, as our mutexes do not need to be distinct
static mutex_t mutex;

mutex_t *gv_mutex_new(void) { return &mutex; }

void gv_mutex_lock(UNUSED mutex_t *m) {}

void gv_mutex_unlock(UNUSED mutex_t *m) {}

void gv_mutex_free(UNUSED mutex_t *m) {}
