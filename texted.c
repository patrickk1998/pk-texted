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

	cat_item(a->end, b->head, a->width);
	a->end->next = b->head->next;
	a->end->next->prev = a->end;
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
		item->next->prev = 0;
		list->head = item->next;
	} else if (item == list->end){
		item->prev->next = 0;
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

void cat_item(struct line_item* a, struct line_item* b, size_t width)
{
	int i = 0;
	for(; i + a->length < width && i < b->length; i++){
		a->text[i+a->length] = b->text[i];
	}
	a->length = i + a->length;
	a->text[a->length] = '\0';
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
		write_line(cur, (int)i);
		cur = cur->next;
		i++;
	}
}

enum inputAction get_action(char *to_insert)
{
	char b;
	enum inputAction action = noop;
	while( read(STDIN_FILENO, &b, 1) ){
		if(to_debug) dprintf(STDERR_FILENO, "Key is %d", (int)b);
		if(b == ('q' & 0x1f)){
			action = quit;
			break;
		}
		if(b == '\033'){
			action = escape_handle();
			break;
		}
		if(b == 127){
			action = backspace;
			break;
		}
		if(isgraph(b) || isblank(b)){
			action = insert;
			*to_insert = b;
			break;
		}
		if(b == 13){
			action = creturn;
			break;
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
	case backspace:
		if(state->cursorColumn > 0){
			int j = -1;
			char* text = state->cursorLine->text;
			for(int i = 0; i <= state->cursorLine->length ; i++){
				if(i != state->cursorColumn) j++;
				text[j] = text[i];
			}
			text[j+1] = '\0';
			state->cursorLine->length--;
			state->cursorColumn--;
		}
		if(state->cursorColumn == 0 && state->cursorLine->prev){
			struct line_item* prev = state->cursorLine->prev;
			cat_item(prev, state->cursorLine, state->llist.width);
			remove_line(&state->llist,state->cursorLine);
			state->cursorLine = prev;
			state->cursorRow--;
			state->cursorColumn = state->cursorLine->length;
		}
		break;
	case insert:
		;
		char *text = state->cursorLine->text;
		for(int i = state->cursorLine->length; i > state->cursorColumn; i--){
			text[i] = text[i-1];
		}
		text[state->cursorColumn] = state->to_insert;
		state->cursorLine->length++; // Make this into a function so text[length] is '\0'.
		text[state->cursorLine->length] = '\0';
		state->cursorColumn++;
		break;
	case creturn:
		;
		struct line_item *cl = state->cursorLine;
		struct line_item *nl = new_line(state->winColumns);
		int j = 0;
		for(int i = state->cursorColumn; i < cl->length; i++){
			nl->text[j] = cl->text[i];
			j++;
		}
		cl->text[state->cursorColumn] = '\0';
		nl->length = cl->length - state->cursorColumn;
		cl->length = state->cursorColumn;
		add_line_at(&state->llist, cl, nl);
		state->cursorRow++;
		state->cursorLine = state->cursorLine->next;
		state->cursorColumn = 0;
		break;
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

void write_line(struct line_item* line, int row)
{
	move_cursor(row,0);
	write(STDOUT_FILENO, line->text, line->length);
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
