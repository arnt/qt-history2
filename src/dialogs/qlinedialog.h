/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qlinedialog.h#5 $
**
** Definition of QFileDialog class
**
** Created : 950428
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QLINEDIALOG_H
#define QLINEDIALOG_H

#ifndef QT_H
#include <qdialog.h>
#include <qstring.h>
#endif // QT_H

struct QLineDialogPrivate;

class Q_EXPORT QLineDialog : public QDialog
{
    Q_OBJECT
public:
    QLineDialog( const QString &label, QWidget* parent = 0, const char* name = 0, bool modal = TRUE );
    ~QLineDialog();
qproperties:
    QString text() const;
    virtual void setText( const QString &text );
public:
    static QString getText( const QString &label, const QString &text = QString::null,
			    bool *ok = 0, QWidget *parent = 0, const char *name = 0 );

private slots:
    void textChanged( const QString &s );
    void tryAccept();

private:
    QLineDialogPrivate *d;

};

#endif
