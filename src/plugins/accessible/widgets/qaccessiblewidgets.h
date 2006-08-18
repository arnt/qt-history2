/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QACCESSIBLEWIDGETS_H
#define QACCESSIBLEWIDGETS_H

#include <QtGui/qaccessiblewidget.h>

#ifndef QT_NO_ACCESSIBILITY

class QTextEdit;

class QAccessibleTextEdit : public QAccessibleWidget
{
public:
    explicit QAccessibleTextEdit(QWidget *o);

    QString text(Text t, int child) const;
    void setText(Text t, int control, const QString &text);
    Role role(int child) const;

protected:
    QTextEdit *textEdit() const;
};

#endif // QT_NO_ACCESSIBILITY

#endif // QACESSIBLEWIDGETS_H
