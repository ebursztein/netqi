/*!	NetQI: http://www.netqi.org
*	Copyright Elie bursztein & LSV, ENS-Cachan, CNRS, INRIA 
*
*/

/*!
 *       \file parser.c
 *       \brief parser functions
 *
 *       To parse rules and dependency graphs
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
extern t_net		net;				///!<network structure
extern t_map		*map;				///!<set name mapping
extern t_player		*admin;				///!<admin player structure
extern t_player		*incident;			///!<incident player structure
extern t_option		option;	
extern t_strategy	*strategy;
extern t_formula	*formula;
//local dirt
int     nested_file = 0;
int	in_block = FALSE;
#define	BLOCK_RULES	1
#define BLOCK_SETS	2
#define BLOCK_GRAPH	3
#define BLOCK_PENALITY	4


//local function prototype
void select_block(char *line, int l);
void parse_block(char *line, int l);
void parse_graph(char *line, int l);
void parse_penality(char *line, int l);
void parse_check_param();
void post_sets();
void parse_strategy(char *line, int l);
void parse_formula(char *line, int l);

/*!
 * parser entry point
 * \version 1.4
 * \date    Aug 2008
 * \author Elie
 */
void parse(char * file)
{
	FILE         	*fp;
	char		line[4096];
	int		l = 0;
	//try to read the file
	if (!(fp = fopen(file, "r")))
		die("can't open game file:\t%s\n", file);
	
        nested_file++;
        if (nested_file > 10)
            die("can't open more than 10 nested game file last file is:\t%s\n", file);
	// Parsing the configuration files and the if the syntax is correct 
	while ((fgets(line, sizeof(line), fp)))
	{
		l++;
		//comment
		if (isspace(line[0]) || line[0] == '#' || line[0] == '/' || line[0] == ';')
			continue;
	
		if (strncmp(line,"nodes=",6) == 0)
		{
			net.nbnode = atoi(&line[6]) + 1;
			if(net.nbnode > NBNODE)
				die("%s:%d:\tCan't use a graph with more than %d nodes\n",option.filename,l,NBNODE);
			if(net.nbnode == 0)
				die("%s:%d:\tGraph must have at least 1 node\n",option.filename,l,NBNODE);
			continue;
		}
                
                if (strncmp(line,"include=",8) == 0)
		{
                    parse(my_strndup(line + 8, strlen(line) - 9));
                    continue;
		}

		if (strncmp(line,"p1=",3) == 0)
		{
			admin->name = my_strndup(line + 3, strlen(line) -4);
			continue;
		}	

		if (strncmp(line,"p2=",3) == 0)
		{
			incident->name = my_strndup(line + 3, strlen(line) - 4);
			continue;
		}	


		if (strncmp(line,"delay=",6) == 0)
		{
			option.delay = atoi(&line[6]);
			continue;
		}	

		if (strncmp(line,"tatlFormula=",12) == 0)
		{
			parse_formula(line, l);
			continue;
		}	
		

		if (strncmp(line,"strategy",8) == 0)
		{
			parse_strategy(line, l);
			continue;
		}
		
		if (strncmp(line,"timeout=",8) == 0)
		{
			option.timeout = atoi(&line[8]);
			continue;
		}

		if (strncmp(line,"default_node_value=",19) == 0)
		{
			option.default_node_value = atoi(&line[20]);
			continue;
		}

		if (strncmp(line,"default_set_value=",18) == 0)
		{
			option.default_set_value = atoi(&line[18]);
			continue;
		}

		if (in_block == FALSE)
			select_block(line, l);
		else
			parse_block(line, l);
	}
	
	if (l < 1)
		die("%s:: appears to be an empty file please check it\n", option.filename);
	if(option.debug == 10)
		strategy_list_display();
	//adding header to the file if it is not stdout
	output_header();
}




/*!
 * Check that every parameters are correctly set
 * \version 1.0
 * \date    2007
 * \author Elie
 */

void	parse_check_param()
{
	if (net.nbnode == 0)
		die("%s:: 'nodes' option must be set\n", option.filename);
}



/*!
 * parse TATL formula
 * @param line the line to parse
 * @param l the line number
 * \version 1.3
 * \date    Aug 2008
 * \author Elie
 */


void parse_formula(char *line, int l)
{
		 
	unsigned int 	i = 0;
	int				node;
	t_formula		*f;
	char			*c;
	char			goal[4096];
	char			set[4096];
	int				setnum = 0;
	int				value = 0;
	
	if (map->nbset == 0)
		die("%s:%d:\tFormula needs to be specified after the <sets>\n", option.filename, l, line);

	while(line[i] != '(' && i < strlen(line)) i++;

	if(i == strlen(line)) {
		die("%s:%d:\tFormula invalid syntax missing (\n", option.filename, l, line); 
	}
	c = (line + i +1);
	f = formula_new(c);

		bzero(goal, 4096);
		bzero(set, 4096);
		value = NA;
		for (i = 0; c[i] != '^' && c[i] != ')' && i < strlen(c); i++) ;
		if(i == strlen(c)) die("%s:%d:\t formula condition error\n", option.filename, l);
		strncpy(goal,c,i);
		
		if(goal[0] == '~')  {
			     if (goal[1] == '+')	value = PALLTRUE;
			else if (goal[1] == '!')	value = PALLFALSE;
			else die("%s:%d:\tTATL formula requiered element operator is '%c' and should be either:\n\t\t\t\t '+' to  verify that the set is TRUE for the node\n\t\t\t\t '!' to verify that the set is FALSE for the node\n", option.filename, l, goal[0]);
			
		} else if(goal[0] == '|') {
			     if (goal[1] == '+')	value = LALLTRUE;
			else if (goal[1] == '!')	value = LALLFALSE;
			else die("%s:%d:\tTATL formula requiered element operator is '%c' and should be either:\n\t\t\t\t '+' to  verify that the set is TRUE for the node\n\t\t\t\t '!' to verify that the set is FALSE for the node\n", option.filename, l, goal[0]);
		
		
                             
                } else if(goal[0] == '$') {
                             if (goal[1] == '+')	value = UALLTRUE;
			else if (goal[1] == '!')	value = UALLFALSE;
                        else die("%s:%d:\tTATL formula requiered element operator is '%c' and should be either:\n\t\t\t\t '+' to  verify that the set is TRUE for the node\n\t\t\t\t '!' to verify that the set is FALSE for the node\n", option.filename, l, goal[0]);

                
                } else	die("%s:%d:\tFormula TATL operator is '%c' and should be either:\n\t\t\t\t '~' to verify that the condition hold in every state\n\t\t\t\t '|' to  verify the condition hold at the end\n\t\t\t\t '$' to  verify the condition hold at the end\n", option.filename, l, goal[0]);
		
		for(i = 2; ((goal[i] != ':') && (goal[i] != ')') && (goal[i] != '\n')); i++)
			set[i-2] = goal[i];
		if ((setnum = map_set_id_by_name(set)) == -1)
			die("%s:%d:\tSet %s is undeclared in the <sets> section\n", option.filename, l, set);
		
		if((goal[i] == ')') || (goal[i] == '\n'))
			die("%s:%d:\tFormula syntax error: no targeted node is specified. %c shoud be ':'\n", option.filename, l, goal[i]);
		node = atoi(&goal[i+1]);
		formula_add_constraint(f, setnum, node, value);
		formula = f;
}



/*!
 * parse the strategy command
 * @param line the line to parse
 * @param l the line number
 * \version 1.3
 * \date    2007
 * \author Elie
 */


void parse_strategy(char *line, int l)
{
		 
	unsigned int 	i = 0;
	char			goal[4096];
	int			goal_type = NA;
	int 			goal_id = 0;
	int			goal_order = 0;
	int			setnum = NA;
	int			value;
	char			*c;
	t_strategy 	*s;
	
	if (map->nbset == 0)
		die("%s:%d:\tStrategy need to be specified after the <sets>\n", option.filename, l, line);
	
	
	while(line[i] != '(' && i < strlen(line)) i++;

	if(i == strlen(line)) {
		die("%s:%d:\tStrategy invalid syntax missing (\n", option.filename, l, line); 
	}
	c = (line + i +1);
	s = strategy_new();
	
	for(i = 0 ; c[i] != ',' && i < strlen(c); i++) ;
	if(i == strlen(c)) {
		die("%s:%d:\tStrategy invalid syntax missing ',' specify a strategy name\n", option.filename, l, line); 
	}
	
	s->name = my_strndup(c, i);
	s->name[i] = '\0';


	
	//move to next argument
	c = c + i;
	for(i = 0 ; c[i] != 'I' && c[i] != 'A'  && i < strlen(c); i++) ;
	if(i == strlen(c)) {
		die("%s:%d:\tStrategy invalid syntax principal is either I or A\n", option.filename, l, line); 
	}
	
	s->player = (c[i]  == 'I') ? INCIDENT : ADMIN;
	
	//move to next argument
	c = c + i;
	for(i = 0 ; c[i] != ',' && i < strlen(c); i++) ;
	if(i == strlen(c)) {
		die("%s:%d:\tStrategy invalid syntax missing ',' specify strategy goal\n", option.filename, l, line); 
	}
	c = c+ i + 1;
	


	for(i = 0 ; c[i] != ','  && i < strlen(line); i++) ;
	if(i == strlen(c)) {
		die("%s:%d:\tStrategy invalid syntax goal must be specified\n", option.filename, l, line); 
	}
	
	//goals
	
	while(1)
	{
		bzero(goal, 4096);
		for (i = 0; c[i] != '^' && c[i] != ',' && i < strlen(c); i++) ;
		if(i == strlen(c)) die("%s:%d:\tStrategy Goal parsing error\n", option.filename, l);
		strncpy(goal,c,i);
			if (strncmp(goal, "MAX", 3) == 0)		goal_type  = MAX;
		else if (strncmp(goal, "MIN", 3) == 0)		goal_type  = MIN;
		else								die("%s:%d:\t Goal Syntax error. It must be either be MAX(goal) or MIN(goal)\n", option.filename, l);
		if(goal[3] != '(')
			die("%s:%d:\t Goal Syntax error. It must be either be MAX(goal) or MIN(goal)\n", option.filename, l);
		goal_id = strategy_goal_id_by_name(&goal[4]);
		goal[strlen(goal) - 1] = '\0';
		if(goal_id == NA)
			die("%s:%d:\t Goal strategy Syntax error. goal is \"%s\" and it must be either Reward, Time, Cost, OReward, OTime, OCost \n", option.filename, l, &goal[4]);	
		
		//adding goal
		s->goal[goal_id] = goal_type;
		s->nbgoal++;
		
		if(c[i] == ',')
			break;
		c += i + 1;
	}
	c += i + 1;
	
	//goal order
	
	while(1)
	{
		bzero(goal, 4096);
		for (i = 0; c[i] != '>' && c[i] != ',' && i < strlen(c); i++) ;
		if(i == strlen(c)) die("%s:%d:\tStrategy Goal  order parsing error\n", option.filename, l);
		strncpy(goal,c,i);
		goal_id = strategy_goal_id_by_name(goal);
		goal[strlen(goal)] = '\0';
		if(goal_id == NA)
			die("%s:%d:\t Goal Order Syntax error. goal is \"%s\" and it must be either Reward, Time, Cost, OReward, OTime, OCost \n", option.filename, l, goal);	
		
		if (s->goal[goal_id] == NA)
			die("%s:%d:\t Goal inconsitency. goal \"%s\" is defined in the order but not in the strategy \n", option.filename, l, &goal[4]);	

		s->goal_order[goal_order++] = goal_id;
		
		if(c[i] == ',')
			break;
		c += i + 1;
	}
	c += i + 1;
	
	while(1)
	{
		bzero(goal, 4096);
		value = NA;
		for (i = 0; c[i] != '^' && c[i] != ')' && c[i] != ',' && i < strlen(c); i++) ;
		if(i == strlen(c)) die("%s:%d:\t Strategy condition error\n", option.filename, l);
		strncpy(goal,c,i);
		
		if(goal[0] == '~')  {
				if (goal[1] == '-')	value = PFALSE;
			else if (goal[1] == '+')	value = PTRUE;
			else if (goal[1] == '*')	value = PALLTRUE;
			else if (goal[1] == '!')	value = PALLFALSE;
			else die("%s:%d:\tSet requiered element operator is '%c' and should be either:\n\t\t\t\t '+' to  verify that the set is TRUE for at least 1 node\n\t\t\t\t '-' to verify that the set is FALSE for at least 1 node\n\t\t\t\t '*' to verify that the set is TRUE for every node\n\t\t\t\t '!' to verify that the set is FALSE for every node\n\n", option.filename, l, goal[0]);
			s->path_cond = PATH;
		} else if(goal[0] == '|') {
			
			        if (goal[1] == '-')	value = LFALSE;
			else if (goal[1] == '+')	value = LTRUE;
			else if (goal[1] == '*')	value = LALLTRUE;
			else if (goal[1] == '!')	value = LALLFALSE;
			else die("%s:%d:\tSet requiered element operator is '%c' and should be either:\n\t\t\t\t '+' to  verify that the set is TRUE for at least 1 node\n\t\t\t\t '-' to verify that the set is FALSE for at least 1 node\n\t\t\t\t '*' to verify that the set is TRUE for every node\n\t\t\t\t '!' to verify that the set is FALSE for every node\n\n", option.filename, l, goal[0]);
			//Dont add this condition because it cause path cond to reset if multiple cond	s->path_cond = FALSE;
		} else if(goal[0] == '$') {
			        if (goal[1] == '-')	value = UFALSE;
			else if (goal[1] == '+')	value = UTRUE;
			else if (goal[1] == '*')	value = UALLTRUE;
			else if (goal[1] == '!')	value = UALLFALSE;
			else die("%s:%d:\tSet requiered element operator is '%c' and should be either:\n\t\t\t\t '+' to  verify that the set is TRUE for at least 1 node\n\t\t\t\t '-' to verify that the set is FALSE for at least 1 node\n\t\t\t\t '*' to verify that the set is TRUE for every node\n\t\t\t\t '!' to verify that the set is FALSE for every node\n\n", option.filename, l, goal[0]);
			s->path_cond = UNTIL;
		} else					die("%s:%d:\tGoal CTL operator is '%c' and should be either:\n\t\t\t\t '~' to verify the condition on every node of the strategy\n\t\t\t\t '|' to  verify the condition only at the last node:\n\t\t\t\t '$' to verify the condition hold on at least on a node of the path\n", option.filename, l, goal[0]);
		
//		printf("set: %s\n", set);
		if ((setnum = map_set_id_by_name(&goal[2])) == -1)
			die("%s:%d:\tSet %s is undeclared in the <sets> section\n", option.filename, l, goal);
		

		
		//add strategy condition
		s->cond[setnum] = value;

		if(c[i] == ')' || c[i] == ',') {
			c += i;
			break;
		}
		c += i + 1;
	}

	//it is a local strategy 	
	if(c[0] == ',') {
		c += 1;
		if(c[0] == '!') {
			s->loc_cond = FALSE;
			c += 1;
		} else
			s->loc_cond = TRUE;
			
		bzero(goal, sizeof(goal));
		for (i = 0; c[i] != ')' && c[i] != ' ' && i < strlen(c); i++) ;
		if(c[i] == ' ') die("%s:%d:\t Strategy location error: there is an extra space\n", option.filename, l);
		if(i == strlen(c)) die("%s:%d:\t Strategy location error: ')' is missing\n", option.filename, l);
		strncpy(goal,c,i);
		s->loc = location_get_id_by_name(goal);
		if(s->loc == 0)
			die("%s:%d:\tStrategy node location is '%s', it need to be an integer\n", option.filename, l, goal);
	} else {
		s->loc = NA;
	}
	
	
	//adding strategy
	net.nbstrat++; 
	s->next = strategy;
	strategy = s;
	
	//printf("%s:i=%d:strlen=%d\n", c, i, strlen(c));	
	
	
	
	//die("%s:%d\n", s->name, s->player);
}

/*!
 * parse the <sets> block
 * @param line the line to parse
 * @param l the line number
 * \version 1.0
 * \date    2007
 * \author Elie
 */

void parse_sets(char *line, int l)
{
	unsigned int i = 0, setnum = 0;
	int node;
	char	*c;

	while(line[i] != ':' && i < strlen(line)) i++;

	if(i != strlen(line)) {
		setnum = map_add(line, i, l);
	 } else if (!(strncmp(line,"</sets>",7))) {
		in_block = FALSE; 
		return;
	} else {
		die("%s:%d:\tSet syntax is invalid\n", option.filename, l, line); 
	}
	c = (line + i +1);
	if(!isdigit(c[0]) && strncmp(c, "rand",4) && strncmp(c, "true",4) && strncmp(c, "false",5) && c[0] != '\n')
		die("%s:%d:\tSet Node value must be numeric or set declared as random (keyword is rand) or true (keyword true) or false (keyword false) \n", option.filename);

	if(strncmp(c, "rand", 4) 	== 0)	{ sets_randomize(setnum);	c += 5;}
	else if(strncmp(c, "true", 4) 	== 0) 	{ sets_allnode(setnum, TRUE);	c += 5;}
	else if(strncmp(c, "false", 5) 	== 0) 	{ sets_allnode(setnum, FALSE);  c += 6;}
	else {
		while((node = atoi(c)))
		{	
			//adding node to the set
			if(node > net.nbnode)
				die("%s:%d:\tCan set node %d, max node is %d\n", option.filename, l, node, net.nbnode);
			if(option.debug == 2)
				out("%s:%d:\tAdding node %d to set %d\n",  option.filename, l, node, setnum);
			net.network_set[node][setnum] = TRUE;
			//next node
			while(c[0] != ',' && strlen(c) > 0) c++;
			if(c[0] == ',') c++;
			//if(c[0] == ':') { c++;	break;}
		}
	}

	/*
	if(strlen(c) > 0)
	{	cost = atoi(c);
		if(option.debug == 2)
			out("Add cost %d to set %s\n", cost, map_set_name_by_id(setnum));
		net.set_cost[setnum] = cost;
	}*/
}


/*!
 * parse the <penality> block
 * @param line the line to parse
 * @param l the line number
 * \version 1.0
 * \date    2007
 * \author Elie
 */

	
void parse_penality(char *line, int l)
{
	t_penality 	*y;
	char		*c;
	char		set[4096];
	int 		setnum;
	unsigned int	i;

	if (!(strncmp(line,"</penality>",11))) {
		in_block = FALSE; 
		return;
	}
	//A:4:!Public:10:ADD
	c = line;
	y = penality_new();


	     if(c[0] == 'A')	y->player = ADMIN;
	else if(c[0] == 'I') 	y->player = INCIDENT;
	else			die("%s:%d:\tPenality synthax error:\t Player must be either A or I\n", option.filename, l);
	c += 2;
	
	y->node = atoi(c);
	
	while((c[0] != ':') && (c[0] != '\n')) c++;

	if (c[0] == '\n')
		die("%s:%d:\tPenality synthax error\n", option.filename, l);
	c++;
	
	if((c[0] == '!')) {
		y->constraint = FALSE;
		c++;
	} else {
		y->constraint = TRUE;
	}

	bzero(set, 4096);

	for (i = 0; i < strlen(c) && c[i] != ':'; i++)
		set[i] = c[i];	

	if (i == strlen(c))
		die("%s:%d:\tPenality synthax error\n", option.filename, l);

	if ((setnum = map_set_id_by_name(set)) == -1)
		die("%s:%d:\tSet %s is undeclared in the <sets> section\n", option.filename, l, set);
	
	y->set = setnum;
	c = c + i + 1;
	
	y->value = atoi(c);
	if(y->value == 0)
		die("%s:%d:\tPenality value can't be 0\n", option.filename, l);
	
	while((c[0] != ':') && (c[0] != '\n')) c++;
	if (c[0] == '\n')
		die("%s:%d:\tPenality synthax error:\t missing penality law\n", option.filename, l);
	c++;

	     if (strncmp(c, "ADD", 3) == 0) y->law = ADD;
	else if (strncmp(c, "EXP", 3) == 0) y->law = EXP;
	else 
		die("%s:%d:\tPenality synthax error:\t wrong penality law\n", option.filename, l);
	
	for(i = 0; i < NBNODE; i++) {
		if (net.penality_counter[i] == FALSE)
		{
			net.penality_counter[i] = TRUE;
			y->counter_id = i;
			penality_add(y);
			return;
		}
	}
	
	die("%s:%d:\tPenality synthax error:\t Too many penality declared\n", option.filename, l);

}



/*!
 * parse the <graph> block
 * @param line the line to parse
 * @param l the line number
 * \version 1.1
 * \date    Jul 2008
 * \author Elie
 */

	
void parse_graph(char *line, int l)
{
	int				node1 = 0, node2 = 0, type = 0;
	unsigned int	i = 0;
	char			set[4096];
	char			*c;
	
	if (!(strncmp(line,"</graph>",8))) {
		in_block = FALSE; 
		return;
	}

	if(!strncmp(line,"eq-rand",7)) {
		graph_random_eq();
		return;
	}

	
	if(!strncmp(line,"dep-rand",8)) {
		graph_random_dep();
		return;
	}

	if(!strncmp(line,"loc-rand",8)) {
		if(atoi(&line[10]) ==  0)
			die("%s:%d:\tGraph syntax error:\t location max random value can't be 0\n", option.filename, l);
		graph_random_loc(atoi(&line[11]));
		return;
	}

	if(!strncmp(line,"value-rand",10)) {
		if(atoi(&line[12]) ==  0)
			die("%s:%d:\tGraph syntax error:\t cost random max random value can't be 0\n", option.filename, l);
		graph_random_cost(atoi(&line[11]));
		return;
	}

	c = line;	
	node1 = atoi(c);

	while (c[0] != '=' && c[0] != '-' && c[0] != ':' && c[0] != '|' && c[0] != '@')
	{
		c++;
		if (c[0] == '\n') die("%s:%d:\tGraph syntax error:\t node relation unknown. use either -> (dep), = (cost), : (cost), | (eq), @ (label)\n", option.filename, l);		
	}

	if(c[0] == '-')
	{
		c++;
		if (c[0] != '>') die("%s:%d:\tGraph syntax error:\t relation is either -> (dep) or = (eq) or : (cost)\n", option.filename, l);
		type = GRAPH_DEPENDENCY;
                c++;
	} else if (c[0] == '=') {
		type = GRAPH_COST;
		c++;
	} else if (c[0] == ':') {
		type = GRAPH_LABEL;
		c++;
	} else if (c[0] == '|') {
		type = GRAPH_EQUIVALENCE;
		c++;
	} else if (c[0] == '@') {
		type = GRAPH_LOC;
		c++;
	} else {
		die("%s:%d:\tGraph syntax error:\t node relation unknown. use either -> (dep), = (cost), : (cost), | (eq), @ (label)\n", option.filename, l);
		c++;
	}

	if (type != GRAPH_LABEL) {
		if(type == GRAPH_LOC) {
				bzero(set, sizeof(set));
				for (i = 0; c[i] != '\n' && i < strlen(c); i++) ;
				strncpy(set,c,i);
				node2 = location_get_id_by_name(set);
		} else {
			node2 = atoi(c);
                        if((type == GRAPH_DEPENDENCY || type == GRAPH_EQUIVALENCE) && node1 == node2 )
                            die("%s:%d:\tGraph syntax error:\t nodes values must be different\n", option.filename, l);

		}
		if (node2 == 0 && node1 == 0)
			die("%s:%d:\tGraph syntax error:\t nodes values must be numerical\n", option.filename, l);
		graph_add_relation(node1, node2, type);
	} else {
		graph_add_label(node1, c, l);
	}
}

/*!
 * parse the <rules> block
 * @param line the line to parse
 * @param l the line number
 * \version 1.2
 * \date   Aug  2008
 * \author Elie
 */

	
void parse_rules(char *line, int l)
{
	unsigned int i = 0;
	int setnum = -1, value;
	char	*c;
	char	set[4096];
	t_rule	*r;
	
	if (!(strncmp(line,"</rules>",8))) {
		in_block = FALSE; 
		return;
	}

	c = line;	
	r = allocate_rule();	

	//player
	switch(line[0]) {
		case 'I':
			r->player = INCIDENT;	
			break;
		case 'A':
			r->player = ADMIN;
			break;
		default:
			die("%s:%d:\tUnknown player %c\n", option.filename, l, line[0]);
			break;
	}
	c += 2;
	
	//time
	if ((r->time = atoi(c)) == 0)
		warning("%s:%d:\trule time is 0\n", option.filename, l);
	
	while(c[0] != ':')
	{
		c++;
		if (c[0] == '\n') die("%s:%d:\tRule syntax error\n", option.filename, l);		
	}
	c++;

	//name
	for (i = 0; c[i] != ':' && i < strlen(c); i++) ;
	if(i == strlen(c)) die("%s:%d:\tRule name error\n", option.filename, l);
	r->name = my_strndup(c, i);

	//pre
	c += i + 1;
	while(1)
	{
		bzero(set, 4096);
		for (i = 0; c[i] != '^' && c[i] != '-' && c[i] != '=' && i < strlen(c); i++) ;
		if(i == strlen(c)) die("%s:%d:\tRule pre condition error\n", option.filename, l);
		strncpy(set,c,i);
//		printf("set: %s\n", set);
		if ((setnum = map_set_id_by_name(set)) == -1)
			die("%s:%d:\tPreconditions:\tSet '%s' is undeclared in the <sets> section\n", option.filename, l, set);
		
		     if(set[0] == '!')	value = FALSE;	 //node not in set
		else if(set[0] == '>')	value = SUCC;	 //succ in set
		else if(set[0] == '~')	value = NOTSUCC; //succ not in set
		else			value = TRUE;	 //node in set
		//add pre cond to the rule
		rule_add_pre(r, setnum, value);
		//no more set in pre
		if(c[i] == '-')
		{
			r->grant_reward = FALSE;
			break;
		}
		
		if(c[i] == '=')
		{
			r->grant_reward = TRUE;
			break;
		}
		c += i + 1;
	}
	c += i + 2;

	//post
	while(1)
	{
		bzero(set, 4096);
		for (i = 0; (c[i] != '^') &&  (c[i] != '\n') && (c[i] != ':')  && (c[i] != '@') && (i < strlen(c)); i++) ;
		//if(i == strlen(c)) die("%s:%d:\tRule post condition error\n", option.filename, l);
		strncpy(set,c,i);
//		printf("post:%s\n", set);
		if ((setnum = map_set_id_by_name(set)) == -1)
			die("%s:%d:\tpostconditions:\tSet '%s' is undeclared in the <sets> section\n", option.filename, l, set);
		     	     if(set[0] == '!')	value = FALSE;	 //node not in set
			else if(set[0] == '>')	value = SUCC;	 //succ in set
			else if(set[0] == '~')	value = NOTSUCC; //succ not in set
			else			value = TRUE;	 //node in set
			//add pre cond to the rule
			rule_add_effect(r, setnum, value);
		
		if(c[i] == '\n' || c[i] == ':' || c[i] == '@') {
			c += i;
			break;
		}
		c += i + 1;
	}


	//cost
	if(c[0] == ':') {
		c += 1;
		r->cost = atoi(c);
		if (r->cost == 0)		
				die("%s:%d:\tRule cost specified is not correct. It must be an integer and it is currently:'%s'", option.filename, l, c);
		for(i = 0; c[i] != ':' && c[i] != '@' && c[i] != '\n'; i++); {
                    ;	
                    //if(c[i] <= '0' || c[i] >= '9')
			//	die("%s:%d:\tRule cost specified is not correct. It must be an integer and it is currently: %s", option.filename, l, c);
		}
		c += i;
	}

	//location
	if(c[0] == '@') {
		c += 1;
		bzero(set, sizeof(set));
		for (i = 0; c[i] != '\n' && c[i] != ':' && i < strlen(c); i++) ;
		strncpy(set,c,i);
		r->loc = location_get_id_by_name(set);
		if (r->loc == 0)		
			die("%s:%d:\tRule node location specified is not correct. It must be an integer and it is currently: %s", option.filename, l, c);
		c += i + 1;
	}

	//succ location
	if(c[0] == ':') {
		c += 1;
		bzero(set, sizeof(set));
		for (i = 0; c[i] != '\n' && c[i] != ':' && i < strlen(c); i++) ;
		strncpy(set,c,i);
		r->sloc = location_get_id_by_name(set);
		if (r->sloc == 0)		
			die("%s:%d:\tRule node successor location specified is not correct. It must be an integer and it is currently: %s", option.filename, l, c);

	}
	///if unspecified the successor location should remain unspecified
	// else {
	//	r->sloc = r->loc;
	//} 
	//parsing finished, clip it
	rule_add(r);
}

/*!
 * select the block parser to call
 * @param line the line to parse
 * @param l the line number
 * \version 1.0
 * \date    2007
 * \author Elie
 */


void parse_block(char *line, int l)
{
	//verify that everything is okay before changing block
	parse_check_param();
	switch (in_block) {
		case	BLOCK_RULES:
			parse_rules(line, l);
		break;
		case	BLOCK_SETS:
			parse_sets(line, l);
		break;
		case	BLOCK_GRAPH:
			parse_graph(line, l);
		break;
		case	BLOCK_PENALITY:
			parse_penality(line, l);
		break;
		default:
			warning("%s:%l:\tUnknown block '%s'\n", option.filename, l, line);
	}
}

/*!
 * change the current bloc when an open block tag is detected
 * @param line the line to parse
 * @param l the line number
 * \version 1.0
 * \date    2007
 * \author Elie
 */

void select_block(char *line, int l)
{
	     if (strncmp(line,"<rules>",7) == 0) 	in_block = BLOCK_RULES;
	else if (strncmp(line,"<sets>",6) == 0) 	{ map_init(); in_block = BLOCK_SETS; }
	else if (strncmp(line,"<graph>",7) == 0)	in_block = BLOCK_GRAPH;
	else if (strncmp(line,"<penality>", 10) == 0)	in_block = BLOCK_PENALITY;
	else	die("%s:%d:\tUnknown Block type:\t%s\n", option.filename, l, line); 
}


