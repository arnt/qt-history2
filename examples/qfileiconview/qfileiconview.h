/****************************************************************************
** $Id: //depot/qt/main/examples/qfileiconview/qfileiconview.h#18 $
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

    virtual void dragEntered();
    virtual void dragLeft();

protected:
    virtual void dropped( QDropEvent *e );

    QString itemFileName;
    QFileInfo *itemFileInfo;
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
    QtFileIconView( const QString &dir, bool isdesktop = FALSE,
                    QWidget *parent = 0, const char *name = 0 );

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
    void viewBottom();
    void viewRight();
    void flowEast();
    void flowSouth();
    void singleClick();
    void doubleClick();
    void alignInGrid();
    void sortAscending();
    void sortDescending();

    void slotItemRightClicked( QIconViewItem *item );
    void slotViewportRightClicked();

protected:
    void readDir( const QDir &dir );
    virtual QDragObject *dragObject();
    void initDragEnter( QDropEvent *e );

    virtual void keyPressEvent( QKeyEvent *e );
    virtual void drawBackground( QPainter *p, const QRect &r );
    static void makeGradient( QPixmap &pmCrop, const QColor &_color1,
                              const QColor &_color2, int _xSize, int _ySize );

    void resizeContents( int, int );

    QDir viewDir;
    int newFolderNum;
    bool isDesktop;
    QSize sz;
    QPixmap pix;
    bool makeNewGradient;

};

#endif
