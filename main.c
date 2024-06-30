#include <stdio.h>
#include <stdlib.h>
#include "texted.h"


static int callback(struct line_item *item, long data)
{
	printf("%d\t %s\n", *(int*)data, item->text);	
	(*(int*)data)++;
	return 1;
}

static int display_callback(struct line_item *item, long data)
{
	write_line(item, *(int*)data);
	(*(int*)data)++;
	return 1;
}

int main(int argc, char* argv[])
{
	read_envs();
	if(to_debug)
		printf("Debugging!");
	char default_file_name[] = "text/example1";
	char *file_name;
	if(argc < 2)
		file_name = default_file_name;
	else 
		file_name = argv[1];
	
	
	struct displayState state = {};
	get_size(&state);	
	state.llist.width = state.winColumns;
	state.llist.head = NULL;

	int fd = create_line_list(file_name, &state.llist);	
	if(fd == -1){
		return 1;
	}
	
	state.cursorLine = state.llist.head;	
	state.displayStart = state.llist.head;


	enable_raw();

	ALT_BUFFER();

	atexit(&clean_screen);

	display_list(&state);
	
	move_cursor(0,0);

	while(1){
		enum inputAction action = get_action(&state.to_insert);
		update_state(action, &state);
		display_state(&state);
	}
/*
	int num = 1;	
	traverse_list(&llist, &callback, (long)&num);

	printf("Opened %s\n", file_name);
*/
	cleanup(fd);
	return 0;
}
