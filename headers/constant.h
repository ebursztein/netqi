/*!	NetQI: http://www.netqi.org
*	Copyright Elie bursztein & LSV, ENS-Cachan, CNRS, INRIA 
*
*/

/*!
 *       \file constant.h
 *       \brief hold constants
 *       \author  Elie
 *       \version 1.12
 *       \date    Feb 2008
 */

#ifndef _CONSTANT_H
#define _CONSTANT_H 1

#define VERSION			"1.2"

#define NBNODE			10000			///!< max number of node
#define NBSET			25			///!< maximum of set
#define NBGOAL			10			///!<number of scenario goals
#define HOST_COST		1			///!< host default cost
#define SET_COST		0			///!< set default cost
#define PROGSTEP		100000			///!< number of step between outputing progress information
#define STALLED_MAX		5			///!< how many consecutives executions without improvement before giving up
#define LATEX			42




#define	ADMIN_NAME		"Admin"			///!< admin fancy name
#define INCIDENT_NAME		"Intruder"		///!< incident fancy name
	
#define	ADMIN			1
#define	INCIDENT		2



#define		ADD		1
#define 	EXP		2

 ///\note Be sure that negative operator are negative, it used in the utility_function to know if set cost need to be added or substracted
#define		UNTIL			69
#define		PATH			68
#define		LEAF			67
#define		LOOP			66
#define		UALLTRUE		61			//all true at one point of the path
#define		UALLFALSE		60			///all false at one point of the path
#define		UTRUE			59			//one true at one point of the path
#define		UFALSE			58			//one false at one point of the path
#define		LALLTRUE		57
#define		LALLFALSE		56
#define		LTRUE			55			
#define		LFALSE			54			//leaf false
#define		PALLFALSE		53			//path all true
#define		PALLTRUE		52
#define		PTRUE			51
#define		PFALSE			50
#define		PENDING			42			//rule is selected by the player and is waiting to be triggered
#define		MIN			5
#define		MAX			4
#define 	ALLSUCC			3
#define 	SUCC			2
#define 	TRUE			1
#define 	ZERO			0			//do not use it					
#define		FALSE			-1 			//don't ever change it
#define		NOTSUCC			-2
#define		NOTALLSUCC		-3
#define 	NA			-4
#define 	UNSET			-42
#define		INFINITY		-100
#define		PROHIBITED		-200			//this choice will lead to an execution that violate the strat property so we avoid it

//goal 

#define		COST		1
#define		OCOST		2
#define		REWARD		3
#define		OREWARD		4
#define		TIME		5
#define		OTIME		6	


//analyze mode
#define		SIMULATION	1
#define		MODELCHECK	2
#define 	STRATEGY	3
//#define 	ABSTRACTINTER	4

//analyze sub mode
#define		FULL		1	
#define		INSTANCE	2

//analysis abstraction

#define 	EXACT		1	///!<abstraction mode is set to exact
#define		UPPER		2	///!<abstraction mode is set to upper approximate
#define		UNDER		3	///!<abstraction mode is set to under approximate

//type of relation
#define GRAPH_UNKNOWN		0
#define GRAPH_DEPENDENCY	1
#define GRAPH_EQUIVALENCE	2
#define GRAPH_COST		3
#define GRAPH_LOC		4
#define GRAPH_LABEL		5

#define FINAL			1
#define PROGRESS		2

//fancy color definition
#define RED	"\033[31m"
#define BRED	"\033[1;31m"
#define GRE	"\033[32m"
#define BGRE	"\033[1;32m"
#define YEL	"\033[33m"
#define BYEL	"\033[1;33m"
#define BLU	"\033[34m"
#define BBLU	"\033[1;34m"
#define PUR	"\033[35m"
#define BPUR	"\033[1;35m"
#define CYA	"\033[36m"
#define BCYA	"\033[1;36m"
//fancy special
#define CLR	"\033[0m"
#define ALRT	"\033[1;41m"

#endif
