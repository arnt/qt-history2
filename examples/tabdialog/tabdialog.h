/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef TABDIALOG_H
#define TABDIALOG_H

#include <qtabdialog.h>
#include <qstring.h>
#include <qfileinfo.h>

class TabDialog : public QTabDialog
{
    Q_OBJECT

public:
    TabDialog( QWidget *parent, const char *name, const QString &_filename );

protected:
    QString filename;
    QFileInfo fileinfo;

    void setupTab1();
    void setupTab2();
    void setupTab3();

};

#endif
