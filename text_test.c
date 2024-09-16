#include "text.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define TEST_FILE_NAME ".pktexted_test_file"
#define TEST_FILE_NAME_SAVE ".pktexted_test_file_save"
#define TEST_FILE_LINES 500
#define TEST_LINE_LEN 80

char rand_buf[TEST_LINE_LEN];
int rand_buf_len;

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

void random_line()
{
		rand_buf_len = (rand() % TEST_LINE_LEN);
		memset(rand_buf, 0, TEST_LINE_LEN);
		for(int j = 0; j < rand_buf_len; j++){
			rand_buf[j] = (rand() % 94) + 32;
		}
}

int generate_file(int seed, int *l)
{
	int fd;
	if((fd = open(TEST_FILE_NAME, O_RDWR | O_CREAT | O_EXCL , 0666)) < 0){
		perror("Problem with creating temporary file in generate_file");
		return -1;
	}

	srand(seed);	
	int lines = (rand() % TEST_FILE_LINES);
	*l = lines;
	for(int i = 0; i < lines; i++){
		random_line();	
		write(fd, rand_buf, rand_buf_len);
		if((i+1) != lines)
			write(fd, "\n", 1);
	}

	lseek(fd, 0, SEEK_SET);
	return fd;
}

int generate_after_file(int seed, struct text *xt, int *fd)
{
	if((fd[0] = open(TEST_FILE_NAME, O_RDWR | O_CREAT | O_EXCL , 0666)) < 0){
		perror("Problem with creating temporary file in generate_after_file");
		return -1;
	}
	if((fd[1] = open(TEST_FILE_NAME_SAVE, O_RDWR | O_CREAT | O_EXCL , 0666)) < 0){
		perror("Problem with creating temporary file in generate_after_file");
		return -1;
	}

	xt->set_fd(xt, fd[1]);
	xt->load_file(xt);
	line_id current_id = xt->get_first_line(xt);
	srand(seed);	
	int lines = (rand() % TEST_FILE_LINES);

	for(int i = 0; i < lines; i++){
		random_line();	
		write(fd[0], rand_buf, rand_buf_len);
		xt->set_text(xt, current_id, rand_buf);
		if((i+1) != lines)
			current_id = xt->insert_after(xt, current_id, "a");
		if((i+1) != lines)
			write(fd[0], "\n", 1);
	}

	lseek(fd[0], 0, SEEK_SET);
	lseek(fd[1], 0, SEEK_SET);
	return 0;
}

int generate_before_file(int seed, struct text *xt, int *fd)
{
	if((fd[0] = open(TEST_FILE_NAME, O_RDWR | O_CREAT | O_EXCL , 0666)) < 0){
		perror("Problem with creating temporary file in generate_before_file");
		return -1;
	}
	if((fd[1] = open(TEST_FILE_NAME_SAVE, O_RDWR | O_CREAT | O_EXCL , 0666)) < 0){
		perror("Problem with creating temporary file in generate_before_file");
		return -1;
	}

	int lines = (rand() % TEST_FILE_LINES);
	for(int i = 0; i < lines; i++){
		write(fd[0], "aaaa", 4);
		if((i+1) != lines)
			write(fd[0], "\n", 1);
	}

	xt->set_fd(xt, fd[1]);
	xt->load_file(xt);
	line_id current_id = xt->get_first_line(xt);
	
	for(int i = 0; i < lines; i++){
		xt->set_text(xt, current_id, "bbbb");
		if((i+1) != lines)
			current_id = xt->insert_before(xt, current_id, "");
	}

	current_id = xt->get_first_line(xt);
	while(current_id){
		xt->set_text(xt, current_id, "aaaa");
		current_id = xt->next_line(xt, current_id);
	}

	lseek(fd[0], 0, SEEK_SET);
	return 0;
}

int traverse_test(struct text *xt)
{
	int fd;
	if((fd = open(TEST_FILE_NAME, O_RDWR | O_CREAT | O_EXCL , 0666)) < 0){
		perror("Problem with creating temporary file in generate_before_file");
		return -1;
	}

	xt->set_fd(xt, fd);
	xt->load_file(xt);
	int lines = 20;	
	line_id current_id = xt->get_first_line(xt);
	xt->set_text(xt, current_id, "aaaa");
	for(int i = 0; i < lines; i++){
			current_id = xt->insert_after(xt, current_id, "bbbb");
	}
	current_id = xt->insert_after(xt, current_id, "cccc");

	while(current_id != xt->get_first_line(xt)){
		current_id = xt->prev_line(xt, current_id);
	}

	if(strcmp("aaaa", xt->get_text(xt, current_id))){
		return -1;
	}

	while(current_id != xt->get_last_line(xt)){
		current_id = xt->next_line(xt, current_id);
	}

	if(strcmp("cccc", xt->get_text(xt, current_id))){
		return -1;
	}

	return 0;
}

int compare_files(int fda, int fdb)
{
	lseek(fda, 0, SEEK_SET);
	lseek(fdb, 0, SEEK_SET);
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
	
	/* Row Width Test */
	if(test_row_width(xt) < 0){
		return 0;
	}

	/* Basic Load and Save Test */
	printf("Starting Basic Load And Save Test\n");
	int lines;
	int fd = generate_file(1, &lines);	
	int fd_save;
	if((fd_save = open(TEST_FILE_NAME_SAVE, O_RDWR | O_CREAT | O_EXCL , 0666)) < 0){
		perror("Problem with creating temporary save file");
		return 1;
	}	
	xt->set_row_width(xt, TEST_LINE_LEN);
	xt->set_fd(xt, fd);
	xt->load_file(xt);
	if(xt->get_total_lines(xt) != lines){
		printf("Number of loaded lines does not match: %d vs %d!\n", xt->get_total_lines(xt), lines);
		return 1;
	}
	xt->set_fd(xt, fd_save);	
	xt->save_file(xt);
	if(compare_files(fd, fd_save) < 0){
		printf("Failed basic load and save test\n");
		return 1;
	} else {
		printf("Passed basic load and save test\n");
	}
	xt->unload_file(xt);
	unlink(TEST_FILE_NAME_SAVE);
	unlink(TEST_FILE_NAME);

	/* Insert Test 1*/		
	printf("Starting Insert Test 1\n");
	int fd_in[2];
	generate_after_file(1, xt, fd_in);
	if(!xt->is_dirty(xt)){
		printf("Failed insertion test 1: dirty bit not set!\n");
		return 1;
	}
	xt->save_file(xt);
	if(compare_files(fd_in[0], fd_in[1]) < 0){
		printf("Failed insertion test 1: Files are not equal!\n");
		return 1;
	} else {
		printf("Passed insertion test 1!\n");
	}
	xt->unload_file(xt);
	unlink(TEST_FILE_NAME_SAVE);
	unlink(TEST_FILE_NAME);
	
	/* Insert Test 2*/
	printf("Starting Insert Test 2\n");
	generate_before_file(1, xt, fd_in);
	if(!xt->is_dirty(xt)){
		printf("Failed insertion test 2: dirty bit not set!\n");
		return 1;
	}
	xt->save_file(xt);
	if(compare_files(fd_in[0], fd_in[1]) < 0){
		printf("Failed insertion test 2: Files are not equal!\n");
		return 1;
	} else {
		printf("Passed insertion test 2!\n");
	}
	xt->unload_file(xt);
	unlink(TEST_FILE_NAME_SAVE);
	unlink(TEST_FILE_NAME);

	/* Traverse Test */
	printf("Starting Traverse Test\n");
	if(traverse_test(xt) < 0){
		printf("Failed traverse test!\n");
		return 1;
	} else {
		printf("Passed traverse test!\n");
	}
	unlink(TEST_FILE_NAME);

	return 0;
}
