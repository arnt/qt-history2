/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qlinedialog.h#5 $
**
** Definition of QLineDialog class
**
** Created : 991212
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


//
//  W A R N I N G
//  -------------
//
//  It is very unlikely that this code will be available in the final
//  Qt release.  It might be available soon after then, but a number
//  of important API changes still need to be made.
//
//  Thus, it is important that you do NOT use this code in an application
//  unless you are willing for your application to be dependent on a
//  specific snapshot releases of Qt.
//

struct QLineDialogPrivate;

class QLineEdit;
class QSpinBox;
class QComboBox;

class Q_EXPORT QLineDialog : public QDialog
{
    Q_OBJECT
    Q_PROPERTY( Type, "type", type, setType )
	
public:
    enum Type { LineEdit, SpinBox, ComboBox, EditableComboBox };

    QLineDialog( const QString &label, QWidget* parent = 0, const char* name = 0,
		 bool modal = TRUE, Type type = LineEdit );
    ~QLineDialog();

    QLineEdit *lineEdit() const;
    QSpinBox *spinBox() const;
    QComboBox *comboBox() const;
    QComboBox *editableComboBox() const;

    void setType( Type t );
    Type type() const;

    static QString getText( const QString &label, const QString &text = QString::null,
			    bool *ok = 0, QWidget *parent = 0, const char *name = 0 );
    static int getInteger( const QString &label, int num = 0, int from = -2147483647, int to = 2147483647,
			   int step = 1, bool *ok = 0, QWidget *parent = 0, const char *name = 0 );
    static double getDouble( const QString &label, double num = 0, double from = -2147483647, double to = 2147483647,
			     int step = 1, bool *ok = 0, QWidget *parent = 0, const char *name = 0 );
    static QString getItem( const QString &label, const QStringList &list, int current = 0, bool editable = TRUE,
			    bool *ok = 0, QWidget *parent = 0, const char *name = 0 );

private slots:
    void textChanged( const QString &s );
    void tryAccept();

private:
    QLineDialogPrivate *d;

};

#endif
