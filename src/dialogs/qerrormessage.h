/****************************************************************************
**
** Definition of a nice qInstallErrorMessage() handler.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the dialogs module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QERRORMESSAGE_H
#define QERRORMESSAGE_H

#ifndef QT_H
#include "qdialog.h"
#include "qhash.h"
#endif // QT_H

#ifndef QT_NO_ERRORMESSAGE
class QPushButton;
class QCheckBox;
class QLabel;
class QTextView;
class QStringList;

class Q_GUI_EXPORT QErrorMessage: public QDialog {
    Q_OBJECT
public:
    QErrorMessage( QWidget* parent=0, const char* name=0 );
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
    QHash<QString, int> doNotShow;

    bool nextPending();

#if defined(Q_DISABLE_COPY) // Disabled copy constructor and operator=
    QErrorMessage( const QErrorMessage & );
    QErrorMessage &operator=( const QErrorMessage & );
#endif
};

#endif //QT_NO_ERRORMESSAGE

#endif
