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

#ifndef VIEW_H
#define VIEW_H

#include <qvbox.h>
#include <qcstring.h>
#include <qurloperator.h>

class QMultiLineEdit;

class View : public QVBox
{
    Q_OBJECT
    
public:
    View();
    
private slots:
    void downloadFile();
    void newData( const QByteArray &ba );

private:
    QMultiLineEdit *fileView;
    QUrlOperator op;
    
    QString getOpenFileName();
};

#endif
