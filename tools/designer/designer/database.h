/**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef DATABASE_H
#define DATABASE_H

#include <qdialog.h>

class QDesignerSqlWidget : public QWidget
{
    Q_OBJECT

public:
    QDesignerSqlWidget( QWidget *parent, const char *name );

public slots:
    void prev();
    void next();
    void first();
    void last();
    void insertRecord();
    void deleteRecord();

protected:
    void paintEvent( QPaintEvent *e );

};

class QDesignerSqlDialog : public QDialog
{
    Q_OBJECT

public:
    QDesignerSqlDialog( QWidget *parent, const char *name );

public slots:
    void prev();
    void next();
    void first();
    void last();
    void insertRecord();
    void deleteRecord();

protected:
    void paintEvent( QPaintEvent *e );

};

#endif
