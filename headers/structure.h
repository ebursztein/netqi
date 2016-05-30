/*!	NetQI: http://www.netqi.org
*	Copyright Elie bursztein & LSV, ENS-Cachan, CNRS, INRIA 
*
*/

/*!
 *       \file structure.h
 *       \brief hold structure def
 *       \author  Elie
 *       \version 1.5
 *       \date    Fev 2008
 */
#ifndef _STRUCTURE_H
#define _STRUCTURE_H
#include "constant.h"


typedef struct	s_rule {	
	unsigned int	id;							///!<rule id used to make easy relation with the rule database.
	char			*name;						///!<rule name
	int			player;						///!< rule's owner
	int			time;						///!< requiered time to launch the rule
	int			grant_reward;					///!< does the rule grant a reward
	int			cost;                                           ///!< rule cost of execution
	int			loc;                                            ///!< rule location scope
	int			sloc;						///!< rule sucessor location scope
	char			pre[NBSET];                                     ///!< direct pre_condition requiered to launch the rules.
	char			pre_succ[NBSET];				///!<successor pre condition
	char			effect[NBSET];					///!< effect on the node.
	char			effect_succ[NBSET];				///!< effect on successor
	struct 			s_rule *next;					///!< chained list
} t_rule;									///!< rule structure

typedef struct s_player {
	int			time;						///!< rule execution time 
	char			*name;						///!< player name
	t_rule 			*rule;						///!< which rule is currently launched
	int			rule_status;					///!< is the rule is executed or is missed
	int			ellapsed;					///!< time ellapsed
	int 			target;						///!< which node is targeted;
	int			target_succ;					///!< target succ
	long long		rule_income;					///!< payoff
	long long		rule_cost;					///!< cost of using the rule
	long long		last_time;                                      ///!< last execution time 
	int     		nbrule;                                         ///!< nb rules
	t_rule			**sorted_rule;					///!< ordered list of rules
	t_rule			*rule_set;
} t_player;									///!< player structure

typedef	struct s_set {
	char			*name;						///!<set name
	int			id;						///!<set id
	char			*color;						///!<color associated with the set
	struct	s_set		*next;			
} t_set;                                                                        ///!< set structure

//using structure in case we need to add more information. Performance is not an issue here.
typedef struct s_location {
	char			*name;						///!<location friendly name
	int			id;						///!<location numerical id
	struct s_location	*next;
} t_location;

typedef struct s_map {
	int			nbset;						///!< number of sets 
	int			nblocation;					///!< number of locations
	t_set			*set;						///!< set structure chained list
	t_location		*location;					///!< location structure chained list
} t_map;

typedef struct	s_net {
	int			nbnode;						///!< number of nodes
	int			nbstrat;					///!< number of strategy
	char			network_set[NBNODE][NBSET]; 			///!<set node mapping
	char			network_edge[NBNODE][NBNODE]; 			///!<node dependency [from][to]
	char			network_node_relation[NBNODE][NBNODE];          ///!<node equality
	int			node_cost[NBNODE];				///!<node cost
	int			node_loc[NBNODE];				///!<node localization
	char			*node_label[NBNODE];				///!<allow to associate a string name to node
	int			penality_counter[NBNODE];			///!<penality counter
} t_net;


typedef struct s_penality {
	int			player;						///!<which player is subject to the constaint
	int			node;						///!<node concerned by the penality
	int			value;						///!<penality value
	int			law;						///!<penality law
	int                     counter_id;					///!<which time counter is associated with the penality
	int                     set;						///!<set id to check
	int			constraint;					///!<constraint value (TRUE/FALSE)
	struct s_penality	*next;
} t_penality;

typedef struct s_option {
	int			generate;					///!<randomize the graph
	int			generateSet;					///!<randomize sets members
	int			mode;						///!<type of analysis wanted
	int			mode_opt;					///!<analysis optionnal mode (used to select the stratgy search mode for instance)
	int			abstraction_type;				///!<specify if a given type of abstraction will be used: upper or under approximation default == EXACT
	char			*filename;					///!<filename for the game to check
        FILE                    *graphfile;                                     ///!<file for the game complete graph output
        FILE			*fileout;					///!<specify a file for the output
	unsigned long		delay;						///!<does we need to run in realtime (sleep for 1 sec between each tick)
	char			quiet;						///!<for quiet output (no warning)
	int			debug;						///!<for debug output control
	unsigned int		timeout;					///!<force the analysis to exist after x sec
	int			default_node_value;				///!<default cost value for graph node
	int			default_set_value;				///!<default cost value for graph node
	unsigned int 		start_time;					///!<starttime
	int			progress;					///!<display progress 
	unsigned int		die;						///!<force the analysis to die at a given node. used for backtrack
} t_option;

typedef struct	s_tree {
	unsigned long long	num;						///!< node num
	char			loop;						///!<is this a loop
//admin state
	int			admin_time; 					///!< rule execution time 
	t_rule*			admin_rule; 					///!< which rule is currently launched
	int			admin_rule_status;				///!< is the rule is executed or is missed
	int			admin_ellapsed;					///!< time ellapsed
	int 			admin_target;					///!< which node is targeted;
	int			admin_target_succ;				///!< target succ
//incident state
	int			incident_time;					///!< rule execution time 
	t_rule*			incident_rule; 					///!< which rule is currently launched
	int			incident_rule_status;				///!< is the rule is executed or is missed
	int			incident_ellapsed;				///!< time ellapsed
	int 			incident_target;				///!< which node is targeted;
	int			incident_target_succ;				///!< target succ

//node set state

	char			target_set_state[NBSET];
	char			target_set_state_succ[NBSET];

//game states
	long long		num_admin_rule;
	long long		num_incident_rule;
	long long		admin_global_income;
	long long		admin_global_cost;
	long long		admin_global_last_time;
	long long		incident_global_income;
	long long		incident_global_cost;
	long long		incident_global_last_time;
	long long		time;						///!< current time
	char			**network_set;					///!<used for strategies
	//char 			network_set[NBNODE][NBSET]; 			///!<set node mapping
	int			penality_counter[NBNODE];			///!<penality counter

			
//structure
	struct s_tree		*father;					///!< node father
	struct s_tree		*child;						///!< node childs
	struct s_tree		*sibilin;					///!< sibilin tree
} t_tree;


typedef struct s_strategy {
	unsigned int		id;						///!< id
	char			*name;						///!< Scenario name
	int			player;						///!< Player ADMIN/INCIDENT
	int			loc;						///!<strategy location scope
	int			loc_cond;					///!<is a location restriction (TRUE) or a location exclusion (FALSE) ?
	int			path_cond;					///!<does the strategy have a condition that hold during all the execution ? if yes it can be used for early evaluation
	unsigned long long	maxtime;					///!<threshold enforcement
	unsigned long long	maxotime;
	unsigned long long	maxcost;
	unsigned long long	maxocost;
	unsigned long long	maxreward;
	unsigned long long	maxoreward;
	unsigned long long	mintime;					///!<threshold enforcement
	unsigned long long	minotime;
	unsigned long long	mincost;
	unsigned long long	minocost;
	unsigned long long	minreward;
	unsigned long long	minoreward;
	unsigned long long	time;						///!< player execution time
	unsigned long long	otime;						///!< opponent execution time
	unsigned long long	cost;						///!< player cost
	unsigned long long	ocost;						///!< opponent cost
	unsigned long long	reward;						///!< player reward
	unsigned long long	oreward;					///!< player opponnent reward
	unsigned long long	nbgoal;
	int			goal[NBGOAL];                                   ///!< what are the goal seeked
	int			goal_order[NBGOAL];				///!< goal order
	int			cond[NBSET];					///!< which are the requiered condition
	t_tree			*trace;						///!< strategy trace
	struct	s_strategy	*next;						///!< chained list
} t_strategy;											///!< Scenario structure

typedef struct s_formula {
	char			*formula;
	int			win;                                            ///!<store if the formula hold or not.
	int			asbox;                                          ///!<allow to quickly verify if one of the formula cond is a CTL box
        int                     asdiam;                                         ///!<allow to quickly verify if one of the formula cond is a CTL diamond
        int                     cond[NBSET];                                    ///!<condition on set to be verified
	int			node[NBSET];                                    ///!<condtition target node
	int			set[NBSET];                                     ///!<condition target set
	char			box[NBSET];                                     ///!<is the condition a box  ? 
        char                    diam[NBSET];                                    ///!<is the condition a diamond ?
        char                    diam_was_true[NBSET];                           ///!<This is set to true as the condition is true for in one state. Used for final evaluation.
        ///!< warning this is not the formula it self: Theses are the values the condition that need to be true to trigger the end of the Model check. 
} t_formula;

typedef struct	s_stat {
	unsigned long long	admin_num_rule;			///!<how many admin rules where triggered
	unsigned long long	admin_num_success_rule;		///!<how many admin success rules where triggered
	unsigned long long	incident_num_rule;		///!<how many incident rules where triggered
	unsigned long long	incident_num_success_rule;	///!<how many incident success rules where triggered
	unsigned long long	state;				///!<number of state
	unsigned long long	exec;				///!<number of execution
	unsigned long long	backtrack;			///!<num of backtrack
} t_stat;									///!<game check statistique
#endif
