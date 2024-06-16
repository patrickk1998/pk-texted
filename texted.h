#ifndef TEXTED_H
#define TEXTED_H

struct line_node{
	struct line_node *next, *prev;
	int width;
	char full;
	char *text;	
};

int create_line_list(char*);

void cleanup(int);

#endif /* TEXTED_H */


