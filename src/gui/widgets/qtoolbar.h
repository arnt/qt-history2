/****************************************************************************
**
** Definition of QToolBar class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QTOOLBAR_H
#define QTOOLBAR_H

#ifndef QT_H
#include "qdockwindow.h"
#endif // QT_H

#ifndef QT_NO_TOOLBAR

class QMainWindow;
class QButton;
class QBoxLayout;
class QToolBarPrivate;

class Q_GUI_EXPORT QToolBar: public QDockWindow
{
    Q_OBJECT
    Q_PROPERTY( QString label READ label WRITE setLabel )

public:
    QToolBar( const QString &label,
	      QMainWindow *, ToolBarDock = DockTop,
	      bool newLine = FALSE, const char* name=0 );
    QToolBar( const QString &label, QMainWindow *, QWidget *,
	      bool newLine = FALSE, const char* name=0, WFlags f = 0 );
    QToolBar( QMainWindow* parent=0, const char* name=0 );
    ~QToolBar();

    void addSeparator();

    void show();
    void hide();

    QMainWindow * mainWindow() const;

    virtual void setStretchableWidget( QWidget * );

    bool event( QEvent * e );

    virtual void setLabel( const QString & );
    QString label() const;

    virtual void clear();

    QSize minimumSize() const;
    QSize minimumSizeHint() const;

    void setOrientation( Orientation o );
    void setMinimumSize( int minw, int minh );

protected:
    void resizeEvent( QResizeEvent *e );
    void styleChange(QStyle &);

private slots:
    void createPopup();

private:
    void init();
    void checkForExtension( const QSize &sz );
    QToolBarPrivate * d;
    QMainWindow * mw;
    QWidget * sw;
    QString l;

    friend class QMainWindow;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QToolBar( const QToolBar & );
    QToolBar& operator=( const QToolBar & );
#endif
};

#endif // QT_NO_TOOLBAR

#endif // QTOOLBAR_H
