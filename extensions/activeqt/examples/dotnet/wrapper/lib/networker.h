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

// lib.h

#pragma once

#using <mscorlib.dll>
using namespace System;

class Worker;

// .NET class
public __gc class netWorker
{
public:
    netWorker();
    ~netWorker();

    __property String *get_StatusString();
    __property void set_StatusString(String *string);

    __event void statusStringChanged(String *args);

private:
    Worker *workerObject;
};
