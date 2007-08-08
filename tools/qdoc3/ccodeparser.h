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
  ccodeparser.h
*/

#ifndef CCODEPARSER_H
#define CCODEPARSER_H

#include "cppcodeparser.h"

class CCodeParser : public CppCodeParser
{
public:
    CCodeParser();
    ~CCodeParser();

    virtual QString language();
    virtual QString headerFileNameFilter();
    virtual QString sourceFileNameFilter();
};

#endif
