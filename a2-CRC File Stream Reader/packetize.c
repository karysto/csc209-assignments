#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "data.h"

FILE           *logfp = NULL;


/* Read a file, break it into packets. */

/*
 * Notes: getopt is a useful library function to make it easier to read in
 * command line arguments, especially those with options. Read the man page
 * (man 3 getopt) for more information.
 */



/*
  Process the packet by assigning the appropriate block_num, block_size
  and crc
  buffer: pointer to the read block
  block_size: size of the read block
  block_num: number we are going to assign to the read block, staring at 0
*/
struct packet ProcessPacket(unsigned char *buffer, int block_size, int block_num){
  struct packet p;
  p.block_num = block_num;
  p.block_size = block_size;
  p.crc = crc_message(XMODEM_KEY, buffer, block_size);
  return p;
}


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
  Checks if fwrite wrote the file correctly.
*/
void fwrite_check(int write_output){
  if (write_output != 1) {
    perror("File was not written propertly");
    exit(EXIT_FAILURE);
  }
}

int main(int argc, char *argv[]) {
    FILE *infp = stdin;
    FILE *outfp = NULL;
    char opt;
    extern int optind;
    extern char *optarg;

    while ((opt = getopt(argc, argv, "f:")) != -1) {
        switch (opt) {
            case 'f':
                infp = fopen(optarg, "r");
                if(!infp) {
                    perror("fopen");
                    exit(1);
                }
                break;
            default: /* '?' */
                fprintf(stderr, "Usage: %s [-f inputfile ] outputfile\n",
                        argv[0]);
                exit(1);
            }
    }

    if(optind >= argc) {
        fprintf(stderr, "Expected outputfile name\n");
        exit(1);
    }

    if(!(outfp = fopen(argv[optind], "w"))){
        perror("fopen");
        exit(1);
    }

    /* The files have been opened for you.  Write the rest of the program here.*/

    int counter = 0; //counter will be used to assign the appropriate block_num
    int write_output_buffer, write_output_payload, block_size;
    unsigned char buffer[MAXSIZE];
    char *payload;

    ///Run the while loop when we read block_size = MAXSIZE
    while(MAXSIZE ==(block_size = fread(buffer, 1, MAXSIZE, infp))){
      if(ferror(infp)){
	perror("The file was not read correctly");
	exit(EXIT_FAILURE);
      }

      struct packet processed_packet;
      struct packet* packet_pointer;
      processed_packet =(ProcessPacket(buffer, block_size, counter));
      packet_pointer = &processed_packet;

      write_output_buffer = fwrite(packet_pointer,sizeof(processed_packet),1, outfp);
      fwrite_check(write_output_buffer);      

      payload = (char *)malloc(block_size);
      malloc_check(payload);
      strncpy(payload, (const char*)buffer, block_size);
      (*packet_pointer).payload = (unsigned char*)payload;
      write_output_payload = fwrite(payload,block_size,1, outfp);
      fwrite_check(write_output_payload);
      free(payload);
      counter++;
   }

    ///Process the remainding block of data
    if (block_size!=0){
      struct packet processed_packet;
      struct packet *packet_pointer;
      processed_packet = ProcessPacket(buffer, block_size, counter);
      packet_pointer = &processed_packet;
      write_output_buffer = fwrite(packet_pointer,sizeof(processed_packet), 1, outfp);
      fwrite_check(write_output_buffer);

      payload = malloc(block_size);
      malloc_check(payload);
      strncpy(payload, (const char*)buffer, block_size);
      (*packet_pointer).payload = (unsigned char*)payload;
      write_output_payload = fwrite(payload,block_size,1,outfp);
      fwrite_check(write_output_buffer);
      free(payload);
    }
    
    if(fclose(infp)==EOF){
      perror("The input file did not close properly");
      exit(EXIT_FAILURE);
    }
    if(fclose(outfp)==EOF){
      perror("The output file did not close properly");
      exit(EXIT_FAILURE);
    }
    return 0;
}
