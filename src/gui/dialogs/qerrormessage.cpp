/****************************************************************************
**
** Implementation of a nice qInstallMsgHandler() handler.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the dialogs module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qerrormessage.h"

#ifndef QT_NO_ERRORMESSAGE

#include "qapplication.h"
#include "qcheckbox.h"
#include "qlabel.h"
#include "qlayout.h"
#include "qmessagebox.h"
#include "qpushbutton.h"
#include "qstringlist.h"
#include "qstylesheet.h"
#include "qtextview.h"

#include <stdio.h>
#include <stdlib.h>

class QErrorMessageTextView : public QTextView
{
public:
    QErrorMessageTextView(QWidget *parent, const char *name)
        : QTextView(parent, name) { }

    virtual QSize minimumSizeHint() const;
    virtual QSize sizeHint() const;
};

QSize QErrorMessageTextView::minimumSizeHint() const
{
    return QSize(50, 50);
}

QSize QErrorMessageTextView::sizeHint() const
{
    return QSize(250, 75);
}

/*! \class QErrorMessage

  \brief The QErrorMessage class provides an error message display dialog.

  \ingroup dialogs
  \ingroup misc

This is basically a QLabel and a "show this message again" checkbox which
remembers what not to show.

There are two ways to use this class:
\list 1
\i For production applications. In this context the class can be used to
display messages which you don't need the user to see more than once. To use
QErrorMessage like this, you create the dialog in the usual way and call the
message() slot, or connect signals to it.

\i For developers. In this context the static qtHandler() installs
a message handler using qInstallMsgHandler() and creates a QErrorMessage
that displays qDebug(), qWarning() and qFatal() messages.
\endlist

In both cases QErrorMessage will queue pending messages, and display
them (or not) in order, as soon as the user presses Enter or clicks OK
after seeing each message.

\img qerrormessage.png

\sa QMessageBox QStatusBar::message()
*/

static QErrorMessage * qtMessageHandler = 0;

static void deleteStaticcQErrorMessage() // post-routine
{
    if (qtMessageHandler) {
        delete qtMessageHandler;
        qtMessageHandler = 0;
    }
}

static bool metFatal = false;

void jump(QtMsgType t, const char * m)
{
    if (!qtMessageHandler)
        return;

    QString rich;

    switch (t) {
    case QtDebugMsg:
    default:
        rich = QErrorMessage::tr("Debug Message:");
        break;
    case QtWarningMsg:
        rich = QErrorMessage::tr("Warning:");
        break;
    case QtFatalMsg:
        rich = QErrorMessage::tr("Fatal Error:");
    }
    rich = QString("<p><b>%1</b></p>").arg(rich);
    rich += QStyleSheet::convertFromPlainText(m,
                QStyleSheetItem::WhiteSpaceNormal);

    // ### work around text engine quirk
    if (rich.endsWith("</p>"))
        rich.truncate(rich.length() - 4);

    if (!metFatal) {
        qtMessageHandler->message(rich);
        metFatal = (t == QtFatalMsg);
    }
}


/*!  Constructs and installs an error handler window.
    The parent \a parent and name \a name are passed on to the QDialog
    constructor.
*/

QErrorMessage::QErrorMessage(QWidget * parent, const char * name)
    : QDialog(parent, name)
{
    QGridLayout * grid = new QGridLayout(this, 3, 2, 11, 6);
    icon = new QLabel(this, "qt_icon_lbl");
#ifndef QT_NO_MESSAGEBOX
    icon->setPixmap(QMessageBox::standardIcon(QMessageBox::Information));
#endif
    grid->addWidget(icon, 0, 0, AlignTop);
    errors = new QErrorMessageTextView(this, "errors");
    grid->addWidget(errors, 0, 1);
    again = new QCheckBox(tr("&Show this message again"), this, "again");
    again->setChecked(true);
    grid->addWidget(again, 1, 1, AlignTop | AlignAuto);
    ok = new QPushButton(tr("&OK"), this, "ok");
    connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
    ok->setFocus();
    grid->addMultiCellWidget(ok, 2, 2, 0, 1, AlignCenter);
    grid->setColStretch(1, 42);
    grid->setRowStretch(0, 42);
    pending = new QStringList;
}


/*! Destroys the object and frees any allocated resources.  Notably,
the list of "do not show again" messages is deleted. */

QErrorMessage::~QErrorMessage()
{
    if (this == qtMessageHandler) {
        qtMessageHandler = 0;
        QtMsgHandler tmp = qInstallMsgHandler(0);
        // in case someone else has later stuck in another...
        if (tmp != jump)
            qInstallMsgHandler(tmp);
    }

    delete pending;
}


/*! \reimp */

void QErrorMessage::done(int a)
{
    if (!again->isChecked())
        doNotShow.insert(errors->text(), 0);
    if (!nextPending()) {
        QDialog::done(a);
        if (this == qtMessageHandler && metFatal)
            exit(1);
    }
}


/*!  Returns a pointer to a QErrorMessage object that outputs the
default Qt messages.  This function creates such an object, if there
isn't one already.
*/

QErrorMessage * QErrorMessage::qtHandler()
{
    if (!qtMessageHandler) {
        qtMessageHandler = new QErrorMessage(0, "automatic message handler");
        qAddPostRoutine(deleteStaticcQErrorMessage); // clean up
#ifndef QT_NO_WIDGET_TOPEXTRA
        if (qApp->mainWidget())
            qtMessageHandler->setWindowTitle(qApp->mainWidget()->windowTitle());
#endif
        qInstallMsgHandler(jump);
    }
    return qtMessageHandler;
}


/*! \internal */

bool QErrorMessage::nextPending()
{
    while (!pending->isEmpty()) {
        QString p = *pending->begin();
        pending->erase(pending->begin());
        if (!p.isEmpty() && !doNotShow.contains(p)) {
            errors->setText(p);
            return true;
        }
    }
    return false;
}


/*! Shows message \a m and returns immediately.  If the user has requested
  that \a m not be shown, this function does nothing.

  Normally, \a m is shown at once, but if there are pending messages,
  \a m is queued for later display.
*/

void QErrorMessage::message(const QString & m)
{
    if (doNotShow.contains(m))
        return;
    pending->append(m);
    if (!isVisible() && nextPending())
        show();
}

#endif // QT_NO_ERRORMESSAGE
