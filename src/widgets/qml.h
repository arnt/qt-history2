/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qml.h#12 $
**
** Definition of QML classes
**
** Created : 931107
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QML_H
#define QML_H

#include "qlist.h"
#include "qdict.h"
#include "qpixmap.h"
#include "qscrollview.h"
#include "qcolor.h"


class QMLStyleSheet;
class QMLStyleData;

class Q_EXPORT QMLStyle : public Qt
{
public:
    QMLStyle( QMLStyleSheet* parent, const QString& name );
    ~QMLStyle();

    QString name() const;

    enum AdditionalStyleValues { Undefined  = - 1};

    enum DisplayMode {DisplayBlock, DisplayInline, DisplayListItem, DisplayNone};
    DisplayMode displayMode() const;
    void setDisplayMode(DisplayMode m);

    int alignment() const;
    void setAlignment( int f);

    int fontWeight() const;
    void setFontWeight(int w);

    int fontSize() const;
    void setFontSize(int s);

    QString fontFamily() const;
    void setFontFamily( const QString& );

    int numberOfColumns() const;
    void setNumberOfColumns(int ncols);

    QColor color() const;
    void setColor( const QColor &);

    bool fontItalic() const;
    void setFontItalic( bool );
    bool definesFontItalic() const;

    bool isAnchor() const;
    void setAnchor(bool anc);

    enum WhiteSpaceMode {WhiteSpaceNormal, WhiteSpacePre };
    WhiteSpaceMode whiteSpaceMode() const;
    void setWhiteSpaceMode(WhiteSpaceMode m);

    enum Margin { MarginLeft, MarginRight, MarginTop, MarginBottom, MarginAll, MarginVertical, MarginHorizontal };
    int margin( Margin m) const;
    void setMargin( Margin, int);

    enum ListStyle { ListDisc, ListCircle, ListSquare, ListDecimal, ListLowerAlpha, ListUpperAlpha };

    ListStyle listStyle() const;
    void setListStyle( ListStyle );

    QString contexts() const;
    void setContexts( const QString& );
    bool allowedInContext( const QMLStyle* ) const;

    bool selfNesting() const;
    void setSelfNesting( bool );

private:
    void init();
    QMLStyleData* d;
};

class Q_EXPORT QMLProvider : public QObject
{
    Q_OBJECT
public:
    QMLProvider( QObject *parent=0, const char *name=0 );
    virtual ~QMLProvider();

    static QMLProvider* defaultProvider();
    static void setDefaultProvider( QMLProvider* );

    virtual QPixmap image(const QString &name);
    virtual QString document(const QString &name);

    virtual void setImage(const QString& name, const QPixmap& pm);
    virtual void setDocument(const QString& name, const QString& contents);

    virtual void setPath( const QString &path );
    QString path() const;
    virtual void setReferenceDocument( const QString &doc );

    // TODO add nifty pixmap cache stuff

private:
    QDict<QPixmap>images;
    QDict<QString>documents;
    QString searchPath;
    QString absoluteFilename( const QString&) const;
};

class QMLNode;
class QMLContainer;

class Q_EXPORT QMLStyleSheet : public QObject
{
    Q_OBJECT
public:
    QMLStyleSheet( QObject *parent=0, const char *name=0 );
    virtual ~QMLStyleSheet();

    static QMLStyleSheet* defaultSheet();
    static void setDefaultSheet( QMLStyleSheet* );


    QMLStyle* style( const QString& name);
    const QMLStyle* style( const QString& name) const;

    void insert( QMLStyle* style);

    virtual QMLNode* tag( const QString& name,
			  const QDict<QString>&attr,
			  QMLProvider& provider,
			  bool emptyTag = FALSE) const;


private:
    void init();
    QDict <QMLStyle> styles;
    QMLStyle* nullstyle;

};


class QMLContainer;
class QMLSimpleDocumentData;

class Q_EXPORT QMLSimpleDocument
{
public:
    QMLSimpleDocument( const QString& contents, const QWidget* w = 0);
    ~QMLSimpleDocument();

    void setWidth( QPainter*, int );
    int width() const;
    int height() const;

    void draw( QPainter*,  int x, int y, const QRegion& clipRegion,
	       const QColorGroup& cg, const QBrush* paper = 0) const;

private:
    QMLSimpleDocumentData* d;
};


class QMLDocument;
class QMLViewData;

class Q_EXPORT QMLView : public QScrollView
{
    Q_OBJECT
public:
    QMLView(QWidget *parent=0, const char *name=0);
    QMLView( const QString& doc, QWidget *parent=0, const char *name=0);
    ~QMLView();

    virtual void setContents( const QString& contents);
    virtual QString contents() const;

    QMLStyleSheet* styleSheet() const;
    void setStyleSheet( QMLStyleSheet* styleSheet );


    // convenience functions
    void setPaper( const QBrush& pap);
    const QBrush& paper();

    void setPaperColorGroup( const QColorGroup& colgrp);
    const QColorGroup &paperColorGroup() const;

    void setProvider( QMLProvider* newProvider );
    QMLProvider* provider() const;

    QString documentTitle() const;

    int heightForWidth( int w ) const;

protected:
    void drawContentsOffset(QPainter*, int ox, int oy,
			    int cx, int cy, int cw, int ch);
    void viewportResizeEvent(QResizeEvent*);
    void viewportMousePressEvent( QMouseEvent* );
    void viewportMouseReleaseEvent( QMouseEvent* );
    void viewportMouseMoveEvent( QMouseEvent* );
    void keyPressEvent( QKeyEvent * );

protected:

    QMLDocument& currentDocument() const;
    void paletteChange( const QPalette & );

private:
    void init();
    void createDocument();
    QMLViewData* d;
};



#if 0
class QMLCursor;

class Q_EXPORT QMLEdit : public QMLView
{
    //    Q_OBJECT
public:
    QMLEdit(QWidget *parent=0, const char *name=0);
    ~QMLEdit();

    void setContents( const QString& contents );
    QString contents();

protected:
    void drawContentsOffset(QPainter*, int ox, int oy,
			    int cx, int cy, int cw, int ch);
    void viewportMousePressEvent( QMouseEvent* );
    void viewportMouseReleaseEvent( QMouseEvent* );
    void viewportMouseMoveEvent( QMouseEvent* );
    void keyPressEvent( QKeyEvent * );
    void viewportResizeEvent(QResizeEvent*);

    void showCursor();
    void hideCursor();

    //private slots:
void cursorTimerDone();

private:
    bool cursor_hidden;
    QTimer* cursorTimer;
    QMLCursor* cursor;

    void updateSelection(int oldY=-1, int newY=-1);

    void updateScreen();
    void* d;
};
#endif

class QMLBrowserData;

class Q_EXPORT QMLBrowser : public QMLView
{
    Q_OBJECT
public:
    QMLBrowser( QWidget *parent=0, const char *name=0 );
    ~QMLBrowser();

    virtual void setDocument(const QString& name);

    void setContents( const QString& contents );

    void scrollToAnchor(const QString& name);


public slots:
    virtual void backward();
    virtual void forward();
    virtual void home();

signals:
    void backwardAvailable( bool );
    void forwardAvailable( bool );
    void highlighted( const QString& );
    void contentsChanged();

protected:
    void viewportMousePressEvent( QMouseEvent* );
    void viewportMouseReleaseEvent( QMouseEvent* );
    void viewportMouseMoveEvent( QMouseEvent* );
    void keyPressEvent( QKeyEvent * e);

private:
    void popupDetail( const QString& contents, const QPoint& pos );
    const QMLContainer* anchor(const QPoint& pos);
    QMLBrowserData *d;

};


#endif // QML_H
