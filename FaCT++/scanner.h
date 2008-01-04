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

//-------------------------------------------------------------------------
//
//  Scanner class for FaCT++
//
//-------------------------------------------------------------------------
#ifndef __SCANNER_H
#define __SCANNER_H

#include <cstdlib>
#include <cstring>
#include <iostream>

#include "grammar.h"

/// max ID length for scanned objects
const unsigned int MaxIDLength = 1024;

/// more-or-less general simple scanner implementation
class TsScanner
{
protected:	// members
		/// input stream
	std::istream* InFile;
		/// buffer for names
	char LexBuff [ MaxIDLength + 1 ];
		/// currently processed line of input (used in error diagnosys)
	unsigned int CurLine;

protected:	// methods
		/// fill buffer with name in '|'-s; c should be starting '|'
	void FillNameBuffer ( register char c );
		/// fill buffer with legal ID chars, starting from c
	void FillBuffer ( register char c );
		/// get next symbol from the stream
	char NextChar ( void ) const { return InFile->get(); }
		/// return given symbol back to stream
	void PutBack ( char c ) const { InFile->putback(c); }
		/// check if given character is legal in ID
	bool isLegalIdChar ( char c ) const;

public:		// interface
		/// c'tor
	TsScanner ( std::istream* inp )
		: InFile(inp)
		, CurLine(1)
		{}
		/// d'tor
	~TsScanner ( void ) {}

		/// get next token from stream
	Token GetLex ( void );
		/// get string collected in buffer
	const char* GetName ( void ) const { return LexBuff; }
		/// get number by string from buffer
	unsigned long GetNumber ( void ) const { return atol ( LexBuff ); }
		/// get current input line
	unsigned int Line ( void ) const { return CurLine; }
		/// check if Buffer contains given Word (in any register)
	bool isKeyword ( const char* Word ) const;
		/// get keyword for a command by given text in buffer; @return BAD_LEX if no keyword found
	Token getCommandKeyword ( void ) const;
		/// get keyword for a concept/role expression; @return BAD_LEX if no keyword found
	Token getExpressionKeyword ( void ) const;
		/// get keyword for a concept/role special name; @return ID if no keyword found
	Token getNameKeyword ( void ) const;

		/// reset scanner on the same file
	void ReSet ( void )
	{
		InFile->clear();
		InFile->seekg ( 0L, std::ios::beg );
		CurLine = 1;
	}
		/// reset scanner to a given file
	void reIn ( std::istream* in ) { InFile = in; CurLine = 1; }

		/// output an error message
	void error ( const char* msg = NULL ) const
	{
		std::cerr << "\nError at input line " << Line() << ": "
				  << (msg?msg:"illegal syntax") << std::endl;
		exit (1);
	}
};	// TsScanner

inline bool TsScanner :: isKeyword ( const char* Word ) const
{
	// fast inequality check
	return strlen(Word) == strlen(LexBuff)
		? !strcmp ( Word, LexBuff )
		: false;
}


#endif
