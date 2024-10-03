#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <string.h>
#include "display.h"
#include "codepoint.h"
#include <fcntl.h>
#include <stdlib.h>
#include <stdarg.h>


#define RENDER_QUEUE_SIZE 3

void (*render_queue[RENDER_QUEUE_SIZE])(struct display *);

void render_scenes(struct display * dis)
{
	for(int i = 0; i < RENDER_QUEUE_SIZE; i++){
		if(render_queue[i] != NULL)
			render_queue[i](dis);
	}
}

void my_resize_handler(struct resize_event *event)
{
	render_scenes(event->dis);
}

void pausefor(int seconds)
{
	struct timeval tmp;	
	gettimeofday(&tmp, NULL);
	time_t time_start = tmp.tv_sec;	
	while( tmp.tv_sec < (time_start + seconds)){
		sleep(1);
		gettimeofday(&tmp, NULL);
	}
}

void Hello_World(struct display *dis)
{
	dis->put_line(dis, stocp("Hello World🖖"), 0);
	dis->display_line(dis, 0);
	dis->put_line(dis, stocp("This is just a test!"), 2);
	dis->display_line(dis, 2);		

	int w, h;
	dis->get_size(dis, &w, &h);
	char cs[32];
	sprintf(cs,"¶ columns: %d and rows: %d", w, h);
	dis->put_line(dis, stocp(cs), 5);
	dis->display_line(dis, 5);

}

void cursor_scene(struct display *dis)
{
	dis->put_line(dis, stocp("The cursor should be here->"), 3); 
	dis->display_line(dis, 3); 
	dis->set_cursor(dis, 3, 27);		
}

void goodbye_scene(struct display *dis)
{
	dis->put_line(dis, stocp("***Goodbye World***"), 0);
	dis->display_line(dis, 0);
}

void color_scene(struct display *dis)
{
	codepoint *row = stocp("%%%%*%cBlack%cRed%cGreen%cYello%cCyan%cBlue%cPurple%c*%%%%", 
		color_black, color_red, color_green, color_yello, 
		color_cyan, color_blue, color_purple, color_default);
	dis->put_line(dis, row, 7);
	dis->display_line(dis, 7);
}

int main(int argc, char *argv[])
{
	struct tty_display ttyd;	
	if(argc < 2){
		// Does not really matter if standard in or standard out as both 
		// should be connected to the same tty device.
		ttyd.fd = STDIN_FILENO; 
	} else {
		if((ttyd.fd = open(argv[1], O_RDWR)) < 0){
			perror("Problem opening tty device");
			return 1;
		}		
	}
	struct display *dis = make_tty_display(&ttyd);
	struct resize_event my_resize_callback = { .dis = dis, .resize = my_resize_handler};
	dis->resize_callback = &my_resize_callback;
		
	int w, h;
	dis->open_display(dis, &w, &h);	

	render_queue[0] = Hello_World;
	render_scenes(dis);

	pausefor(3);

	render_queue[1] = cursor_scene;
	render_scenes(dis);
	
	pausefor(3);

	render_queue[2] = color_scene;
	render_scenes(dis);

	pausefor(10);

	dis->clear_display(dis);
	render_queue[0] = goodbye_scene;
	render_scenes(dis);
	
	pausefor(3);

	dis->close_display(dis);
	printf("width: %d and height: %d\n", w, h);
	return 0;
}
