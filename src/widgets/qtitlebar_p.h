/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qtitlebar_p.h#1 $
**
** Definition of some Qt private functions.
**
** Created : 2000-01-01
**
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QTITLEBAR_P_H
#define QTITLEBAR_P_H


//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of qworkspace.cpp and qdockwindow.cpp.  This header file may change
// from version to version without notice, or even be removed.
//
// We mean it.
//
//


#ifndef QT_H
#include "qbutton.h"
#include "qlabel.h"
#include "qpixmap.h"
#endif // QT_H

#if !defined(QT_NO_MAINWINDOW) || !defined(QT_NO_WORKSPACE)

class QTitleBar;

class Q_EXPORT QTitleBarButton : public QButton
{
    Q_OBJECT
public:
    enum ButtonType
    {
	Icon = 0,
	Min,
	Max,
	Close,
	RestoreUp,
	RestoreDown,
	ContextHelp,
	ShadeUp,
	ShadeDown
    };

    QTitleBarButton( QWidget* parent, ButtonType type, const char *name = 0 );

    QSize sizeHint() const;

    void setPixmap( const QPixmap& );
    void setType( ButtonType type );

protected:
    void drawButton( QPainter * );
    void drawButtonLabel( QPainter * );

private:
    ButtonType type;
};

class Q_EXPORT QTitleBarLabel : public QFrame
{
    Q_OBJECT
public:
    QTitleBarLabel( QWidget *parent = 0, const char* name = 0 );

    void setActive( bool act );
    QColor leftColor() const { return leftc; }
    QColor rightColor() const { return rightc; }

    void setLeftMargin( int x );
    void setRightMargin( int x );

public slots:
    void setText( const QString& );

protected:
    void paintEvent( QPaintEvent* );
    void resizeEvent( QResizeEvent* );

    bool event( QEvent* );

    void drawLabel( QPainter* );
    void frameChanged();

private:
    void cutText();
    void getColors();

    QColor leftc, aleftc, ileftc;
    QColor rightc, arightc, irightc;
    QColor textc, atextc, itextc;
    int leftm;
    int rightm;
    QString titletext;
    QString cuttext;
    bool act;
};

#ifndef QT_NO_WORKSPACE

class QWorkspace;

class Q_EXPORT QTitleBar : public QWidget
{
    Q_OBJECT

friend class QWorkspaceChild;

public:
    QTitleBar (QWorkspace* w, QWidget* win, QWidget* parent, const char* name=0, bool iconMode = FALSE );
    ~QTitleBar();

    bool isActive() const;

    QSize sizeHint() const;

    static QPixmap titleButtonPixmap( int button, bool down );

public slots:
    void setActive( bool );
    void setText( const QString& title );
    void setIcon( const QPixmap& icon );

signals:
    void doActivate();
    void doNormal();
    void doClose();
    void doMaximize();
    void doMinimize();
    void doShade();
    void showOperationMenu();
    void popupOperationMenu( const QPoint& );

protected:
    void resizeEvent( QResizeEvent * );
    void mousePressEvent( QMouseEvent * );
    void mouseReleaseEvent( QMouseEvent * );
    void mouseMoveEvent( QMouseEvent * );
    void enterEvent( QEvent *e );

    bool eventFilter( QObject *, QEvent * );

private:
    QTitleBarButton* closeB;
    QTitleBarButton* maxB;
    QTitleBarButton* iconB;
    QTitleBarButton* shadeB;
    QTitleBarLabel* titleL;
    QLabel* iconL;
    int titleHeight;
    bool buttonDown;
    QPoint moveOffset;
    QWorkspace* workspace;
    QWidget* window;
    bool imode		    :1;
    bool act		    :1;

    QString text;
};

#endif

extern const char * const qt_close_xpm[];
extern const char * const qt_maximize_xpm[];
extern const char * const qt_minimize_xpm[];
extern const char * const qt_normalize_xpm[];
extern const char * const qt_normalizeup_xpm[];
extern const char * const qt_shade_xpm[];
extern const char * const qt_unshade_xpm[];


#endif //!QT_NO_MAINWINDOW || !QT_NO_WORKSPACE

#endif //QTITLEBAR_P_H
