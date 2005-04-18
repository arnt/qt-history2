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

#ifndef KEYSEQUENCEEDITOR_H
#define KEYSEQUENCEEDITOR_H

#include "propertyeditor_global.h"

#include <QtGui/QWidget>
#include <QtGui/QKeySequence>

class QLineEdit;
class QToolButton;

namespace qdesigner { namespace components { namespace propertyeditor {

class QT_PROPERTYEDITOR_EXPORT KeySequenceEditor: public QWidget
{
    Q_OBJECT
public:
    KeySequenceEditor(QWidget *parent = 0);
    virtual ~KeySequenceEditor();

    QKeySequence keySequence() const;

    QToolButton *resetButton() const;
    QLineEdit *lineEdit() const;

    virtual bool eventFilter(QObject *o, QEvent *e);

signals:
    void changed();

public slots:
    void setKeySequence(const QKeySequence &keySequence);
    void reset();

protected:
    void handleKeyEvent(QKeyEvent *e);
    int translateModifiers(Qt::KeyboardModifiers modifier);

private:
    QLineEdit *m_lineEdit;
    QToolButton *m_resetButton;
    bool mouseEnter;
    int num;
    int k1, k2, k3, k4;
};

} } } // namespace qdesigner::components::propertyeditor

#endif // KEYSEQUENCEEDITOR_H
