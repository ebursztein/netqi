/*!	NetQI: http://www.netqi.org
*	Copyright Elie bursztein & LSV, ENS-Cachan, CNRS, INRIA 
*
*/
/*!
 *       \file game.c
 *       \brief game simulation functions
 *
 *       The functions used to run a simulation of an instance of the game
 *       \author  Elie
 *       \version 1.5
 *       \date    Aug 2008
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
extern t_net		net;				
extern t_map		*map;			
extern t_player		*admin;			
extern t_player		*incident;			
extern t_option		option;					
extern t_tree		*tree;			
extern t_tree		*root;	
extern t_tree		*last_branch;		
extern t_stat		stat;
extern t_penality	*penality;
extern t_strategy	*strategy;
extern t_formula	*formula;

	   int		successor = UNSET;			///!is a sucessor is needed


unsigned int		gtime;	

/*!
 * game initialisation function
 * \version 1.0
 * \date    2007
 * \author Elie
 */

void	game_init()
{
	gtime = 0;

	//Create an array from the array list
	rule_to_array();

	///\todo add various sorting here
	//order rules
	if(admin->nbrule)
		order_rule_set_by_time(admin);
	if(incident->nbrule)
		order_rule_set_by_time(incident);

	//assign proper id
	rule_set_id(admin);
	rule_set_id(incident);
	

	//display result if needed
	if  (option.debug == 3)
	{
            if(option.quiet != LATEX)
		out("\n-----------------------\nADMIN RULES\n-----------------------\n");
            else
                out("\\begin{figure}[htp]\n\t\\begin{center}\n\t\t\\[\n\t\t\t\\begin{array}{l}\n");

            player_rule_display(admin); 
            if(option.quiet != LATEX)
                out("\n-----------------------\nINCIDENT RULES\n-----------------------\n");		
            player_rule_display(incident);
            if(option.quiet == LATEX)
                out("\t\t\t\\end{array}\n\t\t\\]\n\t\\caption{Rules}\n\t\\label{rules}\n\t\\end{center}\n\\end{figure}");

	}
	
	//selecting first rules
	if(admin->rule == NULL)
		get_rule(admin, 0, 0, FALSE, FALSE);
	if(incident->rule == NULL)
		get_rule(incident, 0, 0, FALSE, FALSE);

}

/*!
 * select which rule will be used by a player
 * @param p player
 * @param start_rule from which rule to start
 * @param start_node from which node to start
 * @param last_rule  what is the last rule used
 * @param last_node  what is the last node targeted
 * @return TRUE is a rule is found, false otherwise
 * \version 1.1
 * \date    Aug 2008
 * \author Elie
 */

int	get_rule(t_player *p, int start_rule, int start_node, int last_rule, int last_node)
{
	assert(p);
	int                 res = 0;
	int                 i = 0;
        int                 tabou = -1;
        
 
	if (p->sorted_rule == NULL)
	{
		warning("A rules set is empty\n");
		return FALSE;
	}


        if(option.debug == 4 && tree) {
            out("State:%lld\tTime:%d\tP:%s", tree->num, gtime, p->name);
            out(" Searching rule from id:%s%d%s, node:%s%d%s\n", CYA, last_rule, CLR, GRE, last_node, CLR);
            if(tree->admin_rule && tree->admin_rule_status == TRUE)
                out("Admin Last rule was %s id %d targed %d", tree->admin_rule->name, tree->admin_rule->id, tree->admin_target);
            if(tree->incident_rule && tree->incident_rule_status == TRUE)
                out("Incident Last rule was %s id %d target %d \n", tree->incident_rule->name, tree->incident_rule->id, tree->incident_target);
        }
        
	for(i = start_rule; i < p->nbrule; i++)
	{	
            if (i == last_rule)
                tabou = last_node;
            else
                tabou  = -1;
            
		if ((res = can_use_rule(p->sorted_rule[i], start_node, tabou)) != FALSE)
		{
			//store it
			p->rule = p->sorted_rule[i];
			p->time = p->rule->time;
			p->ellapsed  = 0;
			p->rule_status = PENDING;
			p->target = res;
			p->target_succ = successor;
			/*if(tree->num  >1630)
			out("STATE\t:get_rule before node add\n");*/
			tree_node_add();	//saving selected rules
			return TRUE;
		}
		//restarting from node 0 for the next rule
		start_node = 0;
	} 
        
	return FALSE;
}








/*!
 * called when  the complet game tree is exhausted. it is the final procedure
 * It take care of the diplay, stat computation and select correct result
 * \version 1.0
 * \date    2007
 * \author Elie
 */

int game_final()
{
	//used to add closing xml tag if needed and results tags
	output_footer_progress();
	if(option.mode == SIMULATION)
		tree_branch_display(root);
	if(option.mode == STRATEGY)
		output_strategy();
	if(option.mode == MODELCHECK) {
		output_modelcheck();
	}
	output_stat(FINAL);
	output_footer();
		
		exit(42);
}



int game_find_new_rule()
{

	if(tree->admin_rule_status == FALSE || tree->admin_rule_status == TRUE || tree->incident_rule_status == FALSE || tree->incident_rule_status == TRUE)
			return FALSE;
	//select new configuration -> to change rule we need to ensure that 1 it is a selection node and two that the time ellapsed  is 0 
	if(tree->admin_rule_status == PENDING && tree->admin_ellapsed == 0)
	{
		//out("%d->rule:%d/%d status:%d\n",tree->num, tree->admin_rule->id +1, admin->nbrule, tree->admin_rule_status);
		//is there an other option ?;
		if(get_rule(admin, tree->admin_rule->id, tree->admin_target + 1, FALSE, FALSE) == TRUE)
			//yes then let's resume
			return TRUE;
	}
	
	//select new configuration -> to change rule we need to ensure that 1 it is a selection node and two that the time ellapsed  is 0 
	if(tree->incident_rule_status == PENDING && tree->incident_ellapsed == 0)
	{
		//is there an other option ?
		if(get_rule(incident, tree->incident_rule->id, tree->incident_target + 1, FALSE, FALSE) == TRUE)
			return TRUE;
	}
	return FALSE;
}


/*!
 * called when a play is finish. Used to verify property and output result
 * \version 1.3
 * \date    Aug 2008
 * \author Elie
 */

int game_backtrack()
{
	///\todo verify property
	///\todo rework the output

	t_tree		*last = NULL;
	int			res;
	
	stat.exec++;
	
	//if(stat.exec == 3)
		//die("3 exec");
	//one instance is enought let's go to final procedure
	if(option.mode == SIMULATION)
		game_final();
	
	//verifying if the execution is more interessting 
	if(option.mode == STRATEGY)
		strategy_evaluate_execution();
	
	//verifying if for the execution the TATL formula hold
	if(option.mode == MODELCHECK) {
		if(formula_eval(formula, tree, TRUE) == FALSE) {
				debug(15, "Formula is false at the end of execution %d\n", stat.exec);
				game_final();
		}
	}
	///\note the branch detection loop is used for debug to stop the analyzer when it loop in same scenario. Thus in normal use it is not used 
	/*
	if(option.debug == 7)
		out("comparing branch\n");
	//check for branch loop
	if ((tree_branch_eq(root, last_branch) == TRUE))
	{
		if(option.debug == 8) {
			out("last-branch\n");
			tree_branch_display(root);
		}
		//branch loop detected meaning we have exhausted all the possibility
		game_final();
	} else {
		//save it
		last_branch = tree_branch_last_save(last_branch, root);
	}
	*/

	
	if (option.debug == 8)
		tree_branch_display(root);
	//backtrack
	last = tree;

	tree_climb_up();
	
	if (last != root)
		tree_node_remove(last);
	else
		game_final();

	while( (res = game_find_new_rule()) != TRUE)
	{;
		last = tree;
		tree_climb_up();
		if(last != root)
			tree_node_remove(last);
		else
			game_final();
	}
	
	/*if(tree->num == 1634)
		out("%sSTATE:%d:\tbacktrack before return:\t rule:%d/%d target:%d/%d  status:%d/%d ellapsed:%d/%d %s\n",RED,tree->num,
		    admin->rule->id, tree->admin_rule->id,
      		 admin->target,  tree->admin_target,
	 	admin->rule_status, tree->admin_rule_status,
		    admin->ellapsed, tree->admin_ellapsed, CLR);*/
	
	return 42;
}


/*!
 * run the timed game
 * \version 1.0
 * \date    2007
 * \author Elie
 */

void game_run()
{
	int true = TRUE;
	unsigned int time_exec = 0; 

	while(true)
	{	
		if(!((admin->rule != NULL && admin->time == 0) || (incident->rule != NULL && incident->time == 0)))
		{
		gtime++;

		//penality 
		if(penality != NULL)
			utility_function_penality();
		//selecting rules 
		if(admin->rule == NULL)
			get_rule(admin, 0, 0, FALSE, FALSE);
		else
			admin->ellapsed++;
		
		if(incident->rule == NULL)
			get_rule(incident, 0, 0, FALSE, FALSE);
		else
			incident->ellapsed++;
		} 
		//no more rules can be applied -> let's backtrack
		if (incident->rule == NULL && admin->rule == NULL)
		{
		/*	if(tree->num  >1630)
				out("STATE%d\t:get_rule backtrack\n",tree->num);*/
			game_backtrack();
		}

		//firing rules
		if (incident->ellapsed >= incident->time && incident->rule)
		{

			//fireable ?
			if (test_rule(incident->rule, incident->target) == TRUE) {
				incident->rule_status = TRUE;
			} else {
				incident->rule_status = FALSE;
			}
			utility_function(incident);
			//output
// 			if(tree->num > 1630)
// 				out("STATE\t:game run before node add\n");
			if (tree_node_add() != LOOP) {
				exec_rule(incident, incident->rule_status);
				//select a  new rule if it is still usefull
                               // if (option.mode == STRATEGY && strategy_early_cut() == TRUE)
                               //get_rule(incident, 0, 0, FALSE, FALSE);

                               get_rule(incident, 0, 0, tree->incident_rule->id, tree->incident_target);

                        }
		}

		if (admin->ellapsed >= admin->time && admin->rule)
		{	
				
			//fireable ?
			if (test_rule(admin->rule, admin->target) == TRUE) {
				admin->rule_status = TRUE;
			} else {
				admin->rule_status = FALSE;
			}
			utility_function(admin);
			//output
	/*		if(tree->num > 1630)
				out("STATE\t:game run before node add\n");*/
			if (tree_node_add() != LOOP) {
				exec_rule(admin, admin->rule_status);
				//select a new rule
                                //if (option.mode == STRATEGY && strategy_early_cut() == TRUE)
                                  //  return;
                                 //get_rule(admin, 0, 0, FALSE, FALSE);

                                get_rule(admin, 0, 0, tree->admin_rule->id, tree->admin_target);

                        }
		}
		


		if(option.timeout && option.mode_opt == INSTANCE)
		{
			time_exec = time(NULL);
			if (time_exec - option.start_time > option.timeout)
			{
				output_timeout();
				game_final();
				return;
			}
		}
		//realtime
		if(option.delay)
			usleep(option.delay);	
	}
}

