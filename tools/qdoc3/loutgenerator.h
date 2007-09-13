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

/*
  loutgenerator.h
*/

#ifndef LOUTGENERATOR_H
#define LOUTGENERATOR_H

#include "bookgenerator.h"

QT_BEGIN_NAMESPACE

class LoutGenerator : public BookGenerator
{
public:
    LoutGenerator();
    ~LoutGenerator();

    virtual QString format();

protected:
    // ###
};

QT_END_NAMESPACE

#endif
