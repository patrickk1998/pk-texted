#include "codepoint.h"

int get_length_byte(unsigned char byte)
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

int get_length(codepoint *p)
{
	return get_length_byte(p->content[0]);
}

int get_next_codepoint(char *stream, codepoint *p)
{
	int len = get_length_byte(stream[0]);	
	for(int i = 0; i < len; i++){
		if(stream[i] == '\0')
			return -1;
		p->content[i] = stream[i];
	}

	return len;
}

int put_next_codepoint(char *stream, codepoint *p)
{
	int len = get_length_byte(p->content[0]);	
	for(int i = 0; i < len; i++)
		stream[i] = p->content[i];	
	return len;
}
