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

#include <qpixmap.h>

#include "ftpviewitem.h"
#include <qmimefactory.h>


FtpViewItem::FtpViewItem( Q3ListView *parent, Type t, const QString &name, const QString &size, const QString &lastModified )
    : Q3ListViewItem(parent,name,size,lastModified), type(t)
{
    // the pixmaps for folders and files are in an image collection
    if ( type == Directory )
	setPixmap( 0, qPixmapFromMimeSource( "folder.png" ) );
    else
	setPixmap( 0, qPixmapFromMimeSource( "file.png" ) );
}

int FtpViewItem::compare( Q3ListViewItem * i, int col, bool ascending ) const
{
    // The entry ".." is always the first one.
    if ( text(0) == ".." ) {
	if ( ascending )
	    return -1;
	else
	    return 1;
    }
    if ( i->text(0) == ".." ) {
	if ( ascending )
	    return 1;
	else
	    return -1;
    }

    // Directories are before files.
    if ( type != ((FtpViewItem*)i)->type ) {
	if ( type == Directory ) {
	    if ( ascending )
		return -1;
	    else
		return 1;
	} else {
	    if ( ascending )
		return 1;
	    else
		return -1;
	}
    }

    // Use default sorting otherwise.
    return Q3ListViewItem::compare( i, col, ascending );
}
