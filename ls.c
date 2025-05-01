#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"

char*
fmtname(char *path)
{
  static char buffer[DIRSIZ+1];
  char *p;

  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  // Return blank-padded name.
  if(strlen(p) >= DIRSIZ)
    return p;
  memmove(buffer, p, strlen(p));
  memset(buffer+strlen(p), ' ', DIRSIZ-strlen(p));
  return buffer;
}

// checkHiddenFile function checks if the file or directory is hidden.
int checkHiddenFile(const char *name){
  return name[0] == '.';
}

void
ls(char *path,int allowHidden)
{
  char buffer[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  if((fd = open(path, 0)) < 0){
    printf(2, "ls: cannot open %s\n", path);
    return;
  }

  if(fstat(fd, &st) < 0){
    printf(2, "ls: cannot stat %s\n", path);
    close(fd);
    return;
  }

  switch(st.type){
  case T_FILE:
    printf(1, "%s %d %d %d\n", fmtname(path), st.type, st.ino, st.size);
    break;

  case T_DIR:
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buffer){
      printf(1, "ls: path too long\n");
      break;
    }
    strcpy(buffer, path);
    p = buffer+strlen(buffer);
    *p++ = '/';
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
      if(de.inum == 0 || (!allowHidden && checkHiddenFile(de.name))) // check if the entry is valid or not  and checks if the hidden flag is 0 and if the file or directory is hidden or not
        continue;
      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;
      if(stat(buffer, &st) < 0){
        printf(1, "ls: cannot stat %s\n", buffer);
        continue;
      }
      // this if and else is to add a / if we are displaying a directory.
      if(st.type == T_DIR)
        printf(1, "%s/ %d %d %d\n", fmtname(buffer), st.type, st.ino, st.size);
      else
        printf(1, "%s %d %d %d\n", fmtname(buffer), st.type, st.ino, st.size);
    }
    break;
  }
  close(fd);
}

int
main(int argc, char *argv[])
{
  int i;
  int allowHidden = 0; // Flag to check the status of hidden files and directories.
  // parsing  and processing the command line arguments.
    for(i = 1; i < argc; i++) {
    // change the status of the allowHidden if -a is parsed.
    if(strcmp(argv[i], "-a") == 0) {
      allowHidden = 1;
      // Remove the '-a' argument from the list to make sure that the xv6 does not treat -a as directory of a file.
      for(int j = i; j < argc - 1; j++) {
        argv[j] = argv[j + 1];
      }
      argc--; //adjusting the argument count.
      i--;
    }
  }

  // default condition: if the no argument is given then it opens the current directory and list the files accordingly.
  if(argc == 1) {
    ls(".", allowHidden);
    exit();
  }

  // if a directory is given then we handle that case in this loop.
  for(i = 1; i < argc; i++) {
    ls(argv[i], allowHidden);
  }

  exit();
}
