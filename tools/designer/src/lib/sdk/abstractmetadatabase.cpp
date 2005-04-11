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

// sdk
#include "abstractmetadatabase.h"
#include "abstractformeditor.h"

// extension
#include <QtDesigner/qextensionmanager.h>
#include <QtDesigner/propertysheet.h>

// Qt
#include <QtCore/qdebug.h>

QDesignerMetaDataBaseInterface::QDesignerMetaDataBaseInterface(QObject *parent)
    : QObject(parent)
{
}

QDesignerMetaDataBaseInterface::~QDesignerMetaDataBaseInterface()
{
}

