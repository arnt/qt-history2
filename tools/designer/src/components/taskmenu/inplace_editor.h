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

#ifndef INPLACE_EDITOR_H
#define INPLACE_EDITOR_H

#include <QtGui/QLineEdit>

class QDesignerFormWindowInterface;

class InPlaceEditor: public QLineEdit
{
    Q_OBJECT
public:
    InPlaceEditor(QWidget *widget, QDesignerFormWindowInterface *fw);
    virtual ~InPlaceEditor();

    virtual bool eventFilter(QObject *object, QEvent *event);

protected:
    virtual void focusOutEvent(QFocusEvent *e);

private:
    QWidget *m_widget;
    bool m_noChildEvent;
};

#endif // INPLACE_EDITOR_H
