#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include "xmodemserver.h"
#include "crc16.h"

#ifndef PORT
#define PORT 55163
#endif
#define LISTENQ 10

struct client*  addclient(int fd, struct client* root){
  struct client *new_node, *temp;
  
  if(root ==NULL){
    fflush(stdout);
    new_node = (struct client *)malloc(sizeof(struct client));

    if (new_node == NULL){
      perror("malloc could not be allocated");
      exit(EXIT_FAILURE);
    }

    new_node->fd = fd;
    new_node->current_block = 1;
    new_node-> state = initial;
    new_node->next = NULL;
    root = new_node;
  }
  else{
    printf("HERE2\n");
    fflush(stdout);
    temp = root;
    while(temp->next != NULL){
      temp = temp->next;
    }
    new_node = (struct client *)malloc(sizeof(struct client));
    
    if (new_node == NULL){
      perror("malloc could not be allocated");
      exit(EXIT_FAILURE);
    }

    temp->next = new_node;
    new_node->fd = fd;
  }
  return root;
}


void dofinish(struct client* p, struct client* root, fd_set* fdlist){
  struct client*curr;
  while(root != NULL){
    curr = root;
    root = root->next;
    close(p->fd);
    fclose(p->fp);
    free(curr);
  }
}

struct client*  removeclient(struct client* client, struct client* root, fd_set* fdlist){
  struct client **p;
  struct client* curr;
  for (p = &root; *p && *p != client; p = &(*p)->next)
    ;
  if(*p && (*p == root)){
    FD_CLR(client->fd, fdlist);
    close(client->fd);
    curr = root;
    free(curr);
    root = NULL;
    free(*p);
  }  
  else if (*p) {
    struct client *t = (*p)->next;
    FD_CLR(client->fd, fdlist);
    close(client->fd);
    free(*p);
    *p = t;
  }
  return root;
}

void doinitial(struct client* p, struct client* root, fd_set* fdlist){
  int inbuf, nbytes, room, where;
  char *after;
  char buf[20];
  inbuf = 0;
  room = sizeof(buf);
  after = buf;

  if ((nbytes = read(p->fd, after, room)) > 0){
    inbuf = inbuf + nbytes;
    where = find_network_newline(buf, inbuf);

    if (where >= 0){
      buf[where] = '\0';
      buf[where+1] = '\0';
      strncpy(p->filename, buf, 20);
      p->fp = open_file_in_dir(p->filename, "filestore");
      write(p->fd, "C", 1);
      p->state = pre_block;
    }

    inbuf = sizeof(buf) - (where +1);
    room = sizeof(buf) - inbuf +1;
    after = &buf [inbuf -1];
  }
  else if (nbytes < 0){
    perror("read");
    root = removeclient(p, root, fdlist);
  }  
  if(strlen(buf) > 20){
    //the filename is too big, drop the client
    root = removeclient(p, root, fdlist);
  }

}

struct client* dopre_block(struct client* p, struct client* root, fd_set* fdlist){
  char temp;
  int nbytes;

  nbytes = read(p->fd, &temp, 1);
  
  if (nbytes < 0){
    perror("read");
    root = removeclient(p, root, fdlist);
  }

  if(temp == EOT){
    temp = ACK;
    write(p->fd, &temp, 1);
    root = removeclient(p, root, fdlist);
    fclose(p->fp);    
  }
  else if(temp == SOH){
    p->blocksize = 128;
    p->state = get_block;
    if (p->current_block == 0){
      p->current_block = 1;
    }
    p->inbuf = 0;
  }  
  else if(temp == STX){
    p->blocksize = 1024;
    p->state = get_block;
    if (p->current_block == 0){
      p->current_block = 1;
    }
    p->inbuf = 0;
  }
  return root;
}

struct client* doget_and_check_block(struct client* p, struct client* root, fd_set* fdlist){
  int nbytes;
  

  if(p->inbuf == 0){
    nbytes = read(p->fd, p->buf, 1);
    
    if (nbytes < 0){
      perror("read");
      root = removeclient(p, root, fdlist);
    }

    p->inbuf++;
  }
  else if(p->inbuf == 1){
    nbytes = read(p->fd, &(p->buf[p->inbuf]), 1);

    if (nbytes < 0){
      perror("read");
      root = removeclient(p, root, fdlist);
    }

    p->inbuf++;
  }
  //now we have read the block_num and inverse
  else if(p->inbuf < ((p->blocksize)+2)){
    nbytes = read(p->fd,&(p->buf[p->inbuf]), p->blocksize);

    if (nbytes < 0){
      perror("read");
      root = removeclient(p, root, fdlist);
    }

    p->inbuf = p->inbuf + nbytes;
  }
  else if(p->inbuf == (p->blocksize+2)){
    nbytes = read(p->fd, &(p->buf[p->inbuf]), 1);

    if (nbytes < 0){
      perror("read");
      root = removeclient(p, root, fdlist);
    }

    p->inbuf++;
  }
  else if(p->inbuf == (p->blocksize +3)){
    nbytes = read(p->fd, &(p->buf[p->inbuf]), 1);

    if (nbytes < 0){
      perror("read");
      root = removeclient(p, root, fdlist);
    }
    p->inbuf++;
      
    //performing check_block 
    if((unsigned char)p->buf[0] != (unsigned char)(255 - p->buf[1])){
      printf("block_num doesn't match inverse\n");
      root = removeclient(p, root, fdlist);  
    }
    else if(p->buf[0] == (p->current_block -1)){
      printf("block_num is a duplicate of the previous one\n");
      fflush(stdout);
      char temp;
      temp = ACK;
      write(p->fd, &temp, 1);
      p->state = pre_block;
      }
    else if(p->buf[0] !=  p->current_block){
      printf("not the block_num we were expecting\n");
      fflush(stdout);
      root = removeclient(p, root, fdlist);
    }
    else{
      char temp;
      char buf[p->blocksize];
      char* payload_position = &(p->buf[2]);
      strncpy(buf, payload_position, p->blocksize);

      //check if the crc matches
      unsigned char crc_first_byte = p->buf[p->blocksize+2];
      unsigned char crc_second_byte = p->buf[p->blocksize+3];
      unsigned short crc = crc_message((unsigned int)XMODEM_KEY, (unsigned char*)buf, p->blocksize);
      
      if(crc_first_byte != crc >>8){
	fflush(stdout);
	temp = NAK;
	write(p->fd, &temp, 1);
	p->inbuf = p->blocksize +2;
      }
      else if(crc_second_byte != (crc&0xFF)){
	temp = NAK;
        write(p->fd, &temp, 1);
	p->inbuf = p->blocksize +2;
      }
      else{
	//everything is fine, we can write the payload
	int nbytes;
	nbytes = fwrite(buf, 1, p->blocksize, p->fp);

	if (nbytes < 0){
	  perror("write");
	  root = removeclient(p, root, fdlist);
	}

	p->current_block = p->current_block +1;	
	temp = ACK;
	write(p->fd, &temp, 1);
	p->state = pre_block;
      }     
    } 
  }
  return root;
}

int main(){

  struct client *root, *p;
  int listenfd, clientfd, maxfd, nready;
  fd_set rset;
  struct sockaddr_in servaddr, clientaddr;
  
  fprintf(stderr, "Port = %d\n", PORT);
  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  memset(&servaddr, '\0', sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = INADDR_ANY;
  servaddr.sin_port = htons(PORT);

  if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof servaddr)){
    perror("bind");
    exit(1);
  }

  if (listen(listenfd, LISTENQ)){
    perror("listen");
    exit(1);
  }

  fd_set fdlist;
  maxfd = listenfd;
  FD_ZERO(&fdlist);
  FD_SET(listenfd, &fdlist);
  root = NULL;

  while(1){
    if(root != NULL && root->next == NULL){
      FD_SET(root->fd, &fdlist);

      if(root->fd > maxfd){
	maxfd = root->fd;
      }

      FD_SET(listenfd, &fdlist);
    }
    else if(root != NULL){   
      p = root;

      while(p->next != NULL){
	FD_SET(p->fd, &fdlist);
	if(p->fd > maxfd){
	  maxfd = p->fd;
	}
	p = p->next;
      }

      FD_SET(p->fd, &fdlist); //we gotta add the last client node which would have next = NULL    

      if(p->fd > maxfd){
	maxfd = p->fd;
      }

      FD_SET(listenfd, &fdlist);
    }

    rset = fdlist;

    if ((nready = (select(maxfd +1, &rset, NULL, NULL, NULL))) < 0){
      perror("select");
    }
    else {
      p = root;

      if(p != NULL && p->next == NULL){
	if(FD_ISSET(p->fd, &rset)){

	  if (p->state == initial){
	    doinitial(p, root, &fdlist);
	  }
	  else if(p->state == pre_block){
	    root = dopre_block(p, root, &fdlist);
	  }
	  else if(p->state == get_block){
	    root = doget_and_check_block(p, root, &fdlist);
	  }
	  else if(p->state == finished){
	    dofinish(p, root, &fdlist);
	  }
	
	}
      }
      else if(p != NULL){
	while(p->next != NULL){

	  if(FD_ISSET(p->fd, &rset)){
	    if (p->state == initial){
	      doinitial(p, root, &fdlist);
	    }   
	    else if(p->state == pre_block){
	      root = dopre_block(p, root, &fdlist);
	    }
	    else if(p->state == get_block){
	      root = doget_and_check_block(p, root, &fdlist);
	    }
	    else if(p->state == finished){
	      dofinish(p, root, &fdlist);
	    }
	  }

	  p = p->next;
	}

	//we gotta check the last client node, which would have its next = NULL
	if(FD_ISSET(p->fd, &rset)){

	    if(p->state == initial){
	      doinitial(p, root, &fdlist);
	    } 
	    else if(p->state == pre_block){
	      root = dopre_block(p, root, &fdlist);
	    }
	    else if(p->state == get_block){
	      root = doget_and_check_block(p, root, &fdlist);
	    }
	    else if(p->state == finished){
	      dofinish(p, root, &fdlist);
	    }
	
	}	  
      }

      if (FD_ISSET(listenfd, &rset)){
	  printf("a new client is connecting\n");
	  socklen_t client_len = sizeof clientaddr;
	  
	  if((clientfd = accept(listenfd, (struct sockaddr *)&clientaddr, &client_len)) < 0){
	    perror("accept");
	    exit(1);
	  }
	  
	  root = addclient(clientfd, root);	  
      }     
    }
    //end of else here    
  }		
  //end of while loop here
  return(0);
}

