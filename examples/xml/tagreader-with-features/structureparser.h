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

/*
$Id$
*/

#ifndef STRUCTUREPARSER_H
#define STRUCTUREPARSER_H

#include <qxml.h>
#include <qptrstack.h>

class Q3ListView;
class Q3ListViewItem;
class QString;

class StructureParser: public QXmlDefaultHandler
{
public:
    StructureParser( Q3ListView * );
    bool startElement( const QString&, const QString&, const QString& ,
                       const QXmlAttributes& );
    bool endElement( const QString&, const QString&, const QString& );

    void setListView( Q3ListView * );

private:
    QPtrStack<Q3ListViewItem> stack;
    Q3ListView * table;
};

#endif
