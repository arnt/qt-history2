/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef FTPVIEWITEM_H
#define FTPVIEWITEM_H

#include <qlistview.h>
#include <qdatetime.h>

class FtpViewItem : public QListViewItem
{
public:
    enum Type {
	Directory,
	File
    };

    FtpViewItem( QListView *parent, Type t, const QString &name, const QString &size, const QString &lastModified );

    int compare( QListViewItem * i, int col, bool ascending ) const;

    bool isDir()
    { return type==Directory; }

private:
    Type type;
};

#endif
