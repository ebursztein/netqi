/*!	NetQI: http://www.netqi.org
*	Copyright Elie bursztein & LSV, ENS-Cachan, CNRS, INRIA 
*
*/

/*!
 *       \file map.c
 *       \brief set mapping function
 *
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

extern t_map 		*map;
extern t_net		net;
extern t_option 	option;

/*!
 * add a set to the map 
 * @param line the parsed line
 * @param size the length of the set name
 * @return return the numeric value of the set used
 * \version 1.0
 * \date   Sept 2007
 * \author Elie
 */
int	map_add(char *line, unsigned int size, int l)
{
	assert(line);
	assert(map != NULL);
	assert((map->nbset == 0 && map->set == NULL) || (map->nbset != 0 &&  map->set != NULL));
	assert(map->nbset >= 0);
	assert(size);
	
	t_set 	*s, *tmp = NULL;
	
	s = xmalloc(sizeof(t_set));
	bzero(s, sizeof(t_set));
	s->name = xmalloc(sizeof(char)*size + 1);
	strncpy(s->name, line, size);
	s->name[size] = '\0';
	s->next = NULL;
	if (map->nbset + 1 > NBSET)
		die("%s:%d:\tThe number of sets exceed the hard limit\n", option.filename, l); 
	
	//first set ?
	if(map->set == NULL)
	{
		map->set = s;
		s->id = map->nbset;
		map->nbset++;
		return s->id;		
	}
	
	tmp = map->set;
	while(tmp->next)
	{
		if(strcmp(tmp->name, s->name) == 0)
			die("%s:%d:\tSet %s already defined\n", option.filename, l, s->name); 
		tmp = tmp->next;
	}
	//cheking the last
	if(strcmp(tmp->name, s->name) == 0)
		die("%s:%d:\tSet %s already defined\n", option.filename, l, s->name); 
	//and clippit
	tmp->next = s;
	s->id = map->nbset;
	map->nbset++;
	if(option.debug == 2)
		out("biding set %s to id %d\n", s->name, s->id);
	return s->id;
}

/*!
 * init map
 * \version 1.0
 * \date   Sept 2007
 * \author Elie
 */
void	map_init()
{
	unsigned int i;

	//mapping
	map = malloc(sizeof(t_map));
	bzero(map, sizeof(t_map));
	map->set = NULL;
	//node relations
	for(i = 0; i < NBSET; i++)
		sets_allnode(i, FALSE);
}

/*!
 * get set id by name
 * @param str set name
 * \version 1.0
 * \date   Sept 2007
 * \author Elie
 */

int map_set_id_by_name(char *str)
{
	assert(str);
	assert(map);
	t_set	*s;
	char	*name;

	if(str[0] == '>' || str[0] == '!' || str[0] == '~')
	{
		name = str + 1;
	} else
		name = str;
	
	s = map->set;
	while(s)
	{
		if(strcmp(s->name, name) == 0)
			return s->id;
		s = s->next;
	}
	return -1;
}

/*!
 * return the meaning of a set value in plain text 
 * @param size of malloc
 * \version 1.0
 * \date    2007
 * \author Elie
 */

char	*map_set_state_by_id(int id)
{
	switch (id) {
		case TRUE: 	return "True";
		case FALSE: 	return "False";
		case SUCC: 	return "True for sucecessor";
		case NOTSUCC: 	return "False for successor";
		case NA:	return "non available";
		default:	return "unknown state";
	}
}

/*!
 * get set name by id
 * @param id set id
 * \version 1.0
 * \date   Sept 2007
 * \author Elie
 */

char *map_set_name_by_id(int id)
{
	t_set	*s;
	
	s = map->set;
	while(s)
	{
		if(s->id == id)
			return s->name;
		s = s->next;
	}
	return NULL;
}

/*!
 * randomize a set initial value(s)
 * \version 1.0
 * \date    2007
 * \author Elie
 */
void sets_randomize(unsigned int setnum)
{
	assert(net.nbnode > 0);
	int i,k;
	k = setnum;

	if (option.debug == 2)
		out("Setting every nodes of set %d(%s) to random\n", k, map_set_name_by_id(k));	

	
	for(i = 0; i < net.nbnode; i++)
	{	
		if(rand() % 2 == 0) {
			net.network_set[i][setnum] = TRUE;
		} else {
			net.network_set[i][setnum] = FALSE;
		}
	}
}

/*!
 * assign every set initial values to true or false
 * \version 1.0
 * \date    2007
 * \author Elie
 */
void sets_allnode(unsigned int setnum, int value)
{
	assert(net.nbnode > 0);
	int i, k;
	k = setnum;
	if (option.debug == 2)
		out("Setting every nodes of set %d (%s) to %s\n", k, map_set_name_by_id(k), value == TRUE ? "TRUE" : "FALSE");	

	for(i = 0; i < net.nbnode; i++)
	{	
		net.network_set[i][setnum] = value;
	}
}
