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
#include <qextensionmanager.h>
#include <propertysheet.h>

// Qt
#include <qdebug.h>

AbstractMetaDataBase::AbstractMetaDataBase(QObject *parent)
    : QObject(parent)
{
}

AbstractMetaDataBase::~AbstractMetaDataBase()
{
}

