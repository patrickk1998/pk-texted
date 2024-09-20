#include "input.h"
#include <unistd.h>
#include <ctype.h>
#include <string.h>

static enum input_type escape_handle(char *);

void get_action(struct input_action *new_action)
{
	char b[3];
	int len = read(STDIN_FILENO, &b, 4);
	if(b[0] == ('q' & 0x1f)){
		new_action->type = quit;
	}
	if(b[0] == '\033'){
		if(len == 1)
			new_action->type = escape;
		else 
			new_action->type = escape_handle(b+1);
	}
	if(b[0] == 127){
		new_action->type = backspace;
	}
	if(isgraph(b[0]) || isblank(b[0])){
		new_action->type = insert;
		new_action->value = b[0];
	}
	if(b[0] == 13){
		new_action->type = creturn;
	}
}

enum input_type escape_handle(char *buf)
{
	enum input_type action = noop;
	if(!strncmp(buf,"[A",2)){
		action = up;
		goto end;
	}
	if(!strncmp(buf,"[B",2)){
		action = down;
		goto end;
	}
	if(!strncmp(buf,"[C",2)){
		action = right;
		goto end;
	}
	if(!strncmp(buf,"[D",2))
		action = left;

end:
	return action;
}

