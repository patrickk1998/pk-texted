#ifndef TEXTED_H
#define TEXTED_H

#define RBUFFER_SIZE 256

extern char to_debug;

struct line_item{
	struct line_item *next, *prev;
	char flag; // used to store size of text, and if line is full or not.
	char *text;	
};

struct line_list{
	struct line_item *head, *end;
	int width;
	int lines;
};

int create_line_list(char*, struct line_list*);

void proccess_rbuffer(char *, int, struct line_list*);

void merge_lists(struct line_list*, struct line_list*);

struct line_item* new_line(int);

void add_line(struct line_list*, struct line_item*, int);

void remove_line(struct line_list*, int);

void traverse_list(struct line_list*, int (*callback)(struct line_item*, long), long);

void read_envs();

void cleanup(int);

#endif /* TEXTED_H */


