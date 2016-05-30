/*!	NetQI: http://www.netqi.org
*	Copyright Elie bursztein & LSV, ENS-Cachan, CNRS, INRIA 
*
*/

/*!
 *       \file tree_branch.c
 *       \brief model check tree branch functions
 *
 *       Used to manipulate tree branch directely
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
extern t_map		*map;			///!<map information
extern t_player		*admin;			///!<admin player structure
extern t_player		*incident;			///!<incident player structure
extern t_option		option;				
extern t_stat		stat;
extern unsigned int	gtime;			///!<game time
extern t_tree		*root;			///!<tree roots
extern t_tree		*tree;			///!<current tree node


//private



/*!
 * delete the last branch to a given tree
 * @param dst destination tree
 * \version 1.0
 * \date    2007
 * \author Elie
 */


void tree_branch_last_delete(t_tree *dst)
{
	t_tree	*t, *delete;
	
	t = dst;
	while(t)
	{
		delete = t;
		t = t->child;
		free(delete);
	}
	
}

/*!
 * compare the last branch of two given tree
 * @param t1 first tree
 * @param t2 second tree
 * @return TRUE is equal, FALSE otherwise
 * \version 1.0
 * \date    2007
 * \author Elie
 */

int tree_branch_eq(t_tree *tree1, t_tree *tree2) {

	assert(tree1 != tree2);
	
	t_tree	*t1, *t2;
	
	t1 = tree1;
	t2 = tree2;
	
	while(t1 && t2)
	{
		if(tree_node_eq(t1, t2) == FALSE)
			return FALSE;
		t1 = t1->child;
		t2 = t2->child;
	}
	
	if((t1 == NULL && t2 != NULL) || (t1 != NULL && t2 == NULL))
		return FALSE;
	return TRUE;
}

/*!
 * copy the last branch to a given tree
 * @param dst destination tree
 * \version 1.0
 * \date    2007
 * \author Elie
 */


t_tree *tree_branch_last_save(t_tree *dst, t_tree *src) {
	
	assert(tree);
	
	t_tree *t; 			///!<temporary node for the walk
	t_tree *child;		///!<temporary child storing
	t_tree *dst_root;	///!<root of the dst
	//cleaning previous save
	tree_branch_last_delete(dst); 
	
	t = src;

	dst = tree_node_dup(t);
	dst_root = dst;
	
	while(t->child)
	{
		t = t->child;
		/*while(t->sibilin)
			t = t->sibilin;
		*/
		child = tree_node_dup(t);
		tree_add_child(dst, child);
		child->father = dst;
		child->sibilin = NULL;
		dst = child;
	}
	return dst_root;
}


/*!
 * display the last computed tree branch 
 * \version 1.0
 * \date    2007
 * \author Elie
 */


void tree_branch_display(t_tree *src) {
	
	assert(src);
	t_tree *t;
	
	t = src;
	if(option.fileout == stdout && option.quiet != LATEX)
		out("\n------Start Game------------\n");
	
	//It so good to have the soft doing the report for you :))
	if(option.quiet == LATEX) {	
		//out("\\begin{array}{lrcl}\n");
            if(option.mode == STRATEGY) {
		out("\\begin{tabular}{|c|c|c|c|c|c|c|c|}\n\\hline\n");
		out("Time & Player & Action & Rule & Target & Succ & Payoff & Cost \\\\ \n \\hline \n \\hline \n");
            } else {
                out("\\begin{tabular}{|c|c|c|c|c|c|}\n\\hline\n");
		out("Time & Player & Action & Rule & Target & Succ \\\\ \n \\hline \n \\hline \n");
 
            }
         }
		
	output_tree_node(t);
	while(t->child)
	{
		t = t->child;
		/*while(t->sibilin)
			t = t->sibilin;*/
		output_tree_node(t);
	}
	if(option.fileout == stdout  && option.quiet != LATEX)
		out("------End Game------------\n");
	
	if(option.quiet == LATEX)	
		out("\\end{tabular}\n");
}
