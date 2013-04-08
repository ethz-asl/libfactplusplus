/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2013 by Dmitry Tsarkov

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "Kernel.h"
#include "Actor.h"

typedef std::multimap<std::string, ReasoningKernel::TConceptExpr*> V2CMap;

std::map<std::string, int> Var2I;
std::vector<std::string> I2Var;


/// fills in variable index
void fillVarIndex ( const V2CMap& query )
{
	size_t n = 0;
	Var2I.clear();
	I2Var.clear();
	for ( V2CMap::const_iterator p = query.begin(), p_end = query.end(); p != p_end; ++p )
		if ( Var2I.count(p->first) == 0 )	// new name
		{
			Var2I[p->first] = n++;
			I2Var.push_back(p->first);
		}
	fpp_assert ( I2Var.size() == n );
}


void
ReasoningKernel :: evaluateQuery ( const V2CMap& query )
{
	// make index of all vars
	fillVarIndex(query);

	if ( I2Var.empty() )
	{
		std::cout << "No query variables\n";
		return;
	}

	// for every var: create an expression of vars
	std::vector<TConceptExpr*> Concepts;
	std::cout << "Tuple <";
	TExpressionManager* pEM = getExpressionManager();
	for ( size_t i = 0; i < I2Var.size(); ++i )
	{
		const std::string& var = I2Var[i];
		if ( i != 0 )
			std::cout << ", ";
		std::cout << var.c_str();
		pEM->newArgList();
		for ( V2CMap::const_iterator p = query.lower_bound(var), p_end = query.upper_bound(var); p != p_end; ++p )
			pEM->addArg(p->second);
		Concepts.push_back(pEM->And());
	}
	std::cout << ">\n";

	if ( I2Var.size() == 1 )	// tree-like query
	{
		Actor a;
		a.needIndividuals();
		getInstances ( Concepts.back(), a );
		const char** names = a.getElements1D();
		for ( const char* name = *names; name; name++ )
			std::cout << name << "\n";
		std::cout << std::endl;
		return;
	}

}
