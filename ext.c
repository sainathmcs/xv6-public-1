#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "fcntl.h"


int
main(int argc, char *argv[])
{
 if(argc != 4){
     printf(2, "Usage: ext <file> <size> <char to write>\n");
        exit();
    }

    int fd, size, i;
    char buf[512];

    if((fd = open(argv[1], O_CREATE | O_RDWR | O_EXTENT)) < 0){
        printf(2, "ext: cannot open %s\n", argv[1]);
        exit();
    }

    size = atoi(argv[2]);
    
    memset(buf, argv[3][0], 512);

    for(i = 0; i < size; i++){
        if(write(fd, buf, 512) != 512){
            printf(2, "ext: write error after writing %d blocks\n", i*512);
            close(fd);
            exit();
        }
    }

    close(fd);

    exit();

}
