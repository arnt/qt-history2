/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef INFODATA_H
#define INFODATA_H

#include <qdict.h>
#include <qstringlist.h>


// The InfoData class manages data, organized in tree structure.
class InfoData
{
public:
    InfoData();
    QStringList list( QString path, bool *found ) const;
    QString get( QString path, bool *found ) const;

private:
    QDict< QStringList > nodes;
    QDict< QString > data;
};

#endif // INFODATA_H

