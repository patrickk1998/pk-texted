/* INPUT FUNCTIONS */ /* <-- Not tested, but look okay */

#ifndef INPUT_H
#define INPUT_H

// Seperate keyboard input from user actions.
enum input_type{
	noop,
	quit,
	up,
	down,
	left,
	right,
	backspace,
	insert,
	creturn,
};

struct input_action{
	enum input_type type;
	char value;
};

void get_action(struct input_action *);

#endif /* INPUT_H */
