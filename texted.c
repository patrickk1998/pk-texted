#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <assert.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "texted.h"

char to_debug = 0;

/* LIST MANIPULATION */

int create_line_list(char *filename, struct line_list *list)
{
	int fd = open(filename, 0); 
	if(fd == -1){
		perror(filename);
		return -1;
	}
	
	char read_buffer[RBUFFER_SIZE];
	
	ssize_t rdb;
	struct line_list tmp_list = { .width = list->width , .head = NULL};
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
		dprintf(STDERR_FILENO, "len %d\n", len);
	while((offset + start) < len){
		// A more complicated width calculation is needed here, for utf-8 endcoding and software tabs.
		for(;rb[offset+start] != '\n' && (offset+start < len) && offset < list->width; offset++){

		}
		cur = new_line(list->width);
		strncpy(cur->text, rb+start, offset);
		cur->length = (int)strlen(cur->text); // more complicated for utf-8 encoding and software tabs.
		if(to_debug)
			printf("added %ld: %s~\n", cur->length, cur->text);
		add_line(list, cur);
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

	//assume for now that no line is ove
	if(to_debug)
		printf("concat: %s + %s\n", a->end->text, b->head->text);

	strcat(a->end->text, b->head->text);
	a->end->next = b->head->next;
	a->end->next->prev = a->end;
	a->end->length = (int)strlen(a->end->text);
	a->end = b->end;
	free_line(b->head);
}

// This function is doing two things, insertion and append;
void add_line(struct line_list *list, struct line_item *new_line)
{
	if(list->head == NULL){
		list->head = new_line;
		list->end = new_line;
	} else {
		list->end->next = new_line;
		new_line->prev = list->end;
		list->end = new_line;
	}

	list->lines++;
	return;
}

void add_line_at(struct line_list* list, struct line_item* current, struct line_item* new)
{
	if(current->next){
		current->next->prev = new;
		new->next = current->next;
	} else {
		list->end = new;
	}
		current->next = new;
		new->prev = current;
	list->lines++;
}

void remove_line(struct line_list *list, struct line_item  *item)
{
	if(item == list->head){
		item->next->prev = item->prev;
		list->head = item->next;
	} else if (item == list->end){
		item->prev->next = item->next;
		list->end = item->prev;
	} else {
		item->prev->next = item->next;
		item->next->prev = item->prev;
	}

	free_line(item);
	list->lines--;
}

void traverse_list(struct line_list *list, int (*callback)(struct line_item*, long), long data)
{
	assert(callback);
	struct line_item *curr = list->head;
	while((*callback)(curr, data) && curr != list->end)
		curr = curr->next;
}

/* ITEM MANIPULATION */

struct line_item* new_line(int width)
{
	struct line_item *item = (struct line_item*)malloc(sizeof(struct line_item));
	memset(item, 0, sizeof(struct line_item));
	// One is added to width so text is always a valid c string.
	item->text = (char*)malloc(width+1);
	memset(item->text, 0, width+1);
	return item;
}

void free_line(struct line_item *item)
{
	free(item->text);
	free(item);
}

void dec_length(struct line_item* item)
{
	item->length--;
	item->text[item->length] = '\0';
}

void inc_length(struct line_item* item)
{
	item->length++;
	item->text[item->length] = '\0';
}

/* HELPER FUNCTIONS */

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

void get_size(struct displayState *state)
{
	struct winsize ws;
	ioctl(STDOUT_FILENO,TIOCGWINSZ, &ws);
	state->winRows = ws.ws_row;
	state->winColumns = ws.ws_col;

	if(to_debug) dprintf(STDERR_FILENO, "Window Size row:%hu col:%hu\n",
			state->winRows,state->winColumns);
}

void display_list(struct displayState *state)
{
	CLEAR_SCREEN();
	unsigned short i = 0;	
	struct line_item *cur = state->displayStart;
	while((i < state->winRows) && cur){
		//write(STDOUT_FILENO, cur->text, cur->length);
		write_line(cur->text, (int)i);
		cur = cur->next;
		i++;
	}
}

enum inputAction get_action()
{
	char b;
	enum inputAction action = noop;
	while( read(STDIN_FILENO, &b, 1) ){
		if(b == ('q' & 0x1f)){
			action = quit;
			break;
		}
		if(b == '\033'){
			action = escape_handle();
			break;
		}
		if(b == '\b'){
			action = backspace;
		}
	}

	if(to_debug)
		dprintf(STDERR_FILENO,"Action is %d\n", action);

	return action;
}


void update_state(enum inputAction action, struct displayState *state)
{
	if(to_debug)
		dprintf(STDERR_FILENO, "row: %d, col %d, text: %s\n",
				state->cursorRow, state->cursorColumn, state->cursorLine->text);

	switch(action){
	case quit:
		exit(0);
	case up:
		if(state->cursorRow > 0){
			state->cursorRow--;
			state->cursorLine = state->cursorLine->prev;
		}
		if(state->cursorRow == 0 && state->cursorLine->prev){
			state->cursorLine = state->cursorLine->prev;
			state->displayStart = state->displayStart->prev;

		}
		break;
	case down:
		if(state->cursorLine->next && state->cursorRow < (state->winRows-1)){
			state->cursorRow++;
			state->cursorLine = state->cursorLine->next;
		}
		if((state->cursorRow == (state->winRows - 1)) && state->cursorLine->next){
			state->cursorLine = state->cursorLine->next;	
			state->displayStart = state->displayStart->next;
		}
		break;
	case left:
		if(state->cursorColumn > 0)
			state->cursorColumn--;
		break;
	case right:
		state->cursorColumn++;
		break;
	}

	if(state->cursorLine->length < state->cursorColumn)
		state->cursorColumn = state->cursorLine->length;

	if(to_debug) dprintf(STDERR_FILENO, "After, row: %d, col %d, text: %s\n",
				state->cursorRow, state->cursorColumn, state->cursorLine->text);


 
}

void display_state(struct displayState *state)
{
	if(to_debug) dprintf(STDERR_FILENO,"moving cursor, row: %d, col %d\n",
			state->cursorRow, state->cursorColumn);
	display_list(state);
	move_cursor(state->cursorRow, state->cursorColumn);
	
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
	int l = sprintf(cs,"\e[%d;%dH", row+1, col+1);
	write(STDOUT_FILENO, cs, l);

	return;
}

void write_line(char *line, int row)
{
	move_cursor(row,0);
	int i = 0;
	for(; line[i] != '\0'; i++);
	write(STDOUT_FILENO, line, i);

}


enum inputAction escape_handle()
{
	enum inputAction action = noop;
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
