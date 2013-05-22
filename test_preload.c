#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#define FAIL_RES(_res, _pre) \
   do { \
      if (_res < 0) { \
         perror(NULL); \
         _pre \
         _exit(1); \
      } \
   } while (0)


struct testPreloadReadData {
   char *fileName;
};


int
testPreloadRead(void *data)
{
   struct testPreloadReadData *tprd = (struct testPreloadReadData *) data;
   int res, fd;
   char buf[255] = {0};

   fd = open(tprd->fileName, O_RDONLY);
   FAIL_RES(fd, {});
   res = read(fd, buf, sizeof buf); 
   FAIL_RES(res, { close(fd); });
   close(fd);
}


int
main(int argc, char **argv)
{
   struct testPreloadReadData data;

   data.fileName = "data.in";
   testPreloadRead(&data);
}
