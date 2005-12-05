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

#include "tabbedbrowser.h"
#include "mainwindow.h"
#include "helpwindow.h"
#include "config.h"

#include <QStyleOptionTab>
#include <QToolTip>
#include <QFileInfo>
#include <QToolButton>
#include <QPixmap>
#include <QIcon>
#include <QStyle>
#include <QTimer>
#include <QStackedWidget>
#include <QTimer>
#include <QTextBlock>
#include <QKeyEvent>

TabbedBrowser::TabbedBrowser(MainWindow *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    init();

    QStackedWidget *stack = qFindChild<QStackedWidget*>(ui.tab);
    Q_ASSERT(stack);
    stack->setContentsMargins(0, 0, 0, 0);
    connect(stack, SIGNAL(currentChanged(int)), parent, SLOT(browserTabChanged()));

    QPalette p = palette();
    p.setColor(QPalette::Inactive, QPalette::Highlight,
        p.color(QPalette::Active, QPalette::Highlight));
    p.setColor(QPalette::Inactive, QPalette::HighlightedText,
        p.color(QPalette::Active, QPalette::HighlightedText));
    setPalette(p);
}

TabbedBrowser::~TabbedBrowser()
{
}

MainWindow *TabbedBrowser::mainWindow() const
{
    return static_cast<MainWindow*>(parentWidget());
}

static QString reduceLabelLength( const QString &s )
{
    int maxLength = 16;
    QString str = s;
    if ( str.length() < maxLength )
        return str;
    str = str.left( maxLength - 3 );
    str += QLatin1String("...");
    return str;
}

void TabbedBrowser::forward()
{
    currentBrowser()->forward();
}

void TabbedBrowser::backward()
{
    currentBrowser()->backward();
}

void TabbedBrowser::setSource( const QString &ref )
{
    HelpWindow * win = currentBrowser();
    win->setSource(ref);
}

void TabbedBrowser::reload()
{
    currentBrowser()->reload();
}

void TabbedBrowser::home()
{
    currentBrowser()->home();
}

HelpWindow *TabbedBrowser::currentBrowser() const
{
    return static_cast<HelpWindow*>(ui.tab->currentWidget());
}

void TabbedBrowser::nextTab()
{
    if(ui.tab->currentIndex()<=ui.tab->count()-1)
        ui.tab->setCurrentIndex(ui.tab->currentIndex()+1);
}

void TabbedBrowser::previousTab()
{
    int idx = ui.tab->currentIndex()-1;
    if(idx>=0)
        ui.tab->setCurrentIndex(idx);
}

HelpWindow *TabbedBrowser::createHelpWindow(const QString &title)
{
    MainWindow *mainWin = mainWindow();
    HelpWindow *win = new HelpWindow(mainWin, 0);
    win->setFrameStyle(QFrame::NoFrame);
    win->setPalette(palette());
    win->setSearchPaths(Config::configuration()->mimePaths());
    ui.tab->addTab(win, reduceLabelLength(title));
    connect(win, SIGNAL(highlighted(QString)),
             (const QObject*) (mainWin->statusBar()), SLOT(showMessage(QString)));
    connect(win, SIGNAL(chooseWebBrowser()), mainWin, SLOT(showWebBrowserSettings()));
    connect(win, SIGNAL(choosePDFReader()), mainWin, SLOT(showPDFReaderSettings()));
    connect(win, SIGNAL(backwardAvailable(bool)),
             mainWin, SLOT(backwardAvailable(bool)));
    connect(win, SIGNAL(forwardAvailable(bool)),
             mainWin, SLOT(forwardAvailable(bool)));
    connect(win, SIGNAL(sourceChanged(QUrl)), this, SLOT(sourceChanged()));

    ui.tab->cornerWidget(Qt::TopRightCorner)->setEnabled(ui.tab->count() > 1);
	win->installEventFilter(this);
	win->viewport()->installEventFilter(this);
    ui.editFind->installEventFilter(this);
    return win;
}

HelpWindow *TabbedBrowser::newBackgroundTab(const QString &url)
{
    HelpWindow *win = createHelpWindow(url);
    return win;
}

void TabbedBrowser::newTab(const QString &lnk)
{
    QString link(lnk);
    if(link.isNull()) {
        HelpWindow *w = currentBrowser();
        if(w)
            link = w->source().toString();
    }
    HelpWindow *win = createHelpWindow(link);
    ui.tab->setCurrentIndex(ui.tab->indexOf(win));
    if(!link.isNull()) {
         win->setSource(link);
    }
}

void TabbedBrowser::zoomIn()
{
    currentBrowser()->zoomIn();
}

void TabbedBrowser::zoomOut()
{
    currentBrowser()->zoomOut();
}

void TabbedBrowser::init()
{

    lastCurrentTab = 0;
    while(ui.tab->count()) {
        QWidget *page = ui.tab->widget(0);
        ui.tab->removeTab(0);
        delete page;
    }

    connect(ui.tab, SIGNAL(currentChanged(int)),
             this, SLOT(transferFocus()));

    QTabBar *tabBar = qFindChild<QTabBar*>(ui.tab);
    QStyleOptionTab opt;
    if (tabBar) {
        opt.init(tabBar);
        opt.shape = tabBar->shape();
    }

    // workaround for sgi style
    QPalette pal = palette();
    pal.setColor(QPalette::Active, QPalette::Button, pal.color(QPalette::Active, QPalette::Background));
    pal.setColor(QPalette::Disabled, QPalette::Button, pal.color(QPalette::Disabled, QPalette::Background));
    pal.setColor(QPalette::Inactive, QPalette::Button, pal.color(QPalette::Inactive, QPalette::Background));

    QToolButton *newTabButton = new QToolButton(this);
    ui.tab->setCornerWidget(newTabButton, Qt::TopLeftCorner);
    newTabButton->setCursor(Qt::ArrowCursor);
    newTabButton->setAutoRaise(true);
    newTabButton->setIcon(QPixmap(QString::fromUtf8(":/trolltech/assistant/images/addtab.png")));
    QObject::connect(newTabButton, SIGNAL(clicked()), this, SLOT(newTab()));
    newTabButton->setToolTip(tr("Add page"));

    QToolButton *closeTabButton = new QToolButton(this);
    closeTabButton->setPalette(pal);
    ui.tab->setCornerWidget(closeTabButton, Qt::TopRightCorner);
    closeTabButton->setCursor(Qt::ArrowCursor);
    closeTabButton->setAutoRaise(true);
    closeTabButton->setIcon(QIcon(QLatin1String(":/trolltech/assistant/images/closetab.png")));
    QObject::connect(closeTabButton, SIGNAL(clicked()), this, SLOT(closeTab()));
    closeTabButton->setToolTip(tr("Close page"));
    closeTabButton->setEnabled(false);

	QObject::connect(ui.toolClose, SIGNAL(clicked()), ui.frameFind, SLOT(hide()));
	QObject::connect(ui.toolPrevious, SIGNAL(clicked()), this, SLOT(findPrevious()));
	QObject::connect(ui.toolNext, SIGNAL(clicked()), this, SLOT(findNext()));
	QObject::connect(ui.editFind, SIGNAL(returnPressed()), this, SLOT(findNext()));
	QObject::connect(ui.editFind, SIGNAL(textEdited(const QString&)),
				     this, SLOT(find(QString)));
	ui.frameFind->setVisible(false);
	ui.labelWrapped->setVisible(false);
	autoHideTimer = new QTimer(this);
	autoHideTimer->setInterval(5000);
	autoHideTimer->setSingleShot(true);
	QObject::connect(autoHideTimer, SIGNAL(timeout()), ui.frameFind, SLOT(hide()));
}

void TabbedBrowser::updateTitle(const QString &title)
{
    ui.tab->setTabText(ui.tab->indexOf(currentBrowser()), title);
}

void TabbedBrowser::newTab()
{
    newTab(QString());
}

void TabbedBrowser::transferFocus()
{
    if(currentBrowser()) {
        currentBrowser()->setFocus();
    }
    mainWindow()->setWindowTitle(Config::configuration()->title()
                             + QLatin1String(" - ")
                             + currentBrowser()->documentTitle());
}

void TabbedBrowser::initHelpWindow(HelpWindow * /*win*/)
{
}

void TabbedBrowser::setup()
{
    newTab(QString());
}

void TabbedBrowser::copy()
{
    currentBrowser()->copy();
}

void TabbedBrowser::closeTab()
{
    if(ui.tab->count()==1)
        return;
    HelpWindow *win = currentBrowser();
    ui.tab->removeTab(ui.tab->indexOf(win));
    QTimer::singleShot(0, win, SLOT(deleteLater()));
    ui.tab->cornerWidget(Qt::TopRightCorner)->setEnabled(ui.tab->count() > 1);
}

QStringList TabbedBrowser::sources() const
{
    QStringList lst;
    int cnt = ui.tab->count();
    for(int i=0; i<cnt; i++) {
        lst.append(((QTextBrowser*) ui.tab->widget(i))->source().toString());
    }
    return lst;
}

QList<HelpWindow*> TabbedBrowser::browsers() const
{
    QList<HelpWindow*> list;
    for (int i=0; i<ui.tab->count(); ++i) {
        Q_ASSERT(::qobject_cast<HelpWindow*>(ui.tab->widget(i)));
        list.append(static_cast<HelpWindow*>(ui.tab->widget(i)));
    }
    return list;
}

void TabbedBrowser::sourceChanged()
{
    HelpWindow *win = ::qobject_cast<HelpWindow *>(QObject::sender());
    Q_ASSERT(win);
    QString docTitle(win->documentTitle());
    if (docTitle.isEmpty())
        docTitle = QLatin1String("...");
    // Make the classname in the title a bit more visible (otherwise
    // we just see the "Qt 4.0 : Q..." which isn't really helpful ;-)
    if (docTitle.startsWith("Qt 4.0: "))
        docTitle = docTitle.mid(8);
    setTitle(win, docTitle);
	ui.frameFind->hide();
    ui.labelWrapped->hide();
	win->setTextCursor(win->cursorForPosition(QPoint(0, 0)));
}

void TabbedBrowser::setTitle(HelpWindow *win, const QString &title)
{
    ui.tab->setTabText(ui.tab->indexOf(win), reduceLabelLength(title));
    if (win == currentBrowser())
        mainWindow()->setWindowTitle(Config::configuration()->title() + QLatin1String(" - ") + title);
}

void TabbedBrowser::keyPressEvent(QKeyEvent *e)
{
	int key = e->key();
	QString ttf = ui.editFind->text();
	QString text = e->text();

	if (ui.frameFind->isVisible()) {
		switch (key) {
		case Qt::Key_Escape:
			ui.frameFind->hide();
            ui.labelWrapped->hide();
			return;
		case Qt::Key_Backspace:
			ttf.chop(1);
			break;
		case Qt::Key_Return:
        case Qt::Key_Enter:
			// Return/Enter key events are not accepted by QLineEdit
			return;
		default:
			if (text.isEmpty())
				return QWidget::keyPressEvent(e);
			ttf += text;
		}
	} else {
		if (text.isEmpty() || text[0].isSpace() || !text[0].isPrint())
			return QWidget::keyPressEvent(e);
		ttf = text;
		ui.frameFind->show();
	}

	ui.editFind->setText(ttf);
	find(ttf, false, false);
}

void TabbedBrowser::findNext()
{
	find(ui.editFind->text(), true, false);
}

void TabbedBrowser::findPrevious()
{
	find(ui.editFind->text(), false, true);
}

void TabbedBrowser::find()
{
	ui.frameFind->show();
	ui.editFind->setFocus(Qt::ShortcutFocusReason);
	ui.editFind->selectAll();
	autoHideTimer->stop();
}

void TabbedBrowser::find(QString ttf, bool forward, bool backward)
{
	HelpWindow *browser = currentBrowser();
	QTextDocument *doc = browser->document();
	QString oldText = ui.editFind->text();
	QTextCursor c = browser->textCursor();
	QTextDocument::FindFlags options;
	QPalette p = ui.editFind->palette();
	p.setColor(QPalette::Active, QPalette::Base, Qt::white);

	if (c.hasSelection())
		c.setPosition(forward ? c.position() : c.anchor(), QTextCursor::MoveAnchor);

	QTextCursor newCursor = c;

	if (!ttf.isEmpty()) {
		if (backward)
			options |= QTextDocument::FindBackward;

		if (ui.checkCase->isChecked())
			options |= QTextDocument::FindCaseSensitively;

		if (ui.checkWholeWords->isChecked())
			options |= QTextDocument::FindWholeWords;

		newCursor = doc->find(ttf, c, options);
		ui.labelWrapped->hide();

		if (newCursor.isNull()) {
			QTextCursor ac(doc);
			ac.movePosition(options & QTextDocument::FindBackward 
							? QTextCursor::End : QTextCursor::Start);
			newCursor = doc->find(ttf, ac, options);
			if (newCursor.isNull()) {
				p.setColor(QPalette::Active, QPalette::Base, QColor(255, 102, 102));
				newCursor = c;
			} else
				ui.labelWrapped->show();
		}
	}

	if (!ui.frameFind->isVisible())
		ui.frameFind->show();
	browser->setTextCursor(newCursor);
	ui.editFind->setPalette(p);
	if (!ui.editFind->hasFocus())
		autoHideTimer->start();
}

bool TabbedBrowser::eventFilter(QObject *o, QEvent *e)
{
    if (o == ui.editFind) {
        if (e->type() == QEvent::FocusIn && autoHideTimer->isActive())
            autoHideTimer->stop();
    } else if (e->type() == QEvent::KeyPress && ui.frameFind->isVisible()) { // assume textbrowser
		QKeyEvent *ke = static_cast<QKeyEvent *>(e);
		if (ke->key() == Qt::Key_Space) {
			keyPressEvent(ke);
			return true;
		}
	}

	return QWidget::eventFilter(o, e);
}
