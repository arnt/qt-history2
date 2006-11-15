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

#ifndef ABSTRACTINTEGRATION_H
#define ABSTRACTINTEGRATION_H

#include <QtDesigner/sdk_global.h>

#include <QtCore/QObject>

QT_BEGIN_HEADER

class QDesignerFormEditorInterface;

class QDESIGNER_SDK_EXPORT QDesignerIntegrationInterface: public QObject
{
    Q_OBJECT
public:
    QDesignerIntegrationInterface(QDesignerFormEditorInterface *core, QObject *parent = 0);

    inline QDesignerFormEditorInterface *core() const;

    virtual QWidget *containerWindow(QWidget *widget) const = 0;

private:
    QDesignerFormEditorInterface *m_core;
};

inline QDesignerFormEditorInterface *QDesignerIntegrationInterface::core() const
{ return m_core; }

QT_END_HEADER

#endif // ABSTRACTINTEGRATION_H
