/****************************************************************************
** $Id: //depot/qt/main/examples/qfileiconview/qfileiconview.h#7 $
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
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

    ItemType type()
    { return itemType; }
    QString filename() { return itemFileName; }

    virtual bool acceptDrop( QMimeSource *e );

    virtual void setText( const QString &text );

    virtual void dragEntered();
    virtual void dragLeft();

protected:
    virtual void dropped( QDropEvent *e );

    QString itemFileName;
    QFileInfo itemFileInfo;
    ItemType itemType;
    bool checkSetText;
    QTimer timer;

protected slots:
    void openFolder();

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
    void viewNormal();
    void viewSmall();
    void alignInGrid();

    void slotItemRightClicked( QIconViewItem *item );
    void slotViewportRightClicked();

protected:
    void readDir( const QDir &dir );
    virtual QDragObject *dragObject();
    virtual int dragItems( QDropEvent *e );

    virtual void keyPressEvent( QKeyEvent *e );

    QDir viewDir;
    int newFolderNum;

};

#endif
