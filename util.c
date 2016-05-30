/*!	NetQI: http://www.netqi.org
*	Copyright Elie bursztein & LSV, ENS-Cachan, CNRS, INRIA 
*
*/

/*!
 *       \file util.c
 *       \brief utility functions
 *
 *       misc function such as output function or bit manipulation
 *       \author  Elie
 *       \version 1.0
 *       \date    Sept 2007
 */
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>
#include "headers/constant.h"
#include "headers/structure.h"
#include "headers/function.h"

extern	t_option option;
extern t_net		net;				///!<network structure
extern t_map		*map;				///!<map information : sets and nodes
extern t_player		*admin;				///!<admin player structure
extern t_player		*incident;			///!<incident player structure




//portable strndup 
char *my_strndup(char *str,  unsigned int n) {
	char *rv = NULL;
	if(n == 0)
		return rv;
	rv = malloc(n + 1);
	memset(rv, 0, n + 1);
	memcpy(rv, str, n); 
	return rv; 
}


/*!
 * secure malloc 
 *
 * check the size and be sure malloc is working
 * @param size of malloc
 * \version 1.0
 * \date    2006
 * \author Elie
 */
void *xmalloc(size_t size)
{
	void *mem;
	if ((int) size <= 0) 
		die("mallocing negative amount of memory");
	mem = malloc(size);
	if (mem == NULL)
		die("Malloc failed, The system probably run out of space.\n");
	return mem;
}


/*!
 * exit facility 
 * @param error message
 * \version 1.0
 * \date    2006
 * \author Elie
 */
void die(char *err, ...) 
{
	va_list ap;

	if(option.fileout != stdout) {
		fprintf(option.fileout,"<error>\n\t");
			va_start(ap, err);
			vfprintf(option.fileout, err, ap);
			va_end(ap);
		fprintf(option.fileout, "</error>\n");
	}
	va_start(ap, err);
	fflush(stdout);
	vfprintf(stderr, err, ap);
	va_end(ap);
	exit(EXIT_FAILURE);
}

/*!
 * Warning facility 
 * @param error message
 * \version 1.0
 * \date    Feb 2007
 * \author Elie
 */
void warning(char *err, ...) 
{
	//no warning if in quiet mode
	if(option.quiet == 0)
		return;
	va_list ap;
	va_start(ap, err);
	//if(option.errout)
	//	vfprintf(option.errout, err, ap);
	//else
		vfprintf(stderr, err, ap);
	va_end(ap);
	//exit(EXIT_FAILURE);
}

/*!
 * unified output function 
 * @param str the string to output
 * \version 1.1
 * \date   Feb 2007
 * \author Elie
 */
int out(char *str, ...) 
{
	assert(str);
	//dowing a buffer might be nice
	int res;
	va_list ap;

	/*if(option.deamon > 0)
	{
		for(res  = FD_SETSIZE; res; res--)
			if (FD_ISSET(res, &active_fd_set))
				send(res, str, strlen(str), 0);
	} */
	
	va_start(ap, str);
	res = vfprintf(option.fileout, str, ap);
	va_end(ap);
	return res;
}

/*!
 * graph output function 
 * @param str the string to output to the graph file
 * \version 1.0
 * \date   Sept 2008
 * \author Elie
 */
int graph_out(char *str, ...) 
{
	assert(str);
	//dowing a buffer might be nice
	int res;
	va_list ap;

	/*if(option.deamon > 0)
	{
		for(res  = FD_SETSIZE; res; res--)
			if (FD_ISSET(res, &active_fd_set))
				send(res, str, strlen(str), 0);
	} */
	
	va_start(ap, str);
	res = vfprintf(option.graphfile, str, ap);
	va_end(ap);
	return res;
}

/*!
 * unified debug function 
 * @param lvl the debug lvl associated to the message
 * @param str the string to output
 * \version 1.1
 * \date   Mar 2007
 * \author Elie
 */
int debug(int lvl, char *str, ...) 
{
	assert(str);
	assert(lvl);
	if (option.debug  != lvl)
		return -1;
	va_list ap;
	va_start(ap, str);
	vfprintf(stderr, str, ap);
	va_end(ap);
	return 42;
}
