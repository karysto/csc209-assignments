#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "data.h"

FILE           *logfp = NULL;


/* Read a packet file, and coalesce packets */

/*
 * Print to stdout the reconstructed data.  If a packet is missing, insert
 * ### into the output
 */


/*
  Checks if malloc has been correctly allocated
  malloc_pointer: pointer to the block malloc has been allocated to
*/
void malloc_check(void * malloc_pointer){
  if (malloc_pointer == NULL){
    perror("malloc could not be allocated");
    exit(EXIT_FAILURE);
  }
}

/*
  Creates the linked lists for the packets, while calling log_message when appropriate
  root: root of the linked list
  data: the packet we want to insert to the linked list
  position: the position this packet has to go into the linked list (i.e. the block_num)
*/
struct list*  insert_node_log_message(struct list *root,struct packet data, int position){
  int i, size;
  size = 1;
  struct list *new_node, *temp;
  temp = root;

  ///If this is the first node to be inserted, make it the root
  if(root ==NULL){
    new_node =(struct list *)malloc(sizeof(struct list));
    malloc_check(new_node);
    new_node->p=data;
    new_node->next=NULL;
    root = new_node;
  }
  else{
    //First, get the size of the linked list
    while(temp->next != NULL){
      temp = temp->next;
      size++;  
    }
    
    ///If the given position is > size, create empty nodes until we reach the position 
    ///before creating the node with the given data at the position
    if(position > size){
      for(i = 1; i <position - size; i++){
        new_node =(struct list *)malloc(sizeof(struct list));
	malloc_check(new_node);
        temp->next = new_node;
        temp = temp->next;
      }
      new_node =(struct list *)malloc(sizeof(struct list));
      malloc_check(new_node);
      new_node->p=data;
      new_node->next=NULL;
      temp->next = new_node;
    }
    ///Else, just insert the data to the node at the appropriate position, if we are replacing data
    ///call the appropriate log message
    else{
      temp = root;
      for(i=1; i<=position-1; i++){
        temp = temp->next;
      }
      if(temp->p.block_num == data.block_num){
	log_message(&data, 3, logfp);
      }
      else{
	log_message(&data, 0, logfp);
      }
      temp->p=data ;
    }
  }
  return root;
}

/*
  function that frees all the nodes
  root: root of the linked list that you want to free
*/
void free_all(struct list *root){
  struct list *curr;
  while(root != NULL){
    curr = root;
    root = root->next;
    free(curr);
  }
}
/*
  Creates the linked lists for the packets
  root: root of the linked list
  data: the packet we want to insert to the linked list
  position: the position this packet has to go into the linked list (i.e. the block_num)
*/
struct list*  insert_node(struct list *root,struct packet data, int position){
  int i, size;
  size = 1;
  struct list *new_node, *temp;
  temp = root;

  ///If this is the first node to be inserted, make it the root
  if(root ==NULL){
    new_node =(struct list *)malloc(sizeof(struct list));
    malloc_check(new_node);
    new_node->p=data;
    new_node->next=NULL;
    root = new_node;
  }
  else{
    //First, get the size of the linked list
    while(temp->next != NULL){
      temp = temp->next;
      size++;
    }

    ///If the given position is > size, create empty nodes until we reach the position
    ///before creating the node with the given data at the position
    if(position > size){
      for(i = 1; i <position - size; i++){
	new_node =(struct list *)malloc(sizeof(struct list));
	malloc_check(new_node);
	temp->next = new_node;
	temp = temp->next;
      }
      new_node =(struct list *)malloc(sizeof(struct list));
      malloc_check(new_node);
      new_node->p=data;
      new_node->next=NULL;
      temp->next = new_node;
    }
    ///Else, just insert the data to the node at the appropriate position, if we are replacing data
    ///call the appropriate log message
    else{
      temp = root;
      for(i=1; i<=position-1; i++){
	temp = temp->next;
      }
      temp->p=data ;
    }
  }
  return root;
}

int main(int argc, char *argv[]) {
    FILE *infp;
    char opt;
    extern int optind;
    extern char *optarg;

    while ((opt = getopt(argc, argv, "l:")) != -1) {
        switch (opt) {
            case 'l':
                logfp = fopen(optarg, "w");
                if(!logfp) {
                    perror("fopen");
                    exit(1);
                }
                break;
            default: /* '?' */
                fprintf(stderr, "Usage: %s [-l logfile ] inputfile\n",
                        argv[0]);
                exit(1);
        }
    }
    if(optind >= argc) {
        fprintf(stderr, "Expected inputfile name\n");
        exit(1);
    }

    if(argc != 2 && argc != 4) {
        fprintf(stderr, "Usage: %s [-l logfile ] inputfile\n", argv[0]);
        exit(1);    
    }

    if((infp = fopen(argv[optind], "r")) == NULL) {
        perror("fopen");
        exit(1);
    }
    
    ///If logfile is called, call log_message at approprite times while processing the data
    if (logfp != NULL){
      int read_output_packet,size_of_payload;
      struct packet read_packet;
      struct list *root;
      char *read_payload;
      root = (struct list *)malloc( sizeof(struct list));
      malloc_check(root);

      while(0 != (read_output_packet = fread(&read_packet, sizeof(read_packet), 1, infp))){
	if(ferror(infp)){
	  perror("The file was not read correctly");
	  exit(EXIT_FAILURE);
	}

        size_of_payload = read_packet.block_size;
        read_payload = malloc(size_of_payload);
	malloc_check(read_payload);
        fread(read_payload, size_of_payload, 1, infp);
	if(ferror(infp)){
	  perror("The file was not read correctly");
	  exit(EXIT_FAILURE);
	}

        if (read_packet.crc == crc_message(XMODEM_KEY, (unsigned char *)read_payload, size_of_payload)){
	  log_message(&read_packet, 1, logfp);
          root = insert_node_log_message(root,read_packet,read_packet.block_num);
          fprintf(stdout, "%s", read_payload);
        }
	else {
	log_message(&read_packet, 2, logfp);
	}
	free(read_payload);
      } 
      free_all(root);
      if(fclose(logfp)==EOF){
	perror("The logfile did not close properly");
	exit(EXIT_FAILURE);
      }
    }
    
    ///log option was not given, so just processs the data
    else{
      int read_output_packet, size_of_payload;
      struct packet read_packet;
      struct list *root;
      char *read_payload;
      root = (struct list *)malloc( sizeof(struct list));
      malloc_check(root);

      while(0 != (read_output_packet = fread(&read_packet, sizeof(read_packet), 1, infp))){
	if(ferror(infp)){
	  perror("The file was not read correctly");
	  exit(EXIT_FAILURE);
	}
	size_of_payload = read_packet.block_size;
	read_payload = malloc(size_of_payload);
	malloc_check(read_payload);
	fread(read_payload, size_of_payload, 1, infp);
	if(ferror(infp)){
	  perror("The file was not read correctly");
	  exit(EXIT_FAILURE);
	}

	if (read_packet.crc == crc_message(XMODEM_KEY, (unsigned char *)read_payload, size_of_payload)){
	  root = insert_node(root,read_packet,read_packet.block_num);
	  fprintf(stdout, "%s", read_payload);
	}
	free(read_payload);
      }
      free_all(root);
    }
     
    if(fclose(infp)==EOF){
      perror("The input file did not close properly");
      exit(EXIT_FAILURE);
    }

return 0;
}
