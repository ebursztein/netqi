/*!	NetQI: http://www.netqi.org
*	Copyright Elie bursztein & LSV, ENS-Cachan, CNRS, INRIA 
*
*/

/*!
 *       \file rule.c
 *       \brief rule functions
 *
 *       functions used to manipulate rules
 *       \author  Elie
 *       \version 1.1
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

//global
extern t_net		net;				///!<network structure
extern t_map		*map;				///!<set name mapping
extern t_player		*admin;				///!<admin player structure
extern t_player		*incident;			///!<incident player structure
extern t_option		option;
extern t_tree		*tree;
extern int		successor;			///!is a sucessor is needed
extern unsigned int	gtime;				///!<game current time
extern unsigned int	nbrule;			///!<the maximum number of rules that a player add: MAX(admin.nbrules, incinde.nbrules)
extern t_formula	*formula;
extern t_strategy	*strategy;
//private function
int cmp_rule_time(const void *a, const void *b);


/*!
 * rule allocate function
 * \version 1.1
 * \date    Jul 2008
 * \author Elie
 */

t_rule	*allocate_rule()
{
	t_rule	*r;
	int i;
	
	r = xmalloc(sizeof(t_rule));
	bzero(r, sizeof(t_rule));
	
	for(i = 0; i < NBSET; i++)
	{
		r->pre[i] 		= NA;
		r->pre_succ[i]		= NA;
		r->effect[i]		= NA;
		r->effect_succ[i] 	= NA;
	}
	r->loc = NA;
	r->sloc = NA;
	return r;	
}

/*!
 * add a precondition to a rule or a precondition on predecessor
 * @param r the targeted rule
 * @param set the targeted set
 * @param value the operation associated with it e.g: adding the node to the set
 * \version 1.0
 * \date    2007
 * \author Elie
 */

void rule_add_pre(t_rule *r, int set, int value)
{
	assert(r);
	assert(set >= 0);
	assert(value == TRUE || value == FALSE || value == SUCC || value == NOTSUCC);
	if(value == TRUE)		r->pre[set] 	 = value;
	else if(value == FALSE)		r->pre[set]	 = value;
	else if(value == SUCC) 		r->pre_succ[set] = TRUE;
	else if(value == NOTSUCC)	r->pre_succ[set] = FALSE;
}


/*!
 * add a effect to a rule
 * @param r the targeted rule
 * @param set the targeted set
 * @param value the operation associated with it e.g: adding the node to the set
 * \version 1.0
 * \date    2007
 * \author Elie
 */

void rule_add_effect(t_rule *r, int set, int value)
{
	
	assert(r);
	assert(set >= 0);
	assert(value == TRUE || value == FALSE || value == SUCC || value == NOTSUCC);
	if(value == TRUE)		r->effect[set] 	 = value;
	else if(value == FALSE)		r->effect[set]	 = value;
	else if(value == SUCC) 		r->effect_succ[set] = TRUE;
	else if(value == NOTSUCC)	r->effect_succ[set] = FALSE;
}

/*!
 * add a rule to player set.
 * @param r the targeted rule
 * \version 1.0
 * \date    2007
 * \author Elie
 */

void rule_add(t_rule *r)
{
	assert(r);

	//parsing ok ! let's add it
	if(r->player == ADMIN) { 	
		r->next = admin->rule_set;
		admin->rule_set = r;
		r->id = admin->nbrule;
		admin->nbrule++;

	} else 	{
		r->next = incident->rule_set;
		incident->rule_set = r;
		r->id = incident->nbrule;
		incident->nbrule++;

	}
}



/*!
 * remove a given rule from the set
 * @param p the player structure associated with the rule set
 * @param rule	 the rule to remove
 * @param pred	the rule predecessor
 * @see preprocessor
 * \version 1.0
 * \date   Mar 2008
 * \author Elie
 */ 

void rule_remove(t_player *p, t_rule *rule, t_rule *pred)
{
	assert(p);
	assert(rule);

	//out("%s:rule:%s\n",p->name, rule->name);
	//if this is the first rule of the set
	if(pred == NULL)
		p->rule_set = rule->next;
	else
		pred->next = rule->next;
	//free(rule);
	p->nbrule--;
	return;
}



/*!
 * assign rule there id according to their position in the ordered set
 * @param r the targeted p
 * \version 1.0
 * \date    2007
 * \author Elie
 */


void rule_set_id(t_player *p)
{
	int i;
	for(i = 0; i < p->nbrule; i++)
		p->sorted_rule[i]->id = i;
}	

/*!
 * order rule by time
 * @param r the targeted rule
 * \version 1.0
 * \date    2007
 * \author Elie
 */


void order_rule_set_by_time(t_player *p)
{
	assert(p);
	assert(p->rule_set);
	debug(3, "%s sorting rules by time\n", p->name); 
	qsort(p->sorted_rule, (size_t) p->nbrule, sizeof(t_rule*), cmp_rule_time);
}


/*!
 * compare rule by exec time : used for qsort
 * @param a the first rule
 * @param b the second rule
 * @see order_rule_set_by_time()
 * \version 1.0
 * \date   2007
 * \author Elie
 */ 

int cmp_rule_time(const void *a, const void *b)
{
	t_rule *r1 = *(t_rule **) (a);
	t_rule *r2 = *(t_rule **) (b);
	if(r1->time <= r2->time)
		return -1;
	return 1;
}

/*!
 * test if a rule can be applied to a given node
 * @param r the rule
 * @param node the node
 * @return TRUE if it is possible FALSE otherwise
 * \version 1.1
 * \date    2008
 * \author Elie
 */

int rule_test_pre_succ(int node, int set, int value, int loc)
{
	assert(value == TRUE || value == FALSE);
	assert(set >= 0);

	int	i;
	
	for(i = 0; i < net.nbnode; i++)
	{
		//if a dependency existe
		if(net.network_edge[node][i] == TRUE)
		{
			//and the pred is in the right location
			if((loc == NA) || (loc == net.node_loc[i])) {
				//out("node %d, set %d, succ%d\n", node, set, i);
				//and if the value of targeted set of the predecessor node is the one
				if (net.network_set[i][set] == value)
				{
					return i;
				}
			}
		}
	}
	//no predecessor meet the pre cond !
	return FALSE;
	
}


/*!
 * test if a rule can be applied to a given node
 * @param r the rule
 * @param node the node
 * @return TRUE if it is possible FALSE otherwise
 * \version 1.1
 * \date    2008
 * \author Elie
 * \note This function is the most called so do not screw with it
 */

int test_rule(t_rule *r, int node)
{
	int i;
	int res = 0;
	
	//location scope test
	if(r->loc != NA)
		if (r->loc != net.node_loc[node])
			return FALSE;
	//direct set
	for(i = 0; i <= map->nbset; i++)
	{
		if (r->pre[i] != NA) {
			if(net.network_set[node][i] != r->pre[i])
			{
					///\note don't enable it unless you really need it because this function is overcalled
					//debug(5,"State:%lld Player:%d\tTime:%d\tRule:%s/%d\t\tTarget %d\t Failed because %s (%d) is set to %s instead of %s\n",tree->num, r->player, gtime, r->name, r->id, node,map_set_name_by_id(i),i, map_set_state_by_id(net.network_set[node][i]),map_set_state_by_id(r->pre[i]));
				return FALSE;
			}
		}
	}
	
	//predecessor conditions
	///\todo fixeme successor test : if multiple pre condition are on successor they must apply to the same node
	for(i = 0; i <= map->nbset; i++)
	{
		if (r->pre_succ[i] != NA) {
			//out("testing succ state: %lld rule:%s node:%d, sloc:%d\n", tree->num,  r->name, node, r->sloc);
			if((res = rule_test_pre_succ(node ,i, r->pre_succ[i], r->sloc)) == FALSE) {
				return FALSE;
			}
		}
	}
	
	//used as global variable do not erase it
	successor = res;
	return TRUE;
}




/*!
 * test if a rule can be applied to one of the network node
 * @param r the rule
 * @param start at which node should the test start
 * @param tabou allow to remove a node
 * @return the node indice if it can be applied otherwise it return false
 * \version 1.1
 * \date    Aug 2008
 * \author Elie
 */
int 	can_use_rule(t_rule *r, int start, int tabou)
{
	int i;
	
	debug(9, "test %s starting at node %d tabou %d\n", r->name, start, tabou);
	for(i = start; i < net.nbnode; i++)
	{
            if (i != tabou)
		if (test_rule(r, i) == TRUE)
			return i;
	}
	return FALSE;
}



/*!
 * apply rule effect to the targeted node
 * @param r the rule
 * @param target the targeted node
 * \version 1.0
 * \date    2007
 * \author Elie
 */
void apply_rule_effect(t_rule *r, int target)
{
	int i;
	
	for(i = 0; i <= map->nbset; i++)
	{
		if (r->effect[i] != NA)
		{
			//printf("setting target %d set %d effect %d\n", target, i, r->effect[i]);
			net.network_set[target][i] = r->effect[i];
		}
	}

}

/*!
 * apply rule succesor effect to the targeted node
 * @param r the rule
 * @param target the targeted node
 * \version 1.0
 * \date    2007
 * \author Elie
 */
void apply_rule_effect_succ(t_rule *r, int target)
{
	int i;
	
	//printf("setting target succ %d\n", target);
	for(i = 0; i <= map->nbset; i++)
	{
		if (r->effect_succ[i] != NA)
		{
			net.network_set[target][i] = r->effect_succ[i];
		}
	}

}

/*!
 * rule execution: reset player structure and apply rule effect is execution is successful.
 * @param p player
 * @param i is the execution is successful ?
 * \version 1.0
 * \date    2007
 * \author Elie
 */

void exec_rule(t_player *p, int success)
{
	int	i;
	if(success == TRUE)
	{
		apply_rule_effect(p->rule, p->target);
		if(p->target_succ != UNSET)
			apply_rule_effect_succ(p->rule,p->target_succ);
		

		//We need to store the network state after the rule for CTL path evaluation
		if(option.mode == STRATEGY || option.mode == MODELCHECK) {
			
			//allocating collum,
			tree->network_set = xmalloc(net.nbnode * sizeof(char *));
			//allocating rows
			for (i = 0; i < net.nbnode; i++) {
				tree->network_set[i] = xmalloc(map->nbset * sizeof(char));
				memcpy(tree->network_set[i], net.network_set[i], map->nbset * sizeof(char));
			}
		}

		//if model check and early cut is usefull then evaluate the formula
		if(option.mode == MODELCHECK && (formula->asbox == TRUE || formula->asdiam == TRUE)) {
			if(formula_eval(formula, tree, FALSE) == FALSE) {
				game_final();
			}
		}
	}
	
	
	//reset
	p->ellapsed 	= 0;
	p->time 		= 0;
	p->rule 		= NULL;
	p->target 		= UNSET;
	p->target_succ	= UNSET;
	p->rule_status	= UNSET;
	successor		= UNSET;
}

/*!
 * display a rule for debug
 * @param r the rule
 * \version 1.1
 * \date    2007
 * \author Elie
 */


void rule_display(t_rule *r)
{
	assert(r != NULL);
	assert(r->name);
	int i;
        
        if (option.quiet == LATEX) {
            //id
            out("\t\t\t\t\\ttRule{%d) }", r->id);
            //preconditions
            out("{");
            for(i = 0; i < NBSET; i++)
            {
		if (r->pre[i] != NA)
		{
                    if(i > 0)
                        out(" \\land");
			out(" %s %s",r->pre[i] == TRUE ? "": "\\lnot", map_set_name_by_id(i));
		}
	
            }

            for(i = 0; i < NBSET; i++)    
            {            
                if (r->pre_succ[i] != NA)            
                {   
                    out(" \\land");
                    out(" \\Diamond %s %s",r->pre_succ[i] == TRUE ? "": "\\lnot", map_set_name_by_id(i));            
                }    
            }

            
            out("}");
            //info
            out("{%d,%s,%s", r->time, r->player == 1 ? "A": "I", r->name);
            if(r->cost)
                out("%d", r->cost);
            out("}");
            
            //postcondition               
            out("{");
            for(i = 0; i < NBSET; i++)
            {
		if (r->effect[i] != NA)
		{
                    if(i > 0)
                        out(" \\land");
			out(" %s %s",r->effect[i] == TRUE ? "": "\\lnot", map_set_name_by_id(i));
		}
	
            }

            for(i = 0; i < NBSET; i++)    
            {            
                if (r->effect_succ[i] != NA)            
                {   
                    out(" \\land");
                    out(" \\Diamond %s %s",r->effect_succ[i] == TRUE ? "": "\\lnot", map_set_name_by_id(i));            
                }    
            }
            out("}");
            //location
            if(r->loc != NA)
                out("{%d}",r->loc);
            else
                out("{}");
            out("\\\\\n");
            return;
        }
        
	out("%s%s%s: id:%s%d%s time:%s%d%s player:%d cost:%d location:%d\t", YEL,  r->name, CLR, GRE,  r->id, CLR, CYA, r->time, CLR, r->player, r->cost, r->loc);
	out("cond: ");
	for(i = 0; i < NBSET; i++)
	{
		if (r->pre[i] != NA)
		{
			out("%s:%s ", map_set_name_by_id(i), map_set_state_by_id(r->pre[i]));
		}
	}
	out("\t");
	out("suc_cond:");
	for(i = 0; i < NBSET; i++)
	{
		if (r->pre_succ[i] != NA)
		{
			out("%s:%s ", map_set_name_by_id(i), map_set_state_by_id(r->pre_succ[i]));
		}
	}
	out("\teffect:");
	for(i = 0; i < NBSET; i++)
	{
		if (r->effect[i] != NA)
		{
			out("%s:%s ", map_set_name_by_id(i), map_set_state_by_id(r->effect[i]));
		}
	}
	out("\tsuc_effect:");
	for(i = 0; i < NBSET; i++)
	{
		if (r->effect_succ[i] != NA)
		{
			out("%s:%s ", map_set_name_by_id(i), map_set_state_by_id(r->effect_succ[i]));
		}
	}
	out("\n");
	
}

/*!
 * display the set of rule of a given player for debug
 * @param p the player
 * \version 1.0
 * \date    2007
 * \author Elie
 */


void player_rule_display(t_player *p)
{
	int i;
	
	for(i = 0; i < p->nbrule; i++)
		rule_display(p->sorted_rule[i]);
}


/*!
 * display the set of unsorted rule of a given player 
 * @param p the player
 * \version 1.0
 * \date    Mar 2008
 * \author Elie
 */

void player_rule_display_unsorted(t_player *p)
{

	t_rule *r = p->rule_set;
	while(r)
	{
		rule_display(r);
		r = r->next;
	}
}

/*!
 * create an array of pointer to rules which is more easily manipulable
 * \version 1.1
 * \date    Mar 2008
 * \author Elie
 */

void rule_to_array()
{
	t_rule	*r;
	int	i = 0;

	//assert(incident->rule_set);
	//assert(admin->rule_set);
	
	if(admin->rule_set) {
		admin->sorted_rule = (t_rule **)xmalloc(sizeof(t_rule *) * admin->nbrule);
		bzero(admin->sorted_rule, (sizeof(t_rule *) * admin->nbrule));
	}
	
	if(incident->rule_set) {
		incident->sorted_rule = (t_rule **)xmalloc(sizeof(t_rule *) * incident->nbrule);
		bzero(incident->sorted_rule, (sizeof(t_rule *) * incident->nbrule));
	}

	r = admin->rule_set;
	for (i = 0; i < admin->nbrule; i++)
	{
		admin->sorted_rule[i] = r;
		if(option.debug == 3)
			out("Admin rule array:\t added %s\n", r->name);
		r = r->next;
	}

	r = incident->rule_set;
	for (i = 0; i < incident->nbrule; i++)
	{
		incident->sorted_rule[i] = r;
		if(option.debug == 3)
			out("Incident rule array:\t added %s\n", r->name);
		r = r->next;
	}
}
