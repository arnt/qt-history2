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

#ifndef DOCUWINDOW_H
#define DOCUWINDOW_H

#include <qmainwindow.h>

class QTextBrowser;

class DocuWindow : public QMainWindow
{
    Q_OBJECT
public:
    DocuWindow( const QString& docu, QWidget *parent, QWidget *source );

public slots:
    void save();
    void print();

private:
    QTextBrowser *browser;
};

#endif // DOCUWINDOW_H
