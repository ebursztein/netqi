/*!	NetQI: http://www.netqi.org
*	Copyright Elie bursztein & LSV, ENS-Cachan, CNRS, INRIA 
*
*/

/*!
 *       \file ouput.c
 *       \brief output formating functions
 *
 *       output function for formating
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

extern	t_option option;
extern t_net		net;				///!<network structure
extern t_map		*map;				///!<map information : sets and nodes
extern t_player		*admin;				///!<admin player structure
extern t_player		*incident;			///!<incident player structure
extern unsigned int	gtime;
extern t_tree		*tree;
extern t_tree		*root;
extern t_stat		stat;
extern t_strategy	*strategy;
extern t_formula	*formula;
//private functions
void output_rule_xml(t_tree *t, int id);
void output_rule_stdout(t_tree *t, int id);
void output_rule_latex(t_tree *t, int id);
void output_admin_rule(t_tree *t);
void output_incident_rule(t_tree *t);
void output_stat_stdout();
void output_stat_xml();
void output_strategy_stdout(t_strategy *s);
void output_strategy_trace_stdout(t_strategy *s);
void output_strategy_trace_xml(t_strategy *s);
void output_strategy_value_xml(t_strategy *s);




/*!
 * output xml file headers
 * \version 1.1
 * \date    2008
 * \author Elie
 */


void output_header()
{
	if(option.fileout != stdout)
	{
		out("<?xml version=\"1.0\"?>\n");
		out("<NetQiGameEngine mode=");
		    
                     if(option.mode == SIMULATION)		out("\"simulation\">\n\t<simulation>\n");
		else if(option.mode == MODELCHECK)		out("\"modelCheck\">\n\t<modelCheck>\n");
		else if(option.mode == STRATEGY)		out("\"strategy\">\n\t<strategies>\n");
	}
	//add the progres xml tag if needed
	output_header_progress();
}

/*!
 * output xml progess header if needed
 * \version 1.0
 * \date    2008
 * \author Elie
 */

void output_header_progress()
{
	if ((option.progress) && (option.fileout != stdout))
		out("\n\t\t<progress>\n");
}

/*!
 * output xml progess footer if needed
 * \version 1.0
 * \date    2008
 * \author Elie
 */

void output_footer_progress()
{
	if ((option.progress) && (option.fileout != stdout))
		out("\n\t\t</progress>");
	if (option.fileout != stdout)
	out("\n\t\t<results>\n");
}

/*!
 * output xml file footer and stats
 * \version 1.0
 * \date    2007
 * \author Elie
 */

void output_footer()
{	

	if(option.fileout != stdout)
	{
		out("\n\t\t</results>\n");
		     if(option.mode == SIMULATION)		out("\t</simulation>\n");
		else if(option.mode == MODELCHECK)		out("\t</modelCheck>\n");
		else if(option.mode == STRATEGY)		out("\t</strategies>\n");
		out("</NetQiGameEngine>\n");
	}
}


/*!
 * output tree node information
 * @param t tree node to display
 * \version 1.0
 * \date    2007
 * \author Elie
 */


void output_tree_node(t_tree *t)
{
		if (t->incident_rule_status ==  TRUE || t->incident_rule_status == FALSE|| ((t->incident_rule_status  == PENDING) &&( t->incident_ellapsed == 0 ))) 	
			output_incident_rule(t);
		if (t->admin_rule_status ==  TRUE || t->admin_rule_status == FALSE || ((t->admin_rule_status  == PENDING) &&( t->admin_ellapsed == 0 )))
			output_admin_rule(t);

}



/*!
 * output strategy with trace
 * \version 1.0
 * \date    2007
 * \author Elie
 */


void output_strategy()
{
	t_strategy	*s;
	
	s = strategy;
	while(s) {
		if (option.fileout != stdout) {
			out("\t\t\t<strategy name=\"%s\">\n\t", s->name);
				output_strategy_value_xml(s);
			out("\t\t\t\t<play>\n");
				output_strategy_trace_xml(s);
			out("\t\t\t\t</play>\n");
			out("\t\t\t</strategy>\n");
		} else {
			out("\n######Best Strategy for %s%s%s########\n",YEL, s->name, CLR);
			out("%s--- Strategy Execution%s\n", PUR, CLR);
			output_strategy_trace_stdout(s);
			out("%s--- Goals Values%s\n", CYA, CLR);
			output_strategy_value_stdout(s);
		}
		s = s->next;
	}
}


/*!
 * output strategy value only
 * \version 1.0
 * \date    2007
 * \author Elie
 */


void output_strategy_value()
{
	t_strategy	*s;
	
	s = strategy;
	while(s) {
		if (option.fileout != stdout)
			output_strategy_value_xml(s);
		else 
			output_strategy_value_stdout(s);
		s = s->next;
	}
}


/*!
 * output strategy trace
 * \version 1.0
 * \date    2007
 * \author Elie
 */

void output_strategy_trace_xml(t_strategy *s) {
	if(s->trace != NULL)
		tree_branch_display(s->trace);
}

/*!
 * output strategy trace
 * \version 1.0
 * \date    2007
 * \author Elie
 */

void output_strategy_trace_stdout(t_strategy *s) {
	if(s->trace != NULL)
		tree_branch_display(s->trace);
}


/*!
 * output strategy values xml
 * \version 1.0
 * \date    2007
 * \author Elie
 */

void output_strategy_value_xml(t_strategy *s) {
	out("\t\t\t<outcome ");
	if(s->player == ADMIN)
		out("player=\"admin\" ");
	else
		out("player=\"intruder\" ");
	out("reward=\"%lld\" cost=\"%lld\" time=\"%lld\" ",s->reward, s->cost, s->time);
	out("oreward=\"%lld\" ocost=\"%lld\" otime=\"%lld\" ", s->oreward, s->ocost, s->otime);
	out("></outcome>\n");
}


/*!
 * output strategy values
 * \version 1.0
 * \date    2007
 * \author Elie
 */

void output_strategy_value_stdout(t_strategy *s) {
	if(s->player == ADMIN)
		out("%s%s%s:",BLU, s->name, CLR);
	else
		out("%s%s%s:",YEL, s->name, CLR);
	out("Reward%s%7lld%s Cost:%s%7lld%s Time:%s%7lld%s ", GRE, s->reward, CLR, RED, s->cost, CLR, PUR, s->time, CLR);
	out("OReward:%s%7lld%s OCost:%s%7lld%s OTime:%s%7lld%s ", GRE, s->oreward, CLR, RED, s->ocost, CLR, PUR, s->otime, CLR);
	out("\n");
}


/*!
 * output modelcheck with trace
 * \version 1.0
 * \date    2007
 * \author Elie
 */


void output_modelcheck()
{
	//char	success = FALSE;
	//no counter example
	/*if (root->child == NULL) {
		if (formula->win == FALSE)
			success = TRUE;
	}*/
	if (option.fileout != stdout) {
		out("\t\t\t<property formula=\"%s\" satisfiability=\"%s\"></property>\n", formula->formula, formula->win == TRUE ? "true" : "false");
		if(formula->win == TRUE)
			return;
		out("\t\t\t\t<trace>\n");
		tree_branch_display(root);
		out("\t\t\t\t</trace>\n");
	} else {
		out("\n######Property satisfiability is"); 
		if(formula->win == TRUE)
			out(" %strue%s ########\n",GRE, CLR);
		else
			out(" %sfalse%s ########\n",RED, CLR);
		if(formula->win == TRUE)
			return;
		out("%s--- Associated trace ---%s\n", PUR, CLR);
		tree_branch_display(root);
	}
	
}




/*!
 * output execution stats
 * \version 1.0
 * \date    2007
 * \author Elie
 */


void output_stat(int type)
{
	if (option.fileout != stdout)
		output_stat_xml();
	else
		output_stat_stdout();
	
		
	if(option.mode == STRATEGY && type != FINAL)
		output_strategy_value();
}

/*!
 * output execution statistics in xml
 * \version 1.0
 * \date    2007
 * \author Elie
 */

void output_stat_xml()
{
	out("\t\t\t<stats ");
	out("filename=\"%s\" ", option.filename);
	out("node=\"%d\" sets=\"%d\" ", net.nbnode -1, map->nbset);
	out("execution_time=\"%d\" ", time(NULL) - option.start_time);
	out("play=\"%lld\" ", stat.exec);
	out("State=\"%lld\" ", stat.state);
	out("Backtrack=\"%lld\" ", stat.backtrack);
	out("admin_rule=\"%lld\" ", stat.admin_num_rule);
	out("Incident=\"%lld\" ", stat.incident_num_rule);
	out("></stats>\n");
}


/*!
 * output execution statistic for stdout
 * \version 1.0
 * \date    2007
 * \author Elie
 */

void output_stat_stdout()
{
	out("\n");
	out("name:%s", option.filename);
	out(" nodes:%d sets:%d", net.nbnode -1 , map->nbset);
	out("\n");
	out("Execution time:%s%8d%s ", YEL, time(NULL) - option.start_time, CLR);
	out("plays: %s%12lld%s\n", YEL, stat.exec, CLR);
	out("States:%s%12lld%s ",BLU, stat.state, CLR);
	out("Backtracks:%s%12lld%s ",CYA,stat.backtrack, CLR);
	out("Ratio S/E:%.2f ",((float)stat.state/(stat.exec +1)));
	out("B/S:%.2f ",((float)stat.backtrack/(stat.state +1)));
	out("\n");
	out("Player execution:\tAdmin :%s%12lld%s ",GRE,stat.admin_num_rule, CLR);
	out("Incident:  %s%12lld%s ",PUR,stat.incident_num_rule, CLR);
	out("Ratio I/A:%.2f",((float)stat.incident_num_rule/(stat.admin_num_rule +1))*100);
	out("\n");
}

/*!
 * output admin rule execution/failure
 * \version 1.0
 * \date    2007
 * \author Elie
 */


void output_admin_rule(t_tree *t)
{
	if (option.fileout != stdout)
		output_rule_xml(t,ADMIN);
	else {
		if (option.quiet == LATEX)
			output_rule_latex(t, ADMIN);
		else
			output_rule_stdout(t, ADMIN);
	}
}

/*!
 * output incident rule execution/failure
 * \version 1.0
 * \date    2007
 * \author Elie
 */


void output_incident_rule(t_tree *t)
{
	if (option.fileout != stdout)
		output_rule_xml(t, INCIDENT);
	else {
		if (option.quiet == LATEX)
			output_rule_latex(t, INCIDENT);
		else
			output_rule_stdout(t, INCIDENT);
	}
}

/*!
 * output  rule execution/failure in XML
 * 
 * @param p player structure
 * \version 1.0
 * \date    2007
 * \author Elie
 */

void output_rule_xml(t_tree *t, int id)
{
	//be sure everything is what it suppose to be
	assert(t);
//	assert(gtime > 0);
	
	if(id == ADMIN)
	{
		out("\t\t\t\t\t<step gtime=\"%lld\" treenode=\"%lld\" player=\"%s\" ruleid=\"%d\" rulename=\"%s\" ruletime=\"%d\" nodename=\"%d\" nodesucc=\"%d\" reward=\"%lld\" cost=\"%lld\" time=\"%lld\" oreward=\"%lld\" ocost=\"%lld\" otime=\"%lld\"",t->time, t->num, admin->name, t->admin_rule->id, t->admin_rule->name, t->admin_rule->time,  t->admin_target, t->admin_target_succ, t->admin_global_income, t->admin_global_cost, t->admin_global_last_time, t->incident_global_income, t->incident_global_cost, t->incident_global_last_time);
		//location
		out(" location_id=\"%d\" location_name=\"%s\"", t->admin_rule->loc, location_get_name_by_id(t->admin_rule->loc));
		out(" location_succ_id=\"%d\" location_succ_name=\"%s\"", t->admin_rule->sloc, location_get_name_by_id(t->admin_rule->sloc));
	
			if(t->admin_rule_status == TRUE) 		out(" result=\"execute\">execute</step>\n");
		else if (t->admin_rule_status == FALSE) 		out(" result=\"fail\">fail</step>\n");
		else if (t->admin_rule_status == PENDING) 	out(" result=\"select\">select</step>\n");
	 
	} else {
		out("\t\t\t\t\t<step gtime=\"%lld\" treenode=\"%lld\" player=\"%s\" ruleid=\"%d\" rulename=\"%s\" ruletime=\"%d\" nodename=\"%d\" nodesucc=\"%d\" reward=\"%lld\" cost=\"%lld\" time=\"%lld\" oreward=\"%lld\" ocost=\"%lld\" otime=\"%lld\"",t->time, t->num, incident->name, t->incident_rule->id, t->incident_rule->name, t->incident_rule->time,  t->incident_target, t->incident_target_succ, t->incident_global_income, t->incident_global_cost, t->incident_global_last_time, t->admin_global_income, t->admin_global_cost, t->admin_global_last_time);

                //location
                out(" location_id=\"%d\" location_name=\"%s\"", t->incident_rule->loc, location_get_name_by_id(t->incident_rule->loc));
                out(" location_succ_id=\"%d\" location_succ_name=\"%s\"", t->incident_rule->sloc, location_get_name_by_id(t->incident_rule->sloc));

	
			if(t->incident_rule_status == TRUE) 		out(" result=\"execute\">execute</step>\n");
		else if (t->incident_rule_status == FALSE) 		out(" result=\"fail\">fail</step>\n");
		else if (t->incident_rule_status == PENDING) 	out(" result=\"select\">select</step>\n");
	}

	
}


/*!
 * output  rule execution/failure in latex format
 * 
 * @param p player structure
 * \version 1.0
 * \date    2007
 * \author Elie
 */

void output_rule_latex(t_tree *t, int id)
{
		assert(t);
	//assert(gtime > 0);
	
	out("%lld &",t->time);
	if(id == ADMIN)
		out("%s & ", admin->name); 
	else 
		out("%s & ", incident->name);



	if(id == ADMIN) {
		
	
		
		//rule status
			if(t->admin_rule_status == TRUE) 		out("{\\bf execute} & ");
		else if (t->admin_rule_status == FALSE) 		out("{\\tt fail} & ");
		else if (t->admin_rule_status == PENDING) 		out("choose & ");

		//rule name
		out("%s & ", t->admin_rule->name);			
		
		//target
                if(net.node_label[t->admin_target] != NULL)
                    out("%d (%s)& ", t->admin_target, net.node_label[t->admin_target]);
                else
                    out("%d & ", t->admin_target);
		
		//successor              
                if(t->admin_target_succ) {
                    if(net.node_label[t->admin_target_succ] != NULL)
                         out("%d (%s) ", t->admin_target_succ, net.node_label[t->admin_target_succ]);   
                    else
                        out("%d  ", t->admin_target_succ);
                } else {
			out("$\\ff$  ");
                }
		if(option.mode == STRATEGY) {
			if(t->admin_rule_status == FALSE || t->admin_rule_status == TRUE)
				out("& %lld & %lld ",t->admin_global_income, t->admin_global_cost);
			else
				out("& - & - ");
		}
		out("\\\\ \\hline \n");
	} else {
		

		//rule status
			if(t->incident_rule_status == TRUE) 		out("{\\bf execute} & ");
		else if (t->incident_rule_status == FALSE) 		out("{\\tt fail} & ");
		else if (t->incident_rule_status == PENDING) 		out("choose & ");

		//rule name
		out("%s & ", t->incident_rule->name);		
		
		
		//target
                if(net.node_label[t->incident_target] != NULL)
                    out("%d (%s)& ", t->incident_target, net.node_label[t->incident_target]);
                else
                    out("%d & ", t->incident_target);
		
		//successor              
                if(t->admin_target_succ) {
                    if(net.node_label[t->incident_target_succ] != NULL)
                         out("%d (%s) ", t->incident_target_succ, net.node_label[t->incident_target_succ]);   
                    else
                        out("%d  ", t->incident_target_succ);
                } else {
			out("$\\ff$  ");
                }
		if(option.mode == STRATEGY) {
			if(t->incident_rule_status == FALSE || t->incident_rule_status == TRUE)
				out("%lld & %lld ",t->incident_global_income, t->incident_global_cost);
			else
				out(" - & - ");
		}
		out("\\\\ \\hline \n");
	}
}

/*!
 * output  rule execution/failure in stdout
 * 
 * @param p player structure
 * \version 1.0
 * \date    2007
 * \author Elie
 */

void output_rule_stdout(t_tree *t, int id)
{
	assert(t);
	//assert(gtime > 0);
	
	out("%5lld:\t", t->time);
	if(id == ADMIN)
		out("%s%10s%s", BLU,admin->name,CLR); 
	else 
		out("%s%10s%s", PUR,incident->name,CLR);



	if(id == ADMIN) {
		
			if(t->admin_rule_status == TRUE) 		out("\t%sexecute%s\t\t",GRE, CLR);
		else if (t->admin_rule_status == FALSE) 		out("\t%sfail to execute%s\t", RED, CLR);
		else if (t->admin_rule_status == PENDING) 	out("\t%schoose%s\t\t", YEL, CLR);
		
		out("%25s\t on ", t->admin_rule->name);
		
                if(net.node_label[t->admin_target] != NULL)
                        out("%s%d%s:%s%s%s", CLR, t->admin_target, CLR, CYA, net.node_label[t->admin_target], CLR);
                else
                        out("%d", t->admin_target);

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
	} else {
		
			if(t->incident_rule_status == TRUE) 		out("\t%sexecute%s\t\t",GRE, CLR);
		else if (t->incident_rule_status == FALSE) 		out("\t%sfail to execute%s\t", RED, CLR);
		else if (t->incident_rule_status == PENDING) 	out("\t%schoose%s\t\t", YEL, CLR);
		
		out("%25s\t on ",t->incident_rule->name);
		
		if(net.node_label[t->incident_target] != NULL)
			out("%d:%s%s%s", t->incident_target, CYA, net.node_label[t->incident_target], CLR);
		else 
			out("%d", t->incident_target);
		
		//location
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
	}
}

/*!
 * output that the game has timeout
 * 
 * @param p player structure
 * \version 1.0
 * \date    2007
 * \author Elie
 */

void output_timeout() 
{
	if(option.fileout == stdout)
		warning("Warning:\t GameEngine timeout: execuction stopped after %d sec\n", option.timeout);
	else
		out("<timeout value=\"%d\"></timeout>\n", option.timeout);
}
