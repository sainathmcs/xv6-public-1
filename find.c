#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"

// formats the file name.
char* fmtname(char *path) {
  static char buf[DIRSIZ+1];
  char *p;

  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  if(strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  memset(buf+strlen(p), ' ', DIRSIZ-strlen(p));
  return buf;
}

// search function is used to serch all the files in  the path
void search(char *path, char *name, int type, int inum, int printi, int inumSpecified, int inumComparisonType) {
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;

    // handling directory open error.
    if((fd = open(path, 0)) < 0) {
        printf(2, "find: cannot open %s\n", path);
        return;
    }

    // traversing through the directory
    while((read(fd, &de, sizeof(de))) == sizeof(de)){
        // skip if its an empty directory or if its a . or .. file
        if(de.inum == 0 || strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
            continue;
        // if the path is too long  break and print the error.
        if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
            printf(1, "find: path too long\n");
            break;
        }

        //constructing the full path of the file/directory
        memmove(buf, path, strlen(path));
        p = buf + strlen(path);
        *p++ = '/';
        memmove(p, de.name, DIRSIZ);
        p[DIRSIZ] = 0;

        //get the file status
        if(stat(buf, &st) < 0){
            printf(1, "find: cannot stat %s\n", buf);
            continue;
        }

        // Debug: Print inode number1, "Debug: Inode of %s is %d\n", buf, st.ino);

        // verifying if the file we are at is what we need and printing the path accprdingly
        
        if((strcmp(name, "") == 0 || strcmp(de.name, name) == 0) && 
           (type == -1 || st.type == type) &&
           (!inumSpecified || (inumComparisonType == 0 && st.ino == inum) ||
             (inumComparisonType == 1 && st.ino > inum) ||
             (inumComparisonType == -1 && st.ino < inum))) {
                //printf(1, "inside f");
            if (printi)// print inumber if  its a printi
                printf(1, "%d ", st.ino);
            printf(1, "%s\n", buf);
            }
        

        // this if handles if the entry is a directory and since we need to recursively search with in the directory.
        if(st.type == T_DIR) {
            search(buf, name, type, inum, printi, inumSpecified, inumComparisonType);
        }
    }
    close(fd);
}






int main(int argc, char *argv[]) {
    char *path, *name = "";
    int type = -1, inum = 0, printi = 0, inumSpecified = 0;
    int inumComparisonType = 0;

    path = argv[1]; //first argument after fing is the search path

    // pasing the conditions and setting flags accordingly
    for(int i = 2; i < argc; i++) {
        if(strcmp(argv[i], "-name") == 0) {
            name = argv[++i];
        } else if(strcmp(argv[i], "-type") == 0) {
            i++;
            if(argv[i][0] == 'f')
                type = T_FILE;
            else if(argv[i][0] == 'd')
                type = T_DIR;
        }  else if(strcmp(argv[i], "-printi") == 0) {
            printi = 1;
        }
        else if(strcmp(argv[i], "-inum") == 0) {
    char* inumArg = argv[++i];
    if(inumArg[0] == '+') {
        inum = atoi(inumArg + 1);
        inumComparisonType = 1;
    } else if(inumArg[0] == '-') {
        inum = atoi(inumArg + 1);
        inumComparisonType = -1;
    } else {
        inum = atoi(inumArg);
        inumComparisonType = 0;
    }
    inumSpecified = 1; // flag indicating that the inode number is mentioned.
}
    }

    // executing  search on specified path and flags.
    search(path, name, type, inum, printi, inumSpecified, inumComparisonType);
    exit();
}
