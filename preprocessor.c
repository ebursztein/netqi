/*!	NetQI: http://www.netqi.org
*	Copyright Elie bursztein & LSV, ENS-Cachan, CNRS, INRIA 
*
*/

/*!
 *       \file preprocessor.c
 *       \brief static optmization on the game structure prior to execution
 *
 *       Analyze staticaly the game supplied to try to optimize it
 *       \author  Elie
 *       \version 1.0
 *       \date    Mar 2008
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
void	preprocess_structure();
void 	preprocess_rules();
void	preprocess_elim_conflicting_rules(int set, int value, int loc, int loc_cond);



 ///\todo compute dynamicaly the table size used in struct tree and  be sure it is a factor of height (taille/8 + 1 ) x 8

/*!
 * general preprocessing function
 * Preprocessing is used to optimize the game before the execution and detect error
 * \version 1.1
 * \date    Apr 2008
 * \author Elie
 */
void preprocessor()
{
	preprocess_structure();
	//try to reduce the number of rules used based on formula structure
	/*out("%d\n", incident->nbrule);
	rule_display(incident->rule_set);*/
	preprocess_rules();
	/*out("%d\n", incident->nbrule);
	rule_display(incident->rule_set);
	rule_display(incident->rule_set->next);
	rule_display(incident->rule_set->next->next);*/
	//die("pre-processing over\n");
}


/*!
 * rules preprocessing function
 * Rules Preprocessing 
 * \version 1.0
 * \date    Mar 2008
 * \author Elie
 */
void	preprocess_rules(){
	int	i;

	if (option.mode == SIMULATION)
		return;
	if (option.mode == STRATEGY && net.nbstrat > 1)
	{
		debug(13, "FAILED\trules preprocessing\tGame have more than 1 strategy\n"); 
		return;
	}
	
	if(option.mode == STRATEGY)
	{
		for (i = 0; i < NBSET; i++)
		{
			if (strategy->cond[i] == PALLTRUE)
				preprocess_elim_conflicting_rules(i, FALSE, strategy->loc, strategy->loc_cond);
			if (strategy->cond[i] == PALLFALSE)
				preprocess_elim_conflicting_rules(i, TRUE, strategy->loc, strategy->loc_cond);
		}	
	}
}

/*!
 * rule removing function
 * remove from the rules list, the set of rules that have a precondition in conflict with the box strategy or formula constraint 
 * \version 1.0
 * \date    Mar 2008
 * \author Elie
 */

void	preprocess_elim_conflicting_rules(int set, int value, int loc, int loc_cond) {
	
	//ensure that the rules sorting is done AFTER the preprocessing
	assert(admin->sorted_rule == NULL);
	int 	loc_is_ok	= FALSE;
	t_rule	*pred 	= NULL;
	t_rule	*curr 	= NULL;

	//Player 1
	curr = admin->rule_set;
	while(curr != NULL)
	{
		//location filtering: quick and dirty 
		if((loc_cond == TRUE) && (loc == curr->loc)) loc_is_ok = TRUE;
		else if((loc_cond == FALSE) && (loc != curr->loc)) loc_is_ok = TRUE;
		else 	loc_is_ok = FALSE; 

		if(((curr->pre[set] == value) || (curr->pre_succ[set] == value)) && (loc_is_ok == TRUE))
		{
			debug(13, "SUCC\trules preprocessing\tremoving %s because precondition will never be satisfied\n", curr->name); 
			rule_remove(admin, curr, pred);
			if(pred == NULL)
				curr = admin->rule_set;
			else
				curr = pred->next;
		} else {
			pred = curr;
			curr = curr->next;
		}
		
	}

	//Player 2
	curr = incident->rule_set;
	pred = NULL;

	while(curr != NULL)
	{
		//location filtering: quick and dirty 
		if((loc_cond == TRUE) && (loc == curr->loc)) loc_is_ok = TRUE;
		else if((loc_cond == FALSE) && (loc != curr->loc)) loc_is_ok = TRUE;
		else 	loc_is_ok = FALSE; 

		if((curr->pre[set] == value) || (curr->pre_succ[set] == value))
		{
			debug(13, "SUCC\trules preprocessing\tremoving %s because precondition will never be satisfied\n", curr->name); 
			rule_remove(incident, curr, pred);
			if(pred == NULL)
				curr = incident->rule_set;
			else
				curr = pred->next;
		} else {
			pred = curr;
			curr = curr->next;
		}
		
	}
}


/*!
 * game structure preprocessing function
 * Perform various sanity check to be sur that the game will run properly 
 * \version 1.0
 * \date    Apr 2008
 * \author Elie
 */
void	preprocess_structure(){

		if(option.mode == STRATEGY && strategy == NULL)
			die("%s:\tThe strategy objectives are not defined in the gamefile\n", option.filename);

		if(option.mode == MODELCHECK && formula == NULL)
			die("%s:\tThe formula to verify is not defined in the gamefile\n", option.filename);	

		if(map->nbset == 0)
			die("%s:\tThere is no sets of variable defined in the gamefile\n", option.filename);

		if(admin->nbrule + incident->nbrule == 0)
			die("%s:\tThere is no players rules defined in the gamefile\n", option.filename);
}

