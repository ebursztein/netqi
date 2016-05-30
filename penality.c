/*!	NetQI: http://www.netqi.org
*	Copyright Elie bursztein & LSV, ENS-Cachan, CNRS, INRIA 
*
*/

/*!
 *       \file penality.c
 *       \brief time cost penality functions
 *
 *	function for time penality manipulation 
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
extern t_penality	*penality;			///!<penalitie structure

/*!
 * allocate a penality structure
 * \version 1.0
 * \date    2007
 * \author Elie
 * @return allocated penality structure
 * @see parse_penality
 */

t_penality	*penality_new() {
	
	t_penality	*y;
	
	y = xmalloc(sizeof(t_penality));
	bzero(y, sizeof(t_penality));
	return y;
}


/*!
 * add a penality structure
 * \version 1.0
 * \date    2007
 * \author Elie
 * @param y the penality structure to add
 * @return TRUE if sucess FALSE otherwise
 */

int	penality_add(t_penality *y) {
	
	assert(y);
	if (option.debug == 12)
		out("Added penalty for node %d with a value %d on set %d", y->node, y->value, y->set);
	y->next = penality;
	penality = y;
	return TRUE;
}
