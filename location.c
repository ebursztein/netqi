/*!	NetQI: http://www.netqi.org
*	Copyright Elie bursztein & LSV, ENS-Cachan, CNRS, INRIA 
*
*/

/*!
 *       \file location.c
 *       \brief the set of function used for location 
 *
 *       handle the corespondance between location name and the numeric identifier
 *       \author  Elie
 *       \version 1.0
 *       \date    Jul 2008
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


//global
t_net		net;			///!<network structure
t_map		*map;			///!<map information : sets and nodes
t_player	*admin;			///!<admin player structure
t_player	*incident;		///!<incident player structure
t_option	option;			///!<store option 
t_stat		stat;			///!<statistic structure
t_tree		*last_branch;		///!<used for branch detection
t_tree		*root;			///!<tree roots
t_tree		*tree;			///!<current tree node
t_strategy	*strategy;		///!<strategies
t_formula	*formula;		///!<tatl formula used for modelcheck
t_penality	*penality;		///!<penalities

///private function
t_location *location_new(char *name);


/*!
 * return the id associated to a location name.
 * if the name is found the coresponding id is returned otherwise a new id is created
 * \version 1.0
 * \date    Jul 2008
 * \author Elie
 * @param name the location name
 * @return the numeric id associated to the location
 */
int location_get_id_by_name(char *name)
{
	assert(name);
	t_location *l;
	
	if(option.debug == 14)
		out("gettting id for '%s'\n", name);
	
	l = map->location;
	while(l) {
		if(strcmp(l->name, name) == 0) {
			if (option.debug == 14)
				out("found id  %d for location %s\n", l->id, l->name);
			return l->id;
		}
		l = l->next;
	}
	//not found add it
	l = location_new(name);
	l->next = map->location;
	map->location = l;
	if (option.debug == 14)
			out("mapping id %d for location %s\n", l->id, l->name);
	return l->id;

}


/*!
 * return the name associated to the location id
 * \version 1.0
 * \date    Jul 2008
 * \author Elie
 * @param id the location id
 * @return the location name
 */
char  *location_get_name_by_id(int id)
{
	t_location *l;
	
	if (id == 0)
		return "";

	if (id == NA)
		return "";

	l = map->location;
	while(l) {
		if(l->id == id) 
			return l->name;
		l = l->next;
	}
	//not found it, since the only unded location is 0 something goes terribly wrong
	die("Something went wrong: can't find anddd location name for the id %d\n", id);
	return "";
}




/*!
 * return a new location structure properly initilized
 * init the name and numerical id
 * \version 1.0
 * \date    Jul 2008
 * \author Elie
 * @param name the location name
 * @return the new initialized location
 */
t_location *location_new(char *name) {
	assert(name);
	
	t_location *l;
	l = xmalloc(sizeof(t_location));
	bzero(l, sizeof(t_location));
	l->name = xmalloc(sizeof(char)*strlen(name) + 1);
	strncpy(l->name, name, strlen(name));
	l->name[strlen(name)] = '\0';
	l->id = ++map->nblocation;
	return l;
}
