/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qinputialog.h#5 $
**
** Definition of QInputDialog class
**
** Created : 991212
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
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

#ifndef QINPUTDIALOG_H
#define QINPUTDIALOG_H

#ifndef QT_H
#include <qdialog.h>
#include <qstring.h>
#endif // QT_H


struct QInputDialogPrivate;

class QLineEdit;
class QSpinBox;
class QComboBox;

class Q_EXPORT QInputDialog : public QDialog
{
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
			    bool *ok = 0, QWidget *parent = 0, const char *name = 0 );
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
    
};

#endif
