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

#ifndef FTPVIEWITEM_H
#define FTPVIEWITEM_H

#include <q3listview.h>
#include <qdatetime.h>

class FtpViewItem : public Q3ListViewItem
{
public:
    enum Type {
	Directory,
	File
    };

    FtpViewItem( Q3ListView *parent, Type t, const QString &name, const QString &size, const QString &lastModified );

    int compare( Q3ListViewItem * i, int col, bool ascending ) const;

    bool isDir()
    { return type==Directory; }

private:
    Type type;
};

#endif
