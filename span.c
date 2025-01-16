#include <sys/stat.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include "span.h"

int find_length(char *file)
{
	int i = -1; //size does not include ending null character.
	utf8 uchar;
	do{
		i++;
		uchar = utf8_next(file);
		file += utf8_size(uchar);
	} while(!utf8_empty(uchar));
	return i;
}

char *find_offset(char *file, int index)
{
	int i = 0;
	utf8 uchar;
	while(i < index){
		i++;
		uchar = utf8_next(file);
		file += utf8_size(uchar);
	}
	return file;	
}

#define GET_MOCK(s) (struct mock_text*)((char *)s - offsetof(struct mock_text, super));
#define	GET_SPAN(s) (mock_span*)((char *)s - offsetof(mock_span, super))

void mtx_set_fd(struct stext *s, int fd)
{
	struct mock_text *mtx = GET_MOCK(s);
	mtx->fd = fd;
}

void mtx_nop(struct stext *s)
{
	return;
}

void mtx_load_file(struct stext *s)
{
	struct mock_text *mtx = GET_MOCK(s);
	struct stat stat;	
	fstat(mtx->fd, &stat);
	mtx->file = malloc(stat.st_size*2);
	mtx->size = stat.st_size;
	mtx->allocated = stat.st_size*2;	
	int offset = 0;
	int rl = 0;
	do{
		rl = read(mtx->fd, mtx->file+offset, mtx->size - offset);
		offset += rl;
	} while(rl != 0);
	mtx->file[mtx->size] = 0;
	mtx->length = find_length(mtx->file);
}

// span starts offempty, with end == start
span *mtx_get_span(struct stext *s, int offset)
{
	struct mock_text *mtx = GET_MOCK(s);
	if(mtx->length <= offset)
		return NULL;
	mock_span *new_span = malloc(sizeof(mock_span));	
	new_span->super.start = offset;
	new_span->super.end = offset;
	new_span->start = find_offset(mtx->file, offset);
	new_span->end = new_span->start;
	new_span->super.s = s;
	//new_span->end = new_span->start + utf8_size(utf8_next(new_span->start));
	return &(new_span->super);
}

void mtx_put_span(span *old_span)
{
	free(GET_SPAN(old_span));	
}

utf8 mtx_peek_span(span *sp)
{	
	mock_span *span = GET_SPAN(sp);
	return utf8_next(span->end);
}

int mtx_grow_span(span *sp)
{
	struct mock_text *mtx = GET_MOCK(sp->s);	
	if(sp->end >= mtx->length)
		return -1;
	sp->end++;			
	mock_span *span = GET_SPAN(sp);
	span->end += utf8_size(utf8_next(span->end));
	return 0;
}

// make this more efficient
utf8 mtx_index_span(span *sp, int index)
{
	mock_span *span = GET_SPAN(sp);
	assert(index >= 0);
	assert(index < (sp->end - sp->start));
	char *offset = find_offset(span->start, index);
	return utf8_next(offset);	
}

struct stext *make_mock_text(struct mock_text *mtx)
{	
	mtx->super.set_fd = mtx_set_fd;
	mtx->super.lock = mtx_nop;
	mtx->super.unlock = mtx_nop;
	mtx->super.load_file = mtx_load_file;
	mtx->super.get_span = mtx_get_span;
	mtx->super.put_span = mtx_put_span;
	mtx->super.index_span = mtx_index_span;
	mtx->super.peek_span = mtx_peek_span;
	mtx->super.grow_span = mtx_grow_span;
	mtx->fd = -1;
	return &mtx->super;
}

