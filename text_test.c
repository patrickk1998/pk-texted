#include "text.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define TEST_FILE_NAME ".pktexted_test_file"
#define TEST_FILE_NAME_SAVE ".pktexted_test_file_save"
#define TEST_FILE_LINES 500
#define TEST_LINE_LEN 80

int test_row_width(struct text *xt)
{
	printf("Starting ROW WIDTH test\n");	
	srand(1);
	int w;
	for(int i = 0; i < 100; i++){
		w = rand() % 1000;
		xt->set_row_width(xt, w);
		if(xt->get_row_width(xt) != w){
			printf("ROW WIDTH failed for %d\n", w);
			return -1;
		} 
	}
	printf("ROW WIDTH passed\n");	
	return 0;
}

int generate_file(int seed)
{
	int fd;
	if((fd = open(TEST_FILE_NAME, O_RDWR | O_CREAT | O_EXCL , 0666)) < 0){
		perror("Problem with creating temporary file in generate_file");
		return -1;
	}
	srand(seed);	
	char c;
	for(int i = 0; i < (rand() % TEST_FILE_LINES); i++){
		for(int j = 0; j < (rand() % TEST_LINE_LEN); j++){
			c = (rand() % 94) + 32;
			write(fd, &c, 1);
		}
		if((i+1) != (rand() % TEST_FILE_LINES))
			write(fd, "\n", 1);
	}
	lseek(fd, 0, SEEK_SET);
	return fd;
}

int compare_files(int fda, int fdb)
{
	char a, b;
	int ar, br;
	while(1){
		ar = read(fda, &a, 1);
		br = read(fdb, &b, 1);
		if(ar != br){
			printf("Files are not the same length!\n");
			return -1;
		}
		if(!ar)
			return 1;
		if(a != b){
			printf("Characters are not equal!\n");
			return -1;
		}
	}	
}

int main()
{
	struct basic_text bxt;
	struct text *xt = make_basic_text(&bxt);
	
	if(test_row_width(xt) < 0){
		return 0;
	}

	printf("Starting basic load and save test\n");
	int fd = generate_file(1);	
	int fd_save;
	if((fd_save = open(TEST_FILE_NAME_SAVE, O_RDWR | O_CREAT | O_EXCL , 0666)) < 0){
		perror("Problem with creating temporary save file");
		return 1;
	}	
	xt->set_row_width(xt, TEST_LINE_LEN);
	xt->set_fd(xt, fd);
	xt->load_file(xt);
	xt->set_fd(xt, fd_save);	
	xt->save_file(xt);
	if(compare_files(fd, fd_save) < 0){
		printf("Failed basic load and save test\n");
	}
}
