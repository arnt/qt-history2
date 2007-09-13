/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_COMMERCIAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef INVOKEMETHOD_H
#define INVOKEMETHOD_H

#include <QtCore/qglobal.h>
#include "ui_invokemethod.h"

QT_BEGIN_NAMESPACE

class QAxBase;

class InvokeMethod : public QDialog, Ui::InvokeMethod
{
    Q_OBJECT
public:
    InvokeMethod(QWidget *parent);

    void setControl(QAxBase *ax);

protected slots:
    void on_buttonInvoke_clicked();
    void on_buttonSet_clicked();

    void on_comboMethods_activated(const QString &method);
    void on_listParameters_currentItemChanged(QTreeWidgetItem *item);

private:
    QAxBase *activex;
};

QT_END_NAMESPACE

#endif // INVOKEMETHOD_H
