#include "../frontend/codepoint.h"
#include <stdio.h>
#include <string.h>

int main()
{
	/** ASCII Characters **/
	printf("**STARTING ASCII TEST**\n");
	char ascii_text[13] = "Hello World\n";	
	codepoint ascii_unicode_decoded[12];
	int j = 0;
	for(int i = 0; i < 12; i++){
		int l = get_next_codepoint(ascii_text + j, &ascii_unicode_decoded[i]);  
		if(l != 1){
			printf("Length of ascii character is not 1\n");
			return 0;
		}	
		j += l;
	}
	j = 0;
	char ascii_text_save[13];
	ascii_text_save[12] = '\0';
	for(int i = 0; i < 12; i++){
		j+= put_next_codepoint(ascii_text_save + j, &ascii_unicode_decoded[i]);		
	}
	if(strcmp(ascii_text_save, ascii_text) != 0){
		printf("Decoded and Encoded text is not the same\n");
		printf("ENCODED: %s, DECODED %s\n", ascii_text, ascii_text_save);
		return 0;
	}
	printf("**PASSED ASCII TEST**\n");


	/** UNICODE Characters **/
	printf("**STARTING UNICODE TEST**\n");
	char unicode_text[44] = "¢a©Ḁ§tḫ⋘🤖🛑b";
	char unicode_len[11] = {2,1,2,3,2,1,3,3,4,4,1};
	codepoint unicode_decoded[11];
	j = 0;
	for(int i = 0; i < 11; i++){
		int l = get_next_codepoint(unicode_text + j, &unicode_decoded[i]);  
		if(l != unicode_len[i]){
			printf("Length of unicode character %d is not correct\n", i);
			return 0;
		}	
		j += l;
	}
	j = 0;
	char unicode_text_save[44] = {'\0'};
	for(int i = 0; i < 11; i++){
		j+= put_next_codepoint(unicode_text_save + j, &unicode_decoded[i]);		
	}
	if(strcmp(unicode_text_save, unicode_text) != 0){
		printf("Decoded and Encoded text is not the same\n");
		printf("ENCODED: %s, DECODED %s\n", unicode_text, unicode_text_save);
		return 0;
	}
	printf("**PASSED UNICODE TEST**\n");

	return 0;
}
