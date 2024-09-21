#include "texted.h"
#include "input.h"
#include <unistd.h>
#include <stdio.h>

#define TEST_QUEUE_MAX_LEN 20

#define ADD_TYPE(t) test_queue[tq_len].type = t; \
	tq_len++;

#define ADD_INSERT(v) test_queue[tq_len].type = insert; \
	test_queue[tq_len].value = v; \
	tq_len++;

int tq_len = 0;

struct input_action test_queue[TEST_QUEUE_MAX_LEN];

void write_action(int fd, struct input_action *action)
{
	char w;
	switch(action->type){
	case insert:
		write(fd, &action->value, 1);
		break;
	case quit:
		w = 'q' & 0x1f;
		write(fd, &w, 1);
		break;
	case backspace:
		w = 127;
		write(fd, &w, 1);
		break;
	case up:
		write(fd, "\033[A", 3);	
		break;
	case down:
		write(fd, "\033[B", 3);	
		break;
	case right:
		write(fd, "\033[C", 3);	
		break;
	case left:
		write(fd, "\033[D", 3);	
		break;
	case creturn:
		write(fd, "\015", 1);
		break;
	case escape:
		write(fd, "\033", 1);
	}
}

int is_same_action(struct input_action *a, struct input_action *b)
{
	if(a->type != b->type)
		return 0;

	if(a->type == insert){
		if(a->value != b->value)
			return 0;
	}

	return 1;
}

int main()
{
		

	// Make it that the program can write to it's own stdin.
	int p[2];
	pipe(p);
	close(STDIN_FILENO);
	dup(p[0]);		

	ADD_TYPE(quit);
	ADD_TYPE(right);
	ADD_INSERT('a');
	ADD_TYPE(left);
	ADD_TYPE(down);
	ADD_INSERT('d');
	ADD_TYPE(backspace);
	ADD_TYPE(creturn);
	ADD_TYPE(creturn);
	ADD_TYPE(escape);
	ADD_INSERT('z');
	ADD_TYPE(quit);
	ADD_INSERT('z');
	ADD_INSERT('y');
	ADD_INSERT('x');
	ADD_TYPE(backspace);
	ADD_TYPE(escape);
	ADD_INSERT('w');
	ADD_TYPE(creturn);
	ADD_TYPE(down);

	printf("STARTING %d TEST(S)\n", tq_len);
	struct input_action b;
	for(int i = 0; i < TEST_QUEUE_MAX_LEN && i< tq_len; i++){
		printf("test: %d\n", i + 1);
		write_action(p[1], test_queue + i);	
		get_action(&b);
		if(!is_same_action(&b, test_queue + i)){
			printf("FAILED TEST\n");
			return 1;	
		}
	}

	printf("PASSED ALL TESTS\n");
}

