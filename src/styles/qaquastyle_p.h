/****************************************************************************
** $Id: $
**
** Definition of Aqua-style guidelines functions
**
** Created : 001129
**
** Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Macintosh may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software. This file and its contents may
** not be distributed onto any other platform or included in any other licensed
** package unless explicit permission is granted.
**
** This file is not available for use under any other license without
** express written permission from the copyright holder.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of a number of Qt sources files.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//
//

#ifndef __QAQUASTYLE_P_H__
#define __QAQUASTYLE_P_H__

#ifndef QT_H
# include <qstyle.h>
# include <qwidget.h>
#endif // QT_H

#ifdef Q_WS_MAC
//#define QMAC_QAQUA_MODIFY_TEXT_COLOURS
#endif

class QAquaFocusWidget : public QWidget
{
    Q_OBJECT
public:
    QAquaFocusWidget(bool noerase=TRUE );
    ~QAquaFocusWidget() {}
    void setFocusWidget( QWidget * widget );
    QWidget* widget() { return d; }
    QSize sizeHint() { return QSize( 0, 0 ); }

protected:
    bool eventFilter( QObject * o, QEvent * e );

protected: 
    virtual void paintEvent( QPaintEvent * ) = 0;
    virtual int focusOutset() { return 3; }
    virtual QRegion focusRegion() { return QRegion( focusOutset() + 2, focusOutset() + 2, 
						    width() - ((focusOutset() + 2) * 2), 
						    height() - ((focusOutset() + 2) * 2)); }

private:
    QWidget *d;
};

struct QAquaAnimatePrivate;
class QAquaAnimate : public QObject
{
    Q_OBJECT
    QAquaAnimatePrivate *d;
public:
    QAquaAnimate();
    ~QAquaAnimate();

    //give the widget over
    virtual bool addWidget(QWidget *);
    virtual void removeWidget(QWidget *);

    //animation things
    enum Animates { AquaPushButton, AquaProgressBar };
    bool animatable(Animates, QWidget *);

    //focus things
    static bool focusable(QWidget *);
    QWidget *focusWidget() const;

protected:
    //finally do the animate..
    virtual void doAnimate(Animates) = 0;
    //finally set the focus
    void setFocusWidget(QWidget *);
    virtual void doFocus(QWidget *w) = 0;

protected:
    bool eventFilter(QObject *, QEvent *);
    void timerEvent( QTimerEvent * );

private slots:
    void objDestroyed(QObject *o);
};

/*
  Hackish method of finding out whether the window is active or not
 */
static inline bool qAquaActive( const QColorGroup & g )
{
    if( g.buttonText() == QColor( 148,148,148 ) )
        return FALSE;
    else
        return TRUE;
}

/* 
   Detects sizes to comply with Aqua Style Guidelines
*/
enum QAquaWidgetSize { QAquaSizeLarge, QAquaSizeSmall, QAquaSizeUnknown };
QAquaWidgetSize qt_aqua_size_constrain(const QWidget *widg, 
				       QStyle::ContentsType ct=QStyle::CT_CustomBase, 
				       QSize szHint=QSize(-1, -1), QSize *insz=NULL);

/* 
  Setup the palette to comply with Aqua Style Guidelines
*/
bool qt_mac_update_palette(QPalette &pal, bool do_init);

/* 
  Setup the font to comply with Aqua Style Guidelines
*/
QCString p2qstring(const unsigned char *c); //qglobal.cpp
void qt_mac_polish_font(QWidget *w, QAquaWidgetSize size=QAquaSizeLarge);

#endif /* __QAQUASTYLE_P_H__ */

