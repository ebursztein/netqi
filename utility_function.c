/*!	NetQI: http://www.netqi.org
*	Copyright Elie bursztein & LSV, ENS-Cachan, CNRS, INRIA 
*
*/

/*!
 *       \file utility_function.c
 *       \brief game utility function routines
 *
 *       Used to compute execution cost Von Neumman utility function
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


//global
//global
extern t_net		net;				///!<network structure
extern t_map		*map;				///!<map information
extern t_player		*admin;				///!<admin player structure
extern t_player		*incident;			///!<incident player structure
extern t_option		option;				
extern t_stat		stat;
extern	unsigned int	gtime;				///!<game time
extern t_tree		*root;				///!<tree roots
extern t_tree		*tree;				///!<current tree node
extern t_penality	*penality;
extern t_formula	*formula;			
/*!
 * compute penality cost
 * @param  p the player
 * \version 1.0
 * \date    2007
 * \author Elie
 */
void	utility_function_penality()
{
	t_penality	*y;
	if (penality == NULL)
		return;
	
	y = penality;
	while(y != NULL) {
		if(net.network_set[y->node][y->set] == y->constraint)
		{
			if(y->law == ADD) {
				if(y->player == ADMIN){
					if(option.debug == 12)
						out("Penality inflicted for node %d to Admin adding %d \n", y->node, y->value);
					tree->admin_global_cost += y->value;
				} else {
					if(option.debug == 12)
						out("Penality inflicted for node %d to Incident adding %d\n", y->node, y->value);
					tree->incident_global_cost += y->value;
				}
			} else {
				die("this type of penalty is not supported");	
			}
			///\todo make the counter system working
		/*
			if(y->law == EXP) {
				net.penality_counter[y->counter_id]++;
				
			}
		*/
		}
		y = y->next;
	}
}

/*!
 * compute income and cost
 * @param  p the player
 * \version 1.0
 * \date    2007
 * \author Elie
 */


void utility_function(t_player *p) {
	assert(p);
	assert(p->rule);
	assert(p->rule_status == TRUE || p->rule_status == FALSE);
	
	int	res = FALSE;
	int	i;

	//out("%s%lld:%s  before current cost %lld for rule %s  status %d%s\n",CYA, tree->num + 1, p->name, p->rule_cost, p->rule->name, p->rule_status, CLR);
	
	//payoff
	if(p->rule_status == TRUE && p->rule->grant_reward == TRUE) {
		//does the effect of the rule target the successor ?
		for (i = 0; i < map->nbset; i++) {
			if(p->rule->effect_succ[i] != NA) {
				res = TRUE;
			}	
		}
				
		if (res != FALSE)
			p->rule_income += net.node_cost[p->target_succ];
		else
			p->rule_income	+= net.node_cost[p->target];
		/*if(p->target_succ != FALSE)
			p->rule_income += net.node_cost[p->target_succ];*/
	}
	

	//cost 
	p->rule_cost += p->rule->cost;
	
	//time
	p->last_time = gtime;
	
}

