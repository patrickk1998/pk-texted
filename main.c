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
	write_line(item->text, *(int*)data);
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

	struct line_list llist = { .width = 80 };
	int fd = create_line_list(file_name, &llist);	
	if(fd == -1){
		return 1;
	}

	enable_raw();

	ALT_BUFFER();

	atexit(&clean_screen);

	int num = 1;
	traverse_list(&llist, &display_callback, (long)&num);

	after_display();

/*
	int num = 1;	
	traverse_list(&llist, &callback, (long)&num);

	printf("Opened %s\n", file_name);
*/
	cleanup(fd);
	return 0;
}
