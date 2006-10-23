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

#ifndef INPLACE_EDITOR_H
#define INPLACE_EDITOR_H

#include <QtGui/QLineEdit>

class QDesignerFormWindowInterface;

namespace qdesigner_internal {

class InPlaceEditor: public QLineEdit
{
    Q_OBJECT
    
    InPlaceEditor(QWidget *widget, QDesignerFormWindowInterface *fw);
public:
    virtual ~InPlaceEditor();

    virtual bool eventFilter(QObject *object, QEvent *event);

    // Create on form window and set focus.
    static InPlaceEditor* create(QWidget *widget,
                                 QDesignerFormWindowInterface *fw,
                                 const QString& text,
                                 const QRect& r);
private:
    QWidget *m_widget;
    const bool m_noChildEvent;
};

}  // namespace qdesigner_internal

#endif // INPLACE_EDITOR_H
