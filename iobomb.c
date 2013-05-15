/* gcc iobomb.c -o libiobomb.so -shared -fPIC -lpthread */

#include <assert.h>
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>


static ssize_t (*iobomb_read) (int fildes, void *buf, size_t nbyte);
static ssize_t (*iobomb_write) (int fildes, const void *buf, size_t nbyte);


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

   iobomb_read = dlsym(lib, "read");
   assert(iobomb_read);

   iobomb_write = dlsym(lib, "write");
   assert(iobomb_write);
}


ssize_t
read (int fildes,
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
      return iobomb_read(fildes, buf, nbyte ? 1 : 0);
   case 3:
      return iobomb_read(fildes, buf, nbyte);
   }

   return iobomb_read(fildes, buf, nbyte);
}


ssize_t
write (int fildes,
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
      return iobomb_write(fildes, buf, nbyte ? 1 : 0);
   case 3:
      return iobomb_write(fildes, buf, nbyte);
   }

   return iobomb_write(fildes, buf, nbyte);
}
