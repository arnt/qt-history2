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

#ifndef SPECIALEDITORSUPPORT_H
#define SPECIALEDITORSUPPORT_H

#include "specialeditor_global.h"
#include <QObject>

class AbstractFormEditor;
class QDesignerSpecialEditorFactory;

class QT_SPECIALEDITORSUPPORT_EXPORT SpecialEditorSupport: public QObject
{
    Q_OBJECT
public:
    SpecialEditorSupport(AbstractFormEditor *core);
    virtual ~SpecialEditorSupport();

    inline AbstractFormEditor *core() const
    { return m_core; }

private:
    AbstractFormEditor *m_core;
    QDesignerSpecialEditorFactory *m_factory;
};

#endif // SPECIALEDITORSUPPORT_H
