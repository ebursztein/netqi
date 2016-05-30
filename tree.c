/*!	NetQI: http://www.netqi.org
*	Copyright Elie bursztein & LSV, ENS-Cachan, CNRS, INRIA 
*
*/

/*!
 *       \file tree.c
 *       \brief model check tree structure function
 *
 *       Used to manipulate the model check tree structure
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
extern t_net		net;					///!<network structure
extern t_map		*map;				///!<map information
extern t_player		*admin;				///!<admin player structure
extern t_player		*incident;				///!<incident player structure
extern t_option		option;				
extern t_stat		stat;
extern unsigned int	gtime	;			///!<game time
extern t_tree		*root;				///!<tree roots
extern t_tree		*tree;				///!<current tree node
extern t_tree		*last_branch;			///!<last

//private

unsigned int		node_num =  0;

t_tree *tree_node_new();
void tree_state_save(t_tree *t);
void tree_save_player_state(t_player *p);



/*!
 * init checking tree
 * 
 *  add an empty state as tree root. used to known when every possibility have been tested
 * 
 * \version 1.0
 * \date    2007
 * \author Elie
 * @see init
 */

void	tree_init() {
	root = tree_node_new();
	root->num = node_num++;
	root->admin_rule_status = UNSET;
	root->incident_rule_status = UNSET;
	tree = root;
}



/*!
 * init a tree node
 * \version 1.0
 * \date    2007
 * \author Elie
 * @return allocated tree node
 * @see tree_node_add
 */

t_tree *tree_node_new() {
	t_tree	*t;
	t = xmalloc(sizeof(t_tree));
	bzero(t, sizeof(t_tree));
	return t;
}



/*!
 * restore game state from a given tree node
 * @param t the tree node that hold the saved state
 * \version 1.0
 * \date    2007
 * \author Elie
 */

void tree_state_restore(t_tree *t) {

	assert(t);
	
	if(option.debug == 7)
	{
		out("%sRestoring state %d:\t", YEL, t->num); 
		if(t->admin_rule != NULL)
			out("admin rule %s/%d target %d status:%d",t->admin_rule->name, t->admin_rule->id, t->admin_target, t->admin_rule_status);
		if(t->incident_rule != NULL) 
			out("\t\tincident rule %s/%d target %d status:%d", t->incident_rule->name, t->incident_rule->id, t->incident_target, t->incident_rule_status);	
		out("%s\n", CLR);
	}
	

	//Players restore
	//painfull but having generic function for 2 players is slow and overkill
	admin->time 			= t->admin_time;
	admin->rule			= t->admin_rule;
	admin->rule_status 		= t->admin_rule_status;
	admin->ellapsed 		= t->admin_ellapsed;	
	admin->target 			= t->admin_target;
	admin->target_succ 		= t->admin_target_succ;
	admin->rule_income		= t->admin_global_income;
	admin->rule_cost		= t->admin_global_cost;
	admin->last_time		= t->admin_global_last_time;
	
	incident->time 			= t->incident_time;
	incident->rule			= t->incident_rule;
	incident->rule_status 	= t->incident_rule_status;
	incident->ellapsed 		= t->incident_ellapsed;	
	incident->target 		= t->incident_target;
	incident->target_succ	= t->incident_target_succ;
	incident->rule_income	= t->incident_global_income;
	incident->rule_cost		= t->incident_global_cost;
	incident->last_time		= t->incident_global_last_time;


	if(admin->rule_status == TRUE)
	{	
		///\note income is from the removed node not here
		
		//set state sav
		memcpy(net.network_set[admin->target], t->target_set_state,(size_t) NBSET);
		memcpy(net.network_set[admin->target_succ], t->target_set_state_succ,(size_t) NBSET);	
		//memcpy(net.penality_counter, t->penality_counter, (size_t)(sizeof(int) * NBNODE));
	}
	
	if(incident->rule_status == TRUE)
	{
		///\note income is from the removed node not here
		
		//set state sav
		memcpy(net.network_set[incident->target], t->target_set_state, NBSET);
		memcpy(net.network_set[incident->target_succ], t->target_set_state_succ, NBSET);
		//memcpy(net.penality_counter, t->penality_counter, (size_t)(sizeof(int) * NBNODE));	
	}
	
	//out("%d:\t%s\t->%d\t\tVuln Net:%d Save:%d\n",t->num, t->admin_rule->name, t->admin_target,net.network_set[0][0], t->target_set_state[0]);
	gtime = t->time;


}

/*!
 * climbing in the tree and restoring state
 * \version 1.0
 * \date    2007
 * \author Elie
 */


void tree_climb_up() {
	assert(tree);
	assert(root);
	
	t_tree *t;

	t = tree;
	
	//restoring  pre rule state
	tree_state_restore(tree);
	if(tree == root)
		return;	
	if(option.debug == 7)
		out("Climbing from %d to %d\n", tree->num, tree->father->num);
		//can't climb beyond the roof. Once you there let's juste trough you from there and die

		//game_final();
	//climbing
	tree = tree->father;
	assert(tree != NULL);
	
	//restoring state
	tree_state_restore(tree);
	stat.backtrack++;
}

/*!
 * store current game state into the given tree node
 * @param t the tree node where to store the game state
 * \version 1.0
 * \date    2007
 * \author Elie
 */

void tree_state_save(t_tree *t) {
	
	assert(admin->rule_status == UNSET || admin->rule_status == FALSE || admin->rule_status == TRUE || admin->rule_status == PENDING);
	assert(incident->rule_status == UNSET || incident->rule_status == FALSE || incident->rule_status == TRUE || incident->rule_status == PENDING);

	if(option.debug == 7)
	{
		out("%sStoring state %d:\t",GRE, node_num); 
		if(admin->rule != NULL)
			out("admin rule %s/%d target %d status:%d",admin->rule->name, admin->rule->id, admin->target, admin->rule_status);
		if(incident->rule != NULL)
			out("\t\tincident rule %s/%d target %d status:%d", incident->rule->name, incident->rule->id, incident->target,  incident->rule_status);	
		out("%s\n",CLR);
	}
	
	t->num = node_num++;
	stat.state++;
	
	//Players update
	//painfull but having generic function for 2 players is slow and overkill
	t->admin_time 			= admin->time;
	t->admin_rule			= admin->rule;
	t->admin_rule_status 		= admin->rule_status;
	t->admin_ellapsed 		= admin->ellapsed;	
	t->admin_target 		= admin->target;
	t->admin_target_succ 		= admin->target_succ;
	t->admin_global_income		= admin->rule_income;
	t->admin_global_cost		= admin->rule_cost;
	t->admin_global_last_time	= admin->last_time;
	
	t->incident_time 		= incident->time;
	t->incident_rule		= incident->rule;
	t->incident_rule_status 	= incident->rule_status;	
	t->incident_ellapsed 		= incident->ellapsed;	
	t->incident_target 		= incident->target;
	t->incident_target_succ 	= incident->target_succ;
	t->incident_global_income	= incident->rule_income;
	t->incident_global_cost		= incident->rule_cost;
	t->incident_global_last_time	= incident->last_time;


	if(admin->rule_status == TRUE || admin->rule_status == FALSE  || (admin->rule_status == PENDING && admin->ellapsed == 0)) {
		t->num_admin_rule++;
		stat.admin_num_rule++;
	}
	
	if(incident->rule_status == TRUE || incident->rule_status == FALSE || (incident->rule_status == PENDING && incident->ellapsed == 0)) {
		t->num_incident_rule++;
		stat.incident_num_rule++;
	}
	
	if(admin->rule_status == TRUE)
	{	
		//set state sav
		memcpy(t->target_set_state, net.network_set[admin->target], NBSET);
		memcpy(t->target_set_state_succ, net.network_set[admin->target_succ], NBSET);
		//memcpy(t->penality_counter, net.penality_counter, (size_t)(sizeof(int) * NBNODE));
		stat.admin_num_success_rule++;
			
	}
	

	if(incident->rule_status == TRUE)
	{
				
		//set state sav
		memcpy(t->target_set_state, net.network_set[incident->target], NBSET);
		memcpy(t->target_set_state_succ, net.network_set[incident->target_succ], NBSET);
		//memcpy(t->penality_counter, net.penality_counter, (size_t)(sizeof(int) * NBNODE));	
		stat.incident_num_success_rule++;
	}
	
	
	
	t->time = gtime;
	
	//out("%d:\t%s\t->%d\t\tVuln Net:%d Save:%d\n",t->num, t->admin_rule->name, t->admin_target,net.network_set[0][0], t->target_set_state[0]);

	
	if(option.progress && (((stat.state) % PROGSTEP) == 0))
	{
		output_stat(PROGRESS);
		if(option.progress == 2)
			tree_branch_display(root);
	}	
}
/*!
 * loop detection
 * @return TRUE if a loop is detected / FALSE otherwise
 * \version 1.0
 * \date    2007
 * \author Elie
 */

int tree_node_eq(t_tree *t1, t_tree *t2)
{
		if(	(t1->admin_rule 		== t2->admin_rule) 				&&
			(t1->incident_rule 		== t2->incident_rule) 			&&
			(t1->admin_target 		== t2->admin_target)			&&
			(t1->incident_target 		== t2->incident_target)			&&
			(t1->admin_target_succ 	== t2->admin_target_succ)		&&
			(t1->incident_target_succ 	== t2->incident_target_succ)		&&
			(t1->admin_rule_status	== t2->admin_rule_status)		&&
			(t1->incident_rule_status	== t2->incident_rule_status)
		  )
		{
			return TRUE;
		}
		
		/*
			out("false for 12 : ra:%d=%d / ri:%d=%d / ta:%d=%d / ti:%d=%d / tas:%d=%d  / tis:%d=%d /  as:%d=%d /  is:%d=%d \n",
			    	t1->admin_rule, t2->admin_rule, 
			    	t1->incident_rule, t2->incident_rule,
        		    	t1->admin_target, t2->admin_target, 
	      			t1->incident_target, t2->incident_target,
	  			t1->admin_target_succ, t2->admin_target_succ,
      				t1->incident_target_succ, t2->incident_target_succ,	   
			 	t1->admin_rule_status,t2->admin_rule_status,
      				t1->incident_rule_status,t2->incident_rule_status);
		*/
		return FALSE;
}
/*!
 * loop detection
 * @return TRUE if a loop is detected / FALSE otherwise
 * \version 1.0
 * \date    2007
 * \author Elie
 */

int tree_detect_loop()
{
	assert(root);
	assert(tree);

	t_tree	*pred;
        char	prev_network_set[NBNODE][NBSET];  ///!<set node mapping of previous node

        
	if (tree == root)
		return FALSE;

        //get the current network state 
        memcpy(prev_network_set, net.network_set, NBNODE*NBSET);
	pred = tree->father;
	while(pred)
	{
            //build back previous network set
            if(admin->rule_status == TRUE)
            {	
                memcpy(prev_network_set[pred->admin_target], pred->target_set_state,(size_t) NBSET);
		memcpy(prev_network_set[pred->admin_target_succ], pred->target_set_state_succ,(size_t) NBSET);	
            }
            if(incident->rule_status == TRUE)
            {
		memcpy(prev_network_set[pred->incident_target], pred->target_set_state, NBSET);
		memcpy(prev_network_set[pred->incident_target_succ], pred->target_set_state_succ, NBSET);
            }
            
            //testing
            if(tree_node_eq(pred, tree) == TRUE)	 {
                //verifying if the network state is the same
                if (memcmp(prev_network_set, net.network_set, NBNODE * NBSET) == 0) {
                    warning("Loop detected\n");
                    return TRUE;
               }
            }
            //climbing up 
            pred = pred->father;
	}

	return FALSE;
}


/*!
 * store current game state every time a rule is triggered
 *
 *@return FALSE if no backtrack was made; true otherwise 
 * \version 1.4
 * \date    Sept 2008
 * \author Elie
 */


int tree_node_add() {
	t_tree 		*t;
	t = tree_node_new();
	tree_state_save(t);
	
	///\todo optimi:  remove the backtrack facility
	if(option.die && t->num == option.die)
		die("");
	
	if(tree == NULL)
	{
		tree = t;
		root = tree;
	} else {
		tree_add_child(tree, t);
		t->father = tree;
		tree = t;
	}
        
        //do we need to store the graph ?
        if(option.graphfile)
            gamegraph_store_node(t);
            
	tree_node_remove_double_choice(tree);
	//loop detection
	if(tree_detect_loop() == TRUE)
	{
		//backtrack only if it is not a choose state
		//if(!((tree->admin_rule_status == PENDING || tree->admin_rule_status == UNSET) && (tree->incident_rule_status == PENDING || tree->incident_rule_status == UNSET)))
		t->loop = TRUE;
		game_backtrack();
		return LOOP;
		//game_run();
	}
	

	
	return FALSE;
}





/*!
 * remove a node from the tree
 * @param t the node to remove
 * \version 1.0
 * \date    2007
 * \author Elie
 */

void tree_node_remove(t_tree *t)
{
	assert(t);
	assert(t->father);

	t_tree *father; ///!<the father
	t_tree *child; ///!<tmp node

		
	father = t->father;
	child = father->child;
	
	//direct child ?
	if(child == t)
	{
		tree_free(t);
		father->child = NULL;
		return;
	}
	//sibilin child ?
	while(child->sibilin != t && child->sibilin != NULL)
	{
		child = child->sibilin;
	}
	
	/*list is like that: child ->t->(t->sibilin)*/
	
	if (child->sibilin == t)
	{
		child->sibilin = t->sibilin;
	} else {
		warning("can't able to find %d as childs something goes wrong\n",t->num);
	}
	
	tree_free(t);
}

/*!
 * free the tree structure 
 * @param t the tree structure
 * \version 1.0
 * \date    Apr 2008
 * \author Elie
 */

void tree_free(t_tree *t) {
	assert(t);

	int i;

	if (t->network_set != NULL) {
		//freeing row
		for (i = 0; i < net.nbnode; i++) 
			free(t->network_set[i]);
		//freeing collum,
		free(t->network_set);
		t->network_set = NULL;
	}
	free(t);
	t = NULL;
}


/*!
 * add a tree node to the list of child 
 * @param f the father
 * @param c the child 
 * \version 1.0
 * \date    2007
 * \author Elie
 */

void tree_add_child(t_tree *f, t_tree *c) {
	
	assert(f);
	assert(c);
	

	c->sibilin = f->child;
	f->child = c;
}

/*!
 * duplicate a node 
 * \version 1.0
 * \date    2007
 * \author Elie
 */

t_tree *tree_node_dup(t_tree *src) {
	
	t_tree *dst;
	
	dst = tree_node_new();
	memcpy(dst, src, sizeof(t_tree));
	///\note we do not duplicate the network_set structure
	dst->network_set = NULL;
	return dst;
}

/*!
 * detect and remove two consecutive choices 
 * @param t the tree to check
 * \version 1.0
 * \date    2007
 * \author Elie
 */
void tree_node_remove_double_choice(t_tree *t)
{
	t_tree	*f; 	///!<the father (the one to kill)
	t_tree	*ff;	///!<the father father
	

	
	if(!t)
		return;
	if(!t->father)
		return;
	if(!t->father->father)
		return;
	
	f = t->father;
	ff = t->father->father;
	
	if(option.debug == 7) {
	//	out("#############display 1###################\n");
	//	tree_branch_display(root);
		out("Detection:%d<-%d<-%d:\trule status\tt:%d/%d",t->num, f->num, ff->num, t->admin_rule_status, t->incident_rule_status);
		out("\tf:%d/%d:\t",f->admin_rule_status,  f->incident_rule_status);
		out(" ff:%d/%d:\n",ff->admin_rule_status,  ff->incident_rule_status);
	}
	
	//is this a pending state ?
	if (!(	(f->admin_rule_status == PENDING || f->incident_rule_status == PENDING) &&
		(f->admin_rule_status == PENDING || f->admin_rule_status == UNSET) 	&&
		(f->incident_rule_status == PENDING || f->incident_rule_status == UNSET) 	
	  ))
		return;

	//the father is also a pending state ?
	if (!(	(t->admin_rule_status == PENDING || t->incident_rule_status == PENDING) &&
		(t->admin_rule_status == PENDING || t->admin_rule_status == UNSET) 	&&
		(t->incident_rule_status == PENDING || t->incident_rule_status == UNSET) 	
	     ))
		return;
	
	//can be two consecutice choice legit : 1 by the admin followed by 1 from the incident. or conversely
	//to decide which is which we need to look at the ellapsed time
	
	//administrator new choice
	
	if(t->admin_rule_status == PENDING && f->admin_rule_status == UNSET && t->incident_ellapsed != 0 && t->incident_rule_status == PENDING)
		return;
	//incident  new choice
	if(t->incident_rule_status == PENDING && f->incident_rule_status == UNSET && t->admin_ellapsed != 0 && t->admin_rule_status == PENDING)
		return;
	
	ff->child = tree;
	tree->father = ff;
	if(option.debug == 7) {
		out("!!!State %d removed!!!\n",f->num);
	}
	tree_free(f);
	

}
