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

#include "qwizard.h"

#ifndef QT_NO_WIZARD

#include "qevent.h"
#include "qlayout.h"
#include "qpushbutton.h"
#include "qcursor.h"
#include "qlabel.h"
#include "qwidgetstack.h"
#include "qapplication.h"
#include "qlist.h"
#include "qpainter.h"
#include "qaccel.h"

/*! \file wizard/wizard.cpp */
/*! \file wizard/wizard.h */

/*!
    \class QWizard qwizard.h
    \brief The QWizard class provides a framework for wizard dialogs.

    \compat

    A wizard is a special type of input dialog that consists of a
    sequence of dialog pages. A wizard's purpose is to walk the user
    through a process step by step. Wizards are useful for complex or
    infrequently occurring tasks that people may find difficult to
    learn or do.

    QWizard provides page titles and displays Next, Back, Finish,
    Cancel, and Help push buttons, as appropriate to the current
    position in the page sequence. These buttons can be
    enabled/disabled using setBackEnabled(), setNextEnabled(),
    setFinishEnabled() and setHelpEnabled().

    Create and populate dialog pages that inherit from QWidget and add
    them to the wizard using addPage(). Use insertPage() to add a
    dialog page at a certain position in the page sequence. Use
    removePage() to remove a page from the page sequence.

    Use currentPage() to retrieve a pointer to the currently displayed
    page. page() returns a pointer to the page at a certain position
    in the page sequence.

    Use pageCount() to retrieve the total number of pages in the page
    sequence. indexOf() will return the index of a page in the page
    sequence.

    QWizard provides functionality to mark pages as appropriate (or
    not) in the current context with setAppropriate(). The idea is
    that a page may be irrelevant and should be skipped depending on
    the data entered by the user on a preceding page.

    It is generally considered good design to provide a greater number
    of simple pages with fewer choices rather than a smaller number of
    complex pages.

    Example code is available here: \l wizard/wizard.cpp \l wizard/wizard.h

    \img qwizard.png A QWizard page
    \caption A QWizard page

*/


class QWizardPrivate
{
public:
    struct Page {
        Page(QWidget * widget, const QString & title):
            w(widget), t(title),
            backEnabled(true), nextEnabled(true), finishEnabled(false),
            helpEnabled(true),
            appropriate(true)
        {}
        QWidget * w;
        QString t;
        bool backEnabled;
        bool nextEnabled;
        bool finishEnabled;
        bool helpEnabled;
        bool appropriate;
    };

    QVBoxLayout * v;
    Page * current;
    QWidgetStack * ws;
    QList<Page *> pages;
    QLabel * title;
    QPushButton * backButton;
    QPushButton * nextButton;
    QPushButton * finishButton;
    QPushButton * cancelButton;
    QPushButton * helpButton;

    QFrame * hbar1, * hbar2;

#ifndef QT_NO_ACCEL
    QAccel * accel;
    int backAccel;
    int nextAccel;
#endif

    Page * page(const QWidget * w)
    {
        if (!w)
            return 0;
        int i = pages.count();
        while(--i >= 0 && pages.at(i) && pages.at(i)->w != w) { }
        return i >= 0 ? pages.at(i) : 0;
    }

};


/*!
    Constructs an empty wizard dialog. The \a parent, \a name, \a
    modal and \a f arguments are passed to the QDialog constructor.
*/

QWizard::QWizard(QWidget *parent, const char *name, bool modal,
                  Qt::WFlags f)
    : QDialog(parent, name, modal, f)
{
    d = new QWizardPrivate();
    d->current = 0; // not quite true, but...
    d->ws = new QWidgetStack(this, "qt_widgetstack");
    d->title = new QLabel(this, "title label");

    // create in nice tab order
    d->nextButton = new QPushButton(this, "next");
    d->finishButton = new QPushButton(this, "finish");
    d->helpButton = new QPushButton(this, "help");
    d->backButton = new QPushButton(this, "back");
    d->cancelButton = new QPushButton(this, "cancel");

    d->ws->installEventFilter(this);

    d->v = 0;
    d->hbar1 = 0;
    d->hbar2 = 0;

    d->cancelButton->setText(tr("&Cancel"));
    d->backButton->setText(tr("< &Back"));
    d->nextButton->setText(tr("&Next >"));
    d->finishButton->setText(tr("&Finish"));
    d->helpButton->setText(tr("&Help"));

    d->nextButton->setDefault(true);

    connect(d->backButton, SIGNAL(clicked()),
             this, SLOT(back()));
    connect(d->nextButton, SIGNAL(clicked()),
             this, SLOT(next()));
    connect(d->finishButton, SIGNAL(clicked()),
             this, SLOT(accept()));
    connect(d->cancelButton, SIGNAL(clicked()),
             this, SLOT(reject()));
    connect(d->helpButton, SIGNAL(clicked()),
             this, SLOT(help()));

#ifndef QT_NO_ACCEL
    d->accel = new QAccel(this, "arrow-key accel");
    d->backAccel = d->accel->insertItem(Qt::ALT + Qt::Key_Left);
    d->accel->connectItem(d->backAccel, this, SLOT(back()));
    d->nextAccel = d->accel->insertItem(Qt::ALT + Qt::Key_Right);
    d->accel->connectItem(d->nextAccel, this, SLOT(next()));
#endif
}


/*!
    Destroys the object and frees any allocated resources, including
    all pages and controllers.
*/

QWizard::~QWizard()
{
    while (!d->pages.isEmpty())
        delete d->pages.takeFirst();
    delete d;
}


/*!
    \reimp
*/

void QWizard::show()
{
    if (!d->current) {
        // No page yet
        if (pageCount() > 0)
            showPage(d->pages.at(0)->w);
        else
            showPage(0);
    }

    QDialog::show();
}


/*!
    \reimp
*/

void QWizard::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::ApplicationFontChange || event->type() == QEvent::FontChange)
        QApplication::postEvent(this, new QEvent(QEvent::LayoutHint));
    QDialog::changeEvent(event);
}


/*!
    Adds \a page to the end of the page sequence, with the title, \a
    title.
*/

void QWizard::addPage(QWidget * page, const QString & title)
{
    if (!page)
        return;
    if (d->page(page)) {
        qWarning("QWizard::addPage(): already added %s/%s to %s/%s",
                  page->className(), page->objectName().local8Bit(),
                  className(), objectName().local8Bit());
        return;
    }
    int i = d->pages.count();

    if (i > 0)
        d->pages.at(i - 1)->nextEnabled = true;

    QWizardPrivate::Page *p = new QWizardPrivate::Page(page, title);
    p->backEnabled = (i > 0);
    d->ws->addWidget(page, i);
    d->pages.append(p);
}

/*!
    Inserts \a page at position \a index into the page sequence, with
    title \a title. If \a index is -1, the page will be appended to
    the end of the wizard's page sequence.
*/

void QWizard::insertPage(QWidget * page, const QString & title, int index)
{
    if (!page)
        return;
    if (d->page(page)) {
        qWarning("QWizard::insertPage(): already added %s/%s to %s/%s",
                  page->className(), page->objectName().local8Bit(),
                  className(), objectName().local8Bit());
        return;
    }

    if (index < 0 || index > d->pages.count())
        index = d->pages.count();

    if(index == d->pages.count())
        d->pages.at(index - 1)->nextEnabled = true;

    QWizardPrivate::Page * p = new QWizardPrivate::Page(page, title);
    p->backEnabled = (index > 0);
    p->nextEnabled = (index < d->pages.count());

    d->ws->addWidget(page, index);
    d->pages.insert(index, p);
}

/*!
    \fn void QWizard::selected(const QString& title)

    This signal is emitted when the current page changes. The
    parameter contains the \a title of the selected page.
*/


/*!
    Makes \a page the current page and emits the selected() signal.

    This virtual function is called whenever a different page is to
    be shown, including the first time the QWizard is shown.
    By reimplementing it (and calling QWizard::showPage()),
    you can prepare each page prior to it being shown.
*/

void QWizard::showPage(QWidget * page)
{
    QWizardPrivate::Page * p = d->page(page);
    if (p) {
        int i;
        for(i = 0; i < d->pages.count() && d->pages.at(i) != p; i++);
        bool notFirst = false;

        if(i) {
            i--;
            while((i >= 0) && !notFirst) {
                notFirst |= appropriate(d->pages.at(i)->w);
                i--;
            }
        }
        setBackEnabled(notFirst);
        setNextEnabled(true);
        d->ws->raiseWidget(page);
        d->current = p;
    }

    layOut();
    updateButtons();
    emit selected(p ? p->t : QString());
}


/*!
    Returns the number of pages in the wizard.
*/

int QWizard::pageCount() const
{
    return d->pages.count();
}

/*!
    Returns the position of page \a page. If the page is not part of
    the wizard -1 is returned.
*/

int QWizard::indexOf(QWidget *page) const
{
    return d->pages.indexOf(d->page(page));
}

/*!
    Called when the user clicks the Back button; this function shows
    the preceding relevant page in the sequence.

    \sa appropriate()
*/
void QWizard::back()
{
    int i = 0;

    while(i < (int)d->pages.count() && d->pages.at(i) &&
           d->current && d->pages.at(i)->w != d->current->w)
        i++;

    i--;
    while(i >= 0 && (!d->pages.at(i) || !appropriate(d->pages.at(i)->w)))
        i--;

    if(i >= 0)
       if(d->pages.at(i))
            showPage(d->pages.at(i)->w);
}


/*!
    Called when the user clicks the Next button, this function shows
    the next relevant page in the sequence.

    \sa appropriate()
*/
void QWizard::next()
{
    int i = 0;
    while(i < d->pages.count() && d->pages.at(i) &&
           d->current && d->pages.at(i)->w != d->current->w)
        i++;
    i++;
    while(i < d->pages.count() &&
           (!d->pages.at(i) || !appropriate(d->pages.at(i)->w)))
        i++;
    // if we fell of the end of the world, step back
    while (i > 0 && (i >= (int)d->pages.count() || !d->pages.at(i)))
        i--;
    if (d->pages.at(i))
        showPage(d->pages.at(i)->w);
}


/*!
    \fn void QWizard::helpClicked()

    This signal is emitted when the user clicks on the Help button.
*/

/*!
    Called when the user clicks the Help button, this function emits
    the helpClicked() signal.
*/

void QWizard::help()
{
    QWidget * page = d->ws->visibleWidget();
    if (!page)
        return;

#if 0
    QWizardPage *wpage = ::qt_cast<QWizardPage*>(page);
    if (wpage)
        emit wpage->helpClicked();
#endif
    emit helpClicked();
}


void QWizard::setBackEnabled(bool enable)
{
    d->backButton->setEnabled(enable);
#ifndef QT_NO_ACCEL
    d->accel->setItemEnabled(d->backAccel, enable);
#endif
}


void QWizard::setNextEnabled(bool enable)
{
    d->nextButton->setEnabled(enable);
#ifndef QT_NO_ACCEL
    d->accel->setItemEnabled(d->nextAccel, enable);
#endif
}


void QWizard::setHelpEnabled(bool enable)
{
    d->helpButton->setEnabled(enable);
}


/*!
  \fn void QWizard::setFinish(QWidget *widget, bool b)

  \obsolete

  If \a b is true the given \a widget has a Finish button;
  otherwise it does not have a Finish button.

  Use setFinishEnabled() instead
*/

/*!
    If \a enable is true, page \a page has a Back button; otherwise \a
    page has no Back button. By default all pages have this button.
*/
void QWizard::setBackEnabled(QWidget * page, bool enable)
{
    QWizardPrivate::Page *p = d->page(page);
    if (!p)
        return;

    p->backEnabled = enable;
    updateButtons();
}


/*!
    If \a enable is true, page \a page has a Next button; otherwise
    the Next button on \a page is disabled. By default all pages have
    this button.
*/

void QWizard::setNextEnabled(QWidget * page, bool enable)
{
    QWizardPrivate::Page *p = d->page(page);
    if (!p)
        return;

    p->nextEnabled = enable;
    updateButtons();
}


/*!
    If \a enable is true, page \a page has a Finish button; otherwise
    \a page has no Finish button. By default \e no page has this
    button.
*/
void QWizard::setFinishEnabled(QWidget * page, bool enable)
{
    QWizardPrivate::Page *p = d->page(page);
    if (!p)
        return;

    p->finishEnabled = enable;
    updateButtons();
}


/*!
    If \a enable is true, page \a page has a Help button; otherwise \a
    page has no Help button. By default all pages have this button.
*/
void QWizard::setHelpEnabled(QWidget * page, bool enable)
{
    QWizardPrivate::Page *p = d->page(page);
    if (!p)
        return;

    p->helpEnabled = enable;
    updateButtons();
}


/*!
    Called when the Next button is clicked; this virtual function
    returns true if \a page is relevant for display in the current
    context; otherwise it is ignored by QWizard and returns false. The
    default implementation returns the value set using
    setAppropriate(). The ultimate default is true.

    \warning The last page of the wizard will be displayed if no page
    is relevant in the current context.
*/

bool QWizard::appropriate(QWidget * page) const
{
    QWizardPrivate::Page *p = d->page(page);
    return !p || p->appropriate;
}


/*!
    If \a appropriate is true then page \a page is considered relevant
    in the current context and should be displayed in the page
    sequence; otherwise \a page should not be displayed in the page
    sequence.

    \sa appropriate()
*/
void QWizard::setAppropriate(QWidget * page, bool appropriate)
{
    QWizardPrivate::Page *p = d->page(page);
    if (p)
        p->appropriate = appropriate;
}


void QWizard::updateButtons()
{
    if (!d->current)
        return;

    int i;
    for(i = 0; i < d->pages.count() && d->pages.at(i) != d->current; i++);
    bool notFirst = false;
    if(i) {
        i--;
        while((i >= 0) && !notFirst) {
            notFirst |= appropriate(d->pages.at(i)->w);
            i--;
        }
    }
    setBackEnabled(d->current->backEnabled && notFirst);
    setNextEnabled(d->current->nextEnabled);
    d->finishButton->setEnabled(d->current->finishEnabled);
    d->helpButton->setEnabled(d->current->helpEnabled);

    if ((d->current->finishEnabled && !d->finishButton->isVisible()) ||
         (d->current->backEnabled && !d->backButton->isVisible()) ||
         (d->current->nextEnabled && !d->nextButton->isVisible()) ||
         (d->current->helpEnabled && !d->helpButton->isVisible()))
        layOut();
}


/*!
Returns a pointer to the current page in the sequence.

*/

QWidget * QWizard::currentPage() const
{
    return d->ws->visibleWidget();
}


/*!
    Returns the title of page \a page, or an empty string if the page widget
    is not part of the wizard.
*/

QString QWizard::title(QWidget * page) const
{
    QWizardPrivate::Page *p = d->page(page);
    return p ? p->t : QString();
}

/*!
    Sets the title for page \a page to \a title.
*/

void QWizard::setTitle(QWidget *page, const QString &title)
{
    QWizardPrivate::Page *p = d->page(page);
    if (p)
        p->t = title;
    if (page == currentPage())
        d->title->setText(title);
}

/*!
    \property QWizard::titleFont
    \brief the font used for page titles

    The default is QApplication::font().
*/
QFont QWizard::titleFont() const
{
    return d->title->font();
}

void QWizard::setTitleFont(const QFont & font)
{
    d->title->setFont(font);
}


/*!
    Returns a pointer to the dialog's Back button

    By default, this button is connected to the back() slot, which is
    virtual so you can reimplement it in a QWizard subclass. Use
    setBackEnabled() to enable/disable this button.
*/
QPushButton * QWizard::backButton() const
{
    return d->backButton;
}


/*!
    Returns a pointer to the dialog's Next button

    By default, this button is connected to the next() slot, which is
    virtual so you can reimplement it in a QWizard subclass. Use
    setNextEnabled() to enable/disable this button.
*/
QPushButton * QWizard::nextButton() const
{
    return d->nextButton;
}


/*!
    Returns a pointer to the dialog's Finish button

    By default, this button is connected to the QDialog::accept()
    slot, which is virtual so you can reimplement it in a QWizard
    subclass. Use setFinishEnabled() to enable/disable this button.
*/
QPushButton * QWizard::finishButton() const
{
    return d->finishButton;
}


/*!
    Returns a pointer to the dialog's Cancel button

    By default, this button is connected to the QDialog::reject()
    slot, which is virtual so you can reimplement it in a QWizard
    subclass.
*/
QPushButton * QWizard::cancelButton() const
{
    return d->cancelButton;
}


/*!
    Returns a pointer to the dialog's Help button

    By default, this button is connected to the help() slot, which is
    virtual so you can reimplement it in a QWizard subclass. Use
    setHelpEnabled() to enable/disable this button.
*/
QPushButton * QWizard::helpButton() const
{
    return d->helpButton;
}


/*!
    This virtual function is responsible for adding the buttons below
    the bottom divider.

    \a layout is the horizontal layout of the entire wizard.
*/

void QWizard::layOutButtonRow(QHBoxLayout * layout)
{
    bool hasHelp = false;
    bool hasEarlyFinish = false;

    int i = d->pages.count() - 2;
    while (!hasEarlyFinish && i >= 0) {
        if (d->pages.at(i) && d->pages.at(i)->finishEnabled)
            hasEarlyFinish = true;
        i--;
    }
    i = 0;
    while (!hasHelp && i < (int)d->pages.count()) {
        if (d->pages.at(i) && d->pages.at(i)->helpEnabled)
            hasHelp = true;
        i++;
    }

    QBoxLayout * h = new QBoxLayout(QBoxLayout::LeftToRight);
    layout->addLayout(h);

    if (hasHelp)
        h->addWidget(d->helpButton);
    else
        d->helpButton->hide();

    h->addStretch(42);

    h->addWidget(d->backButton);

    h->addSpacing(6);

    if (d->pages.size() && d->current == d->pages.at( d->pages.count()-1 ))
        hasEarlyFinish = false;

    if (hasEarlyFinish) {
        d->nextButton->show();
        d->finishButton->show();
        h->addWidget(d->nextButton);
        h->addSpacing(12);
        h->addWidget(d->finishButton);
    } else if (d->pages.isEmpty() ||
                d->current->finishEnabled ||
                d->current == d->pages.at(d->pages.count() - 1)) {
        d->nextButton->hide();
        d->finishButton->show();
        h->addWidget(d->finishButton);
    } else {
        d->nextButton->show();
        d->finishButton->hide();
        h->addWidget(d->nextButton);
    }

    // if last page is disabled - show finished btn. at lastpage-1
    i = d->pages.count() - 1;
    if (i >= 0 && !appropriate(d->pages.at(i)->w) &&
         d->current == d->pages.at(d->pages.count()-2)) {
        d->nextButton->hide();
        d->finishButton->show();
        h->addWidget(d->finishButton);
    }

    h->addSpacing(12);
    h->addWidget(d->cancelButton);
}


/*!
    This virtual function is responsible for laying out the title row.

    \a layout is the horizontal layout for the wizard, and \a
    title is the title for this page. This function is called every
    time \a title changes.
*/

void QWizard::layOutTitleRow(QHBoxLayout * layout, const QString & title)
{
    d->title->setText(title);
    layout->addWidget(d->title, 10);
}


/*

*/

void QWizard::layOut()
{
    delete d->v;
    d->v = new QVBoxLayout(this, 6, 0, "top-level layout");

    QHBoxLayout * l;
    l = new QHBoxLayout(6);
    d->v->addLayout(l, 0);
    layOutTitleRow(l, d->current ? d->current->t : QString());

    if (! d->hbar1) {
        d->hbar1 = new QFrame(this, "<hr>", 0);
        d->hbar1->setFrameStyle(QFrame::Sunken + QFrame::HLine);
        d->hbar1->setFixedHeight(12);
    }

    d->v->addWidget(d->hbar1);

    d->v->addWidget(d->ws, 10);

    if (! d->hbar2) {
        d->hbar2 = new QFrame(this, "<hr>", 0);
        d->hbar2->setFrameStyle(QFrame::Sunken + QFrame::HLine);
        d->hbar2->setFixedHeight(12);
    }
    d->v->addWidget(d->hbar2);

    l = new QHBoxLayout(6);
    d->v->addLayout(l);
    layOutButtonRow(l);
    d->v->activate();
}


/*!
    \reimp
*/

bool QWizard::eventFilter(QObject * o, QEvent * e)
{
    if (o == d->ws && e && e->type() == QEvent::ChildRemoved) {
        QChildEvent * c = (QChildEvent*)e;
        if (c->child() && c->child()->isWidgetType())
            removePage((QWidget *)c->child());
    }
    return QDialog::eventFilter(o, e);
}


/*!
    Removes \a page from the page sequence but does not delete the
    page. If \a page is currently being displayed, QWizard will
    display the page that precedes it, or the first page if this was
    the first page.
*/

void QWizard::removePage(QWidget * page)
{
    if (!page)
        return;

    int i = d->pages.count();
    QWidget* cp = currentPage();
    while(--i >= 0 && d->pages.at(i) && d->pages.at(i)->w != page) { }
    if (i < 0)
        return;
    delete d->pages.takeAt(i);
    d->ws->removeWidget(page);

    if (cp == page) {
        i--;
        if (i < 0)
            i = 0;
        if (pageCount() > 0)
            showPage(QWizard::page(i));
    }
}


/*!
    Returns a pointer to the page at position \a index in the
    sequence, or 0 if \a index is out of range. The first page has
    index 0.
*/

QWidget* QWizard::page(int index) const
{
    if (index >= pageCount() || index < 0)
      return 0;

    return d->pages.at(index)->w;
}

#endif // QT_NO_WIZARD
