/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2004 by Dmitry Tsarkov

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

#ifndef _BIPOINTER_H
#define _BIPOINTER_H

typedef int BipolarPointer;

inline BipolarPointer createBiPointer ( int index, bool pos ) { return (pos ? index : -index); }

inline bool isCorrect ( BipolarPointer p ) { return (p!=0); }
inline bool isValid ( BipolarPointer p ) { return (p!=0); }
inline bool isPositive ( BipolarPointer p ) { return (p>0); }
inline bool isNegative ( BipolarPointer p ) { return (p<0); }

inline unsigned int getValue ( BipolarPointer p ) { return (p>0?p:-p); }

inline BipolarPointer inverse ( BipolarPointer p ) { return -p; }
inline BipolarPointer getPositive ( BipolarPointer p ) { return (p>0?p:-p); }
inline BipolarPointer getNegative ( BipolarPointer p ) { return (p<0?p:-p); }

const BipolarPointer bpINVALID = 0;
const BipolarPointer bpTOP = 1;
const BipolarPointer bpBOTTOM = -1;

#endif // _BIPOINTER_H
