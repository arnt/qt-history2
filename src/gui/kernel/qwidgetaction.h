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

#ifndef QWIDGETACTION_H
#define QWIDGETACTION_H

#include <QtGui/qaction.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

#ifndef QT_NO_ACTION

class QWidgetActionPrivate;

class Q_GUI_EXPORT QWidgetAction : public QAction
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWidgetAction)

public:
    QWidgetAction(QWidget *widget, QObject* parent);
    virtual ~QWidgetAction();
    
    QWidget *createWidget(QWidget *parent);
    void removeWidget(QWidget *widget);
    
    QList<QWidget *> widgets() const;

protected:
    explicit QWidgetAction(QObject* parent);
    virtual bool event(QEvent *);
    virtual QWidget *doCreateWidget(QWidget *parent);
    virtual void doRemoveWidget(QWidget *widget);

private:
    Q_DISABLE_COPY(QWidgetAction)
    Q_PRIVATE_SLOT(d_func(), void _q_widgetDestroyed(QObject *))
    friend class QToolBar;
};

#endif // QT_NO_ACTION

QT_END_HEADER

#endif // QWIDGETACTION_H
