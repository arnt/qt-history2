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

#ifndef ABSTRACTWIDGETBOX_H
#define ABSTRACTWIDGETBOX_H

#include <QWidget>

#include "sdk_global.h"

class DomUI;

class QT_SDK_EXPORT AbstractWidgetBox : public QWidget
{
    Q_OBJECT
public:
    AbstractWidgetBox(QWidget *parent = 0, Qt::WFlags flags = 0);
    virtual ~AbstractWidgetBox();

    virtual int categoryCount() const = 0;
    virtual DomUI *category(int cat_idx) const = 0;
    virtual int widgetCount(int cat_idx) const = 0;
    virtual DomUI *widget(int cat_idx, int wgt_idx) const = 0;

    virtual int addCategory(const QString &name, const QString &icon_file, DomUI *ui) = 0;
    virtual void removeCategory(int cat_idx) = 0;
};

#endif // ABSTRACTWIDGETBOX_H
