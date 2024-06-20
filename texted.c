#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "texted.h"

char to_debug = 0;

int create_line_list(char *filename, struct line_list *list)
{
	int fd = open(filename, 0); 
	if(fd == -1){
		perror(filename);
		return -1;
	}
	
	char read_buffer[RBUFFER_SIZE];
	
	ssize_t rdb;
	struct line_list tmp_list = { .width = list->width };
	while((rdb = read(fd, read_buffer, RBUFFER_SIZE))){
		memset(&tmp_list, 0, sizeof(struct line_list));
		tmp_list.width = list->width;
		proccess_rbuffer(read_buffer, (int)rdb, &tmp_list);
		merge_lists(list, &tmp_list);
	}

	return 0;
}

void proccess_rbuffer(char *rb, int len, struct line_list *list)
{
	int offset = 0;
	int start = 0;
	struct line_item* cur;
	if(to_debug)
		printf("len %d\n", len);
	while((offset + start) < len){
		// A more complicated width calculation is needed here, for utf-8 endcoding and software tabs.
		for(;rb[offset+start] != '\n' && (offset+start < len) && offset < list->width; offset++){

		}
		cur = new_line(list->width);
		strncpy(cur->text, rb+start, offset);
		if(to_debug)
			printf("added: %s\n", cur->text);
		add_line(list, cur, list->lines);
		// For now assume all the lines are less than the width.
		start = start + offset+1;
		offset = 0;
	}

}


void merge_lists(struct line_list *a, struct line_list *b)
{
	if(a == NULL ){
		a = b; // this is actualy an error
		return;
	}

	if(b == NULL)
		return;

	if(a->head == NULL){
		a->head = b->head;
		a->end = b->end;
		a->lines = b->lines;
		return;
	}

	//assume for now that now line is overflowing
	strcat(a->end->text, b->head->text);
	a->end->next = b->head->next;
	a->end->next->prev = a->end;
	a->end = b->end;
	free(b->head);
}

struct line_item* new_line(int width)
{
	struct line_item *item = (struct line_item*)malloc(sizeof(struct line_item));
	memset(item, 0, sizeof(struct line_item));
	// One is added to width so text is always a valid c string.
	item->text = (char*)malloc(width+1);
	memset(item->text, 0, width+1);
	return item;
}

void add_line(struct line_list *list, struct line_item *new_line, int pos)
{
	if(list->head == NULL){
		list->head = new_line;
		list->end = new_line;
	}	
	
	assert(pos >= 0 && pos <= list->lines);

	if(pos == 0){
		new_line->next = list->head;	
		list->head->prev = new_line;
		list->head = new_line;
		goto increment;
	}

	if(pos == list->lines){
		new_line->prev = list->end;
		list->end->next = new_line;
		list->end = new_line;
		goto increment;	
	}

	struct line_item* cur = list->head;
	for(int i = 0; i != pos; i++)
		cur = cur->next;
	
	new_line->next = cur;
	new_line->prev = cur->prev;
	cur->prev = new_line;		
	new_line->prev->next = new_line;
	goto increment;

	increment:
	list->lines++;
	return;

}

void traverse_list(struct line_list *list, int (*callback)(struct line_item*, long), long data)
{
	assert(callback);
	struct line_item *curr = list->head;
	while((*callback)(curr, data) && curr != list->end)
		curr = curr->next;
}

void read_envs()
{
	char *env_debug = getenv("TXED_DEBUG");
	if(env_debug == NULL)
		return;
	for(int i = 0; env_debug[i] != '\0'; i++){
		env_debug[i] = toupper(env_debug[i]);
	}

	if(!(strcmp(env_debug,"TRUE")))
		to_debug = 1;

}

void cleanup(int fd)
{
	close(fd);
}

/* DISPLAY FUNCTIONS */

struct termios termset_orig;

void disable_raw()
{
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &termset_orig);
}

void enable_raw()
{
	struct termios termset;

	tcgetattr(STDIN_FILENO, &termset);

	termset_orig = termset;

	termset.c_lflag &= ~( ECHO | ICANON | IEXTEN);
	termset.c_oflag &= ~(OPOST);
	termset.c_iflag &= ~(ICRNL | IXON);

	tcsetattr(STDIN_FILENO, TCSAFLUSH, &termset);

	atexit(disable_raw);

	return;
}

void after_display()
{
	char b;
	while( read(STDIN_FILENO, &b, 1) ){
		if(b == ('q' & 0x1f))
			exit(0);
	} 

	return;
}


void clean_screen()
{
	char control[] ="\e[?1049l";
	write(STDOUT_FILENO, control, strlen(control));

	SHOW_CURSOR();
}

void move_cursor(int row, int col)
{
 	char cs[16];
	int l = sprintf(cs,"\e[%d;%dH", row, col);
	write(STDOUT_FILENO, cs, l);

	return;
}

void write_line(char *line, int row)
{
	move_cursor(row,0);
	for(int i = 0; line[i] != '\0'; i++){
		// This is bad, need a buffer.
		write(STDOUT_FILENO, line+i, 1);
	}
}

