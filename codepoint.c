#include "codepoint.h"
#include <stddef.h>
#include <stdarg.h>
#include <stdlib.h>

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

int get_next_codepoint(const char *stream, codepoint *p)
{
	int len = get_length_byte(stream[0]);	
	for(int i = 0; i < len; i++){
		if(stream[i] == '\0')
			return -1;
		if(p != NULL)
			p->content[i] = stream[i];
	}
	
	if(p != NULL)
		p->text_color = color_default;
	return len;
}

int put_next_codepoint(char *stream, const codepoint *p)
{
	int len = get_length_byte(p->content[0]);	
	for(int i = 0; i < len; i++)
		stream[i] = p->content[i];	
	return len;
}

codepoint *stocp(const char *str, ...)
{
	enum color current_color = color_default;
	int i = 0;
	int a = 0;
	int j = 0;
	while(str[j] == '%'){
		if(str[j+1] == '%')
			i++;
		j+= 2;
	}
	while((a = get_next_codepoint(str + j, NULL)) > 0){
		j += a;
		i++;
		while(str[j] == '%'){
			if(str[j+1] == '%')
				i++;
			j+= 2;
		}
	}

	codepoint *text = malloc(sizeof(codepoint)*i);

	va_list ap;
	va_start(ap, str);

	i = 0;
	a = 0;
	j = 0;
	while(str[j] == '%'){
		if(str[j + 1] == 'c')
			current_color = va_arg(ap, enum color);
		if(str[j + 1] == '%'){
			get_next_codepoint(str + j + 1, &text[i]);	
			i++;
		}
		j+= 2;
	}
	while((a = get_next_codepoint(str + j, &text[i])) > 0){
		text[i].text_color = current_color;
		j += a;
		i++;
		while(str[j] == '%'){
			if(str[j+1] == 'c')
				current_color = va_arg(ap, enum color);
			if(str[j + 1] == '%'){
				get_next_codepoint(str + j + 1, &text[i]);	
				i++;
			}
			j+= 2;
		}
	}
	text[i-1].line_status = end_line;

	va_end(ap);
	return text;
}

