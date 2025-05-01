#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "fcntl.h"

int
main(int argc, char *argv[])
{
  int fd, offset;
  char* buf;

    if(argc != 4){
        printf(2, "Usage: lseek <file> <offset> <text>\n");
        exit();
    }

    if((fd = open(argv[1], O_CREATE | O_RDWR)) < 0){
        printf(2, "lseek: cannot open %s\n", argv[1]);
        exit();
    }

    offset = atoi(argv[2]);

    if(lseek(fd, offset) < 0){
        printf(2, "lseek: cannot seek %s\n", argv[1]);
        close(fd);
        exit();
    }

    buf = argv[3];

    if(write(fd, buf, strlen(buf)) != strlen(buf)){
        printf(2, "lseek: write error\n");
        close(fd);
        exit();
    }

    close(fd);
    exit();
}
