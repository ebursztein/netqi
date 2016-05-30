/*!	NetQI: http://www.netqi.org
*	Copyright Elie bursztein & LSV, ENS-Cachan, CNRS, INRIA 
*
*/

/*!
 *       \file function.h
 *       \brief hold functions prototypes
 *       \author  Elie
 *       \version 1.7
 *       \date    Aug 2008
 */
#ifndef _FUNCTION_H
#define _FUNCTION_H 1

//rule.c;
int		can_use_rule(t_rule *r, int start, int tabou);
int		test_rule(t_rule *r, int node);
void		rule_store(t_player *p);
void		exec_rule(t_player *p, int success);
t_rule		*allocate_rule();
void		rule_set_id(t_player *p);
void		order_rule_set_by_time(t_player *p);
void		rule_add_pre(t_rule *r, int set, int value);
void		rule_add_effect(t_rule *r, int set, int value);
void		rule_add(t_rule *r);
void		rule_remove(t_player *p, t_rule *rule, t_rule *pred);
void		rule_display(t_rule *r);
void		player_rule_display(t_player *p);
void		rule_to_array();

//game.c
void		game_init();
void		game_run();
int		game_backtrack();
int		game_final();
int             get_rule(t_player *p, int start_rule, int start_node, int last_rule, int last_node);


//main.c 
void		help();

//location.c
int			location_get_id_by_name(char *name);
char		*location_get_name_by_id(int id);

//preprocessor.c
void		preprocessor();
int			map_add(char *line, unsigned int size, int l);
void		map_init();
int			map_set_id_by_name(char *str);
char		*map_set_state_by_id(int id);
char 		*map_set_name_by_id(int id);

//graph.c
void		graph_init();
void		graph_random_eq();
void		graph_random_dep();
void		graph_random_cost(int max);
void		graph_random_loc(int max);
void		graph_add_relation(int node1, int node2, int type);
void		graph_add_label(int node, char* label, int l);

//tree.c
void		tree_init();
int			tree_node_add();
void		tree_node_remove(t_tree *t);
int			tree_node_eq(t_tree *t1, t_tree *t2);
t_tree		*tree_node_dup(t_tree *src);
void		tree_add_child(t_tree *f, t_tree *c); 
void		tree_climb_up();
void		tree_node_remove_double_choice(t_tree *t);
void		tree_free(t_tree *t);


//tree_branch.c
void		tree_branch_display(t_tree *src);
t_tree		*tree_branch_last_save(t_tree *dst, t_tree *src);
int			tree_branch_eq(t_tree *tree1, t_tree *tree2);

//output.c
void		output_header();
void		output_footer();
void		output_tree_node(t_tree *t);
void		output_timeout();
void		output_stat(int type);
void		output_strategy_stat();
void		output_strategy_value_stdout(t_strategy *s); ///\note used for debug 10
void		output_progress();
void		output_strategy();
void		output_modelcheck();
void		output_header_progress();
void		output_footer_progress();

//strategy.c function
t_strategy *strategy_new();
void 		strategy_evaluate_execution();
char 		*strategy_goal_name_by_id(int id);
int 		strategy_goal_id_by_name(char *str);
void		strategy_list_display();
void		strategy_evaluate_loop();
int 		strategy_early_cut();
 
//formula.c function
t_formula	*formula_new();
int			formula_eval(t_formula *f, t_tree *tr, int final);
void		formula_add_constraint(t_formula *f, int set, int node, int value);

//utility_function.c
void		utility_function(t_player *p);
void		utility_function_penality();

//penality.c
t_penality	*penality_new();
int			penality_add(t_penality *y);

//util.c
char		*my_strndup(char *str, unsigned int n);
void		*xmalloc(size_t size);
void		die(char *err, ...);
void		warning(char *err, ...);
int			out(char *str, ...);
int			debug(int lvl, char *str, ...);
void		sets_randomize(unsigned int setnum);
void		sets_allnode(unsigned int setnum, int value);



//parse.c
void		parse(char *file);
#endif

//this macro is mandatory to compile properly under cygwin since bzero is deperacted
#define bzero(b,len) (memset((b), '\0', (len)), (void) 0)
