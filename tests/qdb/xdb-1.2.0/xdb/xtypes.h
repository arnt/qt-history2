/*  $Id: xtypes.h,v 1.4 1999/03/19 10:56:35 willy Exp $

    Xbase project source code

    Copyright (C) 1997  StarTech, Gary A. Kunkel   
    email - xbase@startech.keller.tx.us
    www   - http://www.startech.keller.tx.us/xbase.html

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    V 1.0    10/10/97   - Initial release of software
    V 1.5    1/2/98     - Added memo field support
    V 1.6a   4/1/98     - Added expression support
    V 1.8.0a 1/29/99    - V1.8 upgrade
*/

#ifndef __XB_XTYPES_H__
#define __XB_XTYPES_H__

#include <stdio.h>

typedef unsigned long  int xbULong;
typedef unsigned short int xbUShort;
typedef short int          xbShort;

#define xbLong long
//typedef long             LONG;

typedef float              xbFloat;

typedef double             xbDouble;

#endif 		// __XB_XTYPES_H__
