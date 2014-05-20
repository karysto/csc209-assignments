#include <string.h>
#include <stdio.h>

#include "data.h"

// based on code from:
// http://stackoverflow.com/questions/111928/is-there-a-printf-converter-to-print-in-binary-format?rq=1

const char *to_binary(int x) {
    static char bits[17];
    bits[0] = '\0';
    int z;
    for (z = 1 << 15; z > 0; z >>= 1) {
        strcat(bits, ((x & z) == z) ? "1" : "0");
    }
    return bits;
}

char  *to_binary_8(int x) {
  static char bits[8];
  bits[0] = '\0';
  int z;
  for (z = 1 << 7; z > 0; z >>= 1) {
    strcat(bits, ((x & z) == z) ? "1" : "0");
  }
  return bits;
}



/*  
  Process one bit of the message.
  reg: register (modified by this function)
  key: CRC16 key being used
  next_bit: next bit of message
*/

void crc_bit(unsigned short *reg, unsigned int key, unsigned int next_bit) {
  
  const char *reg_crc;
  reg_crc = to_binary(*reg);
  const char first_element = reg_crc[0];
  *reg = *reg << 1;
  if (next_bit == 1){
    *reg = *reg + 1 ;
  }
  if ((int)first_element == 49){
    *reg = *reg ^ key;
  }
  
}
  
/*
  Process one byte of the message.
  reg: register (modified by this function)
  key: CRC16 key being used
  next_byte: next byte of message
*/

void crc_byte(unsigned short *reg, unsigned int key, unsigned int next_byte) {
  
  char *next_byte_bin = to_binary_8(next_byte);
  int j;
  for (j = 0; j < 8; j++){
    if (next_byte_bin[j] == (int)'1'){
      next_byte_bin[j] = 1;
    }
    crc_bit(reg, key, (unsigned int)next_byte_bin[j]);
  }    
}   

/*
  Process an entire message using CRC16 and return the CRC.
  key: CRC16 key being used
  message: message for which we are calculating the CRC.
  num_bytes: size of the message in bytes.
*/

unsigned short crc_message(unsigned int key,  unsigned char *message, int num_bytes) {
  int i,j;
  unsigned short reg_crc = 0b0000000000000000; //creaing the empty register 
  unsigned short *pointer_reg_crc = &reg_crc;
  
    
  for(i = 0; i < num_bytes; i++){
    crc_byte(pointer_reg_crc, key, (int)(message[i]));
  }
  for(j = 0 ; j < 16; j++){
    crc_bit(pointer_reg_crc, key, 0);
  }
  return *pointer_reg_crc;
}


