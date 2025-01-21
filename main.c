#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "frontend/texted.h"
#include "frontend/input.h"
#include "frontend/display.h"
#include "backend/span.h"
#include <assert.h>

struct my_event{
	struct stext *xt;
	struct dstate *state;
	struct resize_event super;	
};

#define GET_EVENT(s) (struct my_event*)((char *)s - offsetof(struct my_event, super))

void my_resize_handler(struct resize_event *event)
{
	struct my_event *my_event = GET_EVENT(event);
	int h;
	int w;
	event->dis->get_size(event->dis, &w, &h);
	my_event->state->size.height = h;
	my_event->state->size.width = w;
	init_display(my_event->xt, event->dis, my_event->state);
}


int main(int argc, char* argv[])
{
	char default_file_name[] = "example/example1";
	char *file_name;
	char *tty_name = "\0";
	if(argc < 2)
		file_name = default_file_name;
	else 
		file_name = argv[1];
	
	if(argc > 2)
		tty_name = argv[2];	
	

	/* Intializing Display Object and Render */
	struct tty_display ttyd;	
	// Does not really matter if standard in or standard out as both 
	// should be connected to the same tty.
	if(tty_name[0] == '\0'){
		ttyd.fd = STDIN_FILENO; 
	} else {
		if((ttyd.fd = open(tty_name, O_RDWR)) < 0){
			perror("Problem opening tty device");
			return 1;
		}
	}
	struct display *dis = make_tty_display(&ttyd);		
	//int w, h;
	int w = 0;
	int h = 0;
//	struct resize_event my_resize_callback = { .dis = dis, .resize = my_resize_handler};
//	dis->resize_callback = &my_resize_callback;

	/* Open File */
	struct dstate state;
	struct mock_text mxt;
	struct stext *xt = make_mock_text(&mxt);

	struct my_event my_event = { .xt = xt, .state = &state, 
	.super = { .dis = dis, .resize = my_resize_handler}};
	dis->resize_callback = &my_event.super;

	int fd;
	if((fd = open(file_name, O_RDWR , 0666)) < 0){
		perror("Problem with opening text file");
		return 1;
	}		

	dis->open_display(dis, &w, &h);	
	xt->set_fd(xt, fd);
	xt->load_file(xt);

	/* Initalize the Display */
	state.size.height = h;
	state.size.width = w;
	init_display(xt, dis, &state);

	struct input_action action;
	while(1){
		get_action(&action, ttyd.fd);
		if(action.type == quit){
			break;
		}
		/*
		if(action.type != noop)
			update_state(&state, &action);
		render_state(&state, dis);
		*/
	}
	
	dis->close_display(dis);
	return 0;
}
