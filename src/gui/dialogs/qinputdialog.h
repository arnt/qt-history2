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

#ifndef QINPUTDIALOG_H
#define QINPUTDIALOG_H

#include "qdialog.h"
#include "qstring.h"
#include "qlineedit.h"

#ifndef QT_NO_INPUTDIALOG

class QSpinBox;
class QComboBox;
class QInputDialogPrivate;

class Q_GUI_EXPORT QInputDialog : public QDialog
{
#if defined(Q_CC_MSVC)
    friend class QInputDialog;
#endif
    Q_OBJECT
    Q_DECLARE_PRIVATE(QInputDialog)

private:
    enum Type { LineEdit, SpinBox, ComboBox, EditableComboBox };

    QInputDialog(const QString &label, QWidget* parent = 0, Type type = LineEdit, Qt::WFlags f = 0);
#ifdef QT_COMPAT
    QT_COMPAT_CONSTRUCTOR QInputDialog(const QString &label, QWidget* parent, const char* name,
                                       bool modal = true, Type type = LineEdit, Qt::WFlags f = 0);
#endif
    ~QInputDialog();

    QLineEdit *lineEdit() const;
    QSpinBox *spinBox() const;
    QComboBox *comboBox() const;
    QComboBox *editableComboBox() const;

    void setType(Type t);
    Type type() const;

public:
    static QString getText(const QString &caption, const QString &label,
                           QLineEdit::EchoMode echo = QLineEdit::Normal,
                           const QString &text = QString::null, bool *ok = 0,
                           QWidget *parent = 0, const char *name = 0, Qt::WFlags f = 0);
    static int getInteger(const QString &caption, const QString &label, int value = 0,
                          int minValue = -2147483647, int maxValue = 2147483647,
                          int step = 1, bool *ok = 0,
                          QWidget *parent = 0, const char *name = 0, Qt::WFlags f = 0);
    static double getDouble(const QString &caption, const QString &label, double value = 0,
                            double minValue = -2147483647, double maxValue = 2147483647,
                            int decimals = 1, bool *ok = 0,
                            QWidget *parent = 0, const char *name = 0, Qt::WFlags f = 0);
    static QString getItem(const QString &caption, const QString &label, const QStringList &list,
                           int current = 0, bool editable = true, bool *ok = 0,
                           QWidget *parent = 0, const char *name = 0, Qt::WFlags f = 0);

private slots:
    void textChanged(const QString &s);
    void tryAccept();

private:
    Q_DISABLE_COPY(QInputDialog)
};

#endif // QT_NO_INPUTDIALOG

#endif // QINPUTDIALOG_H

