#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include "texted.h"

int create_line_list(char *filename)
{
	int fd = open(filename, 0); 
	if(fd == -1){
		perror(filename);
		return -1;
	}

	return 0;
}


void cleanup(int fd)
{
	close(fd);
}
