/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of Qt Designer.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "ui3reader.h"
#include "parser.h"
#include "domtool.h"
#include <qregexp.h>
#include <qstringlist.h>
#include <globaldefs.h>
#include <qdebug.h>

/*! Extracts a named object property from \a e.
 */
QDomElement Ui3Reader::getObjectProperty( const QDomElement& e, const QString& name )
{
    QDomElement n;
    for ( n = e.firstChild().toElement();
          !n.isNull();
          n = n.nextSibling().toElement() ) {
        if ( n.tagName() == QLatin1String("property")  && n.toElement().attribute(QLatin1String("name")) == name )
            return n;
    }
    return n;
}
