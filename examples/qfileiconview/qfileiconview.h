/****************************************************************************
** $Id: //depot/qt/main/examples/qfileiconview/qfileiconview.h#19 $
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef QTFILEICONVIEW_H
#define QTFILEICONVIEW_H


#include <qiconset.h>
#include <qstring.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qtimer.h>
#include <qiconview.h>

class QtFileIconView;
class QDragObject;
class QResizeEvent;

/*****************************************************************************
 *
 * Class QtFileIconDragItem
 *
 *****************************************************************************/

class QtFileIconDragItem : public QIconDragItem
{
public:
    QtFileIconDragItem();
    QtFileIconDragItem( const QRect &ir, const QRect &tr, const QString &u );
    ~QtFileIconDragItem();
	
    QString url() const;
    void setURL( const QString &u );

protected:
    void makeKey();

    QString url_;

};

/*****************************************************************************
 *
 * Class QtFileIconDrag
 *
 *****************************************************************************/

class QtFileIconDrag : public QIconDrag
{
    Q_OBJECT

public:
    typedef QValueList<QtFileIconDragItem> QtFileIconList;

    QtFileIconDrag( QWidget * dragSource, const char* name = 0 );
    ~QtFileIconDrag();

    const char* format( int i ) const;
    QByteArray encodedData( const char* mime ) const;

    void append( const QtFileIconDragItem &icon_ );

    static bool canDecode( QMimeSource* e );

    static bool decode( QMimeSource *e, QValueList<QtFileIconDragItem> &list_ );
    static bool decode( QMimeSource *e, QStringList &uris );

protected:
    QtFileIconList icons;

};

/*****************************************************************************
 *
 * Class QtFileIconView
 *
 *****************************************************************************/

class QtFileIconView : public QIconView
{
    Q_OBJECT

public:
    QtFileIconView( const QString &dir, QWidget *parent = 0, const char *name = 0 );

    enum ViewMode { Large, Small };

    void setViewMode( ViewMode m );
    ViewMode viewMode() const { return vm; }

public slots:
    void setDirectory( const QString &dir );
    void setDirectory( const QDir &dir );
    void newDirectory();
    QDir currentDir();

signals:
    void directoryChanged( const QString & );
    void startReadDir( int dirs );
    void readNextDir();
    void readDirDone();

protected slots:
    void itemDoubleClicked( QIconViewItem *i );
    void slotDropped( QDropEvent *e );

    void viewLarge();
    void viewSmall();
    void viewBottom();
    void viewRight();
    void flowEast();
    void flowSouth();
    void itemTextTruncate();
    void itemTextWordWrap();
    void sortAscending();
    void sortDescending();
    void arrangeItemsInGrid() {
	QIconView::arrangeItemsInGrid( TRUE );
    }

    void slotRightPressed( QIconViewItem *item );

protected:
    void readDir( const QDir &dir );
    virtual QDragObject *dragObject();
    void initDragEnter( QDropEvent *e );

    virtual void keyPressEvent( QKeyEvent *e );

    QDir viewDir;
    int newFolderNum;
    QSize sz;
    QPixmap pix;
    ViewMode vm;

};

/*****************************************************************************
 *
 * Class QtFileIconViewItem
 *
 *****************************************************************************/

class QtFileIconViewItem : public QIconViewItem
{
    Q_OBJECT

public:
    enum ItemType {
        File = 0,
        Dir,
        Link
    };

    QtFileIconViewItem( QtFileIconView *parent, QFileInfo *fi );

    virtual ~QtFileIconViewItem();

    ItemType type() const
    { return itemType; }
    QString filename() const { return itemFileName; }

    virtual bool acceptDrop( const QMimeSource *e ) const;

    virtual void setText( const QString &text );
    virtual QPixmap *pixmap() const;

    virtual void dragEntered();
    virtual void dragLeft();

    void viewModeChanged( QtFileIconView::ViewMode m );

protected:
    virtual void dropped( QDropEvent *e );

    QString itemFileName;
    QFileInfo *itemFileInfo;
    ItemType itemType;
    bool checkSetText;
    QTimer timer;
    QtFileIconView::ViewMode vm;

protected slots:
    void openFolder();

};


#endif
