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

#include <qapplication.h>
#include <qcursor.h>
#include <qsplitter.h>
#include <qlistbox.h>
#include <qiconview.h>
#include <qpixmap.h>

class QDragEnterEvent;
class QDragDropEvent;


class DDListBox : public QListBox
{
    Q_OBJECT
public:
    DDListBox( QWidget * parent = 0, const char * name = 0, WFlags f = 0 );
    // Low-level drag and drop
    void dragEnterEvent( QDragEnterEvent *evt );
    void dropEvent( QDropEvent *evt );
    void mousePressEvent( QMouseEvent *evt );
    void mouseMoveEvent( QMouseEvent * );
private:
    int dragging;
};


class DDIconViewItem : public QIconViewItem
{
public:
    DDIconViewItem( QIconView *parent, const QString& text, const QPixmap& icon ) :
	QIconViewItem( parent, text, icon ) {}
    DDIconViewItem( QIconView *parent, const QString &text ) :
	QIconViewItem( parent, text ) {}
    // High-level drag and drop
    bool acceptDrop( const QMimeSource *mime ) const;
    void dropped( QDropEvent *evt, const QValueList<QIconDragItem>& );
};


class DDIconView : public QIconView
{
    Q_OBJECT
public:
    DDIconView( QWidget * parent = 0, const char * name = 0, WFlags f = 0 ) :
	QIconView( parent, name, f ) {}
    // High-level drag and drop
    QDragObject *dragObject();
public slots:
    void slotNewItem( QDropEvent *evt, const QValueList<QIconDragItem>& list );
};

