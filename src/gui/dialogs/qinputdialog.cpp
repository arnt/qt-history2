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

#include "qinputdialog.h"

#ifndef QT_NO_INPUTDIALOG

#include "qlayout.h"
#include "qlabel.h"
#include "qlineedit.h"
#include "qpushbutton.h"
#include "qspinbox.h"
#include "qcombobox.h"
#include "qstackedlayout.h"
#include "qvalidator.h"
#include "qapplication.h"

#include "qdialog_p.h"

class QInputDialogPrivate : public QDialogPrivate
{
    Q_DECLARE_PUBLIC(QInputDialog)
public:
    QInputDialogPrivate();
    QLabel *label;
    QPushButton *ok;
    QWidget *input;

    void init(const QString &label, QInputDialog::Type);
    void tryAccept();
};

QInputDialogPrivate::QInputDialogPrivate()
    : QDialogPrivate(),
      label(0)
{
}

void QInputDialogPrivate::init(const QString &lbl, QInputDialog::Type type)
{
    Q_Q(QInputDialog);
    QVBoxLayout *vbox = new QVBoxLayout(q);
    vbox->setMargin(6);
    vbox->setSpacing(6);

    label = new QLabel(lbl, q);
    vbox->addWidget(label);
    vbox->addStretch(1);

    switch (type) {
    case QInputDialog::LineEdit:
        input = new QLineEdit(q);
        break;
    case QInputDialog::SpinBox:
        input = new QSpinBox(q);
        break;
    case QInputDialog::DoubleSpinBox:
        input = new QDoubleSpinBox(q);
        break;
    case QInputDialog::ComboBox:
    case QInputDialog::EditableComboBox: {
        QComboBox *combo = new QComboBox(q);
        if (type == QInputDialog::EditableComboBox)
            combo->setEditable(true);
        input = combo;
    }
        break;
    }
    vbox->addWidget(input);
    vbox->addStretch(1);

    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->setSpacing(6);
    vbox->addLayout(hbox, Qt::AlignRight);

    ok = new QPushButton(q->tr("OK"), q);
    ok->setDefault(true);
    QPushButton *cancel = new QPushButton(q->tr("Cancel"), q);

    QSize bs = ok->sizeHint().expandedTo(cancel->sizeHint());
    ok->setFixedSize(bs);
    cancel->setFixedSize(bs);

    hbox->addStretch();
    hbox->addWidget(ok);
    hbox->addWidget(cancel);

    QObject::connect(ok, SIGNAL(clicked()), q, SLOT(accept()));
    QObject::connect(cancel, SIGNAL(clicked()), q, SLOT(reject()));

    q->resize(q->sizeHint());
}


/*!
    \class QInputDialog
    \brief The QInputDialog class provides a simple convenience dialog to get a single value from the user.
    \ingroup dialogs
    \mainclass

    The input value can be a string, a number or an item from a list. A
    label must be set to tell the user what they should enter.

    Four static convenience functions are provided:
    getText(), getInteger(), getDouble() and getItem(). All the
    functions can be used in a similar way, for example:
    \code
    bool ok;
    QString text = QInputDialog::getText(this,
            "MyApp 3000", "Enter your name:", QLineEdit::Normal, "", &ok);
    if (ok && !text.isEmpty()) {
        // user entered something and pressed OK
    } else {
        // user entered nothing or pressed Cancel
    }
    \endcode
    The \c ok variable is set to true if the user clicked OK, or to
    false if they cancelled.

    \img inputdialogs.png Input Dialogs
*/

/*!
  \enum QInputDialog::Type

  This enum specifies the type of the dialog, i.e. what kind of data you
  want the user to input:

  \value LineEdit  A QLineEdit is used for obtaining string or numeric
  input. The QLineEdit can be accessed using lineEdit().

  \value SpinBox  A QSpinBox is used for obtaining integer input.
  Use spinBox() to access the QSpinBox.

  \value ComboBox  A read-only QComboBox is used to provide a fixed
  list of choices from which the user can choose.
  Use comboBox() to access the QComboBox.

  \value EditableComboBox  An editable QComboBox is used to provide a fixed
  list of choices from which the user can choose, but which also
  allows the user to enter their own value instead.
  Use editableComboBox() to access the QComboBox.
*/


/*!
  Constructs the dialog. The \a label is the text which is shown to
  the user (it should tell the user what they are expected to enter).
  The \a parent is the dialog's parent widget. The \a type parameter
  is used to specify which type of dialog to construct. The \a f
  parameter is passed on to the QDialog constructor.

  \sa getText(), getInteger(), getDouble(), getItem()
*/

QInputDialog::QInputDialog(const QString &label, QWidget* parent, Type type, Qt::WFlags f)
    : QDialog(*new QInputDialogPrivate, parent, f)
{
    Q_D(QInputDialog);
    d->init(label, type);
}

/*!
    Destroys the input dialog.
*/

QInputDialog::~QInputDialog()
{
}

/*!
    Static convenience function to get a string from the user. \a
    title is the text which is displayed in the title bar of the
    dialog. \a label is the text which is shown to the user (it should
    say what should be entered). \a text is the default text which is
    placed in the line edit. The \a mode is the echo mode the line
    edit will use. If \a ok is not-null \e *\a ok will be set to true
    if the user pressed OK and to false if the user pressed
    Cancel. The dialog's parent is \a parent. The dialog will be modal
    and uses the widget flags \a f.

    This function returns the text which has been entered in the line
    edit. It will not return an empty string.

    Use this static function like this:

    \code
    bool ok;
    QString text = QInputDialog::getText(this,
            "MyApp 3000", "Enter your name:", QLineEdit::Normal, "", &ok);
    if (ok && !text.isEmpty()) {
        // user entered something and pressed OK
    } else {
        // user entered nothing or pressed Cancel
    }
    \endcode
*/

QString QInputDialog::getText(QWidget *parent, const QString &title, const QString &label,
                               QLineEdit::EchoMode mode, const QString &text,
                               bool *ok, Qt::WFlags f)
{
    QInputDialog dlg(label, parent, LineEdit, f);
    dlg.setObjectName("qt_inputdlg_gettext");

#ifndef QT_NO_WIDGET_TOPEXTRA
    dlg.setWindowTitle(title);
#endif
    QLineEdit *le = qobject_cast<QLineEdit *>(dlg.d_func()->input);
    le->setText(text);
    le->setEchoMode(mode);

    QString result;
    bool accepted = (dlg.exec() == QDialog::Accepted);
    if (ok)
        *ok = accepted;
    if (accepted)
        result = le->text();

    return result;
}

/*!
    Static convenience function to get an integer input from the
    user. \a title is the text which is displayed in the title bar
    of the dialog.  \a label is the text which is shown to the user
    (it should say what should be entered). \a value is the default
    integer which the spinbox will be set to.  \a minValue and \a
    maxValue are the minimum and maximum values the user may choose,
    and \a step is the amount by which the values change as the user
    presses the arrow buttons to increment or decrement the value.

    If \a ok is not-null *\a ok will be set to true if the user
    pressed OK and to false if the user pressed Cancel. The dialog's
    parent is \a parent. The dialog will be modal and uses the widget
    flags \a f.

    This function returns the integer which has been entered by the user.

    Use this static function like this:

    \code
    bool ok;
    int res = QInputDialog::getInteger( this,
            "MyApp 3000", "Enter a number:", 22, 0, 1000, 2,
            &ok;
    if (ok) {
        // user entered something and pressed OK
    } else {
        // user pressed Cancel
    }
    \endcode
*/

int QInputDialog::getInteger(QWidget *parent, const QString &title, const QString &label,
                             int value, int minValue, int maxValue, int step, bool *ok,
                             Qt::WFlags f)
{
    QInputDialog dlg(label, parent, SpinBox, f);
    dlg.setObjectName("qt_inputdlg_getint");

#ifndef QT_NO_WIDGET_TOPEXTRA
    dlg.setWindowTitle(title);
#endif
    QSpinBox *sb = qobject_cast<QSpinBox *>(dlg.d_func()->input);
    sb->setRange(minValue, maxValue);
    sb->setSingleStep(step);
    sb->setValue(value);

    bool accepted = (dlg.exec() == QDialog::Accepted);
    if (ok)
        *ok = accepted;
    return sb->value();
}

/*!
    Static convenience function to get a floating point number from
    the user. \a title is the text which is displayed in the title
    bar of the dialog. \a label is the text which is shown to the user
    (it should say what should be entered). \a value is the default
    floating point number that the line edit will be set to. \a
    minValue and \a maxValue are the minimum and maximum values the
    user may choose, and \a decimals is the maximum number of decimal
    places the number may have.

    If \a ok is not-null, *\a ok will be set to true if the user
    pressed OK and to false if the user pressed Cancel. The dialog's
    parent is \a parent. The dialog will be modal and uses the widget
    flags \a f.

    This function returns the floating point number which has been
    entered by the user.

    Use this static function like this:

    \code
    bool ok;
    double res = QInputDialog::getDouble(this,
            "MyApp 3000", "Enter a decimal number:", 33.7, 0,
            1000, 2, &ok);
    if (ok) {
        // user entered something and pressed OK
    } else {
        // user pressed Cancel
    }
    \endcode
*/

double QInputDialog::getDouble( QWidget *parent, const QString &title, const QString &label,
                                double value, double minValue, double maxValue,
                                int decimals, bool *ok, Qt::WFlags f)
{
    QInputDialog dlg(label, parent, DoubleSpinBox, f);
    dlg.setObjectName("qt_inputdlg_getdbl");
#ifndef QT_NO_WIDGET_TOPEXTRA
    dlg.setWindowTitle(title);
#endif
    QDoubleSpinBox *sb = qobject_cast<QDoubleSpinBox *>(dlg.d_func()->input);
    sb->setRange(minValue, maxValue);
    sb->setDecimals(decimals);
    sb->setValue(value);

    bool accepted = (dlg.exec() == QDialog::Accepted);
    if (ok)
        *ok = accepted;
    return sb->value();
}

/*!
    Static convenience function to let the user select an item from a
    string list. \a title is the text which is displayed in the title
    bar of the dialog. \a label is the text which is shown to the user (it
    should say what should be entered). \a list is the
    string list which is inserted into the combobox, and \a current is the number
    of the item which should be the current item. If \a editable is true
    the user can enter their own text; if \a editable is false the user
    may only select one of the existing items.

    If \a ok is not-null \e *\a ok will be set to true if the user
    pressed OK and to false if the user pressed Cancel. The dialog's
    parent is \a parent. The dialog will be modal and uses the widget
    flags \a f.

    This function returns the text of the current item, or if \a
    editable is true, the current text of the combobox.

    Use this static function like this:

    \code
    QStringList lst;
    lst << "First" << "Second" << "Third" << "Fourth" << "Fifth";
    bool ok;
    QString res = QInputDialog::getItem(this,
            "MyApp 3000", "Select an item:", lst, 1, true, &ok);
    if (ok) {
        // user selected an item and pressed OK
    } else {
        // user pressed Cancel
    }
    \endcode
*/

QString QInputDialog::getItem(QWidget *parent, const QString &title, const QString &label, const QStringList &list,
                              int current, bool editable, bool *ok, Qt::WFlags f)
{
    QInputDialog dlg(label, parent, editable ? EditableComboBox : ComboBox, f);
    dlg.setObjectName("qt_inputdlg_getitem");
#ifndef QT_NO_WIDGET_TOPEXTRA
    dlg.setWindowTitle(title);
#endif

    QComboBox *combo = qobject_cast<QComboBox *>(dlg.d_func()->input);
    combo->addItems(list);
    combo->setCurrentIndex(current);

    bool accepted = (dlg.exec() == QDialog::Accepted);
    if (ok)
        *ok = accepted;

    return combo->currentText();
}

#endif
