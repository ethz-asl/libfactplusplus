/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2008 by Dmitry Tsarkov

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef _GLOBALDEF_H
#define _GLOBALDEF_H

// global definitions for FaCT++ Reasoning Kernel

// define unused attribute for parameters
#if defined(__GNUC__) && (__GNUC__ >= 4)
#	define ATTR_UNUSED __attribute__((unused))
#else
#	define ATTR_UNUSED
#endif

// uncomment this to have a DAG usage statistics printed
//#define RKG_PRINT_DAG_USAGE

// uncomment this to have a DIG-passed information printed
#define RKG_PRINT_DIG_MESSAGES

// uncomment this to have sorted ontology reasoning
#define RKG_USE_SORTED_REASONING

// uncomment this to have absorption debug messages
//#define RKG_DEBUG_ABSORPTION

// uncomment this to allow dynamic backjumping
//#define RKG_USE_DYNAMIC_BACKJUMPING

#ifdef RKG_USE_DYNAMIC_BACKJUMPING
// uncomment this to use improves S/R with better quality
#	define RKG_IMPROVE_SAVE_RESTORE_DEPSET
#endif

// uncomment this to update role's R&D from super-roles
//#define RKG_UPDATE_RND_FROM_SUPERROLES

// Flag to choose kind of NR support.

// If 0, then simplistic one is used.  For the >= n R, at most one R-successor is created.  For
//	both >= n R and <= n R label is scanned to find contradiction between N's w.r.t. the "same" R.
//	This approach is not applicable for logics with nominals and/or with role hierarchy.

// if 1, then usual one is used.  For >= n R, N different (i.e., they all are in inequality relation)
//	R-successor are created.  The only clash condition is that <= n.R fails to merge nodes due to
//	inequality relation and/or label clash between nodes.
#define RKG_HIERARCHY_NR_TACTIC 1

// uncomment the following line if IR is defined as a list of elements in node label
#define RKG_IR_IN_NODE_LABEL

// switch saving/restoring AFFECTED flag
//#define RKG_SAVE_AFFECTED

#endif
