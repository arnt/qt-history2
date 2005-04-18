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

#ifndef QPROPERTYEDITOR_H
#define QPROPERTYEDITOR_H

#include "propertyeditor_global.h"
#include "qpropertyeditor_items_p.h"

#include <QtGui/QTreeView>

namespace qdesigner { namespace components { namespace propertyeditor {

class QPropertyEditorModel;
class QPropertyEditorDelegate;

class QT_PROPERTYEDITOR_EXPORT QPropertyEditor: public QTreeView
{
    Q_OBJECT
public:
    QPropertyEditor(QWidget *parent = 0);
    ~QPropertyEditor();

    IProperty *initialInput() const;
    bool isReadOnly() const;

    inline QPropertyEditorModel *editorModel() const
    { return m_model; }

signals:
    void propertyChanged(IProperty *property);

public slots:
    void setInitialInput(IProperty *initialInput);
    void setReadOnly(bool readOnly);

protected:
    virtual void drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const;
    virtual void keyPressEvent(QKeyEvent *ev);

private:
    QPropertyEditorModel *m_model;
    QPropertyEditorDelegate *m_itemDelegate;
};

} } } // namespace qdesigner::components::propertyeditor

#endif // QPROPERTYEDITOR_H
