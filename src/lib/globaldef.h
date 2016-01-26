/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2015 by Dmitry Tsarkov

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef GLOBALDEF_H
#define GLOBALDEF_H

// global definitions for FaCT++ Reasoning Kernel

// define unused attribute for parameters and (un)likely macro for conditions
#if defined(__GNUC__) && (__GNUC__ >= 4)
#	define ATTR_UNUSED __attribute__((unused))
#	define likely(cond) __builtin_expect((cond),1)
#	define unlikely(cond) __builtin_expect((cond),0)
#else
#	define ATTR_UNUSED
#	define likely(cond) (cond)
#	define unlikely(cond) (cond)
#endif

// uncomment this to have a DAG usage statistics printed
//#define RKG_PRINT_DAG_USAGE

// uncomment this to have sorted ontology reasoning
#define RKG_USE_SORTED_REASONING

//#define _USE_LOGGING

//#define ENABLE_CHECKING

// uncomment this to allow dynamic backjumping
//#define RKG_USE_DYNAMIC_BACKJUMPING

#ifdef RKG_USE_DYNAMIC_BACKJUMPING
// uncomment this to use improves S/R with better quality
#	define RKG_IMPROVE_SAVE_RESTORE_DEPSET
#endif

// set to 1 to update role's R&D from super-roles
#ifndef RKG_UPDATE_RND_FROM_SUPERROLES
#	define RKG_UPDATE_RND_FROM_SUPERROLES 0
#endif

// uncomment this to allow simple rules processing
//#define RKG_USE_SIMPLE_RULES

// uncomment this to support fairness constraints
//#define RKG_USE_FAIRNESS

// uncomment the following line if IR is defined as a list of elements in node label
#define RKG_IR_IN_NODE_LABEL

// this value is used in classes Reasoner, CGraph and RareSaveStack
const unsigned int InitBranchingLevelValue = 1;

#endif
