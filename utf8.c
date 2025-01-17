#include <assert.h>
#include "utf8.h"



static int get_length_byte(unsigned char byte)
{
	if(byte <= 0x7f)
		return 1; // ASCII character. 
	if((0x7f < byte) && (byte < 0xc0))
		return 0; // continuation byte.	
	if((0xc2 <= byte) && (byte <= 0xdf))
		return 2;
	if((0xe0 <= byte) && (byte <= 0xef))
		return 3;
	if((0xf0 <= byte) && (byte <= 0xf4))
		return 4;
	
	return -1;
}


int utf8_size(utf8 uchar)
{
	return get_length_byte(uchar.x[0]);	
}

utf8 utf8_next(char *stream)
{
	int len = get_length_byte(stream[0]);	
	utf8 uchar;
	assert(len > 0);
	for(int i = 0; i < len; i++){
		if(stream[i] == '\0'){
			uchar.x[0] = 0;	
			break;
		}
		uchar.x[i] = stream[i];
	}
	
	return uchar;
}

utf8 utf8_from_char(char c){
	utf8 uchar;
	uchar.x[0] = c;
	return uchar;
}

int utf8_compare(utf8 uchar, char c)
{
	return uchar.x[0] == c;
}

int utf8_empty(utf8 uchar)
{
	return uchar.x[0] == 0;
}
