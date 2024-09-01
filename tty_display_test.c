#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include "display.h"

#define RENDER_QUEUE_SIZE 2

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
	dis->put_line(dis, "Hello World", 0);
	dis->display_line(dis, 0);
	dis->put_line(dis, "This is just a test!", 2);
	dis->display_line(dis, 2);		

	int w, h;
	dis->get_size(dis, &w, &h);
	char cs[32];
	sprintf(cs,"columns: %d and rows: %d", w, h);
	dis->put_line(dis, cs, 5);
	dis->display_line(dis, 5);
}

void cursor_scene(struct display *dis)
{
	dis->put_line(dis, "The cursor should be here->", 3); 
	dis->display_line(dis, 3); 
	dis->set_cursor(dis, 3, 27);		
}

void goodbye_scene(struct display *dis)
{
	dis->put_line(dis, "***Goodbye World***", 0);
	dis->display_line(dis, 0);
}

int main()
{
	struct tty_display ttyd;	
	// Does not really matter if standard in or standard out as both 
	// should be connected to the same tty.
	ttyd.fd = STDIN_FILENO; 
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

	pausefor(15);
	
	render_queue[0] = goodbye_scene;
	render_scenes(dis);
	
	pausefor(3);

	dis->close_display(dis);
	printf("width: %d and height: %d\n", w, h);
	return 0;
}
