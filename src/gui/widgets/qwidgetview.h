/****************************************************************************
**
** Definition of QViewport widget class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QWIDGETVIEW_H
#define QWIDGETVIEW_H

#ifndef QT_H
#include "qviewport.h"
#endif // QT_H

class QWidgetViewPrivate;

class QWidgetView : public QViewport
{
    Q_OBJECT
    Q_PROPERTY(bool widgetResizable READ widgetResizable WRITE setWidgetResizable)
public:
    QWidgetView(QWidget* parent=0);
    ~QWidgetView();

    QWidget *widget() const;
    void setWidget(QWidget *w);

    bool widgetResizable() const;
    void setWidgetResizable(bool resizable);

    QSize sizeHint() const;

protected:
    bool event(QEvent *);
    bool eventFilter(QObject *, QEvent *);
    void resizeEvent(QResizeEvent *);
    void scrollContentsBy(int dx, int dy);

private:
    Q_DECLARE_PRIVATE(QWidgetView);

};


#endif
