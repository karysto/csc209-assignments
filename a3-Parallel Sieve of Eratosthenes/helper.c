#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <math.h>
#include <errno.h>
#include "helper.h"


/* the child process processes the numbers from 2 to n, 
and creates new child's of its own if the number it gets passed in 
is not a multiple of m , which is initially 2.
n = the input value n 
fd = the fd values piped in the beginning of the program
factor = an array to store the factors */
void  child_process(int n, int fd[2], int factor[2]){
  if(signal(SIGPIPE, SIG_IGN) ==SIG_ERR) {
    perror("signal");
    exit(1);
  }
  close(fd[1]); //the child process will not be writing to the original pipe
  int m; //store the first value of the passed in integers
  int value; //store the subsequent values one by one

  if((read(fd[0], &m, sizeof(int))) == -1){
    /*if there is an error reading from the pipe,
      close the pipe and exit returning an error*/
    close(fd[0]);
    perror("reading from pipe failed in child");
    exit(1);
  }
  
  /*check if m is one of the factors, and process it appropriately*/
  if((n % m) == 0){
    /*check if one of the factors has been already set, if it hasnt,
        check if m^2 = n or if m = n. If one of the factors has already
        been set, then check if that factor times m equals n.
        Print the appropriate messages when required*/
    if (factor[0] == 0){
      factor[0] = m;
      if ((m * m) == n){
        printf("%d %d %d\n", n, factor[0], factor[0]);
	exit(1);
      }
      else if(n == m){
	printf("%d is prime\n",n);
	exit(1);
      }
    }
    else {
      if((factor[0] * m) == n){
        printf("%d %d %d\n", n, factor[0], m);
	exit(1);
      }
      else{
        printf("%d is not the product of two primes\n", n);
	exit(1);
      }
    }
  }

  int read_output;
  /* a while loop that runs until there is nothing more to read from the pipe
     or until an error is encountered. On each iteration, it checkes if the 
     next value is a multiple of m, if so it simply goes on to read the next
     value. If it is not a multiple of m, it forks and creates a new pipe. */ 
  while((read_output =(read(fd[0], &value, sizeof(int)))) > 0){
    /* This if statement checks if its a multiple of m or if the read value is 
       greater then sqrt(n)*/
    if(((value % m) == 0) || value > (int)sqrt(n)){
      /*checks if the read value is multiple of m, if it is perform 
	the necessary actions.*/
      if((n % value) == 0){
	if ((factor[0]) == 0){
	  if(value == n){
	    printf("%d is prime\n", n);
	    exit(1);
	  } 
	  else if (value * value == n){
	    factor[0] = value;
	    printf("%d %d %d\n", n, factor[0], factor[0]);
	    exit(1);
	  }
	  else{
	    factor[0] = value;
	  }
	}
	else {
	  if((factor[0] * value) == n){
	    printf("%d %d %d\n", n, factor[0], value);
	    exit(1);
	  }
	  else{
	    printf("%d is not the product of two primes\n", n);
	    exit(1);
	  }
	}
      }
    }
    else{
      /* its in the else if the read value is not a multiple of m,
	 so we create a new pipe and fork()*/
      int new_fd[2];
      int pipe_result;
      if((pipe_result = pipe(new_fd))== -1){
	perror("pipe failed to execute properly");
	exit(1);
      }
      pid_t child_pid = fork();
      if(child_pid ==0){
	/* its in a new child process*/
	close(new_fd[1]); //we wont be writing to the pipe 
	 int m;           //store the first value of the passed in integers
	 if((read(new_fd[0], &m, sizeof(int))) == -1){
	   close(new_fd[0]);
	   perror("reading from pipe failed in child");
	 }
    
	 /*check if m is one of the factors, and process it appropriately*/
	 if((n % m) == 0){        
	   if (factor[0] == 0){
	     if(m == n){
	       printf("%d is prime\n", n);
	     }
	     else if (m * m == n){
	       factor[0] = m;
	       printf("%d %d %d\n", n, factor[0], factor[0]);
	       exit(1);
	     }
	     else{
	       factor[0] = m;
	     }
	   }
	   else {
	     if((factor[0] * m) == n){
	       printf("%d %d %d\n", n, factor[0], m);
	       exit(1);
	     }
	     else{
	       printf("%d is not the product of two primes\n", n);
	       exit(1);
	     }
	   }
	 }
      }
      else if(child_pid >0){
	close(new_fd[0]);   //won't be reading from the newly created pipe

	/* write the value that is not a multiple of m to the newly 
	   created child, if an error occurs, close the pipe and exit.*/
	if(write(new_fd[1],&value , sizeof(int)) == -1){
	  close(new_fd[1]);
	  perror("writing to pipe failed");
	  exit(1);
	}

	int child_status;
	/*wait for the child to exit*/
	if(wait(&child_status)!= -1){
	  if(WIFEXITED(child_status)){
	    exit(WEXITSTATUS(child_status) +1);
	  }
	}
      }
      else{
	perror("fork failed\n");
	exit(1);
      }
    }
  }

  /* this marks the end of the while loop, meaning that the pipe
     finished reading or an error occured, so close the pipe and return 
     an error*/
  if(read_output <= 0){
    perror("reading from pipe did not execute properly");
    close(fd[0]);
    exit(1);
  }
}

