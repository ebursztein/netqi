/*!	NetQI: http://www.netqi.org
*	Copyright Elie bursztein & LSV, ENS-Cachan, CNRS, INRIA 
*
*/

/*!
 *       \file main.c
 *       \brief Model checker entry point
 *
 *       Netforsight Model checker for incidentLogic
 *	 Hold every global variables
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
t_net		net;					///!<network structure
t_map		*map;			///!<map information : sets and nodes
t_player	*admin;				///!<admin player structure
t_player	*incident;				///!<incident player structure
t_option	option;				///!<store option 
t_stat		stat;				///!<statistic structure
t_tree		*last_branch;		///!<used for branch detection
t_tree		*root;			///!<tree roots
t_tree		*tree;			///!<current tree node
t_strategy	*strategy;		///!<strategies
t_formula	*formula;		///!<tatl formula used for modelcheck
t_penality	*penality;		///!<penalities
/*!
 * general init function 
 * \version 1.0
 * \date    Sept 2007
 * \author Elie
 */
void init()
{
	
	unsigned int i;
	
	option.default_set_value = SET_COST;
	option.default_node_value = HOST_COST;
	option.start_time = time(NULL);
	
	//rand
	i = time(NULL);
	srand(i);


	//init tree;
	
	root = NULL;
	tree = NULL;
	last_branch = NULL;
	tree_init();
	
	//stat initi

	bzero(&stat, sizeof(t_stat));	
	
	//init strategy
	strategy = NULL;
	
	//init formula
	formula = NULL;

	//player init
	admin = malloc(sizeof(t_player));
	bzero(admin, sizeof(t_player));
	admin->name = ADMIN_NAME;
	admin->target = -1;
	admin->target_succ = UNSET;
	admin->rule_status = UNSET;
	incident = malloc(sizeof(t_player));
	bzero(incident, sizeof(t_player));
	incident->name = INCIDENT_NAME;
	incident->target = -1;
	incident->target_succ = UNSET;
	incident->rule_status = UNSET;
	
	//init graph
	graph_init();
	
	//init penality

	penality = NULL;

	//parse game file
	parse(option.filename);

	//preprocessing game to statically optimize it
	preprocessor();
}

/*!
 * Main function 
 * @param ac  argcount
 * @param av  argvalue
 * \version 1.0
 * \date    Sept 2007
 * \author Elie
 */
int main (int ac, char **av) {
	
	char c;	
	

	bzero(&option, sizeof(option));
	option.fileout = stdout;	
	while ((c = getopt(ac, av, "lf:r:o:d:t:hg:wpiGCISD:")) != -1)
   	switch (c)
    	{
	case 'S':
		option.mode 	= STRATEGY;
		option.mode_opt	= FULL;
		break;
	case 'I':
		option.mode 	= STRATEGY;
		option.mode_opt = INSTANCE;
		break;
	case 'C':
		option.mode = MODELCHECK;
		break;
	case 'G':
		option.mode = SIMULATION;
		break;
	case 'D':
		option.die = atoi(optarg);
		break;	
	case 'f':
		option.filename = (char *)my_strndup(optarg, strlen(optarg));
		break;
	case 'h':
		help();
		break;
	case 't':
		option.timeout = atoi(optarg);
		break;		
	case 'r':
		option.delay = atoi(optarg);
		break;
	case 'w':
		option.quiet = 1;
		break;
        case 'i':
                option.quiet = LATEX;
                option.debug = 3;
                
        case 'l':
		option.quiet = LATEX;
		break;
	case 'o':
		if (!(option.fileout = fopen(optarg, "w")))
                    die("Option -o:can't open output file:\t%s\n", optarg);
		break;
        case 'g':
                if (!(option.graphfile = fopen(optarg, "w")))
                    die("Option -g:can't open graph file:\t%s\n", optarg);
                break;
                
	case 'd':
		option.debug = atoi(optarg);
		break;
	case 'p':
			option.progress++;
	break;
	default:
		help();
		break;
	}

	if(option.filename == NULL)
		help();

	//init
	init();
	
	//run
	if(!option.mode)
	{
		warning("Please choose an mode: -G for a single game execution or -C for model check a formula or -S for strategy analysis\n");
		help();
	}
	
	game_init();
	
	game_run();
	return 42;
}

/*!
 * help function
 * \version 1.2
 * \date    Feb 2008
 * \author Elie
 */
void help() {
die(
"NetQi Game Engine version %s\n\
Usage:./NetQi [engine mode: -G|-S|-C] [-f gamefile]\n\
mandatory flags:\n\
		\t\t%s-f gamefile%s: specify the file that contains the game data.\n\
		\t\tEngine mode: %s-G%s for a single play execution or %s-C%s for model check a formula, %s-S%s for strategy analysis, %s-A%s for strategy heuristic.\n\
Optional flags:\n\
		\t\t%s-o filename%s: write the program output in the file in XML format.\n\
                \t\t%s-g filename%s: write the complete game graph in the file in XML format.\n\
		\t\t-t sec: force the engine to timeout after x sec.\n\
		\t\t-p: display progress information each %d states\n\
		\t\t-pp for displaying the execution.\n\
		\t\t-ppp for strategy refinement.\n\
		\t\t-r delay: to add a delay in ms between each iteration.\n\
		\t\t-w: show analysis warning such as loop or timeout trigger.\n\
Latex output:\n\
                \t\t-l: output trace formated in latex.\n\
		\t\t-i: output rules fromated in latex (-i implies -l).\n\
Debuging option -d level:\n\
		\t\t1 for random,	\t2 for sets,		\t3 for rules,\n\
		\t\t4 tested rules,	\t5 rule failure,		\t6 for graph relations,\n\
		\t\t7 for execution tree operation,\n\
		\t\t8 for display each tree branch,		\t\t9 for each node tested (very verbose)\n\
		\t\t10 for strategies, 	11 for strategy abstact, 	12 for penality,\n\
		\t\t13 for preprocessing, \t14 locations,	\t\t15 for TATL formula\n\
",VERSION, CYA, CLR, YEL, CLR,YEL, CLR, YEL, CLR, YEL, CLR, GRE, CLR, PUR, CLR, PROGSTEP);
}

