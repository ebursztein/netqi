/*!	NetQI: http://www.netqi.org
*	Copyright Elie bursztein & LSV, ENS-Cachan, CNRS, INRIA 
*
*/
/*!
 *       \file strategy.c
 *       \brief strategy functions
 *
 *       To parse and select and display strategy
 *       \author  Elie
 *       \version 1.0
 *       \date    Oct 2007
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
extern t_net		net;				///!<network structure
extern t_map		*map;				///!<set name mapping
extern t_player		*admin;				///!<admin player structure
extern t_player		*incident;			///!<incident player structure
extern t_option		option;	
extern t_strategy	*strategy;
extern t_stat		stat;
extern t_tree		*tree;			
extern t_tree		*root;	
extern t_tree		*last_branch;		

//local dirt
int strategy_execution_is_better(t_strategy *s);
int strategy_evaluate_goal(t_strategy *s, int goal_id);
void strategy_store(t_strategy *s);
int strategy_evaluate_cond(t_tree *branch_root, int set, int cond, int loc, int loc_cond);
int strategy_evaluate_state_cond(t_tree *t, int set, int cond, int loc, int loc_cond);
int strategy_early_eval(); ///!<used by strategy early cut to evaluate if condition allow an early cut
int founded = FALSE; ///!<does a suitable strategy already present ?
int current_stalled = FALSE;  ///!<is the current execution stalled or down ?
int stalled_count = 0; ///!<For how many play are we stalled ? used to know that it worst keeping this way 
/*!
 * setup a new strategy structure
 * \version 1.0
 * \date    2007
 * \author Elie
 */
t_strategy *strategy_new() {
	
	t_strategy 	*s;
	unsigned int 	i =  0;
	
	s = xmalloc(sizeof(t_strategy));
	bzero(s, sizeof(t_strategy));
	
	s->loc = NA;
	for(i = 0; i < NBGOAL; i++)
		s->goal[i] = NA;
	
	//one condition by set
	for(i = 0; i < NBSET; i++)
		s->cond[i] = NA;
	s->path_cond = FALSE;
	return s;
}

/*!
 * Change game goal value when loop is detected
 * \version 1.0
 * \date    2007
 * \author Elie
 */
void	strategy_evaluate_loop() {
	if (admin->rule_income > 0)
		admin->rule_income = INFINITY;
	if (incident->rule_income > 0)
		incident->rule_income = INFINITY;
	if(admin->rule_cost > 0)
		admin->rule_cost = INFINITY;
	if(incident->rule_cost > 0)
		incident->rule_cost = INFINITY;
	if(admin->last_time > 0)
		admin->last_time = 0;
	if(incident->last_time > 0)
		incident->last_time = 0;
}

/*!
 * verify is early cut can be made 
 * \version 1.0
 * \date    2007
 * \author Elie
 * @return TRUE is a cut is made , FALSE otherwise
 * @see game_run()
 */

int strategy_early_cut() {
	

	///\todo do early cut for multiple strategies
	///\note stategy early cut evaluation
	//if(option.mode == STRATEGY && option.mode_opt == ABSTRACTINTER) {
	if(net.nbstrat == 1 && strategy->path_cond != FALSE) {
		if(strategy_early_eval() == FALSE)  {
			game_backtrack();
			return TRUE;
		}
	}
	//}
	return FALSE;
}



/*!
 * Evaluate at each step is the current execution can stil be a candidate
 * \version 1.0
 * \date    2007
 * \author Elie
 * @return TRUE if it is still a candidate, false otherwise
 * @see strategy_early_cut()
 */
int strategy_early_eval() {
	t_strategy	*s;
	int j;
	
	s = strategy;
	
	///!\todo do multiple box and until condition evaluation
	///!\todo add cost cut : if MIN(cost) is a primary goal and current cost is upper then cut
	//evaluate condition
	for(j = 0; j < map->nbset; j++) {
		if(s->cond[j] != NA) {
			if(strategy_evaluate_cond(root, j, s->cond[j], s->loc, s->loc_cond) == FALSE) {
				
				if(s->cond[j] == PALLFALSE || s->cond[j] == PALLTRUE || s->cond[j] == PFALSE || s->cond[j] == PTRUE) {
						debug(13,"%sExecution %lld early 'BOX' cut because on set %s cond %d is not meet%s\n",CYA, stat.exec, map_set_name_by_id(j),s->cond[j], CLR);
				 		return  FALSE;
				}
			} else {	
				if (s->cond[j] == UALLFALSE || s->cond[j] == UALLTRUE || s->cond[j] == UFALSE || s->cond[j] == UTRUE) {
						debug(13,"%sExecution %lld early 'UNTIL' cut because on set %s cond %d is not meet%s\n",CYA, stat.exec, map_set_name_by_id(j),s->cond[j], CLR);
					return  FALSE;
				}
				
			}
		}
	}
	return TRUE;
}

/*!
 * Evaluate if an execution is suitable for one of the researched strategy
 * \version 1.0
 * \date    2007
 * \author Elie
 */

void strategy_evaluate_execution() {

	t_strategy	*s;
	
	s = strategy;
	
	while(s)
	{
		if(strategy_execution_is_better(s) == TRUE)
		{
			strategy_store(s);
			//if (option.mode_opt == INSTANCE) ;
			//	game_final();
		} else {
			///\todo break point analysis/optim to add here
			//using breakpoint analysis ??
			if(option.mode_opt == INSTANCE)
			{	
				debug(45, "Stalled %d\n", stalled_count);
				//breakpoint is passed no more hope
				if(founded == TRUE && (current_stalled == FALSE || stalled_count > STALLED_MAX))
					game_final();

			}
		}
		s = s->next;
	}

}

/*!
 * Evaluate if an execution is better that previous one for a given strategy
 * \version 1.0
 * \date    2007
 * \author Elie
 */

int strategy_execution_is_better(t_strategy *s)
{
	unsigned int	i;
	int 		j;
	int	res = 0;
	
	assert(s);

	 
	if(tree->loop == TRUE)
		return FALSE;
	//fastest test first : goal value
	for(i = 0; i < s->nbgoal; i++)
	{
		res = strategy_evaluate_goal(s, s->goal_order[i]);
	
		//it is not an improvement
		if(res < 0){
			if(option.debug == 11)
		 		out("%lld:strat:down\n", stat.exec);
			stalled_count = 0;
			current_stalled = FALSE;
			return FALSE;
		}
		//it is an improvement
		else if(res > 0)
		{
			if(option.debug == 10) {
				out("%sExecution %lld is better because of goal %s%s\n",GRE, stat.exec, strategy_goal_name_by_id(s->goal_order[i]), CLR);	
				output_strategy_value_stdout(s);
			}
			
			//does it meet our condition ?
			for(j = 0; j < map->nbset; j++) {
				if(s->cond[j] != NA) {
					if(strategy_evaluate_cond(root, j, s->cond[j], s->loc, s->loc_cond) == FALSE) {
						if(option.debug == 10)
							out("%sExecution %lld is refused because on set %s cond %d is not meet%s\n",GRE, stat.exec, map_set_name_by_id(j),s->cond[j], CLR);
						if(option.debug == 11)
							out("%lld:strat:down:cond\n", stat.exec);	
						current_stalled = FALSE;
						stalled_count = 0;
						return FALSE;
					}
				}
			}
			
			if(option.debug == 11)
				out("%lld:strat:up\n", stat.exec);
 			founded = TRUE;
			return TRUE;
		}
		//if res == 0 then for this goal it is equivalent so we keep going to the next
		if(res == 0)
			if(option.debug == 11)
				out("%lld:strat:stalled\n", stat.exec);
		current_stalled = TRUE;
		stalled_count++;
	}
	return FALSE;
}

/*!
 * Evaluate if for a  given goal the current execution is better
 * \version 1.0
 * \date    2007
 * \author Elie
 */

int strategy_evaluate_goal(t_strategy *s, int goal_id)
{
	unsigned long long stored 	= 0;
	unsigned long long current 	= 0;

	assert(s);
	assert(tree);
	
	//Cost
	if (goal_id == COST) {	
		if(s->player == ADMIN)
			current = tree->admin_global_cost;
		else
			current = tree->incident_global_cost;
		stored = s->cost;
	}

	else if (goal_id == OCOST) {	
		if(s->player == ADMIN)
			current = tree->incident_global_cost;
		else
			current = tree->admin_global_cost;
		stored = s->ocost;
	}

	//Reward
	else if (goal_id == REWARD) {	
		if(s->player == ADMIN)
			current = tree->admin_global_income;
		else
			current = tree->incident_global_income;
		stored = s->reward; 


	}

	else if (goal_id == OREWARD) {	
		if(s->player == ADMIN)
			current = tree->incident_global_income;
		else
			current = tree->admin_global_income;
		stored = s->oreward; 
	}
	
	//TIME
	else if (goal_id == TIME) {	
		if(s->player == ADMIN)
			current = tree->admin_global_last_time;
		else
			current = tree->incident_global_last_time;
		stored = s->time; 
	}

	else if (goal_id == OTIME) {	
		if(s->player == ADMIN)
			current = tree->incident_global_last_time;
		else
			current = tree->admin_global_last_time;
		stored = s->otime; 
	}
	
	//printf("%lld, %lld\n", current, stored);
	if(s->goal[goal_id] == MAX)
	{
		     if(current > stored)	return 1;
		else if(current < stored)	return -1;
		else return 0;
	} else {
		//0 will be alway the minimum we have to accept the higher to decrease
		if (stored == 0)
			return 1;
		     if(current < stored)	return 1;
		else if(current > stored)	return -1;
		else return 0;
	}
	
}

/*!
 * verify if a condition over path or leaf 
 * 
 * This function implement the CTL box and leaf operator for scenario condition
 * 
 * \version 1.1
 * \date    2007
 * \author Elie
 */


int strategy_evaluate_cond(t_tree *branch_root, int set, int cond, int loc, int loc_cond) {

	assert(branch_root);
	assert(set < map->nbset);
	
	int	check = 0;
	t_tree *t;
	
	
	t = branch_root;
	
	        if(cond == LFALSE || cond == LTRUE || cond == LALLFALSE || cond == LALLTRUE) 	check = LEAF;
	else if(cond == UFALSE || cond == UTRUE || cond == UALLFALSE || cond == UALLTRUE) 	check = UNTIL;
	else 
		check = PATH;
	
	///\note we check only effective rule execution cause otherwise it is obvious that set haven't change
	if(check == PATH  && (t->admin_rule_status == TRUE || t->incident_rule_status == TRUE))
		if(strategy_evaluate_state_cond(t, set, cond, loc, loc_cond) == FALSE)
			return FALSE;
	
	if(check == UNTIL  && (t->admin_rule_status == TRUE || t->incident_rule_status == TRUE))
		if(strategy_evaluate_state_cond(t, set, cond, loc, loc_cond) == TRUE)
			return TRUE;
	
	while(t->child)
	{
		t = t->child;
		/*while(t->sibilin)
			t = t->sibilin;
		*/
		if(check == PATH && (t->admin_rule_status == TRUE || t->incident_rule_status == TRUE))
			if(strategy_evaluate_state_cond(t, set, cond, loc, loc_cond) == FALSE)
				return FALSE;
	
		if(check == UNTIL  && (t->admin_rule_status == TRUE || t->incident_rule_status == TRUE))
			if(strategy_evaluate_state_cond(t, set, cond, loc, loc_cond) == TRUE)
				return TRUE;
	}
	
	if(check == LEAF && (t->admin_rule_status == TRUE || t->incident_rule_status == TRUE))
		if(strategy_evaluate_state_cond(t, set, cond, loc, loc_cond) == FALSE)
			return FALSE;
	
	if(check == UNTIL  && (t->admin_rule_status == TRUE || t->incident_rule_status == TRUE))
		if(strategy_evaluate_state_cond(t, set, cond, loc, loc_cond) == TRUE)
			return TRUE;
	
	if (check != UNTIL)
		return TRUE;
	else 
		return FALSE;
}


/*!
 * verify if a condition is verify for a give state
 * \version 1.1
 * \date    2007
 * \author Elie
 */


int strategy_evaluate_state_cond(t_tree *t, int set, int cond, int loc, int loc_cond) {
	
	assert(t);
	//assert(t->network_set); might be false due to the new memory model
	assert(set < map->nbset);

	int i = 0;

	//if one FALSE ok
	if(cond == PFALSE || cond == LFALSE  || cond == UFALSE)
	{
		for(i = 0; i < net.nbnode; i++)
		{
			if(loc != NA) {
				//restrict to location 
				if(net.node_loc[i] != loc && loc_cond == TRUE)
					continue;
				//exclude a location
				if(net.node_loc[i] == loc && loc_cond == FALSE)
					continue;
			}
			
			if(t->network_set && t->network_set[i][set] == FALSE)
				return TRUE;
		}
	}
	
	//if one TRUE ok
	if(cond == PTRUE || cond == LTRUE  || cond == UTRUE)
	{
		for(i = 0; i < net.nbnode; i++) {
			if(loc != NA) {
				//restrict to location 
				if(net.node_loc[i] != loc && loc_cond == TRUE)
					continue;
				//exclude a location
				if(net.node_loc[i] == loc && loc_cond == FALSE)
					continue;
			}
			
			if(t->network_set && t->network_set[i][set] == TRUE)
				return TRUE;
		}
	}
	
	//if every FALSE ok
	if(cond == PALLFALSE || cond == LALLFALSE  || cond == UALLFALSE)
	{			
		for(i = 0; i < net.nbnode; i++) {
			if(loc != NA) {
				//restrict to location 
				if(net.node_loc[i] != loc && loc_cond == TRUE)
					continue;
				//exclude a location
				if(net.node_loc[i] == loc && loc_cond == FALSE)
					continue;
			}

			if(t->network_set && t->network_set[i][set] != FALSE)
				return FALSE;
		}
		return TRUE;
	}
	
	//if every TRUE ok
	if(cond == PALLTRUE || cond == LALLTRUE  || cond == LALLTRUE)
	{
		for(i = 0; i < net.nbnode; i++) {
			if(loc != NA) {
				//restrict to location 
				if(net.node_loc[i] != loc && loc_cond == TRUE)
					continue;
				//exclude a location
				if(net.node_loc[i] == loc && loc_cond == FALSE)
					continue;
			}
			
			if(t->network_set && t->network_set[i][set] != TRUE)
				return FALSE;
		}
		return TRUE;
	}
		
	return TRUE;
}


/*!
 * Store the current execution as strategy
 * \version 1.0
 * \date    2007
 * \author Elie
 */


void strategy_store(t_strategy *s)
{
	//int i;
	
	//storing value
	if(s->player == ADMIN)
	{
		s->time 	= tree->admin_global_last_time;
		s->otime 	= tree->incident_global_last_time;
		s->reward	= tree->admin_global_income;
		s->oreward 	= tree->incident_global_income;
		s->cost		= tree->admin_global_cost;
		s->ocost	= tree->incident_global_cost;
	} else {
		s->time 	= tree->incident_global_last_time;
		s->otime 	= tree->admin_global_last_time;
		s->reward	= tree->incident_global_income;
		s->oreward 	= tree->admin_global_income;
		s->cost		= tree->incident_global_cost;
		s->ocost	= tree->admin_global_cost;
	}
	//storing execution

	s->trace = tree_branch_last_save(s->trace, root);
	//tree_branch_display(root);
	//tree_branch_display(s->trace);
	/*for(i = 0; i < net.nbnode; i++)
		if(net.network_set[i][2] == TRUE)
			die("What the fuck\n");
	*/
	
	if(option.progress == 3)
		output_strategy();
}


/*!
 * Return the id associated to a given goal name
 * \version 1.0
 * \date    2007
 * \author Elie
 */

int strategy_goal_id_by_name(char *str)
{
	assert(str);
	
	if (strncmp(str, "Cost", 4) == 0)		return COST; 
	if (strncmp(str, "Time", 4) == 0)		return TIME;
	if (strncmp(str, "Reward", 6) == 0)		return REWARD;
	if (strncmp(str, "OCost", 5) == 0)		return OCOST;
	if (strncmp(str, "OReward", 7) == 0)		return OREWARD;
	if (strncmp(str, "OTime", 5) == 0)		return OTIME;
	return NA;	
}


/*!
 * Return the name associated to a given id
 * \version 1.0
 * \date    2007
 * \author Elie
 */

char *strategy_goal_name_by_id(int id)
{
	assert(id < NBGOAL);
	
	switch (id) {
		case COST:
			return "Cost";
		case OCOST:
			return "OCost";
		case REWARD:
			return "Reward";
		case OREWARD:
			return "OReward";
		case TIME:
			return "Time";
		case OTIME:
			return "OTime";
		default:
			die("Unknown goal type");
	}
	return NULL;
}
/*!
 * Display the list of researched strategy
 * \version 1.0
 * \date    2007
 * \author Elie
 */

void strategy_list_display()
{
	t_strategy	*s;
	
	s = strategy;
	
	while(s) {
		out("Strategy:%15s Player:%10s\n", s->name, s->player == ADMIN ? "Admin" : "Incident");
		s = s->next;
	}
}
