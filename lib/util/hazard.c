#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <util/hazard.h>

// if atomics are unavailable, we simply fail all operations

#ifndef __STDC_NO_ATOMICS__
#include <stdatomic.h>
#endif

struct hazard_set {
  size_t size;
#ifndef __STDC_NO_ATOMICS__
  void *_Atomic *hazarded;
  void *_Atomic *deferred;
#endif
  void (*free)(void *);
};

#ifndef __STDC_NO_ATOMICS__
static void *load(void *_Atomic *src) {
  return atomic_load_explicit(src, memory_order_acquire);
}

static void store(void *_Atomic *dst, void *src) {
  atomic_store_explicit(dst, src, memory_order_release);
}

static bool cas(void *_Atomic *dst, void *expected, void *desired) {
  return atomic_compare_exchange_strong_explicit(
      dst, &expected, desired, memory_order_acq_rel, memory_order_acquire);
}
#endif

hazard_set_t *gv_hazard_set_new(size_t size, void (*freefn)(void *)) {
  assert(size > 0);

#ifdef __STDC_NO_ATOMICS__
  return NULL;
  (void)size;
  (void)freefn;
#else

  hazard_set_t *const set = calloc(1, sizeof(*set));
  if (set == NULL) {
    goto fail;
  }

  set->size = size;
  set->free = freefn == NULL ? free : freefn;

  set->hazarded = calloc(size, sizeof(set->hazarded[0]));
  if (set->hazarded == NULL) {
    goto fail;
  }

  set->deferred = calloc(size * (size - 1), sizeof(set->deferred[0]));
  if (size > 1 && set->deferred == NULL) {
    goto fail;
  }

  return set;

fail:
  gv_hazard_set_free(set);
  return NULL;
#endif
}

void gv_hazard(hazard_set_t *set, size_t index, void *target) {
  assert(set != NULL);
  assert(index < set->size);
  assert(target != NULL);

#ifdef __STDC_NO_ATOMICS__
  (void)set;
  (void)index;
  (void)target;
#else
  // only a single pointer per-slot can be hazarded
  assert(load(&set->hazarded[index]) == NULL &&
         "hazarding multiple pointers at once");

  store(&set->hazarded[index], target);
#endif
}

void gv_unhazard(hazard_set_t *set, size_t index, void *target) {
  assert(set != NULL);
  assert(index < set->size);
  assert(target != NULL);

#ifdef __STDC_NO_ATOMICS__
  (void)set;
  (void)index;
  (void)target;
#else
  assert(load(&set->hazarded[index]) != NULL &&
         "unhazarding a pointer when none are hazarded");

  assert(load(&set->hazarded[index]) == target &&
         "unhazarding a pointer that differs from the one hazarded");
  (void)target;

  store(&set->hazarded[index], NULL);
#endif
}

void gv_reclaim(hazard_set_t *set, size_t index, void *target) {
  assert(set != NULL);
  assert(index < set->size);

#ifdef __STDC_NO_ATOMICS__
  (void)set;
  (void)index;
  (void)target;
#else

  // the reclaimer is not allowed to be freeing something while also holding a
  // hazarded pointer
  assert(load(&set->hazarded[index]) == NULL &&
         "reclaiming a pointer while holding a hazarded pointer");

  // first try to free any previously deferred pointers
  void *_Atomic *const deferred = &set->deferred[index * (set->size - 1)];
  for (size_t i = 0; i < set->size - 1; ++i) {
    void *const to_free = load(&deferred[i]);
    if (to_free == NULL) {
      continue;
    }
    bool conflict = false;
    for (size_t j = 0; j < set->size; ++j) {
      if (j == index) {
        // no need to check for conflicts with ourself
        assert(load(&set->hazarded[j]) == NULL);
        continue;
      }
      if (to_free == load(&set->hazarded[j])) {
        // this pointer is in use by slot j
        conflict = true;
        break;
      }
    }
    if (conflict) {
      continue;
    }
    {
      const bool rc = cas(&deferred[i], to_free, NULL);
      // no other thread should be accessing this slot, so the CAS should always
      // succeed
      assert(rc && "racing free on deferred hazard pointers");
      (void)rc;
    }
    set->free(to_free);
  }

  // Now deal with the pointer we were passed. The most likely case is that no
  // one else is using this pointer, so try this first.
  {
    bool conflict = false;
    for (size_t i = 0; i < set->size; ++i) {
      if (i == index) {
        // no need to check for conflicts with ourself
        assert(load(&set->hazarded[i]) == NULL);
        continue;
      }
      if (target == load(&set->hazarded[i])) {
        conflict = true;
        break;
      }
    }
    if (!conflict) {
      set->free(target);
      return;
    }
  }

  // if we reached here, we need to defer this reclamation to later
  for (size_t i = 0; i < set->size - 1; ++i) {
    if (cas(&deferred[i], NULL, target)) {
      return;
    }
  }

  assert(0 && "deferred more than `set->size - 1` reclamations");
#endif
}

void gv_hazard_set_free(hazard_set_t *set) {
  if (set != NULL) {
#ifndef __STDC_NO_ATOMICS__
    // all pointers should have been unhazarded prior to freeing
    for (size_t i = 0; set->hazarded != NULL && i < set->size; ++i) {
      assert(load(&set->hazarded[i]) == NULL &&
             "freeing hazard set while still holding hazarded pointers");
    }

    // run any remaining deferred frees
    for (size_t i = 0; set->deferred != NULL && i < set->size * (set->size - 1);
         ++i) {
      set->free(load(&set->deferred[i]));
    }

    free(set->deferred);
    free(set->hazarded);
#endif
  }
  free(set);
}
