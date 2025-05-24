#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <util/gv_find_me.h>

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

#ifdef __FreeBSD__
#include <sys/sysctl.h>
#include <sys/types.h>
#endif

#if defined(_WIN32) && !defined(__CYGWIN__)
#include <windows.h>
#endif

#if !defined(_WIN32)
#include <unistd.h>
#endif

#ifndef _WIN32
/// `readlink`-alike but dynamically allocates
static char *readln(const char *path) {

  char *resolved = NULL;
  size_t size = 0;

  while (true) {

    // expand target buffer
    {
      char *const r = realloc(resolved, size == 0 ? 1024 : (size * 2));
      if (r == NULL) {
        // failed, out-of-memory
        free(resolved);
        return NULL;
      }
      resolved = r;
      size = size == 0 ? 1024 : (size * 2);
    }

    // attempt to resolve
    {
      const ssize_t written = readlink(path, resolved, size);
      if (written < 0) {
        break;
      }
      if ((size_t)written < size) {
        // success
        resolved[written] = '\0';
        return resolved;
      }
    }
  }

  // failed
  free(resolved);
  return NULL;
}
#endif

char *gv_find_me(void) {

  // macOS
#ifdef __APPLE__
  {
    // determine how many bytes we will need to allocate
    uint32_t buf_size = 0;
    const int rc = _NSGetExecutablePath(NULL, &buf_size);
    assert(rc != 0);
    assert(buf_size > 0);

    char *path = calloc(1, buf_size);
    if (path == NULL) {
      // failed, out-of-memory
      return NULL;
    }

    // retrieve the actual path
    if (_NSGetExecutablePath(path, &buf_size) < 0) {
      free(path);
      return NULL;
    }

    // try to resolve any levels of symlinks if possible
    while (true) {
      char *const buf = readln(path);
      if (buf == NULL)
        return path;

      free(path);
      path = buf;
    }
  }
#elif defined(_WIN32)
  {
    char *path = NULL;
    DWORD path_size = 0;
    int rc = 0;

    do {
      {
        const DWORD size = path_size == 0 ? 1024 : (path_size * 2);
        char *const p = realloc(path, size);
        if (p == NULL) {
          // failed, out-of-memory
          free(path);
          return NULL;
        }
        path = p;
        path_size = size;
      }

      rc = GetModuleFileNameA(NULL, path, path_size);
      if (rc == 0) {
        free(path);
        return NULL;
      }

    } while ((DWORD)rc == path_size);

    return path;
  }
#else

  // Linux, Cygwin
  char *path = readln("/proc/self/exe");
  if (path != NULL)
    return path;

  // DragonFly BSD, FreeBSD
  path = readln("/proc/curproc/file");
  if (path != NULL)
    return path;

  // NetBSD
  path = readln("/proc/curproc/exe");
  if (path != NULL)
    return path;

// /proc-less FreeBSD
#ifdef __FreeBSD__
  {
    int mib[] = {CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, -1};
    static const size_t MIB_LENGTH = sizeof(mib) / sizeof(mib[0]);

    do {
      // determine how long the path is
      size_t buf_size = 0;
      if (sysctl(mib, MIB_LENGTH, NULL, &buf_size, NULL, 0) < 0) {
        break;
      }
      assert(buf_size > 0);

      // make enough space for the target path
      char *buf = calloc(1, buf_size);
      if (buf == NULL) {
        // failed, out-of-memory
        return NULL;
      }

      // resolve it
      if (sysctl(mib, MIB_LENGTH, buf, &buf_size, NULL, 0) == 0) {
        return buf;
      }
      free(buf);
    } while (0);
  }
#endif
#endif

  return NULL;
}
