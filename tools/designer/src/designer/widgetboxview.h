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

#ifndef WIDGETBOXVIEW_H
#define WIDGETBOXVIEW_H

#include <QMainWindow>

class AbstractFormEditor;

class WidgetBoxView: public QMainWindow
{
    Q_OBJECT
public:
    WidgetBoxView(AbstractFormEditor *core, QWidget *parent = 0);
    virtual ~WidgetBoxView();

protected:
    void showEvent(QShowEvent *ev);
    void hideEvent(QHideEvent *ev);

signals:
    void visibilityChanged(bool isVisible);

private:
    AbstractFormEditor *m_core;
};

#endif // WIDGETBOXVIEW_H
