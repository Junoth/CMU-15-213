#include<stdio.h>
#include <unistd.h>

int main(int argc, char *argv[], char *envp[]) {
  execve("list", argv, envp);
}
