#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <math.h>
#include <errno.h>
#include "helper.h"

void pfact(n){
  int fd[2];
  int factor[2] = {0,0}; //an array that will store the factors
  int pipe_result;
  int exit_status;
  int fork_result;
  /* check if pipe was created properly*/ 
  if((pipe_result = pipe(fd)) == -1){
    perror("pipe failed to execute properly");
    exit(1);
  }
  
  /*Sigpipe code snippet provided in the assignment page*/
  if(signal(SIGPIPE, SIG_IGN) ==SIG_ERR) {
    perror("signal");
    exit(1);
  }
  
  /*check if fork was created properly*/
  if((fork_result = fork()) == -1){
    perror("fork failed to execute proeprly");
    exit(1);
  }
  else if(fork_result > 0){
    /* in the master parent process*/
    int i;
    close(fd[0]); //close the read end of the pipe as it won't be reading anything from pipe
    for(i = 2; i<= n; i++){
      /*write the integers from 2 to n to the pipe*/
      if(write(fd[1], &i, sizeof(int)) == -1 ){
	/*if writing to pipe causes an error close the 
	  write end of pipe and break the loop*/
	close(fd[1]);
	i = n+1;
      }      
    } 

    /* get the exit status of its children process, 
       and print it */
    int child_status;
    if(wait(&child_status)!= -1){
      if(WIFEXITED(child_status)){
	exit_status = (WEXITSTATUS(child_status));
      }
    }     
    printf("Number of stages = %d\n", exit_status);
  }
  else{
    /*in the child process, so call the helper function child_process 
      that does the necessary work for the child process*/
    child_process(n, fd, factor);
  }

}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Error: pfact requires 2 arguments: pfact <n>\n");
    return -1;
  }
  int n = strtol(argv[1], (char **)NULL, 10);
  pfact(n);
  return 0;
}
