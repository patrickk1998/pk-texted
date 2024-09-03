#include "input.h"
#include <unistd.h>
#include <ctype.h>
#include <string.h>

static enum input_type escape_handle();

void get_action(struct input_action *new_action)
{
	char b;
	while( read(STDIN_FILENO, &b, 1) ){
		//if(to_debug) dprintf(STDERR_FILENO, "Key is %d", (int)b);
		if(b == ('q' & 0x1f)){
			new_action->type = quit;
			break;
		}
		if(b == '\033'){
			new_action->type = escape_handle();
			break;
		}
		if(b == 127){
			new_action->type = backspace;
			break;
		}
		if(isgraph(b) || isblank(b)){
			new_action->type = insert;
			new_action->value = b;
			break;
		}
		if(b == 13){
			new_action->type = creturn;
			break;
		}
	}

/*	if(to_debug)
		dprintf(STDERR_FILENO,"Action is %d\n", new_action->type); */
}

enum input_type escape_handle()
{
	enum input_type action = noop;
	char buf[4]; 
	read(STDIN_FILENO, buf, 3);
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

