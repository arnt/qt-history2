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

#ifndef FORMEDITOR_H
#define FORMEDITOR_H

#include "formeditor_global.h"

#include <QtDesigner/QtDesigner>

#include <QtCore/QObject>

namespace qdesigner_internal {

class QT_FORMEDITOR_EXPORT FormEditor: public QDesignerFormEditorInterface
{
    Q_OBJECT
public:
    FormEditor(QObject *parent = 0);
    virtual ~FormEditor();
};

}  // namespace qdesigner_internal

#endif // FORMEDITOR_H
