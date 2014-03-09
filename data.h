#include <stdlib.h>
#include <stdio.h>
/* Functions and type declarations for the CRC code */
#define XMODEM_KEY 0x1021

unsigned short	crc_message(unsigned int key, unsigned char *message, int num_bytes);


/* Function and type declarations for packets */

#define MAXSIZE 256
struct packet {
	unsigned short	block_num;
	unsigned short	block_size;
	unsigned short	crc;
	unsigned char  *payload;
};

/* Function and type declaration for list of packets */
struct list {
    struct packet p;
    struct list *next;
};

// Acts as the first node in the linked list.
//struct list *start = malloc(sizeof(struct list)); 

/*****************Start of packet function definitions*****************/


/*****************Start of list function definitions*****************/


struct list *add_to_list(struct list *packets, struct packet added_packet);

struct list *list_create_first(struct list *start, struct packet input_packet);

struct list *search_list(struct list *this_list, struct packet this_packet);

void list_freeall(struct list* start); 

/*******************Start of memory function definitions*******************/

void check_free(void *ptr_void_free);

void check_malloc(void *ptr_void_malloc);

void check_fwrite(int num_bytes_written);

void check_fread(int num_bytes_read, int num_bytes_read_expected);

void check_fclose(FILE *ptr_outfile);

#define INSERTED 0
#define CRC_MATCH 1
#define CRC_NO_MATCH 2
#define DUPLICATE 3

/*
 * logfp is a global variable because it is used inside other functions. We
 * could pass it in as a parameter to the functions that call log_message but
 * since it is nice to see the logging happen in the background, it seems
 * reasonable to make the file pointer a global variable.
 */
extern FILE    *logfp;
void		log_message(struct packet *p, int message_no, FILE * logfp);
