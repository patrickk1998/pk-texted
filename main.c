#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "texted.h"
#include "input.h"
#include "display.h"
#include "text.h"

int main(int argc, char* argv[])
{
	char default_file_name[] = "text/example1";
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
	dis->open_display(dis, &w, &h);	

	/* Open File */
	struct basic_text bxt;
	struct text *xt = make_basic_text(&bxt);
	int fd;
	if((fd = open(file_name, O_RDWR , 0666)) < 0){
		perror("Problem with opening file");
		return 1;
	}		
	xt->set_fd(xt, fd);
	xt->set_row_width(xt, w);
	xt->load_file(xt);

	/* Make State */
	struct displayState state;
	make_state(&state, xt, h, w);
	render_state(&state, dis);

	struct input_action action;
	while(1){
		get_action(&action, ttyd.fd);
		if(action.type == quit){
			break;
		}
		if(action.type != noop)
			update_state(&state, &action);
		render_state(&state, dis);
	}
	
	dis->close_display(dis);
	printf("cursor %d\n", state.cursorRow);
	printf("total lines %d\n", xt->get_total_lines(xt));
	return 0;
}
