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

#ifndef FORMWINDOWCURSOR_H
#define FORMWINDOWCURSOR_H

#include "formeditor_global.h"
#include "formwindow.h"
#include <abstractformwindowcursor.h>

#include <QObject>

class QT_FORMEDITOR_EXPORT FormWindowCursor: public QObject, public AbstractFormWindowCursor
{
    Q_OBJECT
public:
    FormWindowCursor(FormWindow *fw, QObject *parent = 0);
    virtual ~FormWindowCursor();

    virtual AbstractFormWindow *formWindow() const;

    virtual bool movePosition(MoveOperation op, MoveMode mode);

    virtual int position() const;
    virtual void setPosition(int pos, MoveMode mode);

    virtual QWidget *current() const;

    virtual int widgetCount() const;
    virtual QWidget *widget(int index) const;

    virtual bool hasSelection() const;
    virtual int selectedWidgetCount() const;
    virtual QWidget *selectedWidget(int index) const;

    virtual void setProperty(const QString &name, const QVariant &value);
    virtual void setWidgetProperty(QWidget *widget, const QString &name, const QVariant &value);

public slots:
    void update();

private:
    FormWindow *m_formWindow;
    int m_iterator;
};

#endif // FORMWINDOWCURSOR_H
