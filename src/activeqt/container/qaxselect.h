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

#ifndef QAXSELECT_H
#define QAXSELECT_H

#include <QtGui/qdialog.h>
#include "ui_qaxselect.h"

QT_BEGIN_HEADER

QT_MODULE(ActiveQt)

class QAxSelect : public QDialog, private Ui::QAxSelect
{
    Q_OBJECT
public:
    QAxSelect(QWidget *parent = 0, Qt::WindowFlags f = 0);

    QString clsid() const { return ActiveX->text(); }

private Q_SLOTS:
    void on_ActiveXList_clicked(const QModelIndex &index);
    void on_ActiveXList_doubleClicked(const QModelIndex &index);
};

QT_END_HEADER

#endif // QAXSELECT_H
