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

#ifndef QAQUASTYLE_MAC_H
#define QAQUASTYLE_MAC_H

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
#  include <private/qt_mac_p.h>
#  include "qevent.h"
#  include "qstyle.h"
#  include "qwidget.h"
#endif // QT_H


class QAquaFocusWidget : public QWidget
{
    Q_OBJECT
public:
    QAquaFocusWidget(bool noerase=true, QWidget *w=NULL);
    ~QAquaFocusWidget() {}
    void setFocusWidget(QWidget * widget);
    QWidget* widget() const { return d; }
    QSize sizeHint() const { return QSize(0, 0); }

protected:
    void mousePressEvent(QMouseEvent *ev) { ev->ignore(); }
    void mouseMoveEvent(QMouseEvent *ev) { ev->ignore(); }
    void mouseReleaseEvent(QMouseEvent *ev) { ev->ignore(); }
    bool eventFilter(QObject * o, QEvent * e);

protected:
    virtual void paintEvent(QPaintEvent *) = 0;
    virtual int focusOutset() const;
    virtual QRegion focusRegion() const { return QRegion(focusOutset() + 2, focusOutset() + 2,
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
    bool animatable(Animates, const QWidget *);
    void stopAnimate(Animates, QWidget *);
    void startAnimate(Animates, QWidget *);
    static ThemeDrawState getDrawState(QStyle::SFlags flags, const QPalette &pal);

    //focus things
    bool focusable(const QWidget *) const;
    const QWidget *focusWidget() const;

protected:
    //finally do the animate..
    virtual bool doAnimate(Animates) = 0;
    virtual int animateSpeed(Animates) { return 50; }
    //finally set the focus
    void setFocusWidget(QWidget *);
    virtual void doFocus(QWidget *w) = 0;

protected:
    bool eventFilter(QObject *, QEvent *);
    void timerEvent(QTimerEvent *);

private slots:
    void objDestroyed(QObject *o);
};

/*
  Hackish method of finding out whether the window is active or not
 */
static inline bool qAquaActive(const QPalette &pal)
{
    return (pal.currentColorGroup() == QPalette::Active);
}

/*
   Detects sizes to comply with Aqua Style Guidelines
*/
enum QAquaWidgetSize { QAquaSizeLarge, QAquaSizeSmall, QAquaSizeMini, QAquaSizeUnknown };
QAquaWidgetSize qt_aqua_size_constrain(const QWidget *widg,
                                       QStyle::ContentsType ct=QStyle::CT_CustomBase,
                                       QSize szHint=QSize(-1, -1), QSize *insz=NULL);


#endif // QAQUASTYLE_MAC_H 
