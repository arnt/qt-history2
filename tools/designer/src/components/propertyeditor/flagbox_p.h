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

#ifndef FLAGBOX_P_H
#define FLAGBOX_P_H

#include "propertyeditor_global.h"
#include "flagbox_model_p.h"

#include <QtGui/QComboBox>

namespace qdesigner { namespace components { namespace propertyeditor {

class QT_PROPERTYEDITOR_EXPORT FlagBox: public QComboBox
{
    Q_OBJECT
public:
    FlagBox(QWidget *parent = 0);
    virtual ~FlagBox();

    inline FlagBoxModelItem itemAt(int index) const
    { return m_model->itemAt(index); }

    inline FlagBoxModelItem &item(int index)
    { return m_model->item(index); }

    inline QList<FlagBoxModelItem> items() const
    { return m_model->items(); }

    inline void setItems(const QList<FlagBoxModelItem> &items)
    { m_model->setItems(items); }

    inline void hidePopup() {}

private slots:
    void slotActivated(int index);

private:
    FlagBoxModel *m_model;
};

} } } // namespace qdesigner::components::propertyeditor

#endif // FLAGBOX_P_H
