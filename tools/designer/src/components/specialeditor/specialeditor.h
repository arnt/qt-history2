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

#ifndef SPECIALEDITOR_H
#define SPECIALEDITOR_H

#include <extension.h>
#include <QObject>

class QWidget;

class ISpecialEditor
{
public:
    virtual ~ISpecialEditor() {}

    virtual QWidget *createEditor(QWidget *parent) = 0;
    virtual void applyChanges() = 0;
    virtual void revertChanges() = 0;
};

Q_DECLARE_EXTENSION_INTERFACE(ISpecialEditor, "http://trolltech.com/Qt/IDE/SpecialEditor")

#endif // SPECIALEDITOR_H
