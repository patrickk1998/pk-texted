#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "texted.h"

int create_line_list(char *filename, struct line_list* list)
{
	int fd = open(filename, 0); 
	if(fd == -1){
		perror(filename);
		return -1;
	}
	
	char read_buffer[RBUFFER_SIZE];
	
	ssize_t rdb;
	struct line_item* cur;
	while((rdb = read(fd, read_buffer, RBUFFER_SIZE))){
		cur = new_line(list->width);	
		strcpy(cur->text, "An example!");
		add_line(list, cur, 0);
	}

	return 0;
}

struct line_item* new_line(int width)
{
	struct line_item* item = (struct line_item*)malloc(sizeof(struct line_item));
	memset(item, 0, sizeof(struct line_item));
	// One is added to width so text is always a valid c string.
	item->text = (char*)malloc(width+1);
	memset(item->text, 0, width+1);
	return item;
}

void add_line(struct line_list* list, struct line_item* new_line, int pos)
{
	if(list->head == NULL){
		list->head = new_line;
		list->end = new_line;
	}	
	
	if(pos < 0 || pos > list->lines)
		return;	

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

void traverse_list(struct line_list* list, int (*callback)(struct line_item*, long), long data)
{
	struct line_item* curr = list->head;
	while((*callback)(curr, data) && curr != list->end)
		curr = curr->next;
}

void cleanup(int fd)
{
	close(fd);
}



