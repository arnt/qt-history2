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
    QLineEdit *lineEdit;
    QSpinBox *spinBox;
    QComboBox *comboBox, *editComboBox;
    QPushButton *ok;
    QStackedLayout *stack;
    QInputDialog::Type type;

    void init(const QString &label, QInputDialog::Type type);
};


class QInputDialogPrivateStackedLayout : public QStackedLayout
{
public:
    QSize sizeHint() const {
        if (QWidget *w = currentWidget())
            return w->sizeHint();
        return QStackedLayout::sizeHint();
    }
};

QInputDialogPrivate::QInputDialogPrivate()
    : QDialogPrivate(),
      label(0),
      lineEdit(0),
      spinBox(0),
      comboBox(0),
      editComboBox(0)
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

    stack = new QInputDialogPrivateStackedLayout;
    vbox->addLayout(stack);
    lineEdit = new QLineEdit(q);
    stack->addWidget(lineEdit);
    spinBox = new QSpinBox(q);
    stack->addWidget(spinBox);
    comboBox = new QComboBox(q);
    stack->addWidget(comboBox);
    editComboBox = new QComboBox(q);
    stack->addWidget(editComboBox);
    editComboBox->setEditable(true);
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

    QObject::connect(lineEdit, SIGNAL(returnPressed()), q, SLOT(tryAccept()));
    QObject::connect(lineEdit, SIGNAL(textChanged(QString)), q, SLOT(textChanged(QString)));

    QObject::connect(ok, SIGNAL(clicked()), q, SLOT(accept()));
    QObject::connect(cancel, SIGNAL(clicked()), q, SLOT(reject()));

    q->setType(type);

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

#ifdef QT3_SUPPORT
/*!
  \obsolete

  Constructs the dialog. The \a label is the text which is shown to the user
  (it should tell the user what they are expected to enter). The \a parent
  is the dialog's parent widget. The widget is called \a name. If \a
  modal is true (the default) the dialog will be modal. The \a type
  parameter is used to specify which type of dialog to construct. The \a f
  parameter is passed on to the QDialog constructor.

  \sa getText(), getInteger(), getDouble(), getItem()
*/

QInputDialog::QInputDialog(const QString &label, QWidget* parent,
                            const char* name, bool modal, Type type, Qt::WFlags f)
    : QDialog(*new QInputDialogPrivate, parent, f)
{
    Q_D(QInputDialog);
    d->init(label, type);
    setModal(modal);
    setObjectName(name);
}
#endif

/*!
  Returns the line edit which is used in LineEdit mode.
*/

QLineEdit *QInputDialog::lineEdit() const
{
    Q_D(const QInputDialog);
    return d->lineEdit;
}

/*!
  Returns the spinbox which is used in SpinBox mode.
*/

QSpinBox *QInputDialog::spinBox() const
{
    Q_D(const QInputDialog);
    return d->spinBox;
}

/*!
  Returns the combobox that is used in ComboBox mode.
*/

QComboBox *QInputDialog::comboBox() const
{
    Q_D(const QInputDialog);
    return d->comboBox;
}

/*!
  Returns the combobox that is used in EditableComboBox mode.
*/

QComboBox *QInputDialog::editableComboBox() const
{
    Q_D(const QInputDialog);
    return d->editComboBox;
}

/*!
  Sets the input type of the dialog to \a t.
*/

void QInputDialog::setType(Type t)
{
    Q_D(QInputDialog);
    QWidget *input = 0;
    switch (t) {
    case LineEdit:
        input = d->lineEdit;
        break;
    case SpinBox:
        input = d->spinBox;
        break;
    case ComboBox:
        input = d->comboBox;
        break;
    case EditableComboBox:
        input = d->editComboBox;
        break;
    }
    if (input) {
        d->stack->setCurrentIndex(t);
        input->setFocus();
#ifndef QT_NO_ACCEL
        d->label->setBuddy(input);
#endif
    }

    d->type = t;
}

/*!
  Returns the input type of the dialog.

  \sa setType()
*/

QInputDialog::Type QInputDialog::type() const
{
    Q_D(const QInputDialog);
    return d->type;
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
    dlg.lineEdit()->setText(text);
    dlg.lineEdit()->setEchoMode(mode);

    QString result;
    bool accepted = (dlg.exec() == QDialog::Accepted);
    if (ok)
        *ok = accepted;
    if (accepted)
        result = dlg.lineEdit()->text();

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
    dlg.spinBox()->setRange(minValue, maxValue);
    dlg.spinBox()->setSingleStep(step);
    dlg.spinBox()->setValue(value);

    bool accepted = (dlg.exec() == QDialog::Accepted);
    if (ok)
        *ok = accepted;
    return dlg.spinBox()->value();
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
    QInputDialog dlg(label, parent, LineEdit, f);
    dlg.setObjectName("qt_inputdlg_getdbl");
#ifndef QT_NO_WIDGET_TOPEXTRA
    dlg.setWindowTitle(title);
#endif
    dlg.lineEdit()->setValidator(new QDoubleValidator(minValue, maxValue, decimals, dlg.lineEdit()));
    dlg.lineEdit()->setText(QString::number(value, 'f', decimals));
    dlg.lineEdit()->selectAll();

    bool accepted = (dlg.exec() == QDialog::Accepted);
    if (ok)
        *ok = accepted;
    return dlg.lineEdit()->text().toDouble();
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
    if (editable) {
        dlg.editableComboBox()->addItems(list);
        dlg.editableComboBox()->setCurrentIndex(current);
    } else {
        dlg.comboBox()->addItems(list);
        dlg.comboBox()->setCurrentIndex(current);
    }

    QString result;
    bool accepted = (dlg.exec() == QDialog::Accepted);
    if (ok)
        *ok = accepted;
    if (editable)
        result = dlg.editableComboBox()->currentText();
    else
        result = dlg.comboBox()->currentText();

    return result;
}

/*!
  \internal

  This slot is invoked when the text is changed; the new text is passed
  in \a s.
*/

void QInputDialog::textChanged(const QString &s)
{
    Q_D(QInputDialog);
    bool on = true;

    if (d->lineEdit->validator()) {
        QString str = d->lineEdit->text();
        int index = d->lineEdit->cursorPosition();
        on = (d->lineEdit->validator()->validate(str, index) ==
               QValidator::Acceptable);
    } else if (type() != LineEdit) {
        on = !s.isEmpty();
    }
    d->ok->setEnabled(on);
}

/*!
  \internal
*/

void QInputDialog::tryAccept()
{
    Q_D(QInputDialog);
    if (!d->lineEdit->text().isEmpty())
        accept();
}

#endif
