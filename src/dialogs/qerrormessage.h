/****************************************************************************
** $Id: .emacs,v 1.5 1999/05/06 19:35:46 agulbra Exp $
**
** Definition of a nice qInstallErrorMessage() handler
**
** Created : 2000-05-27, after Kalle Dalheimer's birthday
**
** Copyright (C) 2000 by Troll Tech AS.  All rights reserved.
**
****************************************************************************/

#ifndef QERRORHANDLER_H
#define QERRORHANDLER_H

#include "qglobal.h"
#include "qdict.h"
#include "qdialog.h"

class QPushButton;
class QCheckBox;
class QLabel;
class QTextView;
class QStringList;


class QErrorMessage: public QDialog {
    Q_OBJECT
public:
    QErrorMessage( QWidget * parent, const char * name = 0 );
    ~QErrorMessage();

    static QErrorMessage * qtHandler();

public slots:
    void message( const QString & );

protected:
    void done( int );

private:
    QPushButton * ok;
    QCheckBox * again;
    QTextView * errors;
    QLabel * icon;
    QStringList * pending;
    QDict<int> * doNotShow;

    bool nextPending();
};


#endif
