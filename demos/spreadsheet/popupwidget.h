/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef POPUPWIDGET_H
#define POPUPWIDGET_H

#include <QWidget>
#include <QEventLoop>

//The class to show a widget as popup.
class PopupWidget: public QWidget
{
    Q_OBJECT
public:
    PopupWidget(QWidget *parent = 0);
    ~PopupWidget();
    void setWidget(QWidget * widget);
    QWidget *widget() const;
    //position of the popup has to be in the screen coordinates.
    int exec(const QRect& rect);

public slots:
    void closePopup();

protected:
    void hideEvent(QHideEvent * event);

private:
    QWidget *childWidget;
    QEventLoop *eventLoop;
    int returnCode;
};

#endif
