#include <stdio.h>
#include <stdlib.h>
#include "string.h"
#include "data.h"


/*
  Convert a register's binary representation into a 
  16 bit character array. Return a pointer to the array.
  x: the integer value of the register.
*/

const char *reg_to_binary(int x) {
    static char bits[17];
    bits[0] = '\0';
    int z;
    for (z = 1 << 15; z > 0; z >>= 1) {
        strcat(bits, ((x & z) == z) ? "1" : "0");
    }
    return bits;
}


/*
  Convert an byte's binary representation into a 
  8 bit character array. Return a pointer to the array.
  x: the integer value of the byte.
*/
  
char  *byte_to_binary(int x) {
  static char bits[8];
  bits[0] = '\0';
  int z;
  for (z = 1 << 7; z > 0; z >>= 1) {
    strcat(bits, ((x & z) == z) ? "1" : "0");
  }
  return bits;
}


/*
  Process an a single bit using the CRC16 algorithm. 
  reg: the register for the passed in bits.
  key: CRC16 key being used
  next_bit: the next bit of the message to be processed.
*/
  
void crc_bit(unsigned short *reg, unsigned int key, unsigned int next_bit) {

  const char *reg_crc;
  reg_crc = reg_to_binary(*reg);
  
  /* To set the right-most bit to the left-most bit:

     Set the right-most bit to 0 by shifting the register left.
     Set the right-most bit to 1 by adding to the register.
  */
  const char first_element = reg_crc[0];
  *reg <<= 1;

  if (next_bit == 1) { *reg = *reg + 1 ; }

  /* 49 is the integer cast of the ASCII character '1'.
  Apply the XOR if the right-most bit in the register is 1.
  */
  if ((int)first_element == 49) { *reg ^= key; }
  return;
}


/*
  Process an a single byte using the CRC16 algorithm.
  reg: the register for the passed in bits.
  key: CRC16 key being used
  next_byte: the next byte of the message to be processed.
*/

void crc_byte(unsigned short *reg, unsigned int key, unsigned int next_byte) {

  /* Process 8 bits of a message, if the bit is a 1, 
  reflect this in the string representation of the register.
  */  
  char *binary_next_byte = byte_to_binary(next_byte);
  int reg_index_counter;
  for (reg_index_counter = 0; reg_index_counter < 8; reg_index_counter++){
    if (binary_next_byte[reg_index_counter] == (int)'1'){
      binary_next_byte[reg_index_counter] = 1;
    }
    crc_bit(reg, key, (unsigned int)binary_next_byte[reg_index_counter]);
  }
  return;
}

/*
  Process an entire message using CRC16 and return the CRC.
  key: CRC16 key being used
  message: message for which we are calculating the CRC.
  num_bytes: size of the message in bytes.
*/

unsigned short crc_message(unsigned int key,  unsigned char *message, int num_bytes) {
 
  // Initialize the base register.
  int byte_counter,reg_index_counter;
  unsigned short reg_crc = 0b0000000000000000;
  unsigned short *ptr_reg_crc = &reg_crc;

  /* Iterate through the register and shift in the message
  using the CRC16 algorithm. 
  */
  for(byte_counter = 0; byte_counter < num_bytes; byte_counter++){
    crc_byte(ptr_reg_crc, key, (int)(message[byte_counter]));
  }

  /* Iterate through the entire register and shift the message 
  16 more times using the CRC16 algorithm.
  */  
  for(reg_index_counter = 0 ; reg_index_counter < 16; reg_index_counter++){
    //printf("%s\n", reg_to_binary(reg_crc));
    crc_bit(ptr_reg_crc, key, 0);
  }
  return *ptr_reg_crc;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
      fprintf(stderr, "Error: crc16 requires 2 arguments: crc16 <n>\n");
      return -1;
  }

  /*convert the first argument from string to int,
  Professor Reid stated that "atoi" can be used here
  instead of strtol for simplicity purposes. */

  int n = atoi(argv[1]);
  crc16(n);
  return 0;
}
