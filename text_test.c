#include "text.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>

#define TEST_FILE_NAME ".pktexted_test_file"
#define TEST_FILE_NAME_SAVE ".pktexted_test_file_save"
#define TEST_FILE_LINES 500
#define TEST_LINE_LEN 80
#define THREADS_NUM 20

// Random Seeds
#define RAND1 1
#define RAND2 1

char rand_buf[TEST_LINE_LEN];
int rand_buf_len;

char rand_buf[TEST_LINE_LEN];
int rand_buf_len;

int test_row_width(struct text *xt)
{
	printf("Starting Row Width Test\n");	
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
	xt->put_line(xt, current_id);

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
	
	xt->set_text(xt, current_id, "aaaa");
	for(int i = 1; i < lines; i++){
		current_id = xt->insert_before(xt, current_id, "aaaa");
	}
	xt->put_line(xt, current_id);

	lseek(fd[0], 0, SEEK_SET);
	return 0;
}


int generate_traverse_file(struct text *xt)
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
	xt->put_line(xt, current_id);
	
	return fd;
}

struct traverse_test_data{
	struct text *xt;
	int res;
	int i;
};

void *traverse_test(void *p)
{			
	printf("Starting test thread %d\n", ((struct traverse_test_data *)p)->i);
	struct text *xt = ((struct traverse_test_data *)p)->xt;
	line_id end = xt->get_last_line(xt);
	line_id start = xt->get_first_line(xt); 
	line_id current_id = xt->get_line(xt, end);
	while(current_id != start){
		current_id = xt->prev_line(xt, current_id);
	}

	if(strcmp("aaaa", xt->get_text(xt, current_id))){
		((struct traverse_test_data *)p)->res = -1;
		return 0;
	}

	while(current_id != end){
		current_id = xt->next_line(xt, current_id);
	}

	if(strcmp("cccc", xt->get_text(xt, current_id))){
		((struct traverse_test_data *)p)->res = -1;
		return 0;
	}

	xt->put_line(xt, current_id);
	xt->put_line(xt, end);
	xt->put_line(xt, start);

	((struct traverse_test_data *)p)->res = 0;

	return 0;
}

int check_refcount(struct text *xt)
{
	printf("Checking For Reference Count of Zero\n");
	if(basic_refcount_zero(xt) < 0){
		printf("Lines with non-zero refcounts!\n");
		return -1;
	} else {
		printf("All lines have zero references\n");
		return 0;
	}
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

/*
	** Tests **

	There are five tests that cover the whole behavior of the struct text object.

	Row Width Test:
		Checks if row width setter and getter functions work correctly.

	Basic Load and Save Test:
		Checks if load and save function correctly. This tests generates
		a from a file from a random seed RAND1. Refcount on all lines should be zero at the end of the test.

	Insert Test 1:
		Checks if insert_after() function works correctly. The file contents are 
		generated are from random seed RAND2. Refcount on all lines should be zero at the end of the test.
	
	Insert Test 2:
		Checks if insert_before() function works correctly. Refcount on all lines should be zero at the end of the test.

	Traverse Test:
		Checks traversing the lines of the text object using next_line() and prev_line(). This is a multithreaded test 
		also checking that reference counting works in a mulit-threaded enviorment. Refcount on all lines should 
		be zero at the end of the test.
*/

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
	int fd = generate_file(RAND1, &lines);	
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
	check_refcount(xt);
	xt->unload_file(xt);
	unlink(TEST_FILE_NAME_SAVE);
	unlink(TEST_FILE_NAME);

	/* Insert Test 1*/		
	printf("Starting Insert Test 1\n");
	int fd_in[2];
	generate_after_file(RAND2, xt, fd_in);
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
	check_refcount(xt);
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
	check_refcount(xt);
	xt->unload_file(xt);
	unlink(TEST_FILE_NAME_SAVE);
	unlink(TEST_FILE_NAME);

	/* Traverse Test */
	printf("Starting Traverse Test\n");
	generate_traverse_file(xt);	
	struct traverse_test_data tt_data[THREADS_NUM];
	pthread_t tt_threads[THREADS_NUM];
	for(int i = 0; i < THREADS_NUM; i++){
		tt_data[i].xt = xt;
		tt_data[i].i = i;
		pthread_create(tt_threads + i, NULL, traverse_test, (void *)(tt_data + i));
	}
	for(int i = 0; i < THREADS_NUM; i++){
		pthread_join(tt_threads[i], NULL);
	}
	int tmp = 0;
	for(int i = 0; i < THREADS_NUM; i++){
		if(tt_data[i].res < 0)
			tmp = -1;
	}
	if(tmp < 0){
		printf("Failed Traverse Test!\n");	
	} else {
		printf("Passed Traverse Test!\n");	
	}
	check_refcount(xt);
	unlink(TEST_FILE_NAME);

	return 0;
}
