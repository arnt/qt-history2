/****************************************************************************
**
** Definition of Aqua-style guidelines functions.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QAQUASTYLE_P_H
#define QAQUASTYLE_P_H

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

#ifndef QT_H
# include "qstyle.h"
# include "qwidget.h"
#endif // QT_H

class QAquaFocusWidget : public QWidget
{
    Q_OBJECT
public:
    QAquaFocusWidget(bool noerase=TRUE, QWidget *w=NULL);
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
    enum Animates { AquaPushButton, AquaProgressBar, AquaListViewItemOpen };
    bool animatable(Animates, QWidget *);
    bool animatable(Animates, QListViewItem *);
    void stopAnimate(Animates, QWidget *);
    void stopAnimate(Animates, QListViewItem *);

    //focus things
    static bool focusable(QWidget *);
    QWidget *focusWidget() const;

protected:
    //finally do the animate..
    virtual bool doAnimate(Animates) = 0;
    //finally set the focus
    void setFocusWidget(QWidget *);
    virtual void doFocus(QWidget *w) = 0;

protected:
    bool eventFilter(QObject *, QEvent *);
    void timerEvent( QTimerEvent * );

private slots:
    void objDestroyed(QObject *o);
    void lvi(QListViewItem *);
};

/*
  Hackish method of finding out whether the window is active or not
 */
static inline bool qAquaActive( const QColorGroup & g )
{
    if( g.link() == QColor( 148,148,148 ) )
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

#endif /* QAQUASTYLE_P_H */
