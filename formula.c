/*!	NetQI: http://www.netqi.org
*	Copyright Elie bursztein & LSV, ENS-Cachan, CNRS, INRIA 
*
*/

/*!
 *       \file formula.c
 *       \brief tatl formula functions
 *
 *       To parse verify and display strategy
 *       \author  Elie
 *       \version 1.1
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

/*!
 * instanciate a new tatl formula to model check
 * @param formula the formula instaciated
 * \version 1.1
 * \date   Aug 2008
 * \author Elie
 */

t_formula	*formula_new(char *formula)
{
	assert(formula);
	t_formula	*f;
	int 		i;
	
	f = malloc(sizeof(t_formula));
	bzero(f, sizeof(t_formula));
	
	f->win		= FALSE;
	f->asbox	= FALSE;
	
	for(i = 0; i < NBSET; i++) {
		f->cond[i] = NA;
                f->diam_was_true[i] = FALSE;
	}
	f->formula = my_strndup(formula, strlen(formula) - 2);
	return f;
}

/*!
 * evaluate if the formula hold for a given state.
 * @param f formula to verify
 * @param t current play
 * @param final boolean value that indicate if it is the final state of the play or not
 * @return TRUE is the formula hold, FALSE otherwise
 * \version 1.1
 * \date   Aug 2008
 * \author Elie
 */

int	formula_eval(t_formula *f, t_tree *tr, int final)
{
	assert(f);
	assert(tr);
	t_tree *t;
	t = tr;
	
	int i = 0;
	
	if (t->network_set == NULL) {
		//this situation happen when the play has a loop: the last state is then a choose
		if(t->father != NULL && t->father->network_set != NULL) {
			t = t->father;
		} else {
			//last execution == empty tree
			return TRUE;
		}
	}
			
	while(f->cond[i] != NA && i < NBSET) {
            if(f->box[i] == TRUE) {                 //box condition
                    if (t->network_set[f->node[i]][f->set[i]] != f->cond[i]) { //as soon as the condition is violated we scream
                        f->win = FALSE;
                        debug(15,"TATL eval %sreturn false%s for %sbox%s condition at the %srunning/final%s node:%d set:%d, requested cond:%s%d%s/%s%d%s:current cond\n",RED, CLR, YEL, CLR, CYA, CLR,f->node[i], f->set[i], GRE, f->cond[i], CLR, RED, t->network_set[f->node[i]][f->set[i]], CLR);
			return FALSE;
                    }
                
            } else if (f->diam[i] == TRUE) {        //diamond condition
                  
                //mark the condition as true if one state is true
                if (t->network_set[f->node[i]][f->set[i]] == f->cond[i]) {
                    if(f->diam_was_true[i] != TRUE)
                        debug(15,"TATL eval %sset true%s for %sdiamond%s condition at the %srunning/final%s node:%d set:%d, requested cond:%s%d%s/%s%d%s:current cond\n",GRE, CLR, YEL, CLR, CYA, CLR,f->node[i], f->set[i], GRE, f->cond[i], CLR, GRE, t->network_set[f->node[i]][f->set[i]], CLR);
                    f->diam_was_true[i] = TRUE;        
                }
                
                if(final == TRUE) {    //in final stage we verify if at one point the condition was true
                    if(f->diam_was_true[i] != TRUE) {
                        f->win = FALSE;
                        debug(15,"TATL eval %sreturn false%s for %sdiamond%s condition at the %sfinal%s node:%d set:%d, requested cond:%s%d%s/%s%d%s:current cond\n",RED, CLR, YEL, CLR, CYA, CLR,f->node[i], f->set[i], GRE, f->cond[i], CLR, RED, t->network_set[f->node[i]][f->set[i]], CLR);
                        return FALSE;
                    } else { //we need to reset the flag
                        f->diam_was_true[i] = FALSE;
                    }
                }
            
            } else {                                //standard condition
                if (final == TRUE) {
                    if (t->network_set[f->node[i]][f->set[i]] != f->cond[i]) {
                        f->win = FALSE;
                        debug(15,"TATL eval %sreturn false%s for %sstandard%s condition at the %sfinal%s node:%d set:%d, requested cond:%s%d%s/%s%d%s:current cond\n",RED, CLR, YEL, CLR, CYA, CLR,f->node[i], f->set[i], GRE, f->cond[i], CLR, RED, t->network_set[f->node[i]][f->set[i]], CLR);
			return FALSE;
                    }
                }
                    
            }
            i++;
	}
	f->win = TRUE;
	return TRUE;
}

/*!
 * add formula constraint
 * @param f the formula
 * @param set the set
 * @param node the targeted node
 * @param value the value to verify
 * \version 1.1
 * \date   Aug 2008
 * \author Elie
 */

void	formula_add_constraint(t_formula *f, int set, int node, int value) {
	int	i = 0;
	while(f->cond[i] != NA && i < NBSET)
		i++;
	f->node[i] = node;
	f->set[i] = set;
	
	debug(15, "Add TATL constraint set:%d, node:%d, cond:%d\n", set, node, value);
	
	//formula condition transformation for simpler evaluation
        switch(value) {
            case LALLTRUE:  f->cond[i] = TRUE;  break;
            case LALLFALSE: f->cond[i] = FALSE; break;
            case PALLTRUE:  f->cond[i] = TRUE;  f->box[i]   = TRUE; f->asbox    = TRUE; break;
            case PALLFALSE: f->cond[i] = FALSE; f->box[i]   = TRUE; f->asbox    = TRUE; break;
            case UALLTRUE:  f->cond[i] = TRUE;  f->diam[i]  = TRUE; f->asdiam   = TRUE; break;
            case UALLFALSE: f->cond[i] = FALSE; f->diam[i]  = TRUE; f->asdiam   = TRUE; break;        
        }
}	
