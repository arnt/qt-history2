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

#include "QtGui/qdialog.h"
#include "QtCore/qstring.h"
#include "QtGui/qlineedit.h"

#ifndef QT_NO_INPUTDIALOG

class QSpinBox;
class QComboBox;
class QInputDialogPrivate;

class Q_GUI_EXPORT QInputDialog : public QDialog
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QInputDialog)

private:
    enum Type { LineEdit, SpinBox, ComboBox, EditableComboBox };

    explicit QInputDialog(const QString &label, QWidget* parent = 0,
                          Type type = LineEdit, Qt::WFlags f = 0);
#ifdef QT3_SUPPORT
    QT3_SUPPORT_CONSTRUCTOR QInputDialog(const QString &label, QWidget* parent, const char* name,
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
    static QString getText(QWidget *parent, const QString &caption, const QString &label,
                           QLineEdit::EchoMode echo = QLineEdit::Normal,
                           const QString &text = QString::null, bool *ok = 0, Qt::WFlags f = 0);
    static int getInteger(QWidget *parent, const QString &caption, const QString &label, int value = 0,
                          int minValue = -2147483647, int maxValue = 2147483647,
                          int step = 1, bool *ok = 0, Qt::WFlags f = 0);
    static double getDouble(QWidget *parent, const QString &caption, const QString &label, double value = 0,
                            double minValue = -2147483647, double maxValue = 2147483647,
                            int decimals = 1, bool *ok = 0, Qt::WFlags f = 0);
    static QString getItem(QWidget *parent, const QString &caption, const QString &label, const QStringList &list,
                           int current = 0, bool editable = true, bool *ok = 0,Qt::WFlags f = 0);


#ifdef QT3_SUPPORT
    inline static QT3_SUPPORT QString getText(const QString &caption, const QString &label,
                           QLineEdit::EchoMode echo = QLineEdit::Normal,
                           const QString &text = QString::null, bool *ok = 0,
                           QWidget *parent = 0, const char * = 0, Qt::WFlags f = 0)
        { return getText(parent, caption, label, echo, text, ok, f); }
    inline static QT3_SUPPORT int getInteger(const QString &caption, const QString &label, int value = 0,
                          int minValue = -2147483647, int maxValue = 2147483647,
                          int step = 1, bool *ok = 0,
                          QWidget *parent = 0, const char * = 0, Qt::WFlags f = 0)
        { return getInteger(parent, caption, label, value, minValue, maxValue, step, ok, f); }
    inline static QT3_SUPPORT double getDouble(const QString &caption, const QString &label, double value = 0,
                            double minValue = -2147483647, double maxValue = 2147483647,
                            int decimals = 1, bool *ok = 0,
                            QWidget *parent = 0, const char * = 0, Qt::WFlags f = 0)
        { return getDouble(parent, caption, label, value, minValue, maxValue, decimals, ok, f); }
    inline static QT3_SUPPORT QString getItem(const QString &caption, const QString &label, const QStringList &list,
                           int current = 0, bool editable = true, bool *ok = 0,
                           QWidget *parent = 0, const char * = 0, Qt::WFlags f = 0)
        { return getItem(parent, caption, label, list, current, editable, ok, f); }
#endif
private slots:
    void textChanged(const QString &s);
    void tryAccept();

private:
    Q_DISABLE_COPY(QInputDialog)
};

#endif // QT_NO_INPUTDIALOG

#endif // QINPUTDIALOG_H

