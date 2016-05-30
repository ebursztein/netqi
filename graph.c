/*!	NetQI: http://www.netqi.org
*	Copyright Elie bursztein & LSV, ENS-Cachan, CNRS, INRIA 
*
*/

/*!
 *       \file graph.c
 *       \brief dependency graph functions
 *
 *       function to manipulate dependency underlaying graph
 *       \author  Elie
 *       \version 1.2
 *       \date    Dec 2007
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

extern t_option 	option;
extern t_net		net;			///!<network structure
extern t_map		*map;			///!<map information : sets and nodes
extern t_player		*admin;			///!<admin player structure
extern t_player		*incident;		///!<incident player structure

/*!
 * init graph structures
 * \version 1.1
 * \date    2007
 * \author Elie
 */

void graph_init()
{
	int i;

	bzero(&net, sizeof(t_net));
	for(i = 0; i < NBNODE; i++)
	{
		net.node_cost[i] = option.default_node_value;
		net.node_loc[i] = NA;
		net.penality_counter[i] = FALSE;
	}
}

/*!
 * add graph relations
 * @param node1 is the node 1 of the relations
 * @param node2 is the node 2 of the relation or the cost
 * @param type is the nature of the relation
 * \version 1.3
 * \date   Jul 2008
 * \author Elie
 */

void graph_add_relation(int node1, int node2, int type) {
	
	switch (type) {
		case GRAPH_DEPENDENCY:
			if (option.debug == 6)
				out("adding dependency from %d -> %d\n", node1, node2);
			net.network_edge[node1][node2] = TRUE;
			break;
		case GRAPH_EQUIVALENCE:
			if (option.debug == 6)
				out("adding equivalence from %d | %d\n", node1, node2);
			net.network_node_relation[node1][node2] = TRUE;
			break;
		case GRAPH_COST:
			if (option.debug == 6)
				out("adding cost to %d = %d\n", node1, node2);
			net.node_cost[node1] = node2;
			break;
		case GRAPH_LOC:
			if (option.debug == 6)
				out("adding location relation %d is located at %d\n", node1, node2);
			net.node_loc[node1] = node2;
			break;		
		default:
			die("unknown relation");
			break;
	}
}

/*!
 * add graph label
 * @param node is the targeted node
 * @param label is the label to add
 * \version 1.0
 * \date    2008
 * \author Elie
 */


void graph_add_label(int node, char* label, int l) {
	
	if (net.node_label[node] != NULL)
		die("%s:%d:\tGraph syntax error:\t label '%s' is already defined for node %d\n", option.filename, l, net.node_label[node], node);
	label[strlen(label) -1] = '\0';
	net.node_label[node] = (char *)my_strndup(label, strlen(label));
}


/*!
 * graph randomize dependency relations
 * \version 1.0
 * \date    2007
 * \author Elie
 */

void graph_random_dep()
{
	assert(net.nbnode > 0);
	int i, j;
	if (option.debug == 1)
		out("equivalence random\n");
	//network init
	for(i = 0; i < net.nbnode; i++)
	{
		if(option.debug == 1)
			out("\n%d->", i);
		for(j = 0; j < net.nbnode; j++)
		{
			net.network_edge[i][j] 		= rand() % 2;
			if(option.debug == 1)
				out("%d=%d:", j, net.network_edge[i][j]);
		}
	}
}


/*!
 * graph randomize host cost
 * \version 1.1
 * \date    2007
 * \author Elie
 */

void graph_random_cost(int max)
{
	assert(net.nbnode > 0);
	int i;
	if (option.debug == 1)
		out("cost random\n");
	//network init
	for(i = 0; i < net.nbnode; i++)
	{	
		net.node_cost[i] 		= rand() % max;
		net.node_cost[i]  = net.node_cost[i] != 0 ? net.node_cost[i] :  1; 
		if(option.debug == 1)
			out("node %d cost is %d:\n", i, net.node_cost[i]);
	}
}


/*!
 * graph randomize node location
 * \version 1.0
 * \date    2007
 * \author Elie
 */

void graph_random_loc(int max)
{
	assert(net.nbnode > 0);
	int i;
	if (option.debug == 1)
		out("Location random\n");
	//network init
	for(i = 0; i < net.nbnode; i++)
	{	
		net.node_cost[i] 		= rand() % max;
		net.node_cost[i]  = net.node_cost[i] != 0 ? net.node_loc[i] :  1; 
		if(option.debug == 1)
			out("node %d location is %d:\n", i, net.node_loc[i]);
	}
}


/*!
 * graph randomize equivalence relations
 * \version 1.0
 * \date    2007
 * \author Elie
 */
void graph_random_eq()
{
	
	assert(net.nbnode > 0);
	int i, j;

	if(option.debug == 1)
		out("eq random\n");

	//network init
	for(i = 0; i < net.nbnode; i++)
	{
		if(option.debug == 1)
			out("\n%d->", i);
		for(j = 0; j < net.nbnode; j++)
		{
			net.network_node_relation[i][j] = rand() % 2;
			if(option.debug == 1)
				out("%d=%d:", j, net.network_node_relation[i][j]);

		}
	}
}
