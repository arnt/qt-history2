/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qdropsite.h#1 $
**
** Definitation of Drag and Drop support
**
** Copyright (C) 1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QDROPSITE_H
#define QDROPSITE_H

class QDropSitePrivate;
class QWidget;
class QDragEnterEvent;
class QDragMoveEvent;
class QDragLeaveEvent;
class QDropEvent;

class QDropSite {
public:
    QDropSite( QWidget* parent );
    virtual ~QDropSite();

    virtual void dragEnterEvent( QDragEnterEvent * );
    virtual void dragMoveEvent( QDragMoveEvent * );
    virtual void dragLeaveEvent( QDragLeaveEvent * );
    virtual void dropEvent( QDropEvent * );

private:
    QDropSitePrivate *d;
};

#endif
