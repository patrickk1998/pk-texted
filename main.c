#include <stdio.h>
#include "texted.h"


int main(int argc, char* argv[])
{
	char default_file_name[] = "text/example1";
	char *file_name;
	if(argc < 2)
		file_name = default_file_name;
	else 
		file_name = argv[1];

	int fd = create_line_list(file_name);	
	if(fd == -1){
		return 1;
	}

	printf("Opened %s\n", file_name);
	cleanup(fd);
	return 0;
}
