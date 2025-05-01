#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"

int
main(int argc, char *argv[])
{
 if(argc != 3){
     printf(2, "Usage: symlink <oldpath> <newpath>\n");
        exit();
    }

    if(symlink(argv[1], argv[2]) < 0){
        printf(2, "symlink: cannot create symlink %s\n", argv[2]);
        exit();
    }

    exit();
}