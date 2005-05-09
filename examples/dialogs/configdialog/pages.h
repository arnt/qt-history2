/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef PAGES_H
#define PAGES_H

#include <QWidget>

class ConfigurationPage : public QWidget
{
public:
    ConfigurationPage(QWidget *parent = 0);
};

class QueryPage : public QWidget
{
public:
    QueryPage(QWidget *parent = 0);
};

class UpdatePage : public QWidget
{
public:
    UpdatePage(QWidget *parent = 0);
};

#endif
