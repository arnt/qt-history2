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

#include "qmessagebox.h"

#ifndef QT_NO_MESSAGEBOX

#include "qbuffer.h"
#include "qimagereader.h"
#include "qevent.h"
#include "qdesktopwidget.h"
#include "qlabel.h"
#include "qpushbutton.h"
#include "qimage.h"
#include "qapplication.h"
#include "qcursor.h"
#include "qicon.h"
#include "qstyle.h"
#include "qtextdocument.h"
#ifndef QT_NO_ACCESSIBILITY
#include "qaccessible.h"
#endif
#if defined QT_NON_COMMERCIAL
#include "qnc_win.h"
#endif

#include "qdialog_p.h"

class QMessageBoxPrivate : public QDialogPrivate
{
    Q_DECLARE_PUBLIC(QMessageBox)
public:
    QMessageBoxPrivate() {}
    void buttonClicked();
    void init(int, int, int);
    int indexOf(int) const;
    QLabel *label;

    int                 numButtons;             // number of buttons
    QMessageBox::Icon   icon;                   // message box icon
    QLabel              *iconLabel;              // label holding any icon
    int                 button[3];              // button types
    int                 defButton;              // default button (index)
    int                 escButton;              // escape button (index)
    QPushButton        *pb[3];                  // buttons
};

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


/*!
    \class QMessageBox
    \brief The QMessageBox class provides a modal dialog with a short message, an icon, and some buttons.
    \ingroup dialogs
    \mainclass

    Message boxes are used to provide informative messages and to ask
    simple questions.

    QMessageBox provides a range of different messages, arranged
    roughly along two axes: severity and complexity.

    Severity is
    \table
    \row
    \i \img qmessagebox-quest.png
    \i Question
    \i For message boxes that ask a question as part of normal
    operation. Some style guides recommend using Information for this
    purpose.
    \row
    \i \img qmessagebox-info.png
    \i Information
    \i For message boxes that are part of normal operation.
    \row
    \i \img qmessagebox-warn.png
    \i Warning
    \i For message boxes that tell the user about unusual errors.
    \row
    \i \img qmessagebox-crit.png
    \i Critical
    \i For message boxes that tell the user about critical errors.
    \endtable

    The message box has a different icon for each of the severity levels.

    Complexity is one button (OK) for simple messages, or two or even
    three buttons for questions.

    There are static functions for the most common cases.

    Examples:

    If a program is unable to find a supporting file, but can do perfectly
    well without it:

    \code
    QMessageBox::information(this, "Application name",
    "Unable to find the user preferences file.\n"
    "The factory default will be used instead.");
    \endcode

    question() is useful for simple yes/no questions:

    \code
    if (QFile::exists(filename) &&
        QMessageBox::question(
            this,
            tr("Overwrite File? -- Application Name"),
            tr("A file called %1 already exists."
                "Do you want to overwrite it?")
                .arg(filename),
            tr("&Yes"), tr("&No"),
            QString(), 0, 1))
        return false;
    \endcode

    warning() can be used to tell the user about unusual errors, or
    errors which can't be easily fixed:

    \code
    switch(QMessageBox::warning(this, "Application name",
        "Could not connect to the <mumble> server.\n"
        "This program can't function correctly "
        "without the server.\n\n",
        "Retry",
        "Quit", 0, 0, 1)) {
    case 0: // The user clicked the Retry again button or pressed Enter
        // try again
        break;
    case 1: // The user clicked the Quit or pressed Escape
        // exit
        break;
    }
    \endcode

    The text part of all message box messages can be either rich text
    or plain text. If you specify a rich text formatted string, it
    will be rendered using the default stylesheet. See
    QStyleSheet::defaultSheet() for details. With certain strings that
    contain XML meta characters, the auto-rich text detection may
    fail, interpreting plain text incorrectly as rich text. In these
    rare cases, use QStyleSheet::convertFromPlainText() to convert
    your plain text string to a visually equivalent rich text string
    or set the text format explicitly with setTextFormat().

    Note that the Microsoft Windows User Interface Guidelines
    recommend using the application name as the window's caption.

    Below are more examples of how to use the static member functions.
    After these examples you will find an overview of the non-static
    member functions.

    Exiting a program is part of its normal operation. If there is
    unsaved data the user probably should be asked if they want to
    save the data. For example:

    \code
    switch(QMessageBox::information(this, "Application name here",
        "The document contains unsaved changes\n"
        "Do you want to save the changes before exiting?",
        "&Save", "&Discard", "Cancel",
        0,      // Enter == button 0
        2)) { // Escape == button 2
    case 0: // Save clicked or Alt+S pressed or Enter pressed.
        // save
        break;
    case 1: // Discard clicked or Alt+D pressed
        // don't save but exit
        break;
    case 2: // Cancel clicked or Escape pressed
        // don't exit
        break;
    }
    \endcode

    The Escape button cancels the entire exit operation, and pressing
    Enter causes the changes to be saved before the exit occurs.

    Disk full errors are unusual and they certainly can be hard to
    correct. This example uses predefined buttons instead of
    hard-coded button texts:

    \code
    switch(QMessageBox::warning(this, "Application name here",
        "Could not save the user preferences,\n"
        "because the disk is full. You can delete\n"
        "some files and press Retry, or you can\n"
        "abort the Save Preferences operation.",
        QMessageBox::Retry | QMessageBox::Default,
        QMessageBox::Abort | QMessageBox::Escape)) {
    case QMessageBox::Retry: // Retry clicked or Enter pressed
        // try again
        break;
    case QMessageBox::Abort: // Abort clicked or Escape pressed
        // abort
        break;
    }
    \endcode

    The critical() function should be reserved for critical errors. In
    this example errorDetails is a QString or const char*, and QString
    is used to concatenate several strings:

    \code
    QMessageBox::critical(0, "Application name here",
        QString("An internal error occurred. Please ") +
        "call technical support at 1234-56789 and report\n"+
        "these numbers:\n\n" + errorDetails +
        "\n\nApplication will now exit.");
    \endcode

    In this example an OK button is displayed.

    QMessageBox provides a very simple About box which displays an
    appropriate icon and the string you provide:

    \code
    QMessageBox::about(this, "About <Application>",
        "<Application> is a <one-paragraph blurb>\n\n"
        "Copyright 1991-2003 Such-and-such. "
        "<License words here.>\n\n"
        "For technical support, call 1234-56789 or see\n"
        "http://www.such-and-such.com/Application/\n");
    \endcode

    See about() for more information.

    If you want your users to know that the application is built using
    Qt (so they know that you use high quality tools) you might like
    to add an "About Qt" menu option under the Help menu to invoke
    aboutQt().

    If none of the standard message boxes is suitable, you can create a
    QMessageBox from scratch and use custom button texts:

    \code
    QMessageBox mb("Application name here",
        "Saving the file will overwrite the original file on the disk.\n"
        "Do you really want to save?",
        QMessageBox::Information,
        QMessageBox::Yes | QMessageBox::Default,
        QMessageBox::No,
        QMessageBox::Cancel | QMessageBox::Escape);
    mb.setButtonText(QMessageBox::Yes, "Save");
    mb.setButtonText(QMessageBox::No, "Discard");
    switch(mb.exec()) {
    case QMessageBox::Yes:
        // save and exit
        break;
    case QMessageBox::No:
        // exit without saving
        break;
    case QMessageBox::Cancel:
        // don't save and don't exit
        break;
    }
    \endcode

    QMessageBox defines two enum types: Icon and an unnamed button type.
    Icon defines the \c Question, \c Information, \c Warning, and \c
    Critical icons for each GUI style. It is used by the constructor
    and by the static member functions question(), information(),
    warning() and critical(). A function called standardIcon() gives
    you access to the various icons.

    The button types are:
    \list
    \i Ok - the default for single-button message boxes
    \i Cancel - note that this is \e not automatically Escape
    \i Yes
    \i No
    \i Abort
    \i Retry
    \i Ignore
    \i YesAll
    \i NoAll
    \endlist

    Button types can be combined with two modifiers by using OR, '|':
    \list
    \i Default - makes pressing Enter equivalent to
    clicking this button. Normally used with Ok, Yes or similar.
    \i Escape - makes pressing Escape equivalent to clicking this button.
    Normally used with Abort, Cancel or similar.
    \endlist

    The text(), icon() and iconPixmap() functions provide access to the
    current text and pixmap of the message box. The setText(), setIcon()
    and setIconPixmap() let you change it. The difference between
    setIcon() and setIconPixmap() is that the former accepts a
    QMessageBox::Icon and can be used to set standard icons, whereas the
    latter accepts a QPixmap and can be used to set custom icons.

    setButtonText() and buttonText() provide access to the buttons.

    QMessageBox has no signals or slots.

    \inlineimage qmsgbox-m.png Screenshot in Motif style
    \inlineimage qmsgbox-w.png Screenshot in Windows style

    \sa QDialog,
        \link http://www.iarchitect.com/errormsg.htm Isys on error messages \endlink,
        \link guibooks.html#fowler GUI Design Handbook: Message Box \endlink
*/


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

static const int LastButton = QMessageBox::NoAll;

/*
  NOTE: The table of button texts correspond to the button enum.
*/

#ifndef Q_OS_TEMP
static const char * const mb_texts[] = {
#else
const char * mb_texts[] = {
#endif
    0,
    QT_TRANSLATE_NOOP("QMessageBox","OK"),
    QT_TRANSLATE_NOOP("QMessageBox","Cancel"),
    QT_TRANSLATE_NOOP("QMessageBox","&Yes"),
    QT_TRANSLATE_NOOP("QMessageBox","&No"),
    QT_TRANSLATE_NOOP("QMessageBox","&Abort"),
    QT_TRANSLATE_NOOP("QMessageBox","&Retry"),
    QT_TRANSLATE_NOOP("QMessageBox","&Ignore"),
    QT_TRANSLATE_NOOP("QMessageBox","Yes to &All"),
    QT_TRANSLATE_NOOP("QMessageBox","N&o to All"),
    0
};

/*!
    Constructs a message box with no text and a button with the label
    "OK".

    If \a parent is 0, the message box becomes an application-global
    modal dialog box. If \a parent is a widget, the message box
    becomes modal relative to \a parent.

    The \a parent argument is passed to the QDialog
    constructor.
*/

QMessageBox::QMessageBox(QWidget *parent)
    : QDialog(*new QMessageBoxPrivate, parent,
              Qt::MSWindowsFixedSizeDialogHint | Qt::WindowTitleHint | Qt::WindowSystemMenuHint)
{
    Q_D(QMessageBox);
    setModal(true);
    d->init(Ok, 0, 0);
}

/*!
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

    One of the buttons can be OR-ed with the \c QMessageBox::Default
    flag to make it the default button (clicked when Enter is
    pressed).

    One of the buttons can be OR-ed with the \c QMessageBox::Escape
    flag to make it the cancel or close button (clicked when Escape is
    pressed).

    Example:
    \code
    QMessageBox mb("Application Name",
        "Hardware failure.\n\nDisk error detected\nDo you want to stop?",
        QMessageBox::Question,
        QMessageBox::Yes | QMessageBox::Default,
        QMessageBox::No | QMessageBox::Escape);
    if (mb.exec() == QMessageBox::No)
        // try again
    \endcode

    If \a parent is 0, the message box becomes an application-global
    modal dialog box. If \a parent is a widget, the message box
    becomes modal relative to \a parent.

    The \a parent and \a f arguments are passed to
    the QDialog constructor.

    \sa setWindowTitle(), setText(), setIcon()
*/

QMessageBox::QMessageBox(const QString& caption,
                          const QString &text, Icon icon,
                          int button0, int button1, int button2,
                          QWidget *parent, Qt::WFlags f)
    : QDialog(*new QMessageBoxPrivate, parent,
              f | Qt::MSWindowsFixedSizeDialogHint | Qt::WindowTitleHint | Qt::WindowSystemMenuHint)
{
    Q_D(QMessageBox);
    d->init(button0, button1, button2);
#ifndef QT_NO_WIDGET_TOPEXTRA
    setWindowTitle(caption);
#endif
    setText(text);
    setIcon(icon);
}



/*!
    Destroys the message box.
*/

QMessageBox::~QMessageBox()
{
}

static QString * translatedTextAboutQt = 0;

void QMessageBoxPrivate::init(int button0, int button1, int button2)
{
    Q_Q(QMessageBox);
    if (!translatedTextAboutQt) {
        translatedTextAboutQt = new QString;

#if defined(QT_NON_COMMERCIAL)
    QT_NC_MSGBOX
#else
        *translatedTextAboutQt = q->tr(
            "<h3>About Qt</h3>"
            "<p>This program uses Qt version %1.</p>"
            "<p>Qt is a C++ toolkit for cross-platform GUI "
            "application development.</p>"
            "<p>Qt provides single-source "
            "portability across MS&nbsp;Windows, Mac&nbsp;OS&nbsp;X, "
            "Linux, and all major commercial Unix variants."
            "<br>Qt is also available for embedded devices.</p>"
            "<p>Qt is a Trolltech product. "
            "See <tt>http://www.trolltech.com/qt/</tt> "
            "for more information.</p>"
           ).arg(QT_VERSION_STR);
#endif

    }
    label = new QLabel(q);
    label->setAlignment(Qt::AlignTop|Qt::AlignLeft);

    if ((button2 && !button1) || (button1 && !button0)) {
        qWarning("QMessageBox: Inconsistent button parameters");
        button0 = button1 = button2 = 0;
    }
    icon = QMessageBox::NoIcon;
    iconLabel = new QLabel(q);
    iconLabel->setPixmap(QPixmap());
    numButtons = 0;
    button[0] = button0;
    button[1] = button1;
    button[2] = button2;
    defButton = -1;
    escButton = -1;
    int i;
    for (i=0; i<3; i++) {
        int b = button[i];
        if ((b & QMessageBox::Default)) {
            if (defButton >= 0) {
                qWarning("QMessageBox: There can be at most one default button");
            } else {
                defButton = i;
            }
        }
        if ((b & QMessageBox::Escape)) {
            if (escButton >= 0) {
                qWarning("QMessageBox: There can be at most one escape button");
            } else {
                escButton = i;
            }
        }
        b &= QMessageBox::ButtonMask;
        if (b == 0) {
            if (i == 0)                       // no buttons, add an Ok button
                b = QMessageBox::Ok;
        } else if (b < 0 || b > LastButton) {
            qWarning("QMessageBox: Invalid button specifier");
            b = QMessageBox::Ok;
        } else {
            if (i > 0 && button[i-1] == 0) {
                qWarning("QMessageBox: Inconsistent button parameters; "
                           "button %d defined but not button %d",
                           i+1, i);
                b = 0;
            }
        }
        button[i] = b;
        if (b)
            numButtons++;
    }
    for (i=0; i<3; i++) {
        if (i >= numButtons) {
            pb[i] = 0;
        } else {
            pb[i] = new QPushButton(q->tr(mb_texts[button[i]]), q);
            if (defButton == i) {
                pb[i]->setDefault(true);
                pb[i]->setFocus();
            }
            pb[i]->setAutoDefault(true);
            pb[i]->setFocusPolicy(Qt::StrongFocus);
            q->connect(pb[i], SIGNAL(clicked()), SLOT(buttonClicked()));
        }
    }
}


int QMessageBoxPrivate::indexOf(int button) const
{
    int index = -1;
    for (int i = 0; i < numButtons; i++) {
        if (this->button[i] == button) {
            index = i;
            break;
        }
    }
    return index;
}


/*!
    \property QMessageBox::text
    \brief the message box text to be displayed.

    The text will be interpreted either as a plain text or as rich
    text, depending on the text format setting (\l
    QMessageBox::textFormat). The default setting is \c Qt::AutoText, i.e.
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
    setIconPixmap(standardIcon(icon));
    d->icon = icon;
}

#ifdef QT3_SUPPORT
/*!
    \compat

    Constructs a message box with the given \a parent, \a name, and
    window flags, \a f.
    The window title is specified by \a caption, and the message box
    displays message text and an icon specified by \a text and \a icon.

    The buttons that the user can access to respond to the message are
    defined by \a button0, \a button1, and \a button2.
*/
QMessageBox::QMessageBox(const QString& caption,
                          const QString &text, Icon icon,
                          int button0, int button1, int button2,
                          QWidget *parent, const char *name,
                          bool modal, Qt::WFlags f)
    : QDialog(*new QMessageBoxPrivate, parent,
              f | Qt::WStyle_Customize | Qt::WStyle_DialogBorder | Qt::WStyle_Title | Qt::WStyle_SysMenu)
{
    Q_D(QMessageBox);
    setObjectName(name);
    setModal(modal);
    d->init(button0, button1, button2);
#ifndef QT_NO_WIDGET_TOPEXTRA
    setWindowTitle(caption);
#endif
    setText(text);
    setIcon(icon);
}

/*!
    Constructs a message box with the given \a parent and \a name.
*/
QMessageBox::QMessageBox(QWidget *parent, const char *name)
    : QDialog(*new QMessageBoxPrivate, parent,
              Qt::WStyle_Customize | Qt::WStyle_DialogBorder | Qt::WStyle_Title | Qt::WStyle_SysMenu)
{
    Q_D(QMessageBox);
    setObjectName(name);
    setModal(true);
    d->init(Ok, 0, 0);
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
    Returns the pixmap used for a standard icon. This allows the
    pixmaps to be used in more complex message boxes. \a icon
    specifies the required icon, e.g. QMessageBox::Question,
    QMessageBox::Information, QMessageBox::Warning or
    QMessageBox::Critical.
*/

QPixmap QMessageBox::standardIcon(Icon icon)
{
    QPixmap pm;
    switch (icon) {
    case Information:
        pm = QApplication::style()->standardPixmap(QStyle::SP_MessageBoxInformation);
        break;
    case Warning:
        pm = QApplication::style()->standardPixmap(QStyle::SP_MessageBoxWarning);
        break;
    case Critical:
        pm = QApplication::style()->standardPixmap(QStyle::SP_MessageBoxCritical);
        break;
    case Question:
        pm = QApplication::style()->standardPixmap(QStyle::SP_MessageBoxQuestion);
    default:
        break;
    }
    return pm;
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
    d->icon = NoIcon;
}


/*!
    Returns the text of the message box button \a button, or
    an empty string if the message box does not contain the button.

    \sa setButtonText()
*/

QString QMessageBox::buttonText(int button) const
{
    Q_D(const QMessageBox);
    int index = d->indexOf(button);
    return index >= 0 && d->pb[index] ? d->pb[index]->text() : QString();
}


/*!
    Sets the text of the message box button \a button to \a text.
    Setting the text of a button that is not in the message box is
    silently ignored.

    \sa buttonText()
*/

void QMessageBox::setButtonText(int button, const QString &text)
{
    Q_D(QMessageBox);
    int index = d->indexOf(button);
    if (index >= 0 && d->pb[index]) {
        d->pb[index]->setText(text);
        QResizeEvent e(size(), size());
        event(&e);
    }
}


/*!
    \internal
    Internal slot to handle button clicks.
*/

void QMessageBoxPrivate::buttonClicked()
{
    Q_Q(QMessageBox);

    int reply = 0;
    const QObject *s = q->sender();
    for (int i = 0; i < numButtons; i++) {
        if (pb[i] == s)
            reply = button[i];
    }
    q->done(reply);
}


/*!\reimp
*/
QSize QMessageBox::sizeHint() const
{
    Q_D(const QMessageBox);
    ensurePolished();
    d->label->adjustSize();
    QSize labelSize(d->label->size());
    QSize maxButtonSizeHint;
    int n  = d->numButtons;
    for (int i = 0; i < n; i++)
        maxButtonSizeHint = maxButtonSizeHint.expandedTo(d->pb[i]->sizeHint());
    int bw = maxButtonSizeHint.width();
    int bh = maxButtonSizeHint.height();
    int border = bh / 2 - style()->pixelMetric(QStyle::PM_ButtonDefaultIndicator);
    if (border <= 0)
        border = 10;
    int btn_spacing = style()->styleHint(QStyle::SH_MessageBox_UseBorderForButtonSpacing)
                      ? border : 7;
#ifndef Q_OS_TEMP
    int buttons = d->numButtons * bw + (n-1) * btn_spacing;
    int h = bh;
#else
    int visibleButtons = 0;
    for (int i = 0; i < d->numButtons; ++i)
        visibleButtons += d->pb[i]->isVisible() ? 1 : 0;
    int buttons = visibleButtons == 0 ? 0 : visibleButtons * bw + (visibleButtons-1) * btn_spacing;
    int h = visibleButtons == 0 ? 0 : bh;
    n = visibleButtons;
#endif
    if (labelSize.height())
        h += labelSize.height() + 3*border;
    else
        h += 2*border;
    int lmargin = 0;
    if (d->iconLabel->pixmap() && d->iconLabel->pixmap()->width())  {
        d->iconLabel->adjustSize();
        lmargin += d->iconLabel->width() + border;
        if (h < d->iconLabel->height() + 3*border + bh && n)
            h = d->iconLabel->height() + 3*border + bh;
    }
    int w = qMax(buttons, labelSize.width() + lmargin) + 2*border;
    QRect screen = QApplication::desktop()->screenGeometry(pos());
    if (w > screen.width())
        w = screen.width();
    return QSize(w,h);
}


/*!\reimp
*/
void QMessageBox::resizeEvent(QResizeEvent *)
{
    Q_D(QMessageBox);
    int i;
    QSize maxButtonSizeHint;
    int n  = d->numButtons;
    for (i = 0; i < n; i++)
        maxButtonSizeHint = maxButtonSizeHint.expandedTo(d->pb[i]->sizeHint());
    int bw = maxButtonSizeHint.width();
    int bh = maxButtonSizeHint.height();
#ifdef Q_OS_TEMP
    int visibleButtons = 0;
    for (i = 0; i < n; ++i)
        visibleButtons += d->pb[i]->isVisible() ? 1 : 0;
    n  = visibleButtons;
    bw = visibleButtons == 0 ? 0 : bw;
    bh = visibleButtons == 0 ? 0 : bh;
#endif
    int border = bh / 2 - style()->pixelMetric(QStyle::PM_ButtonDefaultIndicator);
    if (border <= 0)
        border = 10;
    bool useBorder = style()->styleHint(QStyle::SH_MessageBox_UseBorderForButtonSpacing);
    int btn_spacing = useBorder ? border : 7;
    int lmargin = 0;
    d->iconLabel->adjustSize();
    bool rtl = layoutDirection() == Qt::RightToLeft;
    if (rtl)
        d->iconLabel->move(width() - border - d->iconLabel->width(), border);
    else
        d->iconLabel->move(border, border);
    if (d->iconLabel->pixmap() && d->iconLabel->pixmap()->width())
        lmargin += d->iconLabel->width() + border;
    d->label->setGeometry((rtl ? 0 : lmargin) + border,
                          border,
                          width() - lmargin -2*border,
                          height() - 3*border - bh);
    int extra_space = (width() - bw*n - 2*border - (n-1)*btn_spacing);
    if (n)
        bw = qMin(bw,  (width() - 2 *border) / n);
    if (useBorder) {
        for (i=0; i<n; i++)
            d->pb[rtl ? n - i - 1 : i]->setGeometry(border + i*bw + qMax(0,i*btn_spacing + extra_space*(i+1)/(n+1)),
                                                    height() - border - bh, bw, bh);
    } else {
        for (i=0; i<n; i++)
            d->pb[rtl ? n - i - 1 : i]->setGeometry(border + i*bw + qMax(0,extra_space/2 + i*btn_spacing),
                                                    height() - border - bh, bw, bh);
    }
}


/*!\reimp
*/
void QMessageBox::keyPressEvent(QKeyEvent *e)
{
    Q_D(QMessageBox);
    if (e->key() == Qt::Key_Escape) {
        if (d->escButton >= 0) {
            QPushButton *pb = d->pb[d->escButton];
            pb->animateClick();
            e->accept();
            return;
        }
    }
#ifndef QT_NO_ACCEL
    if (!(e->modifiers() & Qt::AltModifier)) {
        int key = e->key() & ~((int)Qt::MODIFIER_MASK|(int)Qt::UNICODE_ACCEL);
        if (key) {
            QList<QPushButton *> list = qFindChildren<QPushButton *>(this);
            for (int i = 0; i < list.size(); ++i) {
                QPushButton *pb = list.at(i);
                int acc = pb->shortcut() & ~((int)Qt::MODIFIER_MASK|(int)Qt::UNICODE_ACCEL);
                if (acc == key) {
                    emit pb->animateClick();
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
#ifndef QT_NO_ACCESSIBILITY
    QAccessible::updateAccessibility(this, 0, QAccessible::Alert);
#endif
    QDialog::showEvent(e);
}

/*!\reimp
*/
void QMessageBox::closeEvent(QCloseEvent *e)
{
    Q_D(QMessageBox);
    QDialog::closeEvent(e);
    if (d->escButton != -1)
        setResult(d->button[d->escButton]);
}

/*****************************************************************************
  Static QMessageBox functions
 *****************************************************************************/

/*!
    \fn int QMessageBox::message(const QString &caption,
                        const QString& text,
                        const QString& buttonText,
                        QWidget *parent, const char *name)

  \obsolete

  Opens a modal message box with the given \a caption and showing the
  given \a text. The message box has a single button which has the
  given \a buttonText (or tr("OK")). The message box is centred over
  its \a parent and is called \a name.

  Use information(), warning(), question(), or critical() instead.
*/

/*!
    \fn bool QMessageBox::query(const QString &caption,
                       const QString& text,
                       const QString& yesButtonText,
                       const QString& noButtonText,
                       QWidget *parent, const char *name)

  \obsolete

  Queries the user using a modal message box with up to two buttons.
  The message box has the given \a caption (although some window
  managers don't show it), and shows the given \a text. The left
  button has the \a yesButtonText (or tr("OK")), and the right button
  has the \a noButtonText (or isn't shown). The message box is centred
  over its \a parent and is called \a name.

  Use information(), question(), warning(), or critical() instead.
*/

/*!
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

    One button can be OR-ed with \c QMessageBox::Default, and one
    button can be OR-ed with \c QMessageBox::Escape.

    Returns the identity (QMessageBox::Ok, or QMessageBox::No, etc.)
    of the button that was clicked.

    If \a parent is 0, the message box becomes an application-global
    modal dialog box. If \a parent is a widget, the message box
    becomes modal relative to \a parent.

    \sa question(), warning(), critical()
*/

int QMessageBox::information(QWidget *parent, const QString& caption, const QString& text,
                             int button0, int button1, int button2)
{
    QMessageBox mb(caption, text, Information, button0, button1, button2, parent);
    return mb.exec();
}

/*!
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

    One button can be OR-ed with \c QMessageBox::Default, and one
    button can be OR-ed with \c QMessageBox::Escape.

    Returns the identity (QMessageBox::Yes, or QMessageBox::No, etc.)
    of the button that was clicked.

    If \a parent is 0, the message box becomes an application-global
    modal dialog box. If \a parent is a widget, the message box
    becomes modal relative to \a parent.

    \sa information(), warning(), critical()
*/

int QMessageBox::question(QWidget *parent,
                           const QString& caption, const QString& text,
                           int button0, int button1, int button2)
{
    QMessageBox mb(caption, text, Question, button0, button1, button2, parent);
    return mb.exec();
}


/*!
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

    One button can be OR-ed with \c QMessageBox::Default, and one
    button can be OR-ed with \c QMessageBox::Escape.

    Returns the identity (QMessageBox::Ok, or QMessageBox::No, etc.)
    of the button that was clicked.

    If \a parent is 0, the message box becomes an application-global
    modal dialog box. If \a parent is a widget, the message box
    becomes modal relative to \a parent.

    \sa information(), question(), critical()
*/

int QMessageBox::warning(QWidget *parent,
                          const QString& caption, const QString& text,
                          int button0, int button1, int button2)
{
    QMessageBox mb(caption, text, Warning, button0, button1, button2, parent);
    return mb.exec();
}


/*!
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

    One button can be OR-ed with \c QMessageBox::Default, and one
    button can be OR-ed with \c QMessageBox::Escape.

    Returns the identity (QMessageBox::Ok, or QMessageBox::No, etc.)
    of the button that was clicked.

    If \a parent is 0, the message box becomes an application-global
    modal dialog box. If \a parent is a widget, the message box
    becomes modal relative to \a parent.

    \sa information(), question(), warning()
*/

int QMessageBox::critical(QWidget *parent,
                           const QString& caption, const QString& text,
                           int button0, int button1, int button2)
{
    QMessageBox mb(caption, text, Critical, button0, button1, button2, parent);
    return mb.exec();
}


/*!
    Displays a simple about box with caption \a caption and text \a
    text. The about box's parent is \a parent.

    about() looks for a suitable icon in four locations:
    \list 1
    \i It prefers \link QWidget::windowIcon() parent->icon() \endlink
    if that exists.
    \i If not, it tries the top-level widget containing \a parent.
    \i If that fails, it tries the \link
    QApplication::mainWidget() main widget. \endlink
    \i As a last resort it uses the Information icon.
    \endlist

    The about box has a single button labelled "OK".

    \sa QWidget::windowIcon() QApplication::mainWidget()
*/

void QMessageBox::about(QWidget *parent, const QString &caption,
                         const QString& text)
{
    QMessageBox mb(caption, text, Information, Ok + Default, 0, 0, parent);
#ifndef QT_NO_WIDGET_TOPEXTRA
    QIcon icon = mb.windowIcon();
    QSize size = icon.actualSize(QSize(64, 64));
    mb.setIconPixmap(icon.pixmap(size));
#endif
    mb.exec();
}


/*! \reimp
*/
void QMessageBox::changeEvent(QEvent *ev)
{
    Q_D(QMessageBox);
    if(ev->type() == QEvent::StyleChange) {
        if (d->icon != NoIcon) {
            // Reload icon for new style
            setIcon(d->icon);
        }
    }
    QWidget::changeEvent(ev);
}


static int textBox(QWidget *parent, QMessageBox::Icon severity,
                   const QString& caption, const QString& text,
                   const QString& button0Text,
                   const QString& button1Text,
                   const QString& button2Text,
                   int defaultButtonNumber,
                   int escapeButtonNumber)
{
    int b[3];
    b[0] = 1;
    b[1] = button1Text.isEmpty() ? 0 : 2;
    b[2] = button2Text.isEmpty() ? 0 : 3;

    int i;
    for(i=0; i<3; i++) {
        if (b[i] && defaultButtonNumber == i)
            b[i] += QMessageBox::Default;
        if (b[i] && escapeButtonNumber == i)
            b[i] += QMessageBox::Escape;
    }

    QMessageBox mb(caption, text, severity, b[0], b[1], b[2], parent);
    if (button0Text.isEmpty())
        mb.setButtonText(1, QMessageBox::tr(mb_texts[QMessageBox::Ok]));
    else
        mb.setButtonText(1, button0Text);
    if (b[1])
        mb.setButtonText(2, button1Text);
    if (b[2])
        mb.setButtonText(3, button2Text);

#ifndef QT_NO_CURSOR
    mb.setCursor(Qt::ArrowCursor);
#endif
    return mb.exec() - 1;
}


/*!
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

int QMessageBox::information(QWidget *parent, const QString &caption,
                              const QString& text,
                              const QString& button0Text,
                              const QString& button1Text,
                              const QString& button2Text,
                              int defaultButtonNumber,
                              int escapeButtonNumber)
{
    return textBox(parent, Information, caption, text,
                    button0Text, button1Text, button2Text,
                    defaultButtonNumber, escapeButtonNumber);
}

/*!
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
int QMessageBox::question(QWidget *parent, const QString &caption,
                           const QString& text,
                           const QString& button0Text,
                           const QString& button1Text,
                           const QString& button2Text,
                           int defaultButtonNumber,
                           int escapeButtonNumber)
{
    return textBox(parent, Question, caption, text,
                    button0Text, button1Text, button2Text,
                    defaultButtonNumber, escapeButtonNumber);
}


/*!
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

int QMessageBox::warning(QWidget *parent, const QString &caption,
                                 const QString& text,
                                 const QString& button0Text,
                                 const QString& button1Text,
                                 const QString& button2Text,
                                 int defaultButtonNumber,
                                 int escapeButtonNumber)
{
    return textBox(parent, Warning, caption, text,
                    button0Text, button1Text, button2Text,
                    defaultButtonNumber, escapeButtonNumber);
}


/*!
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

int QMessageBox::critical(QWidget *parent, const QString &caption,
                                  const QString& text,
                                  const QString& button0Text,
                                  const QString& button1Text,
                                  const QString& button2Text,
                                  int defaultButtonNumber,
                                  int escapeButtonNumber)
{
    return textBox(parent, Critical, caption, text,
                    button0Text, button1Text, button2Text,
                    defaultButtonNumber, escapeButtonNumber);
}

#ifndef QT_NO_IMAGEIO_XPM
// helper
extern void qt_read_xpm_image_or_array(QImageReader *, const char * const *, QImage &);
#endif

/*!
    Displays a simple message box about Qt, with caption \a caption
    and centered over \a parent (if \a parent is not 0). The message
    includes the version number of Qt being used by the application.

    This is useful for inclusion in the Help menu of an application.
    See the examples/menu/menu.cpp example.

    QApplication provides this functionality as a slot.

    \sa QApplication::aboutQt()
*/

void QMessageBox::aboutQt(QWidget *parent, const QString &caption)
{
    QMessageBox mb(parent);

#ifndef QT_NO_WIDGET_TOPEXTRA
    QString c = caption;
    if (c.isEmpty())
        c = tr("About Qt");
    mb.setWindowTitle(c);
#endif
    mb.setText(*translatedTextAboutQt);
#ifndef QT_NO_IMAGEIO
    QPixmap pm;
    QImage logo(qtlogo_xpm);

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
    if (pm.fromImage(logo))
        mb.setIconPixmap(pm);
#endif
    mb.setButtonText(0, tr("OK"));
    if (mb.d_func()->pb[0]) {
        mb.d_func()->pb[0]->setAutoDefault(true);
        mb.d_func()->pb[0]->setFocusPolicy(Qt::StrongFocus);
        mb.d_func()->pb[0]->setDefault(true);
        mb.d_func()->pb[0]->setFocus();
    }
    mb.exec();
}

/*!
    \property QMessageBox::textFormat
    \brief the format of the text displayed by the message box

    The current text format used by the message box. See the \l
    Qt::TextFormat enum for an explanation of the possible options.

    The default format is \c Qt::AutoText.

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

#define d d_func()
#include "moc_qmessagebox.cpp"
#endif
