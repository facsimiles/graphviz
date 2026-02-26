/// @file
/// @brief Implementation of mutex.h backed by C++11 `std::mutex`

#include "config.h"

#include <cassert>
#include <mutex>
#include <new>
#include <util/mutex.h>

struct mutex {
  std::mutex impl;
};

mutex_t *gv_mutex_new(void) {
  try {
    return new mutex;
  } catch (std::bad_alloc &) {
    return nullptr;
  }
}

void gv_mutex_lock(mutex_t *m) {
  assert(m != nullptr);
  m->impl.lock();
}

void gv_mutex_unlock(mutex_t *m) {
  assert(m != nullptr);
  m->impl.unlock();
}

void gv_mutex_free(mutex_t *m) { delete m; }
