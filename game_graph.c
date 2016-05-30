/*!	NetQI: http://www.netqi.org
*	Copyright Elie bursztein & LSV, ENS-Cachan, CNRS, INRIA 
*
*/

/*!
 *       \file  game_graph.c
 *       \brief output game graph function
 *
 *       Used to construct the game graph output
 *       \author  Elie
 *       \version 1.0
 *       \date    Sept 2008
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
extern unsigned int	gtime	;			///!<game time
extern t_tree		*root;				///!<tree roots
extern t_tree		*tree;				///!<current tree node
extern t_tree		*last_branch;			///!<last

//local function
void gamegraph_rule_info(t_tree *t, int id);

/*!
 * store node into the graphfile
 * @param  node to store
 * @return none
 * \version 1.0
 * \date    Sept 2008
 * \author Elie
 */

void gamegraph_store_node(t_tree *t) {

    graph_out("<node");
    graph_out("id=\"%lld\" fatherID=\"%lld\" ", t->num, t->father->num);
	if (t->incident_rule_status ==  TRUE || t->incident_rule_status == FALSE|| ((t->incident_rule_status  == PENDING) &&( t->incident_ellapsed == 0 ))) 	
			gamegraph_rule_info(t, INCIDENT);
	if (t->admin_rule_status ==  TRUE || t->admin_rule_status == FALSE || ((t->admin_rule_status  == PENDING) &&( t->admin_ellapsed == 0 )))
			gamegraph_rule_info(t, ADMIN);
    graph_out("></node>\n");
}



/*!
 * output  rule execution/failure data for each nod
 * @param t tree node
 * \version 1.0
 * \date    Oct 2008
 * \author Elie
 */

void gamegraph_rule_info(t_tree *t, int id)
{
	assert(t);
	if(id == ADMIN)
		graph_out("player=\"%s\" ",admin->name); 
	else 
		graph_out("player=\"%s\" ",incident->name); 



	if(id == ADMIN && t->admin_rule) {
			 
			if(t->admin_rule_status == TRUE) 		graph_out("status=\"execute\" ");
		else if (t->admin_rule_status == FALSE) 	graph_out("status=\"fail\" ");
		else if (t->admin_rule_status == PENDING) 	graph_out("status=\"choose\" ");
		
		graph_out("rule=\"%s\" ", t->admin_rule->name);
		
                if(net.node_label[t->admin_target] != NULL)
                        graph_out("node=\"%s\" ", net.node_label[t->admin_target], CLR);
				else
                        graph_out("node=\"%d\" ", t->admin_target);

/*
				//location
				if(t->admin_rule->loc != NA)
					out("@%s%s%s", YEL, location_get_name_by_id(t->admin_rule->loc), CLR);

                if(t->admin_target_succ) {
                        if(net.node_label[t->admin_target_succ] != NULL)
                                out(" succ %d:%s%s%s",  t->admin_target_succ, CYA, net.node_label[t->admin_target_succ], CLR);
                         else
                                out(" %d", t->admin_target_succ);
						//location
						if(t->admin_rule->sloc != NA)
							out("@%s%s%s", YEL, location_get_name_by_id(t->admin_rule->sloc), CLR);
				}

		
		if(option.mode == STRATEGY)
			if(t->admin_rule_status == FALSE || t->admin_rule_status == TRUE)
				out("\tReward %5lld Cost %5lld",t->admin_global_income, t->admin_global_cost);
		
		out("\n");
*/
	} else if (t->incident_rule){
		
			if(t->incident_rule_status == TRUE) 		graph_out("status=\"execute\" ");
		else if (t->incident_rule_status == FALSE)		graph_out("status=\"fail\" ");
		else if (t->incident_rule_status == PENDING) 	graph_out("status=\"choose\" ");
		
		graph_out("rule=\"%s\" ",t->incident_rule->name);
		
		if(net.node_label[t->incident_target] != NULL)
			graph_out("node=\"%s\" ",net.node_label[t->incident_target]);
		else 
			graph_out("node=\"%d\" ", t->incident_target);
		
		//location
		/*
		if(t->incident_rule->loc != NA)
				out("@%s%s%s", YEL, location_get_name_by_id(t->incident_rule->loc), CLR);
		
		if(t->incident_target_succ) {
			if(net.node_label[t->incident_target_succ] != NULL)
                        	out(" succ %d:%s%s%s", t->incident_target_succ, CYA, net.node_label[t->incident_target_succ], CLR);
               		 else
                        	out(" %d", t->incident_target_succ);
			//location
			if(t->incident_rule->sloc !=NA)
				out("@%s%s%s", YEL, location_get_name_by_id(t->incident_rule->sloc), CLR);
		}

		
		if(option.mode == STRATEGY)
			if(t->incident_rule_status == FALSE || t->incident_rule_status == TRUE)
				out("\tReward %5lld Cost %5lld", t->incident_global_income, t->incident_global_cost);
		out("\n");
		*/
	}
}
