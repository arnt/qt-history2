/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qinputialog.h#5 $
**
** Definition of QInputDialog class
**
** Created : 991212
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the dialogs module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.  This file is part of the dialogs
** module and therefore may only be used if the dialogs module is specified
** as Licensed on the Licensee's License Certificate.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QINPUTDIALOG_H
#define QINPUTDIALOG_H

#ifndef QT_H
#include <qdialog.h>
#include <qstring.h>
#include <qlineedit.h>
#endif // QT_H

#ifndef QT_NO_INPUTDIALOG

class QInputDialogPrivate;

class QSpinBox;
class QComboBox;

class Q_EXPORT QInputDialog : public QDialog
{
#if defined(_CC_MSVC_)
    friend class QInputDialog;
#endif
    Q_OBJECT
	
private:
    enum Type { LineEdit, SpinBox, ComboBox, EditableComboBox };

    QInputDialog( const QString &label, QWidget* parent = 0, const char* name = 0,
		 bool modal = TRUE, Type type = LineEdit );
    ~QInputDialog();

    QLineEdit *lineEdit() const;
    QSpinBox *spinBox() const;
    QComboBox *comboBox() const;
    QComboBox *editableComboBox() const;

    void setType( Type t );
    Type type() const;

public:
    static QString getText( const QString &caption, const QString &label, const QString &text = QString::null,
			    bool *ok = 0, QWidget *parent = 0, const char *name = 0 ); // ### merge with below in 3.0
    static QString getText( const QString &caption, const QString &label, QLineEdit::EchoMode echo,  //### make default Normal in 3.0
			    const QString &text = QString::null, bool *ok = 0, QWidget *parent = 0, const char *name = 0 );

    static int getInteger( const QString &caption, const QString &label, int num = 0, int from = -2147483647,
			   int to = 2147483647,
			   int step = 1, bool *ok = 0, QWidget *parent = 0, const char *name = 0 );
    static double getDouble( const QString &caption, const QString &label, double num = 0,
			     double from = -2147483647, double to = 2147483647,
			     int step = 1, bool *ok = 0, QWidget *parent = 0, const char *name = 0 );
    static QString getItem( const QString &caption, const QString &label, const QStringList &list,
			    int current = 0, bool editable = TRUE,
			    bool *ok = 0, QWidget *parent = 0, const char *name = 0 );

private slots:
    void textChanged( const QString &s );
    void tryAccept();

private:
    QInputDialogPrivate *d;
    // just to avoid warnings...
    friend class QInputDialogPrivate;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QInputDialog( const QInputDialog & );
    QInputDialog &operator=( const QInputDialog & );
#endif
};

#endif // QT_NO_INPUTDIALOG

#endif // QINPUTDIALOG_H
