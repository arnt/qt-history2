/****************************************************************************
**
** Definition of QInputDialog class.
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

#ifndef QINPUTDIALOG_H
#define QINPUTDIALOG_H

#ifndef QT_H
#include "qdialog.h"
#include "qstring.h"
#include "qlineedit.h"
#endif // QT_H

#ifndef QT_NO_INPUTDIALOG

class QSpinBox;
class QComboBox;
class QInputDialogPrivate;

class Q_EXPORT QInputDialog : public QDialog
{
#if defined(Q_CC_MSVC)
    friend class QInputDialog;
#endif
    Q_OBJECT

private:
    enum Type { LineEdit, SpinBox, ComboBox, EditableComboBox };

    QInputDialog( const QString &label, QWidget* parent=0, const char* name=0,
		 bool modal = TRUE, Type type = LineEdit, WFlags f = 0 );
    ~QInputDialog();

    QLineEdit *lineEdit() const;
    QSpinBox *spinBox() const;
    QComboBox *comboBox() const;
    QComboBox *editableComboBox() const;

    void setType( Type t );
    Type type() const;

public:
    static QString getText( const QString &caption, const QString &label, QLineEdit::EchoMode echo = QLineEdit::Normal,
			    const QString &text = QString::null, bool *ok = 0, QWidget *parent = 0, const char *name = 0, WFlags f = 0 );
    static int getInteger( const QString &caption, const QString &label, int value = 0, int minValue = -2147483647,
			   int maxValue = 2147483647,
			   int step = 1, bool *ok = 0, QWidget *parent = 0, const char *name = 0, WFlags f = 0 );
    static double getDouble( const QString &caption, const QString &label, double value = 0,
			     double minValue = -2147483647, double maxValue = 2147483647,
			     int decimals = 1, bool *ok = 0, QWidget *parent = 0, const char *name = 0, WFlags f = 0 );
    static QString getItem( const QString &caption, const QString &label, const QStringList &list,
			    int current = 0, bool editable = TRUE,
			    bool *ok = 0, QWidget *parent = 0, const char *name = 0, WFlags f = 0 );

private slots:
    void textChanged( const QString &s );
    void tryAccept();

private:
    QInputDialogPrivate *d;
    friend class QInputDialogPrivate; /* to avoid 'has no friends' warnings... */

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QInputDialog( const QInputDialog & );
    QInputDialog &operator=( const QInputDialog & );
#endif
};

#endif // QT_NO_INPUTDIALOG

#endif // QINPUTDIALOG_H

