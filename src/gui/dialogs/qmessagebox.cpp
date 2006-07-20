/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QT_NO_MESSAGEBOX

#include <QtGui/qdialogbuttonbox.h>
#include <QtGui/qlabel.h>
#include <QtCore/qlist.h>
#include <QtGui/qstyle.h>
#include <QtGui/qstyleoption.h>
#include <QtGui/qgridlayout.h>
#include <QtGui/qpushbutton.h>
#include <QtGui/qaccessible.h>
#include <QtGui/qicon.h>
#include <QtGui/qtextdocument.h>
#include <QtGui/qmessagebox.h>
#include <QtGui/qapplication.h>
#include "qdialog_p.h"

enum Button { Old_Ok = 1, Old_Cancel = 2, Old_Yes = 3, Old_No = 4, Old_Abort = 5, Old_Retry = 6,
              Old_Ignore = 7, Old_YesAll = 8, Old_NoAll = 9, Old_ButtonMask = 0xFF,
              NewButtonFlag = 0xFFFFFC00 };

#ifndef QT_NO_IMAGEFORMAT_XPM
/* XPM */
static const char * const qtlogo_xpm[] = {
/* width height ncolors chars_per_pixel */
"50 50 17 1",
/* colors */
"  c #000000",
". c #495808",
"X c #2A3304",
"o c #242B04",
"O c #030401",
"+ c #9EC011",
"@ c #93B310",
"# c #748E0C",
"$ c #A2C511",
"% c #8BA90E",
"& c #99BA10",
"* c #060701",
"= c #181D02",
"- c #212804",
"; c #61770A",
": c #0B0D01",
"/ c None",
/* pixels */
"/$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$/",
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$+++$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$@;.o=::=o.;@$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$+#X*         **X#+$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$#oO*         O  **o#+$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$&.* OO              O*.&$$$$$$$$$$$$$",
"$$$$$$$$$$$$@XOO            * OO    X&$$$$$$$$$$$$",
"$$$$$$$$$$$@XO OO  O  **:::OOO OOO   X@$$$$$$$$$$$",
"$$$$$$$$$$&XO      O-;#@++@%.oOO      X&$$$$$$$$$$",
"$$$$$$$$$$.O  :  *-#+$$$$$$$$+#- : O O*.$$$$$$$$$$",
"$$$$$$$$$#*OO  O*.&$$$$$$$$$$$$+.OOOO **#$$$$$$$$$",
"$$$$$$$$+-OO O *;$$$$$$$$$$$&$$$$;*     o+$$$$$$$$",
"$$$$$$$$#O*  O .+$$$$$$$$$$@X;$$$+.O    *#$$$$$$$$",
"$$$$$$$$X*    -&$$$$$$$$$$@- :;$$$&-    OX$$$$$$$$",
"$$$$$$$@*O  *O#$$$$$$$$$$@oOO**;$$$#    O*%$$$$$$$",
"$$$$$$$;     -+$$$$$$$$$@o O OO ;+$$-O   *;$$$$$$$",
"$$$$$$$.     ;$$$$$$$$$@-OO OO  X&$$;O    .$$$$$$$",
"$$$$$$$o    *#$$$$$$$$@o  O O O-@$$$#O   *o$$$$$$$",
"$$$$$$+=    *@$$$$$$$@o* OO   -@$$$$&:    =$$$$$$$",
"$$$$$$+:    :+$$$$$$@-      *-@$$$$$$:    :+$$$$$$",
"$$$$$$+:    :+$$$$$@o* O    *-@$$$$$$:    :+$$$$$$",
"$$$$$$$=    :@$$$$@o*OOO      -@$$$$@:    =+$$$$$$",
"$$$$$$$-    O%$$$@o* O O    O O-@$$$#*   OX$$$$$$$",
"$$$$$$$. O *O;$$&o O*O* *O      -@$$;    O.$$$$$$$",
"$$$$$$$;*   Oo+$$;O*O:OO--      Oo@+=    *;$$$$$$$",
"$$$$$$$@*  O O#$$$;*OOOo@@-O     Oo;O*  **@$$$$$$$",
"$$$$$$$$X* OOO-+$$$;O o@$$@-    O O     OX$$$$$$$$",
"$$$$$$$$#*  * O.$$$$;X@$$$$@-O O        O#$$$$$$$$",
"$$$$$$$$+oO O OO.+$$+&$$$$$$@-O         o+$$$$$$$$",
"$$$$$$$$$#*    **.&$$$$$$$$$$@o      OO:#$$$$$$$$$",
"$$$$$$$$$+.   O* O-#+$$$$$$$$+;O    OOO:@$$$$$$$$$",
"$$$$$$$$$$&X  *O    -;#@++@#;=O    O    -@$$$$$$$$",
"$$$$$$$$$$$&X O     O*O::::O      OO    Oo@$$$$$$$",
"$$$$$$$$$$$$@XOO                  OO    O*X+$$$$$$",
"$$$$$$$$$$$$$&.*       **  O      ::    *:#$$$$$$$",
"$$$$$$$$$$$$$$$#o*OO       O    Oo#@-OOO=#$$$$$$$$",
"$$$$$$$$$$$$$$$$+#X:* *     O**X#+$$@-*:#$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$%;.o=::=o.#@$$$$$$@X#$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$$+++$$$$$$$$$$$+$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",
"/$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$/",
};
#endif // QT_NO_IMAGEFORMAT_XPM

class QMessageBoxPrivate : public QDialogPrivate
{
    Q_DECLARE_PUBLIC(QMessageBox)

public:
    QMessageBoxPrivate() : escapeButton(0), defaultButton(0), clickedButton(0) { }

    void init(const QString &title = QString(), const QString &text = QString());
    void _q_buttonClicked(QAbstractButton *);

    QAbstractButton *findButton(int button0, int button1, int button2, int flags);
    void addOldButtons(int button0, int button1, int button2);

    static int showOldMessageBox(QWidget *parent, QMessageBox::Icon icon,
                                 const QString &title, const QString &text,
                                 int button0, int button1, int button2);
    static int showOldMessageBox(QWidget *parent, QMessageBox::Icon icon,
                                 const QString &title, const QString &text,
                                 const QString &button0Text,
                                 const QString &button1Text,
                                 const QString &button2Text,
                                 int defaultButtonNumber,
                                 int escapeButtonNumber);

    static QMessageBox::StandardButton showMessageBoxEx(QWidget *parent,
                QMessageBox::Icon icon, const QString& title, const QString& text,
                QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton);

    QLabel *label;
    QMessageBox::Icon icon;
    QLabel *iconLabel;
    QDialogButtonBox *buttonBox;
    QList<QAbstractButton *> buttonList;
    QAbstractButton *escapeButton;
    QPushButton *defaultButton;
    QAbstractButton *clickedButton;
};

static QString *translatedTextAboutQt = 0;

void QMessageBoxPrivate::init(const QString &title, const QString &text)
{
    Q_Q(QMessageBox);

    if (!translatedTextAboutQt) {
        translatedTextAboutQt = new QString;

#if defined(QT_NON_COMMERCIAL)
    QT_NC_MSGBOX
#else
        *translatedTextAboutQt = QMessageBox::tr(
            "<h3>About Qt</h3>"
            "%1<p>Qt is a C++ toolkit for cross-platform "
            "application development.</p>"
            "<p>Qt provides single-source "
            "portability across MS&nbsp;Windows, Mac&nbsp;OS&nbsp;X, "
            "Linux, and all major commercial Unix variants. Qt is also"
            " available for embedded devices as Qtopia Core.</p>"
            "<p>Qt is a Trolltech product. See "
            "<a href=\"http://www.trolltech.com/qt/\">www.trolltech.com/qt/</a> for more information.</p>"
           )
#if QT_EDITION != QT_EDITION_OPENSOURCE
           .arg(QMessageBox::tr("<p>This program uses Qt version %1.</p>"))
#else
           .arg(QMessageBox::tr("<p>This program uses Qt Open Source Edition version %1.</p>"
                   "<p>Qt Open Source Edition is intended for the development "
                   "of Open Source applications. You need a commercial Qt "
                   "license for development of proprietary (closed source) "
                   "applications.</p>"
                   "<p>Please see <a href=\"http://www.trolltech.com/company/model.html\">www.trolltech.com/company/model.html</a> "
                   "for an overview of Qt licensing.</p>"))
#endif
           .arg(QLatin1String(QT_VERSION_STR));
#endif
    }

    label = new QLabel;
    label->setObjectName(QLatin1String("qt_msgboxex_label"));
    label->setTextInteractionFlags(Qt::TextInteractionFlags(q->style()->styleHint(QStyle::SH_MessageBox_TextInteractionFlags)));
    label->setAlignment(Qt::AlignTop|Qt::AlignLeft);

    icon = QMessageBox::NoIcon;
    iconLabel = new QLabel;
    iconLabel->setObjectName(QLatin1String("qt_msgboxex_icon_label"));
    iconLabel->setPixmap(QPixmap());

    buttonBox = new QDialogButtonBox;
    buttonBox->setObjectName(QLatin1String("qt_msgboxex_buttonbox"));
    buttonBox->setCenterButtons(q->style()->styleHint(QStyle::SH_MessageBox_CenterButtons));
    QObject::connect(buttonBox, SIGNAL(clicked(QAbstractButton *)),
                     q, SLOT(_q_buttonClicked(QAbstractButton *)));

    QGridLayout *grid = new QGridLayout;
    grid->addWidget(iconLabel, 0, 0, 1, 1, Qt::AlignTop);
    grid->addWidget(label, 0, 1, 1, 1);
    grid->addItem(new QSpacerItem(0, 10), 1, 0, 1, 2);
    grid->addWidget(buttonBox, 2, 0, 1, 2);
    grid->setSizeConstraint(QLayout::SetFixedSize);
    q->setLayout(grid);

    if (!title.isEmpty() || !text.isEmpty()) {
#ifdef Q_WS_MAC
        // Make our message box look a little more mac like.
        QString finalText = QLatin1String("<p><b>") + title + QLatin1String("</b></p>");
        if (Qt::mightBeRichText(text))
            finalText += QLatin1String("<br><br>") + text;
        else
            finalText += Qt::convertFromPlainText(text);

        q->setText(finalText);
#else
        q->setWindowTitle(title);
        q->setText(text);
#endif
    }
    q->setModal(true);
}

void QMessageBoxPrivate::_q_buttonClicked(QAbstractButton *button)
{
    Q_Q(QMessageBox);
    clickedButton = button;
    q->done(buttonList.indexOf(button)); // does not trigger closeEvent
}

/*!
    \class QMessageBox
    \brief The QMessageBox class provides a modal dialog with a short message,
           an icon, and buttons laid out depending on the current style.
    \ingroup dialogs
    \mainclass

    Message boxes are used to provide informative messages and to ask simple questions.

    QMessageBox supports four severity levels, indicated by an icon:

    \table
    \row
    \o \img qmessagebox-quest.png
    \o \l Question
    \o For message boxes that ask a question as part of normal
    operation. Some style guides recommend using Information for this
    purpose.
    \row
    \o \img qmessagebox-info.png
    \o \l Information
    \o For message boxes that are part of normal operation.
    \row
    \o \img qmessagebox-warn.png
    \o \l Warning
    \o For message boxes that tell the user about unusual errors.
    \row
    \o \img qmessagebox-crit.png
    \o \l Critical
    \o For message boxes that tell the user about critical errors.
    \endtable

    The easiest way to pop up a message box in Qt is to call one
    of the static functions QMessageBox::information(),
    QMessageBox::question(), QMessageBox::critical(),
    and QMessageBox::warning(). For example:

    \code
        int ret = QMessageBox::warning(this, tr("My Application"),
                          tr("The document has been modified.\n"
                             "Do you want to save your changes?"),
                          QMessageBox::Save | QMessageBox::Discard
                          | QMessageBox::Cancel,
                          QMessageBox::Save);
    \endcode




    The order of the buttons is platform-dependent.


    The text part of all message box messages can be either rich text
    or plain text. With certain strings that contain XML meta characters,
    the auto-rich text detection may fail, interpreting plain text
    incorrectly as rich text. In these rare cases, use Qt::convertFromPlainText()
    to convert your plain text string to a visually equivalent rich text string
    or set the text format explicitly with setTextFormat().

    Note that the Microsoft Windows User Interface Guidelines
    recommend using the application name as the window's title.

    If none of the standard message boxes is suitable, you can create a
    QMessageBox from scratch. You can use QMessageBox::addButton() to add
    the standard buttons in QMessageBox::StandardButton. QMessageBox::addButton()
    has an additional overload, that takes a custom text and the button role
    as an argument. The button role is used to automatically determine the
    position of the button within the dialog box.

    The text(), icon() and iconPixmap() functions provide access to the
    current text and pixmap of the message box. The setText(), setIcon()
    and setIconPixmap() let you change it. The difference between
    setIcon() and setIconPixmap() is that the former accepts a
    QMessageBox::Icon and can be used to set standard icons, whereas the
    latter accepts a QPixmap and can be used to set custom icons.

    setButtonText() and buttonText() provide access to the buttons.

    QMessageBox has no signals or slots.

    The \l{dialogs/standarddialogs}{Standard Dialogs} example shows
    how to use QMessageBox as well as other built-in Qt dialogs.

    \sa QDialogButtonBox, QMessageBox, {fowler}{GUI Design Handbook: Message Box}, {Standard Dialogs Example}, {Application Example}
*/


/*!
    Constructs a message box with no text and no buttons.

    If \a parent is 0, the message box becomes an application-global
    modal dialog box. If \a parent is a widget, the message box
    becomes modal relative to \a parent.

    The \a parent argument is passed to the QDialog constructor.
*/
QMessageBox::QMessageBox(QWidget *parent)
: QDialog(*new QMessageBoxPrivate, parent, Qt::MSWindowsFixedSizeDialogHint | Qt::WindowTitleHint | Qt::WindowSystemMenuHint)
{
    Q_D(QMessageBox);
    d->init();
}

/*!
    Constructs a message box with the given \a icon, \a title, \a text,
    and standard \a buttons. (Buttons can also be added at any time
    using addButton().)

    If \a parent is 0, the message box becomes an application-global
    modal dialog box. If \a parent is a widget, the message box
    becomes modal relative to \a parent.

    The \a parent and \a f arguments are passed to the QDialog constructor.

    \sa setWindowTitle(), setText(), setIcon(), setStandardButtons()
*/
QMessageBox::QMessageBox(Icon icon, const QString &title, const QString &text,
                             StandardButtons buttons, QWidget *parent, Qt::WindowFlags f)
: QDialog(*new QMessageBoxPrivate, parent, f | Qt::MSWindowsFixedSizeDialogHint | Qt::WindowTitleHint | Qt::WindowSystemMenuHint)
{
    Q_D(QMessageBox);
    d->init(title, text);
    setIcon(icon);
    setStandardButtons(buttons);
}

/*!
    Destroys the message box.
*/
QMessageBox::~QMessageBox()
{
}

/*!
    \enum QMessageBox::ButtonRole

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
    \enum QMessageBox::StandardButton

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
    \omitvalue NoButton
*/

/*!
    Creates a button with the given \a text, adds it to the message box for the
    specified \a role, and returns it.

    \sa removeButton(), button(), setStandardButtons()
*/
void QMessageBox::addButton(QAbstractButton *button, ButtonRole role)
{
    Q_D(QMessageBox);
    if (!button)
        return;
    removeButton(button);
    d->buttonBox->addButton(button, (QDialogButtonBox::ButtonRole)role);
    d->buttonList.append(button);
}

/*!
    \overload

    Creates a button with the given \a text, adds it to the message box for the
    specified \a role, and returns it.
*/
QPushButton *QMessageBox::addButton(const QString& text, ButtonRole role)
{
    QPushButton *pushButton = new QPushButton(text);
    addButton(pushButton, role);
    return pushButton;
}

/*!
    \overload

    Adds a standard \a button to the message box if it is valid to do so, and
    returns the push button.
*/
QPushButton *QMessageBox::addButton(StandardButton button)
{
    Q_D(QMessageBox);
    QPushButton *pushButton = d->buttonBox->addButton((QDialogButtonBox::StandardButton)button);
    if (pushButton)
        d->buttonList.append(pushButton);
    return pushButton;
}

/*!
    Removes \a button from the button box without deleting it.

    \sa addButton(), setStandardButtons()
*/
void QMessageBox::removeButton(QAbstractButton *button)
{
    Q_D(QMessageBox);
    d->buttonList.removeAll(button);
    if (d->escapeButton == button)
        d->escapeButton = 0;
    if (d->defaultButton == button)
        d->defaultButton = 0;
    d->buttonBox->removeButton(button);
}

/*!
    \property QMessageBox::standardButtons
    \brief collection of standard buttons in the message box

    This property controls which standard buttons are used by the message box.

    \sa addButton()
*/
void QMessageBox::setStandardButtons(StandardButtons buttons)
{
    Q_D(QMessageBox);
    d->buttonBox->setStandardButtons(QDialogButtonBox::StandardButtons(int(buttons)));
    d->buttonList = d->buttonBox->buttons();

    if (!d->buttonList.contains(d->escapeButton))
        d->escapeButton = 0;
    if (!d->buttonList.contains(d->defaultButton))
        d->defaultButton = 0;
}

QMessageBox::StandardButtons QMessageBox::standardButtons() const
{
    Q_D(const QMessageBox);
    return QMessageBox::StandardButtons(int(d->buttonBox->standardButtons()));
}

/*!
    Returns the standard button enum value corresponding to the given \a button,
    or NoButton if the given \a button isn't a standard button.

    \sa button(), buttons(), standardButtons()
*/
QMessageBox::StandardButton QMessageBox::standardButton(QAbstractButton *button) const
{
    Q_D(const QMessageBox);
    return (QMessageBox::StandardButton)d->buttonBox->standardButton(button);
}

/*!
    Returns a pointer corresponding to the standard button \a which,
    or 0 if the standard button doesn't exist in this message box.

    \sa standardButtons, buttons()
*/
QAbstractButton *QMessageBox::button(StandardButton which) const
{
    Q_D(const QMessageBox);
    return d->buttonBox->button(QDialogButtonBox::StandardButton(which));
}

/*!
    Returns the button that is activated when escape is presed.  Returns 0
    if no escape button was set.

    \sa addButton()
*/
QAbstractButton *QMessageBox::escapeButton() const
{
    Q_D(const QMessageBox);
    return d->escapeButton;
}

/*!
    Sets the button that gets activated when the \key Escape key is
    pressed to \a button.

    \sa addButton(), clickedButton()
*/
void QMessageBox::setEscapeButton(QAbstractButton *button)
{
    Q_D(QMessageBox);
    if (d->buttonList.contains(button))
        d->escapeButton = button;
}

/*!
    Returns the button that was clicked by the user,
    or 0 if the user hit the \key Escape key and
    no \l{setEscapeButton()}{escape button} was set.

    If exec() hasn't been called yet, returns 0.

    Example:

    \code
        QMessageBox messageBox(this);
        ...
        messageBox.exec();
        if (messageBox.clickedButton() == okButton) {
            ...
        }
    \endcode

    \sa standardButton(), button()
*/
QAbstractButton *QMessageBox::clickedButton() const
{
    Q_D(const QMessageBox);
    return d->clickedButton;
}

/*!
    Returns the button that should be the message box's
    \l{QPushButton::setDefaultButton()}{default button}. Returns 0
    if no default button was set.

    \sa addButton(), setDefaultButton()
*/
QPushButton *QMessageBox::defaultButton() const
{
    Q_D(const QMessageBox);
    return d->defaultButton;
}

/*!
    Sets the message box's \l{QPushButton::setDefaultButton()}{default button}
    to \a button.

    \sa addButton(), defaultButton()
*/
void QMessageBox::setDefaultButton(QPushButton *button)
{
    Q_D(QMessageBox);
    if (!d->buttonList.contains(button))
        return;
    d->defaultButton = button;
    button->setDefault(true);
    button->setFocus();
}

/*!
    \property QMessageBox::text
    \brief the message box text to be displayed.

    The text will be interpreted either as a plain text or as rich
    text, depending on the text format setting (\l
    QMessageBox::textFormat). The default setting is Qt::AutoText, i.e.
    the message box will try to auto-detect the format of the text.

    The default value of this property is an empty string.

    \sa textFormat
*/
QString QMessageBox::text() const
{
    Q_D(const QMessageBox);
    return d->label->text();
}

void QMessageBox::setText(const QString &text)
{
    Q_D(QMessageBox);
    d->label->setText(text);
    bool wordwrap = d->label->textFormat() == Qt::RichText
                    || (d->label->textFormat() == Qt::AutoText && Qt::mightBeRichText(text));
    d->label->setWordWrap(wordwrap);
}

/*!
    \enum QMessageBox::Icon

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
    \property QMessageBox::icon
    \brief the message box's icon

    The icon of the message box can be one of the following predefined
    icons:
    \list
    \i QMessageBox::NoIcon
    \i QMessageBox::Question
    \i QMessageBox::Information
    \i QMessageBox::Warning
    \i QMessageBox::Critical
    \endlist

    The actual pixmap used for displaying the icon depends on the
    current \link QWidget::style() GUI style\endlink. You can also set
    a custom pixmap icon using the \l QMessageBox::iconPixmap
    property. The default icon is QMessageBox::NoIcon.

    \sa iconPixmap
*/
QMessageBox::Icon QMessageBox::icon() const
{
    Q_D(const QMessageBox);
    return d->icon;
}

void QMessageBox::setIcon(Icon icon)
{
    Q_D(QMessageBox);
    setIconPixmap(QMessageBox::standardIcon((QMessageBox::Icon)icon));
    d->icon = icon;
}

/*!
    \property QMessageBox::iconPixmap
    \brief the current icon

    The icon currently used by the message box. Note that it's often
    hard to draw one pixmap that looks appropriate in all GUI styles;
    you may want to supply a different pixmap for each platform.

    \sa icon
*/
QPixmap QMessageBox::iconPixmap() const
{
    Q_D(const QMessageBox);
    return *d->iconLabel->pixmap();
}

void QMessageBox::setIconPixmap(const QPixmap &pixmap)
{
    Q_D(QMessageBox);
    d->iconLabel->setPixmap(pixmap);
    d->iconLabel->setFixedSize(d->iconLabel->sizeHint());
    d->icon = NoIcon;
}

/*!
    \property QMessageBox::textFormat
    \brief the format of the text displayed by the message box

    The current text format used by the message box. See the \l
    Qt::TextFormat enum for an explanation of the possible options.

    The default format is Qt::AutoText.

    \sa setText()
*/
Qt::TextFormat QMessageBox::textFormat() const
{
    Q_D(const QMessageBox);
    return d->label->textFormat();
}

void QMessageBox::setTextFormat(Qt::TextFormat format)
{
    Q_D(QMessageBox);
    d->label->setTextFormat(format);
    bool wordwrap = format == Qt::RichText
                || (format == Qt::AutoText && Qt::mightBeRichText(d->label->text()));
    d->label->setWordWrap(wordwrap);
}

/*!\reimp
*/
void QMessageBox::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);
}

/*!\reimp
*/
void QMessageBox::closeEvent(QCloseEvent *e)
{
    Q_D(QMessageBox);
    QDialog::closeEvent(e);
    d->clickedButton = d->escapeButton;
    setResult(d->buttonList.indexOf(d->escapeButton));
}

/*!\reimp
*/
void QMessageBox::changeEvent(QEvent *ev)
{
    Q_D(QMessageBox);
    if (ev->type() == QEvent::StyleChange) {
        if (d->icon != NoIcon)
            setIcon(d->icon);
    }
    QDialog::changeEvent(ev);
}

/*!\reimp
*/
void QMessageBox::keyPressEvent(QKeyEvent *e)
{
    Q_D(QMessageBox);
    if (e->key() == Qt::Key_Escape
#ifdef Q_WS_MAC
        || (e->modifiers() == Qt::ControlModifier && e->key() == Qt::Key_Period)
#endif
        ) {
        if (d->escapeButton)
            d->escapeButton->animateClick();
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
                QAbstractButton *pb = d->buttonList.at(i);
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
void QMessageBox::showEvent(QShowEvent *e)
{
    Q_D(QMessageBox);
    if (d->buttonList.isEmpty())
        addButton(Ok);
#ifndef QT_NO_ACCESSIBILITY
    QAccessible::updateAccessibility(this, 0, QAccessible::Alert);
#endif
    QDialog::showEvent(e);
}

static QMessageBox::StandardButton showMessageBoxEx(QWidget *parent,
    QMessageBox::Icon icon,
    const QString& title, const QString& text,
    QMessageBox::StandardButtons buttons,
    QMessageBox::StandardButton defaultButton)
{
    // necessary for source compatibility with Qt 4.0 and 4.1
    // handles (Yes, No) and (Yes|Default, No)
    if (defaultButton && !(buttons & defaultButton))
        return (QMessageBox::StandardButton)
                    QMessageBoxPrivate::showOldMessageBox(parent, icon, title,
                                                            text, int(buttons),
                                                            int(defaultButton), 0);

    QMessageBox msgBox(icon, title, text, QMessageBox::NoButton, parent);
    QDialogButtonBox *buttonBox = qFindChild<QDialogButtonBox*>(&msgBox);
    Q_ASSERT(buttonBox != 0);

    uint mask = QMessageBox::FirstButton;
    while (mask <= QMessageBox::LastButton) {
        uint sb = buttons & mask;
        mask <<= 1;
        if (!sb)
            continue;
        QPushButton *button = msgBox.addButton((QMessageBox::StandardButton)sb);
        // Choose the first accept role as the default
        if (msgBox.defaultButton())
            continue;
        if ((defaultButton == QMessageBox::NoButton && buttonBox->buttonRole(button) == QDialogButtonBox::AcceptRole)
            || (defaultButton != QMessageBox::NoButton && sb == defaultButton))
            msgBox.setDefaultButton(button);
    }
    if (msgBox.exec() == -1)
        return QMessageBox::Cancel;
    return msgBox.standardButton(msgBox.clickedButton());
}

/*!
    Opens an information message box with the title \a title and
    the text \a text. The standard buttons \a buttons is added to the
    message box. \a defaultButton specifies the button be used as the
    defaultButton. If the \a defaultButton is set to QMessageBox::NoButton,
    QMessageBox picks a suitable default automatically.

    Returns the identity of the standard button that was activated. If escape
    was pressed, returns QMessageBox::Cancel.

    If \a parent is 0, the message box becomes an application-global
    modal dialog box. If \a parent is a widget, the message box
    becomes modal relative to \a parent.

    \sa question(), warning(), critical()
*/
QMessageBox::StandardButton QMessageBox::information(QWidget *parent, const QString &title,
                               const QString& text, StandardButtons buttons,
                               StandardButton defaultButton)
{
    return showMessageBoxEx(parent, Information, title, text, buttons,
                            defaultButton);
}


/*!
    Opens a question message box with the title \a title and
    the text \a text. The standard buttons \a buttons is added to the
    message box. \a defaultButton specifies the button be used as the
    defaultButton. If the \a defaultButton is set to QMessageBox::NoButton,
    QMessageBox picks a suitable default automatically.

    Returns the identity of the standard button that was activated. If escape
    was pressed, returns QMessageBox::Cancel.

    If \a parent is 0, the message box becomes an application-global
    modal dialog box. If \a parent is a widget, the message box
    becomes modal relative to \a parent.

    \sa information(), warning(), critical()
*/
QMessageBox::StandardButton QMessageBox::question(QWidget *parent, const QString &title,
                            const QString& text, StandardButtons buttons,
                            StandardButton defaultButton)
{
    return showMessageBoxEx(parent, Question, title, text, buttons, defaultButton);
}

/*!
    Opens a warning message box with the title \a title and
    the text \a text. The standard buttons \a buttons is added to the
    message box. \a defaultButton specifies the button be used as the
    defaultButton.  If the \a defaultButton is set to QMessageBox::NoButton,
    QMessageBox picks a suitable default automatically.

    Returns the identity of the standard button that was activated. If escape
    was pressed, returns QMessageBox::Cancel.

    If \a parent is 0, the message box becomes an application-global
    modal dialog box. If \a parent is a widget, the message box
    becomes modal relative to \a parent.

    \sa question(), information(), critical()
*/
QMessageBox::StandardButton QMessageBox::warning(QWidget *parent, const QString &title,
                        const QString& text, StandardButtons buttons,
                        StandardButton defaultButton)
{
    return showMessageBoxEx(parent, Warning, title, text, buttons, defaultButton);
}

/*!
    Opens a critical message box with the title \a title and
    the text \a text. The standard buttons \a buttons is added to the
    message box. \a defaultButton specifies the button be used as the
    defaultButton. If the \a defaultButton is set to QMessageBox::NoButton,
    QMessageBox picks a suitable default automatically.

    Returns the identity of the standard button that was activated. If escape
    was pressed, returns QMessageBox::Cancel.

    If \a parent is 0, the message box becomes an application-global
    modal dialog box. If \a parent is a widget, the message box
    becomes modal relative to \a parent.

    \sa question(), warning(), information()
*/
QMessageBox::StandardButton QMessageBox::critical(QWidget *parent, const QString &title,
                         const QString& text, StandardButtons buttons,
                         StandardButton defaultButton)
{
    return showMessageBoxEx(parent, Critical, title, text, buttons, defaultButton);
}

/*!
    Displays a simple about box with title \a title and text \a
    text. The about box's parent is \a parent.

    about() looks for a suitable icon in four locations:
    \list 1
    \i It prefers \link QWidget::windowIcon() parent->icon() \endlink
    if that exists.
    \i If not, it tries the top-level widget containing \a parent.
    \i If that fails, it tries the \link
    QApplication::activeWindow() active window. \endlink
    \i As a last resort it uses the Information icon.
    \endlist

    The about box has a single button labelled "OK".

    \sa QWidget::windowIcon() QApplication::activeWindow()
*/
void QMessageBox::about(QWidget *parent, const QString &title,
                          const QString &text)
{
    QMessageBox mb(title, text, Information, Ok + Default, 0, 0, parent);
    QIcon icon = mb.windowIcon();
    QSize size = icon.actualSize(QSize(64, 64));
    mb.setIconPixmap(icon.pixmap(size));
    mb.exec();
}

/*!
    Displays a simple message box about Qt, with caption \a caption
    and centered over \a parent (if \a parent is not 0). The message
    includes the version number of Qt being used by the application.

    This is useful for inclusion in the Help menu of an application.
    See the examples/menu/menu.cpp example.

    QApplication provides this functionality as a slot.

    \sa QApplication::aboutQt()
*/
void QMessageBox::aboutQt(QWidget *parent, const QString &title)
{
    QMessageBox mb(parent);

    QString c = title;
    if (c.isEmpty())
        c = tr("About Qt");
    mb.setWindowTitle(c);
    mb.setText(*translatedTextAboutQt);
#ifndef QT_NO_IMAGEFORMAT_XPM
    QImage logo(qtlogo_xpm);
#else
    QImage logo;
#endif

    if (qGray(mb.palette().color(QPalette::Active, QPalette::Text).rgb()) >
        qGray(mb.palette().color(QPalette::Active, QPalette::Base).rgb()))
    {
        // light on dark, adjust some colors
        logo.setColor(0, 0xffffffff);
        logo.setColor(1, 0xff666666);
        logo.setColor(2, 0xffcccc66);
        logo.setColor(4, 0xffcccccc);
        logo.setColor(6, 0xffffff66);
        logo.setColor(7, 0xff999999);
        logo.setColor(8, 0xff3333ff);
        logo.setColor(9, 0xffffff33);
        logo.setColor(11, 0xffcccc99);
    }
    QPixmap pm = QPixmap::fromImage(logo);
    if (!pm.isNull())
        mb.setIconPixmap(pm);
    mb.addButton(QMessageBox::Ok);
    mb.exec();
}

// for binary compatibility
QSize QMessageBox::sizeHint() const
{
    return QDialog::sizeHint();
}

/////////////////////////////////////////////////////////////////////////////////////////
// Source and binary compatibility routines for 4.0 and 4.1

static QMessageBox::StandardButton newButton(int button)
{
    // this is needed for source compatibility with Qt 4.0 and 4.1
    if (button == QMessageBox::NoButton || (button & NewButtonFlag))
        return QMessageBox::StandardButton(button & QMessageBox::ButtonMask);

#if QT_VERSION < 0x050000
    // this is needed for binary compatibility with Qt 4.0 and 4.1
    switch (button & Old_ButtonMask) {
    case Old_Ok:
        return QMessageBox::Ok;
    case Old_Cancel:
        return QMessageBox::Cancel;
    case Old_Yes:
        return QMessageBox::Yes;
    case Old_No:
        return QMessageBox::No;
    case Old_Abort:
        return QMessageBox::Abort;
    case Old_Retry:
        return QMessageBox::Retry;
    case Old_Ignore:
        return QMessageBox::Ignore;
    case Old_YesAll:
        return QMessageBox::YesToAll;
    case Old_NoAll:
        return QMessageBox::NoToAll;
    default:
        return QMessageBox::NoButton;
    }
#endif
}

static inline int cleanButton(int button)
{
    return button & QMessageBox::ButtonMask;
}

QAbstractButton *QMessageBoxPrivate::findButton(int button0, int button1, int button2, int flags)
{
    int index = -1;

    if (button0 & flags) {
        index = 0;
    } else if (button1 & flags) {
        index = 1;
    } else if (button2 & flags) {
        index = 2;
    }
    return buttonList.value(index);
}

void QMessageBoxPrivate::addOldButtons(int button0, int button1, int button2)
{
    Q_Q(QMessageBox);
    q->addButton(newButton(button0));
    q->addButton(newButton(button1));
    q->addButton(newButton(button2));
    q->setDefaultButton(
        static_cast<QPushButton *>(findButton(button0, button1, button2, QMessageBox::Default)));
    q->setEscapeButton(findButton(button0, button1, button2, QMessageBox::Escape));
}

int QMessageBoxPrivate::showOldMessageBox(QWidget *parent, QMessageBox::Icon icon,
                                            const QString &title, const QString &text,
                                            int button0, int button1, int button2)
{
    QMessageBox messageBox(icon, title, text, QMessageBox::NoButton, parent);
    messageBox.d_func()->addOldButtons(button0, button1, button2);
    int result = messageBox.exec();

    switch (result) {
    case 0:
        return cleanButton(button0);
    case 1:
        return cleanButton(button1);
    case 2:
        return cleanButton(button2);
    default:
        return -1;
    }
}

int QMessageBoxPrivate::showOldMessageBox(QWidget *parent, QMessageBox::Icon icon,
                                            const QString &title, const QString &text,
                                            const QString &button0Text,
                                            const QString &button1Text,
                                            const QString &button2Text,
                                            int defaultButtonNumber,
                                            int escapeButtonNumber)
{
    QMessageBox messageBox(icon, title, text, QMessageBox::NoButton, parent);
    QString myButton0Text = button0Text;
    if (myButton0Text.isEmpty())
        myButton0Text = QDialogButtonBox::tr("OK");
    messageBox.addButton(myButton0Text, QMessageBox::ActionRole);
    if (!button1Text.isEmpty())
        messageBox.addButton(button1Text, QMessageBox::ActionRole);
    if (!button2Text.isEmpty())
        messageBox.addButton(button2Text, QMessageBox::ActionRole);

    const QList<QAbstractButton *> &buttonList = messageBox.d_func()->buttonList;
    messageBox.setDefaultButton(static_cast<QPushButton *>(buttonList.value(defaultButtonNumber)));
    messageBox.setEscapeButton(buttonList.value(escapeButtonNumber));

    return messageBox.exec();
}

/*!
    \obsolete
    Constructs a message box with a \a caption, a \a text, an \a icon,
    and up to three buttons.

    The \a icon must be one of the following:
    \list
    \i QMessageBox::NoIcon
    \i QMessageBox::Question
    \i QMessageBox::Information
    \i QMessageBox::Warning
    \i QMessageBox::Critical
    \endlist

    Each button, \a button0, \a button1 and \a button2, can have one
    of the following values:
    \list
    \i QMessageBox::NoButton
    \i QMessageBox::Ok
    \i QMessageBox::Cancel
    \i QMessageBox::Yes
    \i QMessageBox::No
    \i QMessageBox::Abort
    \i QMessageBox::Retry
    \i QMessageBox::Ignore
    \i QMessageBox::YesAll
    \i QMessageBox::NoAll
    \endlist

    Use QMessageBox::NoButton for the later parameters to have fewer
    than three buttons in your message box. If you don't specify any
    buttons at all, QMessageBox will provide an Ok button.

    One of the buttons can be OR-ed with the QMessageBox::Default
    flag to make it the default button (clicked when Enter is
    pressed).

    One of the buttons can be OR-ed with the QMessageBox::Escape
    flag to make it the cancel or close button (clicked when Escape is
    pressed).

    \quotefromfile snippets/dialogs/dialogs.cpp
    \skipto // hardware failure
    \skipto QMessageBox mb("Application Name",
    \printuntil // try again

    If \a parent is 0, the message box becomes an application-global
    modal dialog box. If \a parent is a widget, the message box
    becomes modal relative to \a parent.

    The \a parent and \a f arguments are passed to
    the QDialog constructor.

    \sa setWindowTitle(), setText(), setIcon()
*/
QMessageBox::QMessageBox(const QString &title, const QString &text, Icon icon,
                             int button0, int button1, int button2, QWidget *parent,
                             Qt::WindowFlags f)
    : QDialog(*new QMessageBoxPrivate, parent,
              f | Qt::MSWindowsFixedSizeDialogHint | Qt::WindowTitleHint | Qt::WindowSystemMenuHint)
{
    Q_D(QMessageBox);
    d->init(title, text);
    setIcon(icon);
    d->addOldButtons(button0, button1, button2);
}

/*!
    \obsolete
    Opens an information message box with the caption \a caption and
    the text \a text. The dialog may have up to three buttons. Each of
    the buttons, \a button0, \a button1 and \a button2 may be set to
    one of the following values:

    \list
    \i QMessageBox::NoButton
    \i QMessageBox::Ok
    \i QMessageBox::Cancel
    \i QMessageBox::Yes
    \i QMessageBox::No
    \i QMessageBox::Abort
    \i QMessageBox::Retry
    \i QMessageBox::Ignore
    \i QMessageBox::YesAll
    \i QMessageBox::NoAll
    \endlist

    If you don't want all three buttons, set the last button, or last
    two buttons to QMessageBox::NoButton.

    One button can be OR-ed with QMessageBox::Default, and one
    button can be OR-ed with QMessageBox::Escape.

    Returns the identity (QMessageBox::Ok, or QMessageBox::No, etc.)
    of the button that was clicked.

    If \a parent is 0, the message box becomes an application-global
    modal dialog box. If \a parent is a widget, the message box
    becomes modal relative to \a parent.

    \sa question(), warning(), critical()
*/
int QMessageBox::information(QWidget *parent, const QString &title, const QString& text,
                               int button0, int button1, int button2)
{
    return QMessageBoxPrivate::showOldMessageBox(parent, Information, title, text,
                                                   button0, button1, button2);
}

/*!
    \internal

    For source compatibility for the following cases
    1. Yes|Default, No|Escape
    2. Yes, No|Escape
*/
int QMessageBox::information(QWidget *parent, const QString &title, const QString& text,
                               StandardButtons button0, int button1, int button2)
{
    return QMessageBoxPrivate::showOldMessageBox(parent, Information, title, text,
                                                   button0, button1, button2);
}

/*!
    \obsolete
    \overload

    Displays an information message box with caption \a caption, text
    \a text and one, two or three buttons. Returns the index of the
    button that was clicked (0, 1 or 2).

    \a button0Text is the text of the first button, and is optional.
    If \a button0Text is not supplied, "OK" (translated) will be used.
    \a button1Text is the text of the second button, and is optional.
    \a button2Text is the text of the third button, and is optional.
    \a defaultButtonNumber (0, 1 or 2) is the index of the default
    button; pressing Return or Enter is the same as clicking the
    default button. It defaults to 0 (the first button). \a
    escapeButtonNumber is the index of the Escape button; pressing
    Escape is the same as clicking this button. It defaults to -1;
    supply 0, 1 or 2 to make pressing Escape equivalent to clicking
    the relevant button.

    If \a parent is 0, the message box becomes an application-global
    modal dialog box. If \a parent is a widget, the message box
    becomes modal relative to \a parent.

    Note: If you do not specify an Escape button then if the Escape
    button is pressed then -1 will be returned.  It is suggested that
    you specify an Escape button to prevent this from happening.

    \sa question(), warning(), critical()
*/

int QMessageBox::information(QWidget *parent, const QString &title, const QString& text,
                               const QString& button0Text, const QString& button1Text,
                               const QString& button2Text, int defaultButtonNumber,
                               int escapeButtonNumber)
{
    return QMessageBoxPrivate::showOldMessageBox(parent, Information, title, text,
                                                   button0Text, button1Text, button2Text,
                                                   defaultButtonNumber, escapeButtonNumber);
}

/*!
    \obsolete
    Opens a question message box with the caption \a caption and the
    text \a text. The dialog may have up to three buttons. Each of the
    buttons, \a button0, \a button1 and \a button2 may be set to one
    of the following values:

    \list
    \i QMessageBox::NoButton
    \i QMessageBox::Ok
    \i QMessageBox::Cancel
    \i QMessageBox::Yes
    \i QMessageBox::No
    \i QMessageBox::Abort
    \i QMessageBox::Retry
    \i QMessageBox::Ignore
    \i QMessageBox::YesAll
    \i QMessageBox::NoAll
    \endlist

    If you don't want all three buttons, set the last button, or last
    two buttons to QMessageBox::NoButton.

    One button can be OR-ed with QMessageBox::Default, and one
    button can be OR-ed with QMessageBox::Escape.

    Returns the identity (QMessageBox::Yes, or QMessageBox::No, etc.)
    of the button that was clicked.

    If \a parent is 0, the message box becomes an application-global
    modal dialog box. If \a parent is a widget, the message box
    becomes modal relative to \a parent.

    \sa information(), warning(), critical()
*/
int QMessageBox::question(QWidget *parent, const QString &title, const QString& text,
                            int button0, int button1, int button2)
{
    return QMessageBoxPrivate::showOldMessageBox(parent, Question, title, text,
                                                   button0, button1, button2);
}

/*!
    \internal
*/
int QMessageBox::question(QWidget *parent, const QString &title, const QString& text,
                               StandardButtons button0, int button1, int button2)
{
    return QMessageBoxPrivate::showOldMessageBox(parent, Question, title, text,
                                                   button0, button1, button2);
}

/*!
    \obsolete
    \overload

    Displays a question message box with caption \a caption, text \a
    text and one, two or three buttons. Returns the index of the
    button that was clicked (0, 1 or 2).

    \a button0Text is the text of the first button, and is optional.
    If \a button0Text is not supplied, "OK" (translated) will be used.
    \a button1Text is the text of the second button, and is optional.
    \a button2Text is the text of the third button, and is optional.
    \a defaultButtonNumber (0, 1 or 2) is the index of the default
    button; pressing Return or Enter is the same as clicking the
    default button. It defaults to 0 (the first button). \a
    escapeButtonNumber is the index of the Escape button; pressing
    Escape is the same as clicking this button. It defaults to -1;
    supply 0, 1 or 2 to make pressing Escape equivalent to clicking
    the relevant button.

    If \a parent is 0, the message box becomes an application-global
    modal dialog box. If \a parent is a widget, the message box
    becomes modal relative to \a parent.

    Note: If you do not specify an Escape button then if the Escape
    button is pressed then -1 will be returned.  It is suggested that
    you specify an Escape button to prevent this from happening.

    \sa information(), warning(), critical()
*/
int QMessageBox::question(QWidget *parent, const QString &title, const QString& text,
                            const QString& button0Text, const QString& button1Text,
                            const QString& button2Text, int defaultButtonNumber,
                            int escapeButtonNumber)
{
    return QMessageBoxPrivate::showOldMessageBox(parent, Question, title, text,
                                                   button0Text, button1Text, button2Text,
                                                   defaultButtonNumber, escapeButtonNumber);
}


/*!
    \obsolete
    Opens a warning message box with the caption \a caption and the
    text \a text. The dialog may have up to three buttons. Each of the
    button parameters, \a button0, \a button1 and \a button2 may be
    set to one of the following values:

    \list
    \i QMessageBox::NoButton
    \i QMessageBox::Ok
    \i QMessageBox::Cancel
    \i QMessageBox::Yes
    \i QMessageBox::No
    \i QMessageBox::Abort
    \i QMessageBox::Retry
    \i QMessageBox::Ignore
    \i QMessageBox::YesAll
    \i QMessageBox::NoAll
    \endlist

    If you don't want all three buttons, set the last button, or last
    two buttons to QMessageBox::NoButton.

    One button can be OR-ed with QMessageBox::Default, and one
    button can be OR-ed with QMessageBox::Escape.

    Returns the identity (QMessageBox::Ok, or QMessageBox::No, etc.)
    of the button that was clicked.

    If \a parent is 0, the message box becomes an application-global
    modal dialog box. If \a parent is a widget, the message box
    becomes modal relative to \a parent.

    \sa information(), question(), critical()
*/
int QMessageBox::warning(QWidget *parent, const QString &title, const QString& text,
                           int button0, int button1, int button2)
{
    return QMessageBoxPrivate::showOldMessageBox(parent, Warning, title, text,
                                                   button0, button1, button2);
}

/*!
    \internal
*/
int QMessageBox::warning(QWidget *parent, const QString &title, const QString& text,
                           StandardButtons button0, int button1, int button2)
{
    return QMessageBoxPrivate::showOldMessageBox(parent, Warning, title, text,
                                                   button0, button1, button2);
}

/*!
    \obsolete
    \overload

    Displays a warning message box with a caption, a text, and 1, 2 or
    3 buttons. Returns the number of the button that was clicked (0,
    1, or 2).

    \a button0Text is the text of the first button, and is optional.
    If \a button0Text is not supplied, "OK" (translated) will be used.
    \a button1Text is the text of the second button, and is optional,
    and \a button2Text is the text of the third button, and is
    optional. \a defaultButtonNumber (0, 1 or 2) is the index of the
    default button; pressing Return or Enter is the same as clicking
    the default button. It defaults to 0 (the first button). \a
    escapeButtonNumber is the index of the Escape button; pressing
    Escape is the same as clicking this button. It defaults to -1;
    supply 0, 1, or 2 to make pressing Escape equivalent to clicking
    the relevant button.

    If \a parent is 0, the message box becomes an application-global
    modal dialog box. If \a parent is a widget, the message box
    becomes modal relative to \a parent.

    Note: If you do not specify an Escape button then if the Escape
    button is pressed then -1 will be returned.  It is suggested that
    you specify an Escape button to prevent this from happening.

    \sa information(), question(), critical()
*/
int QMessageBox::warning(QWidget *parent, const QString &title, const QString& text,
                           const QString& button0Text, const QString& button1Text,
                           const QString& button2Text, int defaultButtonNumber,
                           int escapeButtonNumber)
{
    return QMessageBoxPrivate::showOldMessageBox(parent, Warning, title, text,
                                                   button0Text, button1Text, button2Text,
                                                   defaultButtonNumber, escapeButtonNumber);
}

/*!
    \obsolete
    Opens a critical message box with the caption \a caption and the
    text \a text. The dialog may have up to three buttons. Each of the
    button parameters, \a button0, \a button1 and \a button2 may be
    set to one of the following values:

    \list
    \i QMessageBox::NoButton
    \i QMessageBox::Ok
    \i QMessageBox::Cancel
    \i QMessageBox::Yes
    \i QMessageBox::No
    \i QMessageBox::Abort
    \i QMessageBox::Retry
    \i QMessageBox::Ignore
    \i QMessageBox::YesAll
    \i QMessageBox::NoAll
    \endlist

    If you don't want all three buttons, set the last button, or last
    two buttons to QMessageBox::NoButton.

    One button can be OR-ed with QMessageBox::Default, and one
    button can be OR-ed with QMessageBox::Escape.

    Returns the identity (QMessageBox::Ok, or QMessageBox::No, etc.)
    of the button that was clicked.

    If \a parent is 0, the message box becomes an application-global
    modal dialog box. If \a parent is a widget, the message box
    becomes modal relative to \a parent.

    \sa information(), question(), warning()
*/

int QMessageBox::critical(QWidget *parent, const QString &title, const QString& text,
                            int button0, int button1, int button2)
{
    return QMessageBoxPrivate::showOldMessageBox(parent, Critical, title, text,
                                                   button0, button1, button2);
}

/*!
    \internal
*/
int QMessageBox::critical(QWidget *parent, const QString &title, const QString& text,
                           StandardButtons button0, int button1, int button2)
{
    return QMessageBoxPrivate::showOldMessageBox(parent, Critical, title, text,
                                                   button0, button1, button2);
}

/*!
    \obsolete
    \overload

    Displays a critical error message box with a caption, a text, and
    1, 2 or 3 buttons. Returns the number of the button that was
    clicked (0, 1 or 2).

    \a button0Text is the text of the first button, and is optional.
    If \a button0Text is not supplied, "OK" (translated) will be used.
    \a button1Text is the text of the second button, and is optional,
    and \a button2Text is the text of the third button, and is
    optional. \a defaultButtonNumber (0, 1 or 2) is the index of the
    default button; pressing Return or Enter is the same as clicking
    the default button. It defaults to 0 (the first button). \a
    escapeButtonNumber is the index of the Escape button; pressing
    Escape is the same as clicking this button. It defaults to -1;
    supply 0, 1, or 2 to make pressing Escape equivalent to clicking
    the relevant button.

    If \a parent is 0, the message box becomes an application-global
    modal dialog box. If \a parent is a widget, the message box
    becomes modal relative to \a parent.

    \sa information(), question(), warning()
*/
int QMessageBox::critical(QWidget *parent, const QString &title, const QString& text,
                            const QString& button0Text, const QString& button1Text,
                            const QString& button2Text, int defaultButtonNumber,
                            int escapeButtonNumber)
{
    return QMessageBoxPrivate::showOldMessageBox(parent, Critical, title, text,
                                                   button0Text, button1Text, button2Text,
                                                   defaultButtonNumber, escapeButtonNumber);
}

/*!
    \obsolete

    Returns the text of the message box button \a button, or
    an empty string if the message box does not contain the button.

    Use button() and QPushButton::text() instead.
*/
QString QMessageBox::buttonText(int button) const
{
    Q_D(const QMessageBox);
    if (QAbstractButton *abstractButton = d->buttonList.value(button)) {
        return abstractButton->text();
    } else if (button == 0) {
        return QDialogButtonBox::tr("OK");
    }
    return QString();
}

/*!
    \obsolete

    Sets the text of the message box button \a button to \a text.
    Setting the text of a button that is not in the message box is
    silently ignored.

    Use addButton() instead.
*/
void QMessageBox::setButtonText(int button, const QString &text)
{
    Q_D(QMessageBox);
    if (QAbstractButton *abstractButton = d->buttonList.value(button)) {
        abstractButton->setText(text);
    } else if (button == 0) {
        addButton(text, QMessageBox::ActionRole);
    }
}

#ifdef QT3_SUPPORT
/*!
    \compat

    Constructs a message box with the given \a parent, \a name, and
    window flags, \a f.
    The window title is specified by \a title, and the message box
    displays message text and an icon specified by \a text and \a icon.

    The buttons that the user can access to respond to the message are
    defined by \a button0, \a button1, and \a button2.
*/
QMessageBox::QMessageBox(const QString& title,
                          const QString &text, Icon icon,
                          int button0, int button1, int button2,
                          QWidget *parent, const char *name,
                          bool modal, Qt::WindowFlags f)
    : QDialog(*new QMessageBoxPrivate, parent,
              f | Qt::WStyle_Customize | Qt::WStyle_DialogBorder | Qt::WStyle_Title | Qt::WStyle_SysMenu)
{
    Q_D(QMessageBox);
    setObjectName(QString::fromAscii(name));
    d->init(title, text);
    d->addOldButtons(button0, button1, button2);
    setModal(modal);
    setIcon(icon);
}

/*!
    \compat
    Constructs a message box with the given \a parent and \a name.
*/
QMessageBox::QMessageBox(QWidget *parent, const char *name)
    : QDialog(*new QMessageBoxPrivate, parent,
              Qt::WStyle_Customize | Qt::WStyle_DialogBorder | Qt::WStyle_Title | Qt::WStyle_SysMenu)
{
    Q_D(QMessageBox);
    setObjectName(QString::fromAscii(name));
    d->init();
}

/*!
  \obsolete

  Returns the pixmap used for a standard icon. This
  allows the pixmaps to be used in more complex message boxes.
  \a icon specifies the required icon, e.g. QMessageBox::Information,
  QMessageBox::Warning or QMessageBox::Critical.

  \a style is unused.
*/

QPixmap QMessageBox::standardIcon(Icon icon, Qt::GUIStyle style)
{
    Q_UNUSED(style);
    return QMessageBox::standardIcon(icon);
}
#endif

/*!
    \obsolete

    Returns the pixmap used for a standard icon. This allows the
    pixmaps to be used in more complex message boxes. \a icon
    specifies the required icon, e.g. QMessageBox::Question,
    QMessageBox::Information, QMessageBox::Warning or
    QMessageBox::Critical.

    Call QStyle::pixelMetric() with QStyle::SP_MessageBoxInformation etc.
    instead.
*/

QPixmap QMessageBox::standardIcon(Icon icon)
{
    int iconSize = QApplication::style()->pixelMetric(QStyle::PM_MessageBoxIconSize);
    QIcon tmpIcon;
    switch (icon) {
    case Information:
        tmpIcon = QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation);
        break;
    case Warning:
        tmpIcon = QApplication::style()->standardIcon(QStyle::SP_MessageBoxWarning);
        break;
    case Critical:
        tmpIcon = QApplication::style()->standardIcon(QStyle::SP_MessageBoxCritical);
        break;
    case Question:
        tmpIcon = QApplication::style()->standardIcon(QStyle::SP_MessageBoxQuestion);
    default:
        break;
    }
    if (!tmpIcon.isNull())
        return tmpIcon.pixmap(iconSize, iconSize);
    return QPixmap();
}


/*!
    \macro QT_REQUIRE_VERSION(int argc, char **argv, const char *version)
    \relates QMessageBox

    This macro can be used to ensure that the application is run
    against a recent enough version of Qt. This is especially useful
    if your application depends on a specific bug fix introduced in a
    bug-fix release (e.g., 4.0.2).

    The \a argc and \a argv parameters are the \c main() function's
    \c argc and \c argv parameters. The \a version parameter is a
    string literal that specifies which version of Qt the application
    requires (e.g., "4.0.2").

    Example:

    \code
        #include <QApplication>
        #include <QMessageBox>

        int main(int argc, char *argv[])
        {
            QT_REQUIRE_VERSION(argc, argv, "4.0.2")

            QApplication app(argc, argv);
            ...
            return app.exec();
        }
    \endcode
*/

#include "moc_qmessagebox.cpp"
#endif // QT_NO_MESSAGEBOX
