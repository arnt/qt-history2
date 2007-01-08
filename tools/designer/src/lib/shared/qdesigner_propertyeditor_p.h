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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of Qt Designer.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//


#ifndef DESIGNERPROPERTYEDITOR_H
#define DESIGNERPROPERTYEDITOR_H

#include "shared_global_p.h"
#include <QtDesigner/QtDesigner>

namespace qdesigner_internal {

// Extends the QDesignerPropertyEditorInterface by property comment handling and
// a signal for resetproperty.

class QDESIGNER_SHARED_EXPORT QDesignerPropertyEditor: public QDesignerPropertyEditorInterface
{
    Q_OBJECT
public:
    QDesignerPropertyEditor(QWidget *parent = 0, Qt::WindowFlags flags = 0);

Q_SIGNALS:
    void propertyCommentChanged(const QString &name, const QString &value);
    void resetProperty(const QString &name);
    void addDynamicProperty(const QString &name, const QVariant &value);
    void removeDynamicProperty(const QString &name);

public Q_SLOTS:
    virtual void setPropertyComment(const QString &name, const QString &value) = 0;
};

}  // namespace qdesigner_internal

#endif // DESIGNERPROPERTYEDITOR_H
