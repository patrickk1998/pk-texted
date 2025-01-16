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
#include "display.h"
#include "text.h"
#include "span.h"

/* New span stuff implemented here */

void init_display(struct stext *xt, struct display *dis, int h, int w)
{	
	xt->lock(xt);
	

	// first we get the spans for each row
	span **spans = malloc(sizeof(span *)*h);				
	int offset = 0; // utf8 offset
	for(int row = 0; row < h; row++){
		spans[row] = xt->get_span(xt, offset);
		if(spans[row] == NULL)
			break;
		for(int col = 0; col < w; col++){
			utf8 uchar = xt->peek_span(spans[row]);		
			if(utf8_empty(uchar))
				break;
			if(utf8_compare(uchar, '\n')){
				offset++;
				break;
			}
			xt->grow_span(spans[row]);
			offset++;
		}		
	}


	// Then we convert the spans into code points;
	display_str str;
	str.codepoints = malloc(sizeof(codepoint) * w);
	for(int row = 0; row < h; row++){
		if(spans[row] == NULL)
			break;
		for(int col = 0; col < w; col++){
			if(col == (spans[row]->end - spans[row]->start))
				break;
			utf8 uchar = xt->index_span(spans[row], col);		
			str.num = col + 1;
			codepoint_from_uchar(&str.codepoints[col], uchar);
		}
		// check if span is empty
		if(spans[row]->end - spans[row]->start == 0)
			str.num = 0;
		dis->put_str(dis, &str, row);
		dis->display_line(dis, row);
	}
	free(str.codepoints);

	xt->unlock(xt);
}

