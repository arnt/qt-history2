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

#ifndef QAXSELECT_H
#define QAXSELECT_H

#include <qdialog.h>
#include "ui_qaxselect.h"

class QAxSelect : public QDialog, private Ui::QAxSelect
{
    Q_OBJECT
public:
    QAxSelect(QWidget *parent = 0, Qt::WFlags f = 0);

    QString clsid() const { return ActiveX->text(); }

private slots:
    void on_ActiveXList_clicked(const QModelIndex& index, Qt::MouseButton button, Qt::KeyboardModifiers modifiers);
    void on_ActiveXList_doubleClicked(const QModelIndex& index, Qt::MouseButton button, Qt::KeyboardModifiers modifiers);
};

#endif
