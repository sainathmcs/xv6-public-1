#include "types.h"
#include "user.h"
#include "fcntl.h"
#include "stat.h"

int
main(int argc, char *argv[])
{
  if(argc != 2){
    printf(2, "Usage: stat <file>\n");
    exit();
    }
    char *file = argv[1];
    struct stat st;
    
    if(stat(file, &st) < 0){
        printf(2, "stat: cannot stat %s\n", file);
        exit();
    }

    printf(1, "F Stat\n");
    printf(1, "type: %d\n", st.type);
    printf(1, "dev: %d\n", st.dev);
    printf(1, "ino: %d\n", st.ino);
    printf(1, "nlink: %d\n", st.nlink);
    printf(1, "size: %d\n", st.size);

    for(int i = 0; i < 10; i++)
    {
        printf(1, "Blocks[%d]: addrs: %d range: %d\n", i, st.addrs[i], st.range[i]);
    }
    
    exit();
}