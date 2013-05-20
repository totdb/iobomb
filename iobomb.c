/* gcc iobomb.c -o libiobomb.so -shared -fPIC -ldl */ 
#include <assert.h>
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/uio.h>
#include <string.h>


/**
 * Define the functions you want to override here. The functions will be
 * generated using the CPP, as well as the code to enable/disable the override
 * at runtime.  The overridden method implementation (provided by you), should
 * be implemented as iobomb_perturb_(functionname)
 */
#define IOBOMB_OVERRIDES \
   _ACTION(ssize_t, read, int fildes, void *buf, size_t n_byte) \
   _ACTION(ssize_t, write, int fildes, const void *buf, size_t n_byte) \
   _ACTION(ssize_t, readv, int fildes, const struct iovec *iov, int iovcnt) \
   _ACTION(ssize_t, writev, int fildes, const struct iovec *iov, int iovcnt) \
   _ACTION(ssize_t, recv, int sockfd, const void *buf, size_t len, int flags) \
   _ACTION(ssize_t, send, int sockfd, const void *buf, size_t len, int flags) 


/**
 * original function pointer declarations
 *
 */
#define _ACTION(_ret, _func, ...) \
   static _ret (*iobomb_orig_##_func) (__VA_ARGS__);
IOBOMB_OVERRIDES
#undef _ACTION

/**
 * Perturbing function forward declarations
 */
#define _ACTION(_ret, _func, ...) \
   static _ret iobomb_perturb_##_func (__VA_ARGS__);
IOBOMB_OVERRIDES
#undef _ACTION


/**
 * Function pointer declaration, points at original or perturb version
 *
 */
#define _ACTION(_ret, _func, ...) \
   static _ret (*iobomb_current_##_func) (__VA_ARGS__);
IOBOMB_OVERRIDES
#undef _ACTION

/**
 * Implementation section
 *
 * Builds the actual method definitions use to override pre-existing calls
 *
 */

#define _ACTION(_ret, _func, ...) \
_ret _func(__VA_ARGS__)\
{\
   void *args = __builtin_apply_args(); \
   size_t size = 10 * sizeof (void *); \
   void *ret = __builtin_apply((void*)iobomb_current_##_func, args, size); \
   __builtin_return(ret); \
}
IOBOMB_OVERRIDES
#undef _ACTION

static void
iobomb_enable(const char *name, int enable)
{
#define _ACTION(_ret, _func, ...) \
   do { \
      if (strcmp(name, #_func) == 0) { \
         iobomb_current_##_func = enable ? iobomb_perturb_##_func : iobomb_orig_##_func; \
         return; \
      } \
   } while(0);
IOBOMB_OVERRIDES
#undef _ACTION
}

#define IOBOMB_CONFIG ".iobombrc"

static void
iobomb_read_config()
{
}


__attribute__((constructor))
static void
iobomb_init (void)
{
   void *lib;

#if __APPLE__
   lib = dlopen("/usr/lib/libc.dylib", RTLD_LAZY);
#else
   lib = dlopen("/lib64/libc.so.6", RTLD_LAZY);
#endif
   if (!lib) {
      lib = dlopen("libc.so.6", RTLD_LAZY);
   }
   assert(lib);

#  define _ACTION(_ret, _func, ...) \
   iobomb_orig_##_func = dlsym(lib, #_func); \
   assert(iobomb_orig_##_func); \
   iobomb_current_##_func = iobomb_orig_##_func;
IOBOMB_OVERRIDES
#  undef _ACTION

#  define _ACTION(_ret, _func, ...) \
   iobomb_enable(#_func, 1);
IOBOMB_OVERRIDES
#  undef _ACTION

   iobomb_read_config();
}


static ssize_t
iobomb_perturb_read (int fildes,
                     void *buf,
                     size_t nbyte)
{
   static unsigned i;

   switch (__sync_fetch_and_add(&i, 1) % 4) {
   case 0:
      errno = EAGAIN;
      return -1;
   case 1:
      errno = EINTR;
      return -1;
   case 2:
      return iobomb_orig_read(fildes, buf, nbyte ? 1 : 0);
   case 3:
      return iobomb_orig_read(fildes, buf, nbyte);
   }

   return iobomb_orig_read(fildes, buf, nbyte);
}


static ssize_t
iobomb_perturb_write (int fildes,
                      const void *buf,
                      size_t nbyte)
{
   static unsigned i;

   switch (__sync_fetch_and_add(&i, 1) % 4) {
   case 0:
      errno = EAGAIN;
      return -1;
   case 1:
      errno = EINTR;
      return -1;
   case 2:
      return iobomb_orig_write(fildes, buf, nbyte ? 1 : 0);
   case 3:
      return iobomb_orig_write(fildes, buf, nbyte);
   }

   return iobomb_orig_write(fildes, buf, nbyte);
}


static ssize_t
iobomb_perturb_writev(int fildes,
                      const struct iovec *iov,
                      int iovcnt)
{
   static unsigned i;

   switch (__sync_fetch_and_add(&i, 1) % 4) {
   case 0:
      errno = EAGAIN;
      return -1;
   case 1:
      errno = EINTR;
      return -1;
   case 2:
      return iobomb_orig_writev(fildes, iov, iovcnt ? 1 : 0);
   case 3:
      return iobomb_orig_writev(fildes, iov, iovcnt);
   }

   return iobomb_orig_writev(fildes, iov, iovcnt);
}

static ssize_t
iobomb_perturb_readv(int fildes,
                     const struct iovec *iov,
                     int iovcnt)
{
   static unsigned i;

   switch (__sync_fetch_and_add(&i, 1) % 4) {
   case 0:
      errno = EAGAIN;
      return -1;
   case 1:
      errno = EINTR;
      return -1;
   case 2:
      return iobomb_orig_readv(fildes, iov, iovcnt ? 1 : 0);
   case 3:
      return iobomb_orig_readv(fildes, iov, iovcnt);
   }

   return iobomb_orig_readv(fildes, iov, iovcnt);
}


static ssize_t
iobomb_perturb_send(int sockfd,
                    const void *buf,
                    size_t len,
                    int flags)
{
   static unsigned i;

   switch (__sync_fetch_and_add(&i, 1) % 4) {
   case 0:
      errno = EAGAIN;
      return -1;
   case 1:
      errno = EINTR;
      return -1;
   case 2:
      return iobomb_orig_send(sockfd, buf, len ? 1 : 0, flags);
   case 3:
      return iobomb_orig_send(sockfd, buf, len, flags);
   }

   return iobomb_orig_send(sockfd, buf, len, flags);
}


static ssize_t
iobomb_perturb_recv(int sockfd,
                    const void *buf,
                    size_t len,
                    int flags)
{
   static unsigned i;

   switch (__sync_fetch_and_add(&i, 1) % 4) {
   case 0:
      errno = EAGAIN;
      return -1;
   case 1:
      errno = EINTR;
      return -1;
   case 2:
      return iobomb_orig_recv(sockfd, buf, len ? 1 : 0, flags);
   case 3:
      return iobomb_orig_recv(sockfd, buf, len, flags);
   }

   return iobomb_orig_recv(sockfd, buf, len, flags);
}
