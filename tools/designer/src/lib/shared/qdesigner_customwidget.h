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

#ifndef QDESIGNER_CUSTOMWIDGET_H
#define QDESIGNER_CUSTOMWIDGET_H

#include "qdesigner_widget.h"

class FormWindow;
class QDesignerWidgetDataBaseItemInterface;

class QT_SHARED_EXPORT QDesignerCustomWidget: public QDesignerWidget
{
    Q_OBJECT
    Q_PROPERTY(bool compat READ isCompat WRITE setCompat STORED false)
    Q_PROPERTY(bool container READ isContainer WRITE setContainer STORED false)
public:
    QDesignerCustomWidget(QDesignerFormWindowInterface *formWindow, QWidget *parent = 0);
    virtual ~QDesignerCustomWidget();

    QDesignerWidgetDataBaseItemInterface *widgetItem() const;

    QString widgetClassName() const;
    void setWidgetClassName(const QString &widgetClassName);

    bool isCompat() const;
    void setCompat(bool compat);

    bool isContainer() const;
    void setContainer(bool container);

private:
    void createWidgetItem();

private:
    QString m_widgetClassName;
};

#endif // QDESIGNER_CUSTOMWIDGET_H
