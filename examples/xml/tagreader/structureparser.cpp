/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "structureparser.h"

#include <stdio.h>
#include <qstring.h>
 
bool StructureParser::startDocument()
{
    indent = "";
    return TRUE;
}

bool StructureParser::startElement( const QString&, const QString&, 
                                    const QString& qName, 
                                    const QXmlAttributes& )
{
    printf( "%s%s\n", (const char*)indent, (const char*)qName );
    indent += "    ";
    return TRUE;
}

bool StructureParser::endElement( const QString&, const QString&, const QString& )
{
    indent.remove( (uint)0, 4 );
    return TRUE;
}
