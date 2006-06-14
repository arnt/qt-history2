/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QT_NO_MESSAGEBOXEX

#include "qmessageboxex.h"
#include <QtGui/QDialogButtonBox>
#include <QtGui/QLabel>
#include <QtCore/QList>
#include <QtGui/QStyle>
#include <QtGui/QStyleOption>
#include <QtGui/QGridLayout>
#include <QtGui/QPushButton>
#include <QtGui/QAccessible>
#include <QtGui/QIcon>
#include <QtGui/QMessageBox>
#include <QtGui/QTextDocument>
#include "qdialog_p.h"

class QMessageBoxExPrivate : public QDialogPrivate
{
    Q_DECLARE_PUBLIC(QMessageBoxEx)
public:
    QMessageBoxExPrivate() { }

    void init();
    void _q_buttonClicked(QAbstractButton *);

    QLabel *label;
    QMessageBoxEx::Icon icon;
    QLabel *iconLabel;
    QDialogButtonBox *buttonBox;
    QList<QPushButton *> buttonList;
    int escapeButtonId;
    int defaultButtonId;
};

void QMessageBoxExPrivate::init()
{
    Q_Q(QMessageBoxEx);
    label = new QLabel;
    label->setObjectName(QLatin1String("qt_msgboxex_label"));
    if (q->style()->styleHint(QStyle::SH_MessageBox_TextSelectable))
        label->setFocusPolicy(Qt::ClickFocus);
    label->setAlignment(Qt::AlignTop|Qt::AlignLeft);

    icon = QMessageBoxEx::NoIcon;
    iconLabel = new QLabel;
    iconLabel->setObjectName(QLatin1String("qt_msgboxex_icon_label"));
    iconLabel->setPixmap(QPixmap());

    buttonBox = new QDialogButtonBox;
    buttonBox->setObjectName(QLatin1String("qt_msgboxex_buttonbox"));
    QObject::connect(buttonBox, SIGNAL(clicked(QAbstractButton *)),
                     q, SLOT(_q_buttonClicked(QAbstractButton *)));

    escapeButtonId = -1;
    defaultButtonId = -1;

    QGridLayout *grid = new QGridLayout;
    grid->addWidget(iconLabel, 0, 0, 1, 1); 
    grid->addWidget(label, 0, 1, 1, 1);
    grid->addWidget(buttonBox, 1, 0, 1, 2);
    q->setLayout(grid);
}

void QMessageBoxExPrivate::_q_buttonClicked(QAbstractButton *button)
{
    Q_Q(QMessageBoxEx);
    q->done(buttonList.indexOf(static_cast<QPushButton *>(button)));
}

/*!
    \class QMessageBoxEx
    \since 4.2
    \brief The QMessageBoxEx class provides a modal dialog with a short message, 
           an icon, and buttons laid out depending on the current style.
    \ingroup dialogs
    \mainclass

    Message boxes are used to provide informative messages and to ask simple questions.
    QMessageBoxEx provides all the functionality of QMessageBox. In addition, 
    QMessageBoxEx has the capability to add any number of buttons to the message box.
    The buttons in the box are laid out depending on the platform.

    Like QMessageBox, QMessageBoxEx provides a range of different messages depending on
    the severity of the message. The message box has a different icon for each of
    the severity levels.You can use the static functions, QMessageBox::information(), 
    QMessageBox::question(), QMessageBox::critical and QMessageBox::warning() 
    for the most common cases.

    The text part of all message box messages can be either rich text
    or plain text. With certain strings that contain XML meta characters, 
    the auto-rich text detection may fail, interpreting plain text 
    incorrectly as rich text. In these rare cases, use Qt::convertFromPlainText() 
    to convert your plain text string to a visually equivalent rich text string
    or set the text format explicitly with setTextFormat().

    Note that the Microsoft Windows User Interface Guidelines
    recommend using the application name as the window's caption.

    If none of the standard message boxes is suitable, you can create a
    QMessageBoxEx from scratch. You can use QMessageBoxEx::addButton() to add
    the standard buttons in QMessageBox::StandardButton. QMessageBoxEx::addButton()
    has an additional overload, that takes a custom text and the button role 
    as an argument. The button role is used to automatically determine the 
    position of the button within the dialog box.

    The text(), icon() and iconPixmap() functions provide access to the
    current text and pixmap of the message box. The setText(), setIcon()
    and setIconPixmap() let you change it. The difference between
    setIcon() and setIconPixmap() is that the former accepts a
    QMessageBoxEx::Icon and can be used to set standard icons, whereas the
    latter accepts a QPixmap and can be used to set custom icons.

    setButtonText() and buttonText() provide access to the buttons.

    QMessageBoxEx has no signals or slots.

    The \l{dialogs/standarddialogs}{Standard Dialogs} example shows
    how to use QMessageBoxEx as well as other built-in Qt dialogs.

    \sa QDialogButtonBox, QMessageBox, {fowler}{GUI Design Handbook: Message Box}, {Standard Dialogs Example}, {Application Example}
*/


/*!
    Constructs a message box with no text and no buttons.

    If \a parent is 0, the message box becomes an application-global
    modal dialog box. If \a parent is a widget, the message box
    becomes modal relative to \a parent.

    The \a parent argument is passed to the QDialog constructor.
*/
QMessageBoxEx::QMessageBoxEx(QWidget *parent)
: QDialog(*new QMessageBoxExPrivate, parent, Qt::MSWindowsFixedSizeDialogHint | Qt::WindowTitleHint | Qt::WindowSystemMenuHint)
{
    Q_D(QMessageBoxEx);
    d->init();
    setModal(true);
}

/*!
    Constructs a message box with a \a caption, a \a text and an \a icon.
    Buttons to the message box can be added using addButton().

   If \a parent is 0, the message box becomes an application-global
    modal dialog box. If \a parent is a widget, the message box
    becomes modal relative to \a parent.

    The \a parent and \a f arguments are passed to the QDialog constructor.

    \sa setWindowTitle(), setText(), setIcon()
*/
QMessageBoxEx::QMessageBoxEx(const QString &caption, const QString &text, 
                             Icon icon, QWidget *parent, Qt::WFlags f)
: QDialog(*new QMessageBoxExPrivate, parent, f | Qt::MSWindowsFixedSizeDialogHint | Qt::WindowTitleHint | Qt::WindowSystemMenuHint)
{
    Q_D(QMessageBoxEx);
    d->init();
    setModal(true);

#ifdef Q_WS_MAC
    // Make our message box look a little more mac like.
    QString finalText = QLatin1String("<p><b>") + caption + QLatin1String("</b></p>");
    if (Qt::mightBeRichText(text))
        finalText += QLatin1String("<br><br>") + text;
    else
        finalText += Qt::convertFromPlainText(text);

    setText(finalText);
#else
    setWindowTitle(caption);
    setText(text);
#endif

    setIcon(icon);
}

/*!
    Destroys the message box.
*/
QMessageBoxEx::~QMessageBoxEx()
{
}

/*!
    \enum QMessageBoxEx::ButtonRole

    This enum describes the roles that can be used to describe buttons in
    the message box. Combinations of these roles are as flags used to
    describe different aspects of their behavior.

    \value InvalidRole The button is invalid.
    \value AcceptRole Clicking the button causes the dialog to be accepted
           (e.g. OK).
    \value RejectRole Clicking the button causes the dialog to be rejected
           (e.g. Cancel).
    \value DestructiveRole Clicking the button causes a destructive change
           (e.g. for Discarding Changes).
    \value ActionRole Clicking the button causes changes to the elements in
           the dialog (e.g. reset all the values or read defaults).
    \value HelpRole The button can be clicked to request help.
    \omitvalue NRoles
*/

/*!
    Creates a button with the given \a text, adds it to the message box for the
    specified \a role, and returns the corresponding button id. If \a role is
    invalid, no button is created, and -1 is returned.

    Button id's start from 0. Id's are guaranteed to be in the order in which
    they were added.
*/
int QMessageBoxEx::addButton(const QString& text, ButtonRole role)
{
    Q_D(QMessageBoxEx);
    QPushButton *button = d->buttonBox->addButton(text, (QDialogButtonBox::ButtonRole)role);
    if (!button)
        return -1;
    d->buttonList.append(button);
    return d->buttonList.count() - 1;
}

/*!
    \enum QMessageBoxEx::StandardButton

    These enums describe flags for standard buttons. Each button has a
    defined \l ButtonRole.

    \value Ok An "OK" button defined with the \l AcceptRole.
    \value Open A "Open" button defined with the \l AcceptRole.
    \value Save A "Save" button defined with the \l AcceptRole.
    \value Cancel A "Cancel" button defined with the \l RejectRole.
    \value Close A "Close" button defined with the \l RejectRole.
    \value Discard A "Discard" or "Don't Save" button, depending on the platform,
                    defined with the \l DestructiveRole.
    \value Apply An "Apply" button defined with the \l ActionRole.
    \value Reset A "Reset" button defined with the \l ActionRole.
    \value Help A "Help" button defined with the \l HelpRole.
    \value SaveAll A "Save All" button defined with the \l AcceptRole
    \value Yes A "Yes" button defined with the \l AcceptRole
    \value YesToAll A "Yes to All" button defined with the \l AcceptRole
    \value No A "No" button defined with the \l RejectRole
    \value NoToAll A "No to All" button defined with the \l RejectRole
    \value Abort An "Abort" button defined with the \l RejectRole
    \value Retry A "Retry" button defined with the \l ActionRole
    \value Ignore An "Ignore" button defined with the \l ActionRole
    \omitvalue NoButtons
*/

/*!
    Adds a standard \a button to the message  box if it is valid to do so, and 
    returns the button id. If \a button is invalid, it is not added to the 
    button box, and -1 is returned.
*/
int QMessageBoxEx::addButton(StandardButton button)
{
    Q_D(QMessageBoxEx);
    QPushButton *pb = d->buttonBox->addButton((QDialogButtonBox::StandardButton)button);
    if (!pb)
        return -1;
    d->buttonList.append(pb);
    return d->buttonList.count() - 1;
}

/*!
    Returns the QPushButton associated with the button id \a id
*/
QPushButton *QMessageBoxEx::button(int id) const
{
    Q_D(const QMessageBoxEx);
    if (id < 0 || id >= d->buttonList.count())
        return 0;
    return d->buttonList.at(id);
}

/*!
    Returns the button id that gets activated when escape is presed.  Returns -1 
    if no escape button was set.

    \sa addButton(), setEscapeButton()
*/
int QMessageBoxEx::escapeButton() const
{
    Q_D(const QMessageBoxEx);
    return d->escapeButtonId;
}

/*!
    Sets the button that gets activated when escape key is pressed to \a id.

    \sa addButton()
*/
void QMessageBoxEx::setEscapeButton(int id)
{
    Q_D(QMessageBoxEx);
    if (id < 0 || id >= d->buttonList.count())
        return;
    d->escapeButtonId = id;
}

/*!
    Returns the button id that gets activated by default.  Returns -1 if no 
    default button was set.

    \sa addButton(), setDefaultButton()
*/
int QMessageBoxEx::defaultButton() const
{
    Q_D(const QMessageBoxEx);
    return d->defaultButtonId;
}

/*!
    Sets the button that gets activated by default \a id.

    \sa addButton(), defaultButton()
*/
void QMessageBoxEx::setDefaultButton(int id)
{
    Q_D(QMessageBoxEx);
    if (id < 0 || id >= d->buttonList.count())
        return;
    d->defaultButtonId = id;
    QPushButton *button = d->buttonList[id];
    button->setDefault(true);
    button->setFocus();
}

/*!
    \property QMessageBoxEx::text
    \brief the message box text to be displayed.

    The text will be interpreted either as a plain text or as rich
    text, depending on the text format setting (\l
    QMessageBoxEx::textFormat). The default setting is Qt::AutoText, i.e.
    the message box will try to auto-detect the format of the text.

    The default value of this property is an empty string.

    \sa textFormat
*/
QString QMessageBoxEx::text() const
{
    Q_D(const QMessageBoxEx);
    return d->label->text();
}

void QMessageBoxEx::setText(const QString &text)
{
    Q_D(QMessageBoxEx);
    d->label->setText(text);
}

/*!
    \enum QMessageBoxEx::Icon

    This enum has the following values:

    \value NoIcon the message box does not have any icon.

    \value Question an icon indicating that
    the message is asking a question.

    \value Information an icon indicating that
    the message is nothing out of the ordinary.

    \value Warning an icon indicating that the
    message is a warning, but can be dealt with.

    \value Critical an icon indicating that
    the message represents a critical problem.

*/

/*!
    \property QMessageBoxEx::icon
    \brief the message box's icon

    The icon of the message box can be one of the following predefined
    icons:
    \list
    \i QMessageBoxEx::NoIcon
    \i QMessageBoxEx::Question
    \i QMessageBoxEx::Information
    \i QMessageBoxEx::Warning
    \i QMessageBoxEx::Critical
    \endlist

    The actual pixmap used for displaying the icon depends on the
    current \link QWidget::style() GUI style\endlink. You can also set
    a custom pixmap icon using the \l QMessageBoxEx::iconPixmap
    property. The default icon is QMessageBoxEx::NoIcon.

    \sa iconPixmap
*/
QMessageBoxEx::Icon QMessageBoxEx::icon() const
{
    Q_D(const QMessageBoxEx);
    return d->icon;
}

void QMessageBoxEx::setIcon(Icon icon)
{
    Q_D(QMessageBoxEx);
    setIconPixmap(QMessageBox::standardIcon((QMessageBox::Icon)icon));
    d->icon = icon;
}

/*!
    \property QMessageBoxEx::iconPixmap
    \brief the current icon

    The icon currently used by the message box. Note that it's often
    hard to draw one pixmap that looks appropriate in all GUI styles;
    you may want to supply a different pixmap for each platform.

    \sa icon
*/
QPixmap QMessageBoxEx::iconPixmap() const
{
    Q_D(const QMessageBoxEx);
    return *d->iconLabel->pixmap();
}

void QMessageBoxEx::setIconPixmap(const QPixmap &pixmap)
{
    Q_D(QMessageBoxEx);
    d->iconLabel->setPixmap(pixmap);
    d->iconLabel->setFixedSize(d->iconLabel->sizeHint());
    d->icon = NoIcon;
}

/*!
    \property QMessageBoxEx::textFormat
    \brief the format of the text displayed by the message box

    The current text format used by the message box. See the \l
    Qt::TextFormat enum for an explanation of the possible options.

    The default format is Qt::AutoText.

    \sa setText()
*/
Qt::TextFormat QMessageBoxEx::textFormat() const
{
    Q_D(const QMessageBoxEx);
    return d->label->textFormat();
}

void QMessageBoxEx::setTextFormat(Qt::TextFormat format)
{
    Q_D(QMessageBoxEx);
    d->label->setTextFormat(format);
    bool wordwrap = format == Qt::RichText
                || (format == Qt::AutoText && Qt::mightBeRichText(d->label->text()));
    d->label->setWordWrap(wordwrap);
}

/*!\reimp
*/
bool QMessageBoxEx::event(QEvent *ev)
{
    return QDialog::event(ev);
}

/*!\reimp
*/
void QMessageBoxEx::closeEvent(QCloseEvent *e)
{
    Q_D(QMessageBoxEx);
    QDialog::closeEvent(e);
    setResult(d->escapeButtonId);
}

/*!\reimp
*/
void QMessageBoxEx::changeEvent(QEvent *ev)
{
    Q_D(QMessageBoxEx);
    if (ev->type() == QEvent::StyleChange) {
        if (d->icon != NoIcon)
            setIcon(d->icon);
    }
    QWidget::changeEvent(ev);
}

/*!\reimp
*/
void QMessageBoxEx::keyPressEvent(QKeyEvent *e)
{
    Q_D(QMessageBoxEx);
    if (e->key() == Qt::Key_Escape
#ifdef Q_WS_MAC
        || (e->modifiers() == Qt::ControlModifier && e->key() == Qt::Key_Period)
#endif
        ) {
        if (d->escapeButtonId != -1) {
            QPushButton *pb = d->buttonList[d->escapeButtonId];
            pb->animateClick();
        }
        e->accept();
        close();
        return;
    }
    // Ask trenton if this fits better in QDialogButtonBox
#ifndef QT_NO_SHORTCUT
    if (!(e->modifiers() & Qt::AltModifier)) {
        int key = e->key() & ~((int)Qt::MODIFIER_MASK|(int)Qt::UNICODE_ACCEL);
        if (key) {
            for (int i = 0; i < d->buttonList.size(); ++i) {
                QPushButton *pb = d->buttonList.at(i);
                int acc = pb->shortcut() & ~((int)Qt::MODIFIER_MASK|(int)Qt::UNICODE_ACCEL);
                if (acc == key) {
                    pb->animateClick();
                    return;
                }
            }
        }
    }
#endif
    QDialog::keyPressEvent(e);
}

/*!\reimp
*/
void QMessageBoxEx::showEvent(QShowEvent *e)
{
    Q_D(QMessageBoxEx);
    if (d->buttonList.isEmpty())
        addButton(Ok);
#ifndef QT_NO_ACCESSIBILITY
    QAccessible::updateAccessibility(this, 0, QAccessible::Alert);
#endif
    QDialog::showEvent(e);
}

static QMessageBoxEx::StandardButton showMessageBoxEx(QWidget *parent,
    QMessageBoxEx::Icon icon,
    const QString& caption, const QString& text,
    QMessageBoxEx::StandardButtons buttons, 
    QMessageBoxEx::StandardButton defaultButton)
{
    QMessageBoxEx msgBox(caption, text, icon, parent);
    QDialogButtonBox *buttonBox = msgBox.findChild<QDialogButtonBox *>();
    Q_ASSERT(buttonBox != 0);

    int mask = 1;
    QList<int> buttonList;
    while (mask) {
        int sb = buttons & mask;
        mask <<= 1;
        if (!sb)
            continue;
        buttonList.append(sb);
        int id = msgBox.addButton((QMessageBoxEx::StandardButton)sb);
        QPushButton *button = msgBox.button(id);
        if ((defaultButton == QMessageBoxEx::NoButton && buttonBox->buttonRole(button) == QDialogButtonBox::AcceptRole)
            || (defaultButton != QMessageBoxEx::NoButton && sb == defaultButton))
            msgBox.setDefaultButton(id);
    }
    int retval = msgBox.exec();
    if (retval == -1)
        return QMessageBoxEx::Cancel;
    return (QMessageBoxEx::StandardButton)buttonList[retval];
}

/*!
    Opens an information message box with the caption \a caption and
    the text \a text. The standard buttons \a buttons is added to the
    message box. \a defaultButton specifies the button be used as the
    defaultButton. If the \a defaultButton is set to QMessageBoxEx::NoButton,
    QMessageBoxEx picks a suitable default automatically.
  
    Returns the identity of the standard button that was activated. If escape
    was pressed, returns QMessageBoxEx::Cancel.

    If \a parent is 0, the message box becomes an application-global
    modal dialog box. If \a parent is a widget, the message box
    becomes modal relative to \a parent.

    \sa question(), warning(), critical()
*/
QMessageBoxEx::StandardButton QMessageBoxEx::information(QWidget *parent, const QString &caption,
                               const QString& text, StandardButtons buttons,
                               StandardButton defaultButton)
{
    return showMessageBoxEx(parent, Information, caption, text, buttons, 
                            defaultButton);
}

/*!
    Opens a question message box with the caption \a caption and
    the text \a text. The standard buttons \a buttons is added to the
    message box. \a defaultButton specifies the button be used as the
    defaultButton. If the \a defaultButton is set to QMessageBoxEx::NoButton,
    QMessageBoxEx picks a suitable default automatically.
   
    Returns the identity of the standard button that was activated. If escape
    was pressed, returns QMessageBoxEx::Cancel.

    If \a parent is 0, the message box becomes an application-global
    modal dialog box. If \a parent is a widget, the message box
    becomes modal relative to \a parent.

    \sa information(), warning(), critical()
*/
QMessageBoxEx::StandardButton QMessageBoxEx::question(QWidget *parent, const QString &caption,
                            const QString& text, StandardButtons buttons,
                            StandardButton defaultButton)
{
    return showMessageBoxEx(parent, Question, caption, text, buttons, defaultButton);
}

/*!
    Opens a warning message box with the caption \a caption and
    the text \a text. The standard buttons \a buttons is added to the
    message box. \a defaultButton specifies the button be used as the
    defaultButton.  If the \a defaultButton is set to QMessageBoxEx::NoButton,
    QMessageBoxEx picks a suitable default automatically.
   
    Returns the identity of the standard button that was activated. If escape
    was pressed, returns QMessageBoxEx::Cancel.

    If \a parent is 0, the message box becomes an application-global
    modal dialog box. If \a parent is a widget, the message box
    becomes modal relative to \a parent.

    \sa question(), information(), critical()
*/
QMessageBoxEx::StandardButton QMessageBoxEx::warning(QWidget *parent, const QString &caption,
                        const QString& text, StandardButtons buttons,
                        StandardButton defaultButton)
{
    return showMessageBoxEx(parent, Warning, caption, text, buttons, defaultButton);
}

/*!
    Opens a critical message box with the caption \a caption and
    the text \a text. The standard buttons \a buttons is added to the
    message box. \a defaultButton specifies the button be used as the
    defaultButton. If the \a defaultButton is set to QMessageBoxEx::NoButton,
    QMessageBoxEx picks a suitable default automatically.

    Returns the identity of the standard button that was activated. If escape
    was pressed, returns QMessageBoxEx::Cancel.

    If \a parent is 0, the message box becomes an application-global
    modal dialog box. If \a parent is a widget, the message box
    becomes modal relative to \a parent.

    \sa question(), warning(), information()
*/
QMessageBoxEx::StandardButton QMessageBoxEx::critical(QWidget *parent, const QString &caption,
                         const QString& text, StandardButtons buttons,
                         StandardButton defaultButton)
{
    return showMessageBoxEx(parent, Critical, caption, text, buttons, defaultButton);
}

#include "moc_qmessageboxex.cpp"
#endif // QT_NO_MESSAGEBOXEX
