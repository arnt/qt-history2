/**********************************************************************
**   Copyright (C) 2000 Troll Tech AS.  All rights reserved.
**
**   This file is part of Qt GUI Designer.
**
**   This file may be distributed under the terms of the GNU General
**   Public License version 2 as published by the Free Software
**   Foundation and appearing in the file COPYING included in the
**   packaging of this file. If you did not get the file, send email
**   to info@trolltech.com
**
**   The file is provided AS IS with NO WARRANTY OF ANY KIND,
**   INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR
**   A PARTICULAR PURPOSE.
**
**********************************************************************/

#include "defs.h"

int size_type_to_int( QSizePolicy::SizeType t )
{
    if ( t == QSizePolicy::Fixed )
	return 0;
    if ( t == QSizePolicy::Minimum )
	return 1;
    if ( t == QSizePolicy::Maximum )
	return 2;
    if ( t == QSizePolicy::Preferred )
	return 3;
    if ( t == QSizePolicy::MinimumExpanding )
	return 4;
    if ( t == QSizePolicy::Expanding )
	return 5;
    return 0;
}

QString size_type_to_string( QSizePolicy::SizeType t )
{
    if ( t == QSizePolicy::Fixed )
	return "Fixed";
    if ( t == QSizePolicy::Minimum )
	return "Minimum";
    if ( t == QSizePolicy::Maximum )
	return "Maximum";
    if ( t == QSizePolicy::Preferred )
	return "Preferred";
    if ( t == QSizePolicy::MinimumExpanding )
	return "MinimumExpanding";
    if ( t == QSizePolicy::Expanding )
	return "Expanding";
    return 0;
}

QSizePolicy::SizeType int_to_size_type( int i )
{	
    if ( i == 0 )
	return QSizePolicy::Fixed;
    if ( i == 1 )
	return QSizePolicy::Minimum;
    if ( i == 2 )
	return QSizePolicy::Maximum;
    if ( i == 3 )
	return QSizePolicy::Preferred;
    if ( i == 4 )
	return QSizePolicy::MinimumExpanding;
    if ( i == 5 )
	return QSizePolicy::Expanding;
    return QSizePolicy::Preferred;
}
