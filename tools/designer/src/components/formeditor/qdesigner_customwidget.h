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

class QDesignerCustomWidget: public QDesignerWidget
{
    Q_OBJECT
    Q_PROPERTY(QString widgetClassName READ widgetClassName WRITE setWidgetClassName)
    Q_PROPERTY(bool compat READ isCompat WRITE setCompat)
public:
    QDesignerCustomWidget(FormWindow *formWindow, QWidget *parent = 0);
    virtual ~QDesignerCustomWidget();

    QString widgetClassName() const;
    void setWidgetClassName(const QString &widgetClassName);
        
    bool isCompat() const;
    void setCompat(bool compat);
    
    // ### extends

private:
    QString m_widgetClassName;
    bool m_compat;
};

#endif // QDESIGNER_CUSTOMWIDGET_H
