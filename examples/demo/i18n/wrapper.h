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

#ifndef WRAPPER_H
#define WRAPPER_H

#include <qvbox.h>
#include <qtranslator.h>
using namespace Qt;

class Wrapper : public QVBox
{
public:
    Wrapper(QWidget *parent, int i, const char *name = 0)
	: QVBox(parent, name, WDestructiveClose), translator(this), id(i)
    {
    }

    QTranslator translator;
    int id;
};


#endif // WRAPPER_H
