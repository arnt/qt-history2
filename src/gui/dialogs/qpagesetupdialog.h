/****************************************************************************
**
** Definition of print dialog.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the dialogs module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QPAGESETUPDIALOG_H
#define QPAGESETUPDIALOG_H

#ifndef QT_H
#include "qdialog.h"
#endif // QT_H

#ifndef QT_NO_PAGESETUPDIALOG

class QPageSetupDialogPrivate;
class QPrinter;

class Q_GUI_EXPORT QPageSetupDialog : public QDialog
{
    Q_OBJECT
public:
    QPageSetupDialog(QPrinter *printer, QWidget* parent=0, const char* name=0);
    ~QPageSetupDialog();

    virtual void accept();

private:
    QPageSetupDialogPrivate *d;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QPageSetupDialog(const QPageSetupDialog &);
    QPageSetupDialog &operator=(const QPageSetupDialog &);
#endif
};

#endif // QT_NO_PAGESETUPDIALOG

#endif // QPAGESETUPDIALOG_H
