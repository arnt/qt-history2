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

/*  TRANSLATOR TrWindow

  This is the application's main window.
*/

#include "trwindow.h"
#include "finddialog.h"
#include "msgedit.h"
#include "phrasebookbox.h"
#include "printout.h"
#include "about.h"
#include "statistics.h"
#include "contextmodel.h"
#include "messagemodel.h"
#include "phrasemodel.h"

#include <qaction.h>
#include <qapplication.h>
#include <qbitmap.h>
#include <qdockwindow.h>
#include <qfile.h>
#include <qfiledialog.h>
#include <qfileinfo.h>
#include <qheaderview.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qmenu.h>
#include <qregexp.h>
#include <qsettings.h>
#include <qstatusbar.h>
#include <qtoolbar.h>
#include <QWhatsThis>
//#include <qassistantclient.h>
#include <qdesktopwidget.h>
#include <qprintdialog.h>

#define pagecurl_mask_width 53
#define pagecurl_mask_height 51
static const uchar pagecurl_mask_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0x0f, 0x80, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0f, 0x00, 0xfe, 0xff,
   0xff, 0xff, 0xff, 0x0f, 0x00, 0xf0, 0xff, 0xff, 0xff, 0xff, 0x0f, 0x00,
   0xc0, 0xff, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x80, 0xff, 0xff, 0xff, 0xff,
   0x0f, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0xfe, 0xff,
   0xff, 0xff, 0x0f, 0x00, 0x00, 0xfc, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00,
   0xfc, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0xf8, 0xff, 0xff, 0xff, 0x0f,
   0x00, 0x00, 0xf0, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0xf0, 0xff, 0xff,
   0xff, 0x0f, 0x00, 0x00, 0xf0, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0xf0,
   0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0xe0, 0xff, 0xff, 0xff, 0x0f, 0x00,
   0x00, 0xe0, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0xe0, 0xff, 0xff, 0xff,
   0x0f, 0x00, 0x00, 0xe0, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0xe0, 0xff,
   0xff, 0xff, 0x0f, 0x00, 0x00, 0xe0, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00,
   0xe0, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0xe0, 0xff, 0xff, 0xff, 0x0f,
   0x00, 0x00, 0xe0, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0xe0, 0xff, 0xff,
   0xff, 0x0f, 0x00, 0x00, 0xe0, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0xe0,
   0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0xe0, 0xff, 0xff, 0xff, 0x0f, 0x00,
   0x00, 0xe0, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0xc0, 0xff, 0xff, 0xff,
   0x0f, 0x00, 0x00, 0xc0, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0x00, 0xfc,
   0xff, 0xff, 0x0f, 0x00, 0x00, 0x00, 0x00, 0xf8, 0xff, 0x0f, 0x00, 0x00,
   0x00, 0x00, 0x00, 0xfc, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x0f,
   0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00,
   0xe0, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x0f, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08 };

typedef QList<MetaTranslatorMessage> TML;

static const int ErrorMS = 600000; // for error messages
static const int MessageMS = 2500;

QPixmap *TrWindow::pxOn = 0;
QPixmap *TrWindow::pxOff = 0;
QPixmap *TrWindow::pxObsolete = 0;
QPixmap *TrWindow::pxDanger = 0;
QPixmap *TrWindow::pxObs = 0;
QPixmap *TrWindow::pxEmpty = 0;

enum Ending {End_None, End_FullStop, End_Interrobang, End_Colon,
              End_Ellipsis};

static Ending ending(QString str)
{
    str = str.simplified();
    int ch = 0;
    if (!str.isEmpty())
        ch = str.right(1)[0].unicode();

    switch (ch) {
    case 0x002e: // full stop
        if (str.endsWith(QString("...")))
            return End_Ellipsis;
        else
            return End_FullStop;
    case 0x0589: // armenian full stop
    case 0x06d4: // arabic full stop
    case 0x3002: // ideographic full stop
        return End_FullStop;
    case 0x0021: // exclamation mark
    case 0x003f: // question mark
    case 0x00a1: // inverted exclamation mark
    case 0x00bf: // inverted question mark
    case 0x01c3: // latin letter retroflex click
    case 0x037e: // greek question mark
    case 0x061f: // arabic question mark
    case 0x203c: // double exclamation mark
    case 0x203d: // interrobang
    case 0x2048: // question exclamation mark
    case 0x2049: // exclamation question mark
    case 0x2762: // heavy exclamation mark ornament
        return End_Interrobang;
    case 0x003a: // colon
        return End_Colon;
    case 0x2026: // horizontal ellipsis
        return End_Ellipsis;
    default:
        return End_None;
    }
}

const QPixmap TrWindow::pageCurl()
{
    QPixmap pixmap;
    pixmap.load(":/images/pagecurl.png" );
    if ( !pixmap.isNull() ) {
        QBitmap pageCurlMask(pagecurl_mask_width, pagecurl_mask_height,
            pagecurl_mask_bits, true);
        pixmap.setMask(pageCurlMask);
    }

    return pixmap;
}

TrWindow::TrWindow()
    : QMainWindow(0, Qt::WType_TopLevel)
{
    setAttribute(Qt::WA_DeleteOnClose);

#ifndef Q_WS_MAC
    setWindowIcon(QPixmap(":/images/appicon.png" ));
#endif

    // Create the application global listview symbols
    pxOn  = new QPixmap(":/images/s_check_on.png");
    pxOff = new QPixmap(":/images/s_check_off.png");
    pxObsolete = new QPixmap(":/images/d_s_check_obs.png");
    pxDanger = new QPixmap(":/images/s_check_danger.png");
    pxObs = new QPixmap(":/images/s_check_obs.png");
    pxEmpty = new QPixmap(":/images/s_check_empty.png");

    setCorner(Qt::TopLeftCorner, Qt::DockWindowAreaLeft);
    setCorner(Qt::TopRightCorner, Qt::DockWindowAreaRight);
    setCorner(Qt::BottomLeftCorner, Qt::DockWindowAreaLeft);
    setCorner(Qt::BottomRightCorner, Qt::DockWindowAreaRight);

    // Set up the Scope dock window
    dwScope = new QDockWindow(this);
    dwScope->setAllowedAreas(Qt::AllDockWindowAreas);
    dwScope->setFeatures(QDockWindow::AllDockWindowFeatures);
    dwScope->setWindowTitle(tr("Context"));

    tv = new QTreeView(dwScope);
    cmdl = new ContextModel(dwScope);
    tv->setModel(cmdl);
    tv->setAlternatingRowColors(true);
    tv->setOddRowColor(TREEVIEW_ODD_COLOR);
    
    tv->setSelectionBehavior(QAbstractItemView::SelectRows);
    tv->setSelectionMode(QAbstractItemView::SingleSelection);
    tv->setRootIsDecorated(false);
    dwScope->setWidget(tv);
    extendDockWindowArea(Qt::DockWindowAreaLeft, dwScope, Qt::Horizontal);

    QFontMetrics fm(font());
    tv->header()->setResizeMode(QHeaderView::Stretch, 1);
    tv->header()->resizeSection(0, fm.width(ContextModel::tr("Done")) + 10);
    tv->header()->resizeSection(2, 55);
        
    me = new MessageEditor(&tor, this);
    setCentralWidget(me);
    stv = me->sourceTextView();
    mmdl = qt_cast<MessageModel *>(stv->model());
    ptv = me->phraseView();
    pmdl = qt_cast<PhraseModel *>(ptv->model());

    setupMenuBar();
    setupToolBars();

    progress = new QLabel(statusBar());
    statusBar()->addWidget(progress, 0, true);
    modified = new QLabel(QString(" %1 ").arg(tr("MOD")), statusBar());
    statusBar()->addWidget(modified, 0, true);

    numFinished = 0;
    numNonobsolete = 0;
    numMessages = 0;
    updateProgress();

    dirty = false;
    updateCaption();

    finddlg = new FindDialog(this);
    findMatchCase = false;
    findWhere = 0;
    foundWhere = 0;
    foundOffset = 0;

    connect(tv->selectionModel(), SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)),
        this, SLOT(showNewScope(const QModelIndex &, const QModelIndex &)));
    connect(stv, SIGNAL(clicked(const QModelIndex &, Qt::MouseButton, Qt::KeyboardModifiers)),
        this, SLOT(toggleFinished(const QModelIndex &, Qt::MouseButton)));
    connect(me, SIGNAL(translationChanged(const QString&)),
        this, SLOT(updateTranslation(const QString&)));
    connect(me, SIGNAL(finished(bool)), this, SLOT(updateFinished(bool)));
    connect(me, SIGNAL(prevUnfinished()), this, SLOT(prevUnfinished()));
    connect(me, SIGNAL(nextUnfinished()), this, SLOT(nextUnfinished()));
    connect(me, SIGNAL(focusSourceList()), this, SLOT(focusSourceList()));
    connect(me, SIGNAL(focusPhraseList()), this, SLOT(focusPhraseList()));
    connect(finddlg, SIGNAL(findNext(const QString&, int, bool)), 
        this, SLOT(findNext(const QString&, int, bool)));
    connect(tv->header(), SIGNAL(sectionClicked(int, Qt::MouseButton, Qt::KeyboardModifiers)),
        this, SLOT(sortContexts(int, Qt::MouseButton)));
    connect(stv->header(), SIGNAL(sectionClicked(int, Qt::MouseButton, Qt::KeyboardModifiers)),
        this, SLOT(sortMessages(int, Qt::MouseButton)));
    connect(ptv->header(), SIGNAL(sectionClicked(int, Qt::MouseButton, Qt::KeyboardModifiers)),
        this, SLOT(sortPhrases(int, Qt::MouseButton)));

    tv->setWhatsThis(tr("This panel lists the source contexts."));
    stv->setWhatsThis(tr("This panel lists the source texts. "
        "Items that violate validation rules "
        "are marked with a warning."));
    
    QSize as( qApp->desktop()->size() );
    as -= QSize( 30, 30 );
    resize( QSize( 1000, 800 ).boundedTo( as ) );
    readConfig();
    stats = 0;
    srcWords = 0;
    srcChars = 0;
    srcCharsSpc = 0;
}

TrWindow::~TrWindow()
{
    writeConfig();
    delete stats;
}

void TrWindow::sortContexts(int section, Qt::MouseButton state)
{
    if ((state == Qt::LeftButton) && (section == 1)) {
        Qt::SortOrder order;
        int column;

        if (cmdl->sortParameters(order, column)) {
            if ((order == Qt::AscendingOrder) && (column == section))
                order = Qt::DescendingOrder;
            else
                order = Qt::AscendingOrder;
        }
        else {
            order = Qt::AscendingOrder;
        }

        if (cmdl->contextsInList() > 0) {
            tv->header()->setSortIndicator(section, order);
            tv->header()->setSortIndicatorShown(true);
            cmdl->sort(section, QModelIndex::Null, order);
            tv->clearSelection();
            mmdl->setContextItem(0);
        }
    }
}

void TrWindow::sortMessages(int section, Qt::MouseButton state)
{
    if ((state == Qt::LeftButton) && 
        ((section == 1) || (section == 2))) {
        ContextItem *c = mmdl->contextItem();
        Qt::SortOrder order;
        int column;

        if ((c != 0) && (c->sortParameters(order, column))) {
            if ((order == Qt::AscendingOrder) && (column == section))
                order = Qt::DescendingOrder;
            else
                order = Qt::AscendingOrder;
        }
        else {
            order = Qt::AscendingOrder;
        }

        if (c != 0) {
            stv->header()->setSortIndicator(section, order);
            stv->header()->setSortIndicatorShown(true);
            mmdl->sort(section, QModelIndex::Null, order);
            stv->clearSelection();
        }
    }
}

void TrWindow::sortPhrases(int section, Qt::MouseButton state)
{
    if ((state == Qt::LeftButton) && 
        ((section >= 0) && (section <= 2))) {

        Qt::SortOrder order;
        int column;

        if ((pmdl->sortParameters(order, column))) {
            if ((order == Qt::AscendingOrder) && (column == section))
                order = Qt::DescendingOrder;
            else
                order = Qt::AscendingOrder;
        }
        else {
            order = Qt::AscendingOrder;
        }

        ptv->header()->setSortIndicator(section, order);
        ptv->header()->setSortIndicatorShown(true);
        pmdl->sort(section, QModelIndex::Null, order);
        ptv->clearSelection();
    }
}

void TrWindow::openFile( const QString& name )
{
    if (name.isEmpty())
        return;
    
    statusBar()->message(tr("Loading..."));
    qApp->processEvents();
    tor.clear();

    if (!tor.load(name)) {
        statusBar()->clear();
        QMessageBox::warning(this, tr("Qt Linguist"), tr("Cannot open '%1'.").arg(name));
        return;
    }

    mmdl->setContextItem(0);
    cmdl->clearContextList();
    numFinished = 0;
    numNonobsolete = 0;
    numMessages = 0;
//    foundScope = 0;

    TML all = tor.messages();
    QHash<QString, ContextItem*> contexts;

    srcWords = 0;
    srcChars = 0;
    srcCharsSpc = 0;

    foreach (MetaTranslatorMessage mtm, all) {
        qApp->processEvents();
        ContextItem *c;
        if (contexts.contains(QString(mtm.context()))) {
            c = contexts.value( QString(mtm.context()));
        }
        else {
            c = new ContextItem(tor.toUnicode(mtm.context(), mtm.utf8()));
            cmdl->appendContextItem(c);
            contexts.insert(QString(mtm.context()), c);
        }
        if (QByteArray(mtm.sourceText()) == ContextComment) {
            c->appendToComment(tor.toUnicode(mtm.comment(), mtm.utf8()));
        }
        else {
            MessageItem *tmp = new MessageItem(mtm, tor.toUnicode(mtm.sourceText(),
                mtm.utf8()), tor.toUnicode(mtm.comment(), mtm.utf8()), c);
            c->appendMessageItem(tmp);
            updateDanger(tmp);
            if (mtm.type() != MetaTranslatorMessage::Obsolete) {
                numNonobsolete++;
                if (mtm.type() == MetaTranslatorMessage::Finished)
                    numFinished++;
                doCharCounting(tmp->sourceText(), srcWords, srcChars, srcCharsSpc);
            }
            else {
                c->incrementObsoleteCount();
            }
            numMessages++;
        }
    }

    cmdl->updateAll();
    tv->clearSelection();

    setEnabled(true);
    updateProgress();
    filename = name;
    dirty = false;
    updateCaption();
    me->showNothing();
    doneAndNextAct->setEnabled(false);
    doneAndNextAlt->setEnabled(false);
    statusBar()->message(tr("%1 source phrase(s) loaded.").arg(numMessages), MessageMS);
    foundWhere = 0;
    foundOffset = 0;

    if (cmdl->contextsInList() > 0) {
        findAct->setEnabled(true);
        findAgainAct->setEnabled(false);
    }

    addRecentlyOpenedFile(name, recentFiles);
    updateStatistics();
}

void TrWindow::open()
{
    if (maybeSave()) {
        QString newFilename = QFileDialog::getOpenFileName( this, QString(), filename,
            tr("Qt translation source (*.ts)\nAll files (*)"));
        openFile(newFilename);
    }
}

void TrWindow::save()
{
    if (filename.isEmpty())
        return;

    if (tor.save(filename)) {
        dirty = false;
        updateCaption();
        statusBar()->message(tr("File saved."), MessageMS);
    } else {
        QMessageBox::warning(this, tr("Qt Linguist"), tr("Cannot save '%1'.")
            .arg(filename));
    }
}

void TrWindow::saveAs()
{
    QString newFilename = QFileDialog::getSaveFileName(this, QString(), filename,
        tr( "Qt translation source (*.ts)\nAll files (*)"));
    if (!newFilename.isEmpty()) {
        filename = newFilename;
        save();
        updateCaption();
    }
}

void TrWindow::release()
{
    QString newFilename = filename;
    newFilename.replace(QRegExp(".ts$"), "");
    newFilename += QString(".qm");

    newFilename = QFileDialog::getSaveFileName(this, tr("Release"), newFilename,
        tr("Qt message files for released applications (*.qm)\nAll files (*)"));
    if (!newFilename.isEmpty()) {
        if (tor.release(newFilename))
            statusBar()->message(tr("File created."), MessageMS);
        else
            QMessageBox::warning(this, tr("Qt Linguist"),
            tr("Cannot save '%1'.").arg(newFilename));
    }
}

void TrWindow::print()
{
    int pageNum = 0;
    QList <ContextItem *> ctxtList;
    QList <MessageItem *> msgList;
    const MessageItem *m;
    ContextItem *c;

    QPrintDialog dlg(&printer, this);
    if (dlg.exec()) {
        QApplication::setOverrideCursor(Qt::WaitCursor);
        printer.setDocName(filename);
        statusBar()->message(tr("Printing..."));
        PrintOut pout(&printer);
        ctxtList = cmdl->contextList();
        
        for (int i=0; i<ctxtList.count(); i++) {
            c = ctxtList.at(i);
            pout.vskip();
            pout.setRule(PrintOut::ThickRule);
            pout.setGuide(c->context());
            pout.addBox(100, tr("Context: %1").arg(c->context()),
                PrintOut::Strong);
            pout.flushLine();
            pout.addBox(4);
            pout.addBox(92, c->comment(), PrintOut::Emphasis);
            pout.flushLine();
            pout.setRule(PrintOut::ThickRule);

            msgList = c->messageItemList();
            for (int j=0; j<msgList.count(); j++) {
                m = msgList.at(j);
                pout.setRule(PrintOut::ThinRule);

                QString type;
                switch (m->message().type()) {
                case MetaTranslatorMessage::Finished:
                    type = tr("finished");
                    break;
                case MetaTranslatorMessage::Unfinished:
                    type = m->danger() ? tr("unresolved") : QString("unfinished");
                    break;
                case MetaTranslatorMessage::Obsolete:
                    type = tr("obsolete");
                    break;
                default:
                    type = QString("");
                }
                pout.addBox(40, m->sourceText());
                pout.addBox(4);
                pout.addBox(40, m->translation());
                pout.addBox(4);
                pout.addBox(12, type, PrintOut::Normal, Qt::AlignRight);
                if (!m->comment().isEmpty()) {
                    pout.flushLine();
                    pout.addBox(4);
                    pout.addBox(92, m->comment(), PrintOut::Emphasis);
                }
                pout.flushLine(true);

                if (pout.pageNum() != pageNum) {
                    pageNum = pout.pageNum();
                    statusBar()->message(tr("Printing... (page %1)")
                        .arg(pageNum));
                }
            }
        }
        pout.flushLine(true);
        QApplication::restoreOverrideCursor();
        statusBar()->message(tr("Printing completed"), MessageMS);
    } else {
        statusBar()->message(tr("Printing aborted"), MessageMS);
    }
}

void TrWindow::find()
{
    finddlg->show();
    finddlg->setActiveWindow();
    finddlg->raise();
}

void TrWindow::findAgain()
{
    if (cmdl->contextsInList() <= 0)
        return;

    int pass = 0;
    int scopeNo = 0;
    int itemNo = 0;
    
    QModelIndex indxItem = stv->currentIndex();
    if (indxItem.isValid())
        itemNo = indxItem.row();
    QModelIndex indxScope = tv->currentIndex();
    if (indxScope.isValid())
        scopeNo = indxScope.row();

    QString delayedMsg;

    //scopeNo = foundScope;
    ContextItem *c = cmdl->contextItem(cmdl->index(scopeNo, 1));
    MessageItem *m; // = c->messageItem(foundItem);
    
#if 1
    /*
      As long as we don't implement highlighting of the text in the QTextView,
      we may have only one match per message.
    */
    foundOffset = (int) 0x7fffffff;
#else
    foundOffset++;
#endif

    while (pass < cmdl->contextsInList()) {
        for (int mit = itemNo; mit < c->messageItemsInList(); ++mit) {
            m = c->messageItem(mit);
            switch (foundWhere) {
                case 0:
                    foundWhere = FindDialog::SourceText;
                    foundOffset = 0;
                    // fall-through
                case FindDialog::SourceText:
                    if (searchItem(m->sourceText(), scopeNo, mit)) {
                        finddlg->hide();
                        if (!delayedMsg.isEmpty())
                            statusBar()->message(delayedMsg, MessageMS);
                        return;
                    }
                    foundWhere = FindDialog::Translations;
                    foundOffset = 0;
                    // fall-through
                case FindDialog::Translations:
                    if (searchItem(m->translation(), scopeNo, mit)) {
                        finddlg->hide();
                        if (!delayedMsg.isEmpty())
                            statusBar()->message(delayedMsg, MessageMS);
                        return;
                    }
                    foundWhere = FindDialog::Comments;
                    foundOffset = 0;
                    // fall-through
                case FindDialog::Comments: // what about comments in messages?
                    if (searchItem(c->fullContext(), scopeNo, mit)) {
                        finddlg->hide();
                        if (!delayedMsg.isEmpty())
                            statusBar()->message(delayedMsg, MessageMS);
                        return;
                    }
                    foundWhere = 0;
                    foundOffset = 0;
            }
        }
        ++pass;
        
        ++scopeNo;
        if (scopeNo >= cmdl->contextsInList()) {
            scopeNo = 0;
            delayedMsg = tr("Search wrapped.");
        }
        c = cmdl->contextItem(cmdl->index(scopeNo, 1));
    }

    // This is just to keep the current scope and source text item
    // selected if a search failed.
/*    setCurrentContextRow(oldScope.row());
    setCurrentMessageRow(oldItemNo.row()); */

    qApp->beep();
    QMessageBox::warning( this, tr("Qt Linguist"),
                          QString( tr("Cannot find the string '%1'.") ).arg(findText) );
//    foundItem   = 0;
    foundWhere  = 0;
    foundOffset = 0;
}

bool TrWindow::searchItem(const QString &searchWhat, int c, int m)
{
    if ((findWhere & foundWhere) != 0) {
        foundOffset = searchWhat.indexOf(findText, foundOffset,
            findMatchCase ? Qt::CaseSensitive : Qt::CaseInsensitive);
        if (foundOffset >= 0) {
            //foundItem = m;
            //foundScope = c;
            setCurrentContextRow(c);
            setCurrentMessageRow(m);
            return true;
        }
    }
    foundOffset = 0;
    return false;
}

void TrWindow::newPhraseBook()
{
    QString name;
    for (;;) {
        name = QFileDialog::getSaveFileName(this, tr("Create New Phrase Book"), 
            QString::null, tr("Qt phrase books (*.qph)\nAll files (*)"));
        if (name.isEmpty())
            break;
        else if (!QFile::exists(name)) {
            break;
            QMessageBox::warning(this, tr("Qt Linguist"),
                tr("A file called '%1' already exists."
                "  Please choose another name.").arg(name));
        }
    }
    if (!name.isEmpty()) {
        PhraseBook pb;
        if (savePhraseBook(name, pb)) {
            if (openPhraseBook(name))
                statusBar()->message(tr("Phrase book created."), MessageMS);
        }
    }
}

bool TrWindow::phraseBooksContains(QString name)
{
    foreach(PhraseBook pb, phraseBooks[PhraseCloseMenu]) {
        if (pb.fileName() == name)
            return true;
    }

    return false;
}

PhraseBook TrWindow::phraseBookFromFileName(QString name) const
{
    foreach(PhraseBook pb, phraseBooks[PhraseCloseMenu]) {
        if (pb.fileName() == name)
            return pb;
    }

    return PhraseBook(); // empty phrasebook
}

void TrWindow::openPhraseBook()
{
    QString phrasebooks(qInstallPathData());
    QString name = QFileDialog::getOpenFileName(this, tr("Open Phrase Book"),
        phrasebooks + "/phrasebooks", tr("Qt phrase books (*.qph)\nAll files (*)"));
    if (!name.isEmpty() && !phraseBooksContains(name)) {
        if (openPhraseBook(name)) {
            int n = phraseBookFromFileName(name).count();
            statusBar()->message(tr("%1 phrase(s) loaded.").arg(n), MessageMS);
        }
    }
}

void TrWindow::closePhraseBook(QAction *action)
{
    PhraseBook pb = phraseBooks[PhraseCloseMenu].value(action);
    phraseBooks[PhraseCloseMenu].remove(action);
    closePhraseBookp->removeAction(action);

    QAction *act = phraseBooks[PhraseEditMenu].key(pb);
    phraseBooks[PhraseEditMenu].remove(act);
    editPhraseBookp->removeAction(act);

    act = phraseBooks[PhrasePrintMenu].key(pb);
    qDebug("Remove: %d", phraseBooks[PhrasePrintMenu].remove(act));
    printPhraseBookp->removeAction(act);

    updatePhraseDict();
}

void TrWindow::editPhraseBook(QAction *action)
{
    PhraseBook pb = phraseBooks[PhraseEditMenu].value(action);
    PhraseBookBox box(pb.fileName(), pb, this);
    box.setWindowTitle(tr("%1 - %2").arg(tr("Qt Linguist"))
        .arg(friendlyPhraseBookName(pb)));
    box.resize(500, 300);
    box.exec();

    // delete phrasebook from all menus before changing
    // this avoids detachment
    phraseBooks[PhraseEditMenu].remove(action);
    QAction *closeact = phraseBooks[PhraseCloseMenu].key(pb);
    phraseBooks[PhraseCloseMenu].remove(closeact);
    QAction *printact = phraseBooks[PhrasePrintMenu].key(pb);
    phraseBooks[PhrasePrintMenu].remove(printact);

    phraseBooks[PhraseEditMenu].insert(action, box.phraseBook());
    phraseBooks[PhraseCloseMenu].insert(closeact, box.phraseBook());
    phraseBooks[PhrasePrintMenu].insert(printact, box.phraseBook());

    updatePhraseDict();
}

void TrWindow::printPhraseBook(QAction *action)
{
    PhraseBook phraseBook = phraseBooks[PhrasePrintMenu].value(action);

    int pageNum = 0;

    QPrintDialog dlg(&printer, this);
    if (dlg.exec()) {
        printer.setDocName(phraseBook.fileName());
        statusBar()->message(tr("Printing..."));
        PrintOut pout(&printer);
        pout.setRule(PrintOut::ThinRule);
        foreach (Phrase p, phraseBook) {
            pout.setGuide(p.source());
            pout.addBox(29, p.source());
            pout.addBox(4);
            pout.addBox(29, p.target());
            pout.addBox(4);
            pout.addBox(34, p.definition(), PrintOut::Emphasis);

            if (pout.pageNum() != pageNum) {
                pageNum = pout.pageNum();
                statusBar()->message(tr("Printing... (page %1)")
                    .arg(pageNum));
            }
            pout.setRule(PrintOut::NoRule);
            pout.flushLine(true);
        }
        pout.flushLine(true);
        statusBar()->message(tr("Printing completed"), MessageMS);
    } else {
        statusBar()->message(tr("Printing aborted"), MessageMS);
    }
}

void TrWindow::revertSorting()
{
    if (cmdl->contextsInList() < 0)
        return;

    tv->header()->setSortIndicator(1, Qt::AscendingOrder);
    tv->header()->setSortIndicatorShown(true);
    cmdl->sort(1, QModelIndex::Null, Qt::AscendingOrder);
    tv->clearSelection();
    mmdl->setContextItem(0);

    foreach(ContextItem *c, cmdl->contextList()) {
        c->sortMessages(1, Qt::AscendingOrder);
    }
    stv->header()->setSortIndicator(1, Qt::AscendingOrder);
    stv->header()->setSortIndicatorShown(true);
}

void TrWindow::manual()
{
    QString path = QDir::cleanPath(QString(qInstallPath()) + 
        QDir::separator() + "bin/");
#if defined(Q_OS_MAC)
    path += QDir::separator() + ".app/Contents/MacOS/";
#endif
	// TODO -> enable!
    /*QAssistantClient *ac = new QAssistantClient(path, this);
    ac->showPage(QString(qInstallPath()) + "/doc/html/linguist-manual.html");*/
}

void TrWindow::about()
{
    AboutDialog about(this);
    about.versionLabel->setText(tr("Version %1").arg(QT_VERSION_STR));
    about.exec();
}

void TrWindow::aboutQt()
{
    QMessageBox::aboutQt(this, tr("Qt Linguist"));
}

void TrWindow::setupPhrase()
{
    bool enabled = !phraseBooks[PhraseCloseMenu].isEmpty();
    closePhraseBookId->setEnabled(enabled);
    editPhraseBookId->setEnabled(enabled);
    printPhraseBookId->setEnabled(enabled);
}

void TrWindow::closeEvent(QCloseEvent *e)
{
    if (maybeSave())
        e->accept();
    else
        e->ignore();
}

bool TrWindow::maybeSave()
{
    if (dirty) {
        switch (QMessageBox::information(this, tr("Qt Linguist"),
            tr("Do you want to save '%1'?").arg(filename),
            QMessageBox::Yes | QMessageBox::Default,
            QMessageBox::No,
            QMessageBox::Cancel | QMessageBox::Escape))
        {
            case QMessageBox::Cancel:
                return false;
            case QMessageBox::Yes:
                save();
                return !dirty;
            case QMessageBox::No:
                break;
        }
    }
    return true;
}

void TrWindow::updateCaption()
{
    QString cap;
    bool enable = !filename.isEmpty();
    saveAct->setEnabled(enable);
    saveAsAct->setEnabled(enable);
    releaseAct->setEnabled(enable);
    printAct->setEnabled(enable);
    acceleratorsAct->setEnabled(enable);
    endingPunctuationAct->setEnabled(enable);
    phraseMatchesAct->setEnabled(enable);
    revertSortingAct->setEnabled(enable);

    if (filename.isEmpty())
        cap = tr("Qt Linguist by Trolltech");
    else
        cap = tr("%1 - %2").arg( tr("Qt Linguist by Trolltech"))
        .arg(filename);
    setWindowTitle(cap);
    modified->setEnabled(dirty);
}

//
// New scope selected - select a new list of source text items
// for that scope.
//
void TrWindow::showNewScope(const QModelIndex &current, const QModelIndex &old)
{
    stv->clearSelection();
    statusBar()->clear();

    if (current.isValid()) {
        ContextItem *c = cmdl->contextItem(current);
        mmdl->setContextItem(c);
        Qt::SortOrder sortOrder;
        int sortColumn;
        
        if (c->sortParameters(sortOrder, sortColumn)) {
            stv->header()->setSortIndicator(sortColumn, sortOrder);
            stv->header()->setSortIndicatorShown(true);
        }
        else {
            stv->header()->setSortIndicatorShown(false);
        }
    }

    Q_UNUSED(old);
}

void TrWindow::showNewCurrent(const QModelIndex &current, const QModelIndex &old)
{
    ContextItem *c = mmdl->contextItem();

    if (current.isValid()) {
        MessageItem *m = c->messageItem(current.row());

        me->showMessage(m->sourceText(), m->comment(), c->fullContext(),
            m->translation(), m->message().type(), getPhrases(m->sourceText()));
        if (m->danger())
            printDanger(m);
        else
            statusBar()->clear();

        doneAndNextAct->setEnabled(m->message().type() != 
            MetaTranslatorMessage::Obsolete);
    }
    else {
        me->showNothing();
        doneAndNextAct->setEnabled(false);
    }
    
    doneAndNextAlt->setEnabled(doneAndNextAct->isEnabled());
    //selectAllAct->setEnabled(doneAndNextAct->isEnabled());

    Q_UNUSED(old);
}

void TrWindow::insertMessage(MessageItem *m)
{
    if (!dirty) {
        dirty = true;
        updateCaption();
    }

    tor.insert(m->message());
}

void TrWindow::updateTranslation(const QString &translation)
{
    QModelIndex item = stv->currentIndex();
    if (!item.isValid())
        return;

    ContextItem *c = mmdl->contextItem();
    MessageItem *m = c->messageItem(item.row());

    if (translation != m->translation()) {
        m->setTranslation(translation);
        
        updateDanger(m, true);
        mmdl->updateItem(item);

        if (m->finished())
            updateFinished(false);
        else
            insertMessage(m);
    }
}

void TrWindow::updateFinished(bool finished)
{
    QModelIndex item = stv->currentIndex();
    if (!item.isValid())
        return;

    ContextItem *c = mmdl->contextItem();
    MessageItem *m = c->messageItem(item.row());

    if (finished != m->finished()) {
        numFinished += finished ? +1 : -1;
        updateProgress();
        m->setFinished(finished);
        mmdl->updateItem(item);
        insertMessage(m);
        cmdl->updateItem(tv->currentIndex());
        updateStatistics();
    }
}

void TrWindow::doneAndNext()
{
    if (!stv->currentIndex().isValid())
        return;

    ContextItem *c = mmdl->contextItem();
    MessageItem *m = c->messageItem(stv->currentIndex().row());

    if (!m->danger()) {
        updateFinished(true);
        nextUnfinished();
        me->setEditorFocus();
    }
    else {
        qApp->beep();
    }
}

void TrWindow::toggleFinished(const QModelIndex &index, Qt::MouseButton button)
{
    if (!index.isValid() || (index.column() != 0))
        return;

    ContextItem *c = mmdl->contextItem();
    MessageItem *m = c->messageItem(index.row());

    if (m->message().type() == MetaTranslatorMessage::Obsolete)
        return;

    if (m->danger())
        printDanger(m);

    if (!m->danger() && !m->finished())
        updateFinished(true);
    else if (m->finished())
        updateFinished(false);

    Q_UNUSED(button);
}

int TrWindow::findCurrentContextRow()
{
     //a better way to do this (?)
    int strt = 0;

    for (strt = 0; strt < cmdl->contextsInList(); ++strt) {
        if (tv->selectionModel()->isRowSelected(strt, QModelIndex()))
            return strt;
    }

    //if no context is selected
    setCurrentContextRow(0);
    return 0;
}

int TrWindow::findCurrentMessageRow()
{
     //a better way to do this (?)
    int strt;
    ContextItem *cntxt = mmdl->contextItem();

    if (cntxt == 0)
        return -2;

    for (strt = 0; strt<cntxt->messageItemsInList(); ++strt) {
        if (stv->selectionModel()->isRowSelected(strt, QModelIndex()))
            return strt;
    }

    //if no message is selected, select the first one.. if it exists
    if (cntxt->messageItemsInList() <= 0)
        return -2; //no messages in this context

    setCurrentMessageRow(0);
    return -1; // so that the next message will be 0
}

bool TrWindow::setNextContext(int *currentrow, bool checkUnfinished)
{
    QModelIndex mindx;
    ++(*currentrow);

    for (; *currentrow < cmdl->contextsInList(); ++(*currentrow)) {
        if (!checkUnfinished) {
            setCurrentContextRow(*currentrow);
            return true; //it is one more item
        }

        mindx = cmdl->index(*currentrow, 0);
        if (cmdl->contextItem(mindx)->unfinished() > 0) {
            setCurrentContext(mindx);
            return true; // found a unfinished context
        }
    }

    return false; // sorry, you are done :)
}

bool TrWindow::setPrevContext(int *currentrow, bool checkUnfinished)
{
    QModelIndex mindx;
    --(*currentrow);

    for (; *currentrow >= 0; --(*currentrow)) {
        if (!checkUnfinished) {
            setCurrentContextRow(*currentrow);
            return true; //it is one more item
        }

        mindx = cmdl->index(*currentrow, 0);
        if (cmdl->contextItem(mindx)->unfinished() > 0) {
            setCurrentContext(mindx);
            return true; // found a unfinished context
        }
    }

    return false; // sorry, you are done :)
}

bool TrWindow::setNextMessage(int *currentrow, bool checkUnfinished)
{
    QModelIndex mindx;
    ContextItem *cntxt = mmdl->contextItem();
    ++(*currentrow);

    for (; *currentrow < cntxt->messageItemsInList(); ++(*currentrow)) {
        if (!checkUnfinished) {
            setCurrentMessageRow(*currentrow);
            return true; //it is one more item
        }

        mindx = mmdl->index(*currentrow, 0);
        if (!cntxt->messageItem(mindx.row())->finished())
        {
            setCurrentMessage(mindx);
            return true; // found a unfinished message
        }
    }

    return false; // sorry, you are done in this context :)
}

bool TrWindow::setPrevMessage(int *currentrow, bool checkUnfinished)
{
    QModelIndex mindx;
    ContextItem *cntxt = mmdl->contextItem();
    --(*currentrow);

    for (; *currentrow >= 0; --(*currentrow)) {
        if (!checkUnfinished) {
            setCurrentMessageRow(*currentrow);
            return true; //it is one more item
        }

        mindx = mmdl->index(*currentrow, 0);
        if (!cntxt->messageItem(mindx.row())->finished())
        {
            setCurrentMessage(mindx);
            return true; // found a unfinished message
        }
    }

    return false; // sorry, you are done in this context :)
}

void TrWindow::nextUnfinished()
{
    if (nextUnfinishedAct->isEnabled()) {
        if (!next(true)) {
            // If no Unfinished message is left, the user has finished the job.  We
            // congratulate on a job well done with this ringing bell.
            statusBar()->message(tr("No untranslated phrases left."), MessageMS);
            qApp->beep();
        }
    }
}

void TrWindow::prevUnfinished()
{
    if (nextUnfinishedAct->isEnabled()) {
        if (!prev(true)) {
            // If no Unfinished message is left, the user has finished the job.  We
            // congratulate on a job well done with this ringing bell.
            statusBar()->message(tr("No untranslated phrases left."), MessageMS);
            qApp->beep();
        }
    }
}

void TrWindow::prev()
{
    if(prev(false))
        stv->ensureItemVisible(stv->currentIndex());
}

bool TrWindow::prev(bool checkUnfinished)
{
    int curContext = findCurrentContextRow();
    int curMessage = findCurrentMessageRow();

    if ((curMessage != -2) && setPrevMessage(&curMessage, checkUnfinished))
        return true; // found it!

    // search the other contexts
    while (setPrevContext(&curContext, checkUnfinished)) {
        curMessage = mmdl->contextItem()->messageItemsInList();
        if (setPrevMessage(&curMessage, checkUnfinished))
            return true; // found it!
    }

    // search all the messages in all the contexts, from bottom
    curContext = cmdl->contextsInList();
    while (setPrevContext(&curContext, checkUnfinished)) {
        curMessage = mmdl->contextItem()->messageItemsInList();
        if (setPrevMessage(&curMessage, checkUnfinished))
            return true; // found it!
    }

    return false;
}

bool TrWindow::next(bool checkUnfinished)
{
    int curContext = findCurrentContextRow();
    int curMessage = findCurrentMessageRow();

    if ((curMessage != -2) && setNextMessage(&curMessage, checkUnfinished))
        return true; // found it!

    // search the other contexts
    while (setNextContext(&curContext, checkUnfinished)) {
        curMessage = -1;
        if (setNextMessage(&curMessage, checkUnfinished))
            return true; // found it!
    }

    // search all the messages in all the contexts, from top
    curContext = -1;
    while (setNextContext(&curContext, checkUnfinished)) {
        curMessage = -1;
        if (setNextMessage(&curMessage, checkUnfinished))
            return true; // found it!
    }

    return false;
}

void TrWindow::next()
{
    next(false);
}


void TrWindow::findNext(const QString &text, int where, bool matchCase)
{
    findText = text;
    if (findText.isEmpty())
        findText = QString("magicwordthatyoushouldavoid");
    findWhere = where;
    findMatchCase = matchCase;
    findAgainAct->setEnabled(true);
    findAgain();
}

void TrWindow::revalidate()
{
    if (cmdl->contextsInList() <= 0)
        return;

    ContextItem *c;
    MessageItem *m;

    for (int ci=0; ci<cmdl->contextsInList(); ++ci) {
        c = cmdl->contextItem(cmdl->index(ci, 0));
        for (int mi=0; mi<c->messageItemsInList(); ++mi) {
            m = c->messageItem(mi);
            updateDanger(m);
            if (mmdl->contextItem() == c)
                mmdl->updateItem(mmdl->index(mi, 0));
        }
        cmdl->updateItem(cmdl->index(ci, 0));
    }
}

QString TrWindow::friendlyString(const QString& str)
{
    QString f = str.toLower();
    f.replace(QRegExp(QString("[.,:;!?()-]")), QString(" "));
    f.replace("&", QString(""));
    f = f.simplified();
    f = f.toLower();
    return f;
}

void TrWindow::setupMenuBar()
{
    QMenuBar *m = menuBar();
    QMenu *filep = new QMenu(this);
    QMenu *editp  = new QMenu(this);
    QMenu *translationp = new QMenu(this);
    QMenu *validationp = new QMenu(this);
    validationp->setCheckable(true);
    phrasep = new QMenu(this);
    closePhraseBookp = new QMenu(this);
    editPhraseBookp = new QMenu(this);
    printPhraseBookp = new QMenu(this);
    QMenu *viewp = new QMenu(this);
    viewp->setCheckable(true);
    QMenu *helpp = new QMenu(this);

    m->addMenu(filep)->setText(tr("&File"));
    m->addMenu(editp)->setText(tr("&Edit"));
    m->addMenu(translationp)->setText(tr("&Translation"));
    m->addMenu(validationp)->setText(tr("V&alidation"));
    m->addMenu(phrasep)->setText(tr("&Phrases"));
    m->addMenu(viewp)->setText(tr("&View"));
    m->addMenu(helpp)->setText(tr("&Help"));

    connect(closePhraseBookp, SIGNAL(triggered(QAction *)),
        this, SLOT(closePhraseBook(QAction *)));
    connect(editPhraseBookp, SIGNAL(triggered(QAction *)),
        this, SLOT(editPhraseBook(QAction *)));
    connect(printPhraseBookp, SIGNAL(triggered(QAction *)),
        this, SLOT(printPhraseBook(QAction *)));

    // File menu
    openAct = filep->addAction(loadPixmap("fileopen.png"), 
        tr("&Open..."), this, SLOT(open()));
    openAct->setShortcut(QKeySequence("Ctrl+O"));
    filep->addSeparator();
    saveAct = filep->addAction(loadPixmap("filesave.png"), 
        tr("&Save"), this, SLOT(save()));
    saveAct->setShortcut(QKeySequence("Ctrl+S"));
    saveAsAct = filep->addAction(tr("Save &As..."), this, SLOT(saveAs()));
    releaseAct = filep->addAction(tr("&Release..."), this, SLOT(release()));
    filep->addSeparator();
    printAct = filep->addAction(loadPixmap("print.png"),
        tr("&Print..."), this, SLOT(print()));
    printAct->setShortcut(QKeySequence("Ctrl+P"));
    filep->addSeparator();

    recentFilesMenu = new QMenu(this);
    filep->addMenu(recentFilesMenu)->setText(tr("Re&cently opened files"));
    connect(recentFilesMenu, SIGNAL(aboutToShow()), this, 
        SLOT(setupRecentFilesMenu()));
    connect(recentFilesMenu, SIGNAL(triggered(QAction *)), this,
        SLOT(recentFileActivated(QAction *)));

    filep->addSeparator();

    exitAct = filep->addAction(tr("E&xit"), this, SLOT(close()));
    exitAct->setShortcut(QKeySequence("Ctrl+Q"));
    // Edit menu
    undoAct = editp->addAction(loadPixmap("undo.png"), tr("&Undo"), me, SLOT(undo()));
    undoAct->setShortcut(QKeySequence("Ctrl+Z"));
    undoAct->setEnabled(false);
    connect(me, SIGNAL(undoAvailable(bool)), undoAct, SLOT(setEnabled(bool)));
    redoAct = editp->addAction(loadPixmap("redo.png"), tr("&Redo"), me, SLOT(redo()));
    redoAct->setShortcut(QKeySequence("Ctrl+Y"));
    redoAct->setEnabled(false);
    connect(me, SIGNAL(redoAvailable(bool)), redoAct, SLOT(setEnabled(bool)));
    editp->addSeparator();
    cutAct = editp->addAction(loadPixmap("editcut.png"), tr("Cu&t"), me, SLOT(cut()));
    cutAct->setShortcut(QKeySequence("Ctrl+X"));
    cutAct->setEnabled(false);
    connect(me, SIGNAL(cutAvailable(bool)), cutAct, SLOT(setEnabled(bool)));
    copyAct = editp->addAction(loadPixmap("editcopy.png"), tr("&Copy"), me, SLOT(copy()));
    copyAct->setShortcut(QKeySequence("Ctrl+C"));
    copyAct->setEnabled(false);
    connect(me, SIGNAL(copyAvailable(bool)), copyAct, SLOT(setEnabled(bool)));
    pasteAct = editp->addAction(loadPixmap("editpaste.png"), tr("&Paste"), me, SLOT(paste()));
    pasteAct->setShortcut(QKeySequence("Ctrl+V"));
    pasteAct->setEnabled(false);
    connect(me, SIGNAL(pasteAvailable(bool)), pasteAct, SLOT(setEnabled(bool)));
    selectAllAct = editp->addAction(tr("Select &All"), me, SLOT(selectAll()));
    selectAllAct->setShortcut(QKeySequence("Ctrl+A"));
    selectAllAct->setEnabled(false);
    editp->addSeparator();
    findAct = editp->addAction(loadPixmap("searchfind.png"), tr("&Find..."), this, SLOT(find()));
    findAct->setShortcut(QKeySequence("Ctrl+F"));
    findAct->setEnabled(false);
    findAgainAct = editp->addAction(tr("Find &Next"), this, SLOT(findAgain()));
    findAgainAct->setShortcut(Qt::Key_F3);
    findAgainAct->setEnabled(false);

    // Translation menu
    // when updating the accelerators, remember the status bar
    prevUnfinishedAct = translationp->addAction(loadPixmap("prevunfinished.png"), 
        tr("&Prev Unfinished"), this, SLOT(prevUnfinished()));
    prevUnfinishedAct->setShortcut(QKeySequence("Ctrl+K"));
    nextUnfinishedAct = translationp->addAction(loadPixmap("nextunfinished.png"), 
        tr("&Next Unfinished"), this, SLOT(nextUnfinished()));
    nextUnfinishedAct->setShortcut(QKeySequence("Ctrl+L"));

    prevAct = translationp->addAction(loadPixmap("prev.png"), tr("P&rev"),
                          this, SLOT(prev()));
    prevAct->setShortcut(QKeySequence("Ctrl+Shift+K"));
    nextAct = translationp->addAction(loadPixmap("next.png"), tr("Ne&xt"),
                          this, SLOT(next()));
    nextAct->setShortcut(QKeySequence("Ctrl+Shift+L"));
    doneAndNextAct = translationp->addAction(loadPixmap("doneandnext.png"), tr("Done and &Next"),
                                 this, SLOT(doneAndNext()));
    doneAndNextAct->setShortcut(QKeySequence("Ctrl+Enter"));
    doneAndNextAct->setEnabled(false);

    doneAndNextAlt = new QAction(this);
    doneAndNextAlt->setShortcut(QKeySequence("Ctrl+Return"));
    doneAndNextAlt->setEnabled(false);
    connect(doneAndNextAlt, SIGNAL(triggered()), this, SLOT(doneAndNext()));

    beginFromSourceAct = translationp->addAction(tr("&Begin from Source"),
        me, SLOT(beginFromSource()));
    beginFromSourceAct->setShortcut(QKeySequence("Ctrl+B"));
    beginFromSourceAct->setEnabled(false);
    connect(me, SIGNAL(updateActions(bool)), beginFromSourceAct, SLOT(setEnabled(bool)));

    // Phrasebook menu
    newPhraseBookAct = phrasep->addAction(tr("&New Phrase Book..."),
        this, SLOT(newPhraseBook()));
    newPhraseBookAct->setShortcut(QKeySequence("Ctrl+N"));
    openPhraseBookAct = phrasep->addAction(loadPixmap("book.png"), tr("&Open Phrase Book..."),
        this, SLOT(openPhraseBook()));
    openPhraseBookAct->setShortcut(QKeySequence("Ctrl+H"));
    closePhraseBookId = phrasep->addMenu(closePhraseBookp);
    closePhraseBookId->setText(tr("&Close Phrase Book"));
    phrasep->addSeparator();
    editPhraseBookId = phrasep->addMenu(editPhraseBookp);
    editPhraseBookId->setText(tr("&Edit Phrase Book..."));
    printPhraseBookId = phrasep->addMenu(printPhraseBookp);
    printPhraseBookId->setText(tr("&Print Phrase Book..."));
    connect(phrasep, SIGNAL(aboutToShow()), this, SLOT(setupPhrase()));

    // Validation menu
    acceleratorsAct = validationp->addAction(loadPixmap("accelerator.png"), tr("&Accelerators"),
        this, SLOT(revalidate()));
    acceleratorsAct->setCheckable(true);
    acceleratorsAct->setChecked(true);
    endingPunctuationAct = validationp->addAction(loadPixmap("punctuation.png"), tr("&Ending Punctuation"),
        this, SLOT(revalidate()));
    endingPunctuationAct->setCheckable(true);
    endingPunctuationAct->setChecked(true);
    phraseMatchesAct = validationp->addAction(loadPixmap("phrase.png"), tr("&Phrase Matches"),
        this, SLOT(revalidate()));
    phraseMatchesAct->setCheckable(true);
    phraseMatchesAct->setChecked(true);

    // View menu
    revertSortingAct = viewp->addAction(tr("&Revert Sorting"),
                                   this, SLOT(revertSorting()));
    doGuessesAct = viewp->addAction(tr("&Display guesses"),
                               this, SLOT(toggleGuessing()));
    doGuessesAct->setCheckable(true);
    doGuessesAct->setChecked(true);
    toggleStats = viewp->addAction(tr("&Statistics"), this, SLOT(toggleStatistics()));
    toggleStats->setCheckable(true);
    viewp->addSeparator();

    tbMenu = new QMenu(this);
    QMenu *dwMenu = new QMenu(this);
    dwMenu->addAction(dwScope->toggleViewAction());
    dwMenu->addAction(me->sourceDockWnd()->toggleViewAction());
    dwMenu->addAction(me->phraseDockWnd()->toggleViewAction());

    viewp->addMenu(tbMenu)->setText(tr("&Toolbars"));
    viewp->addMenu(dwMenu)->setText(tr("Vie&ws"));

    connect(viewp, SIGNAL(aboutToShow()), this, 
        SLOT(updateViewMenu()));

    // Help
    manualAct = helpp->addAction(tr("&Manual"), this, SLOT(manual()));
    manualAct->setShortcut(Qt::Key_F1);
    helpp->addSeparator();
    aboutAct = helpp->addAction(tr("&About"), this, SLOT(about()));
    aboutQtAct = helpp->addAction(tr("About &Qt"), this, SLOT(aboutQt()));
    helpp->addSeparator();

    whatsThisAct = helpp->addAction(loadPixmap("whatsthis.xpm"), tr("&What's This?"),
                               this, SLOT(onWhatsThis()));

    whatsThisAct->setShortcut(Qt::SHIFT + Qt::Key_F1);

    openAct->setWhatsThis(tr("Open a Qt translation source file (TS file) for"
        " editing."));
    saveAct->setWhatsThis(tr("Save changes made to this Qt translation "
        "source file."));
    saveAsAct->setWhatsThis(tr("Save changes made to this Qt translation"
        "source file into a new file."));
    releaseAct->setWhatsThis(tr("Create a Qt message file suitable for"
        " released applications"
        " from the current message file."));
    printAct->setWhatsThis(tr("Print a list of all the phrases in the current"
        " Qt translation source file."));
    exitAct->setWhatsThis(tr("Close this window and exit."));

    undoAct->setWhatsThis(tr("Undo the last editing operation performed on the"
        " translation."));
    redoAct->setWhatsThis(tr("Redo an undone editing operation performed on"
        " the translation."));
    cutAct->setWhatsThis(tr("Copy the selected translation text to the"
        " clipboard and deletes it."));
    copyAct->setWhatsThis(tr("Copy the selected translation text to the"
        " clipboard."));
    pasteAct->setWhatsThis(tr("Paste the clipboard text into the"
        " translation.") );
    selectAllAct->setWhatsThis( tr("Select the whole translation text."));
    findAct->setWhatsThis(tr("Search for some text in the translation "
        "source file.") );
    findAgainAct->setWhatsThis(tr("Continue the search where it was left."));

    newPhraseBookAct->setWhatsThis(tr("Create a new phrase book."));
    openPhraseBookAct->setWhatsThis(tr("Open a phrase book to assist"
        " translation."));
    acceleratorsAct->setWhatsThis(tr("Toggle validity checks of"
        " accelerators."));
    endingPunctuationAct->setWhatsThis(tr("Toggle validity checks"
        " of ending punctuation."));
    phraseMatchesAct->setWhatsThis(tr("Toggle checking that phrase"
        " suggestions are used."));

    revertSortingAct->setWhatsThis(tr("Sort the items back in the same order"
        " as in the message file."));

    doGuessesAct->setWhatsThis(tr("Set whether or not to display translation guesses."));
    manualAct->setWhatsThis(tr("Display the manual for %1.").arg(tr("Qt Linguist")));
    aboutAct->setWhatsThis(tr("Display information about %1.").arg(tr("Qt Linguist")));
    aboutQtAct->setWhatsThis(tr("Display information about the Qt toolkit by"
        " Trolltech."));
    whatsThisAct->setWhatsThis(tr("Enter What's This? mode."));

    beginFromSourceAct->setWhatsThis(tr("Copies the source text into"
        " the translation field."));
    nextAct->setWhatsThis(tr("Moves to the next item."));
    prevAct->setWhatsThis(tr("Moves to the previous item."));
    nextUnfinishedAct->setWhatsThis(tr("Moves to the next unfinished item."));
    prevUnfinishedAct->setWhatsThis(tr("Moves to the previous unfinished item."));
    doneAndNextAct->setWhatsThis(tr("Marks this item as done and moves to the"
        " next unfinished item."));
    doneAndNextAlt->setWhatsThis(doneAndNextAct->whatsThis());
}

void TrWindow::updateViewMenu()
{
    if (stats)
        toggleStats->setChecked(stats->isVisible());
    else
        toggleStats->setChecked(false);
}

void TrWindow::onWhatsThis()
{
    QWhatsThis::enterWhatsThisMode();
}

void TrWindow::setupToolBars()
{
    QToolBar *filet = new QToolBar(this);
    filet->setWindowTitle(tr("File"));
	this->addToolBar(filet);
    tbMenu->addAction(filet->toggleViewAction());
    
    QToolBar *editt = new QToolBar(this);
    editt->setWindowTitle(tr("Edit"));
	this->addToolBar(editt);
    tbMenu->addAction(editt->toggleViewAction());

    QToolBar *translationst = new QToolBar(this);
    translationst->setWindowTitle(tr("Translation"));
	this->addToolBar(translationst);
    tbMenu->addAction(translationst->toggleViewAction());

    QToolBar *validationt   = new QToolBar(this);
    validationt->setWindowTitle(tr("Validation"));
	this->addToolBar(validationt);
    tbMenu->addAction(validationt->toggleViewAction());

    QToolBar *helpt = new QToolBar(this);
    helpt->setWindowTitle(tr("Help"));
	this->addToolBar(helpt);
    tbMenu->addAction(helpt->toggleViewAction());

    filet->addAction(openAct);
    filet->addAction(saveAct);
    filet->addAction(printAct);
    filet->addSeparator();
    filet->addAction(openPhraseBookAct);

    editt->addAction(undoAct);
    editt->addAction(redoAct);
    editt->addSeparator();
    editt->addAction(cutAct);
    editt->addAction(copyAct);
    editt->addAction(pasteAct);
    editt->addSeparator();
    editt->addAction(findAct);
    
    translationst->addAction(prevAct);
    translationst->addAction(nextAct);
    translationst->addAction(prevUnfinishedAct);
    translationst->addAction(nextUnfinishedAct);
    translationst->addAction(doneAndNextAct);

    validationt->addAction(acceleratorsAct);
    validationt->addAction(endingPunctuationAct);
    validationt->addAction(phraseMatchesAct);

    helpt->addAction(whatsThisAct);
}

void TrWindow::setCurrentContext(const QModelIndex &indx)
{
    tv->setCurrentIndex(indx);
    tv->ensureItemVisible(indx);
}

void TrWindow::setCurrentContextRow(int row)
{
    QModelIndex mdlI = cmdl->index(row,1);
    tv->setCurrentIndex(mdlI);
    tv->ensureItemVisible(mdlI);
}

void TrWindow::setCurrentMessage(const QModelIndex &indx)
{
    stv->setCurrentIndex(indx);
    stv->ensureItemVisible(indx);
}

void TrWindow::setCurrentMessageRow(int row)
{
    QModelIndex mdlI = mmdl->index(row,1);
    stv->setCurrentIndex(mdlI);
    stv->ensureItemVisible(mdlI);
}

QString TrWindow::friendlyPhraseBookName(const PhraseBook &pb) const
{
    return QFileInfo(pb.fileName()).fileName();
}

bool TrWindow::openPhraseBook(const QString& name)
{
    PhraseBook pb;
    if (!pb.load(name)) {
        QMessageBox::warning(this, tr("Qt Linguist"),
            tr("Cannot read from phrase book '%1'.").arg(name));
        return false;
    }

    QAction *a = closePhraseBookp->addAction(friendlyPhraseBookName(pb));
    phraseBooks[PhraseCloseMenu].insert(a, pb);
    a->setWhatsThis(tr("Close this phrase book."));

    a = editPhraseBookp->addAction(friendlyPhraseBookName(pb));
    phraseBooks[PhraseEditMenu].insert(a, pb);
    a->setWhatsThis(tr("Allow you to add, modify, or delete"
        " phrases of this phrase book."));
    a = printPhraseBookp->addAction(friendlyPhraseBookName(pb));
    phraseBooks[PhrasePrintMenu].insert(a, pb);
    a->setWhatsThis(tr("Print the entries of the phrase"
        " book."));
    updatePhraseDict();
    return true;
}

bool TrWindow::savePhraseBook(QString &name, const PhraseBook &pb)
{
    if (!name.contains(".qph") && !name.contains("."))
        name += ".qph";

    if (!pb.save(name)) {
        QMessageBox::warning(this, tr("Qt Linguist"),
            tr("Cannot create phrase book '%1'.").arg(name));
        return false;
    }
    return true;
}

void TrWindow::updateProgress()
{
    if (numNonobsolete == 0)
        progress->setText(QString("    " "    "));
    else
        progress->setText(QString(" %1/%2 ").arg(numFinished)
        .arg(numNonobsolete));
    prevUnfinishedAct->setEnabled(numFinished != numNonobsolete);
    nextUnfinishedAct->setEnabled(numFinished != numNonobsolete);

    prevAct->setEnabled(cmdl->contextsInList() > 0);
    nextAct->setEnabled(cmdl->contextsInList() > 0);
}

void TrWindow::updatePhraseDict()
{
    phraseDict.clear();

    foreach (PhraseBook pb, phraseBooks[PhraseCloseMenu]) {
        foreach (Phrase p, pb) {
            QString f = friendlyString(p.source());
            if ( f.length() > 0 ) {
                f = f.split(QChar(' ')).first();
                if (!phraseDict.contains(f)) {
                    PhraseBook pbe;
                    phraseDict.insert(f, pbe);
                }
                phraseDict[f].append(p);
            }
        }
    }
    revalidate();
}

PhraseBook TrWindow::getPhrases(const QString &source)
{
    PhraseBook phrases;
    QString f = friendlyString(source);
    QStringList lookupWords = f.split(QChar(' '));

    foreach (QString s, lookupWords) {
        if (phraseDict.contains(s)) {
            PhraseBook ent = phraseDict.value(s);
            foreach (Phrase p, ent) {
                if (f.indexOf(friendlyString((p).source())) >= 0)
                    phrases.append(p);
            }
        }
    }
    return phrases;
}

void TrWindow::printDanger(MessageItem *m)
{
    danger(m->sourceText(), m->translation(), true);
}

bool TrWindow::updateDanger(MessageItem *m, bool verbose)
{
    bool dngr = danger(m->sourceText(), m->translation(), verbose);

    if (dngr != m->danger())
        m->setDanger(dngr);

    return dngr;
}

bool TrWindow::danger( const QString& source, const QString& translation,
                       bool verbose )
{
    if (acceleratorsAct->isChecked()) {
        bool sk = source.contains(Qt::Key_Ampersand);
        bool tk = translation.contains(Qt::Key_Ampersand);

        if (!sk && tk) {
            if (verbose)
                statusBar()->message(tr("Accelerator possibly superfluous in"
                                         " translation."), ErrorMS);
            return true;
        } else if (sk && !tk) {
            if (verbose)
                statusBar()->message(tr("Accelerator possibly missing in"
                                         " translation."), ErrorMS);
            return true;
        }
    }
    if (endingPunctuationAct->isChecked()) {
        if (ending(source) != ending(translation)) {
            if (verbose)
                statusBar()->message(tr("Translation does not end with the"
                    " same punctuation as the source text."), ErrorMS);
            return true;
        }
    }
    if (phraseMatchesAct->isChecked()) {
        QString fsource = friendlyString(source);
        QString ftranslation = friendlyString(translation);
        QStringList lookupWords = fsource.split(QChar(' '));

        bool phraseFound;
        foreach (QString s, lookupWords) {
            if (phraseDict.contains(s)) {
                PhraseBook ent = phraseDict.value(s);
                phraseFound = false;
                foreach (Phrase p, ent) {
                    if (fsource.indexOf(friendlyString(p.source())) < 0 ||
                        ftranslation.indexOf(friendlyString(p.target())) >= 0) {
                        phraseFound = true;
                        break;
                    }
                }
                if (!phraseFound) {
                    if (verbose)
                        statusBar()->message(tr("A phrase book suggestion for"
                            " '%1' was ignored.").arg(s), ErrorMS );
                    return true;
                }
            }
        }
    }
    if (verbose)
        statusBar()->clear();

    return false;
}

void TrWindow::readConfig()
{
    QString keybase("/Qt Linguist/" +
        QString::number((QT_VERSION >> 16) & 0xff) +
        "." + QString::number((QT_VERSION >> 8) & 0xff) + "/");
    QSettings config("Trolltech", "Linguist");

    QRect r( pos(), size() );
    recentFiles = config.value(keybase + "RecentlyOpenedFiles").toStringList();
    if ( !config.value(keybase + "Geometry/MainwindowMaximized", false).toBool()) {
        r.setX(config.value(keybase + "Geometry/MainwindowX", r.x()).toInt());
        r.setY(config.value(keybase + "Geometry/MainwindowY", r.y()).toInt());
        r.setWidth(config.value(keybase + "Geometry/MainwindowWidth", r.width()).toInt());
        r.setHeight(config.value(keybase + "Geometry/MainwindowHeight", r.height()).toInt());

        QRect desk = QApplication::desktop()->geometry();
        QRect inter = desk.intersect(r);
        resize( r.size() );
        if ( inter.width() * inter.height() > ( r.width() * r.height() / 20 ) ) {
            move( r.topLeft() );
        }
    }

    //TODO
    // ### enable me
/*    QDockWindow * dw;
    dw = (QDockWindow *) lv->parent();
    int place;
    place = config.readNumEntry( keybase + "Geometry/ContextwindowInDock" );
    r.setX( config.readNumEntry( keybase + "Geometry/ContextwindowX" ) );
    r.setY( config.readNumEntry( keybase + "Geometry/ContextwindowY" ) );
    r.setWidth( config.readNumEntry( keybase +
                                     "Geometry/ContextwindowWidth" ) );
    r.setHeight( config.readNumEntry( keybase +
                                      "Geometry/ContextwindowHeight" ) );
    if ( place == QDockWindow::OutsideDock ) {
        dw->undock();
        dw->show();
    }
    dw->setGeometry( r );

    dw = (QDockWindow *) slv->parent();
    place = config.readNumEntry( keybase + "Geometry/SourcewindowInDock" );
    r.setX( config.readNumEntry( keybase + "Geometry/SourcewindowX" ) );
    r.setY( config.readNumEntry( keybase + "Geometry/SourcewindowY" ) );
    r.setWidth( config.readNumEntry( keybase +
                                     "Geometry/SourcewindowWidth" ) );
    r.setHeight( config.readNumEntry( keybase +
                                      "Geometry/SourcewindowHeight" ) );
    if ( place == QDockWindow::OutsideDock ) {
        dw->undock();
        dw->show();
    }
    dw->setGeometry( r );

    dw = (QDockWindow *) plv->parent()->parent();
    place = config.readNumEntry( keybase + "Geometry/PhrasewindowInDock" );
    r.setX( config.readNumEntry( keybase + "Geometry/PhrasewindowX" ) );
    r.setY( config.readNumEntry( keybase + "Geometry/PhrasewindowY" ) );
    r.setWidth( config.readNumEntry( keybase +
                                     "Geometry/PhrasewindowWidth" ) );
    r.setHeight( config.readNumEntry( keybase +
                                      "Geometry/PhrasewindowHeight" ) );
    if ( place == QDockWindow::OutsideDock ) {
        dw->undock();
        dw->show();
    }
    dw->setGeometry( r );*/

    QApplication::sendPostedEvents();
}

void TrWindow::writeConfig()
{
    QString keybase( "/Qt Linguist/" +
                     QString::number( (QT_VERSION >> 16) & 0xff ) +
                     "." + QString::number( (QT_VERSION >> 8) & 0xff ) + "/" );
    QSettings config("Trolltech", "Linguist");

    config.setValue(keybase + "RecentlyOpenedFiles", recentFiles);
    config.setValue(keybase + "Geometry/MainwindowMaximized", isMaximized());
    config.setValue(keybase + "Geometry/MainwindowX", x());
    config.setValue(keybase + "Geometry/MainwindowY", y());
    config.setValue(keybase + "Geometry/MainwindowWidth", width());
    config.setValue(keybase + "Geometry/MainwindowHeight", height());

    //TODO
    // ### enable me
/*    QDockWindow * dw =(QDockWindow *) lv->parent();
    config.writeEntry( keybase + "Geometry/ContextwindowInDock", dw->place() );
    config.writeEntry( keybase + "Geometry/ContextwindowX", dw->x() );
    config.writeEntry( keybase + "Geometry/ContextwindowY", dw->y() );
    config.writeEntry( keybase + "Geometry/ContextwindowWidth", dw->width() );
    config.writeEntry( keybase + "Geometry/ContextwindowHeight", dw->height() );

    dw =(QDockWindow *) slv->parent();
    config.writeEntry( keybase + "Geometry/SourcewindowInDock",
                       dw->place() );
    config.writeEntry( keybase + "Geometry/SourcewindowX", dw->geometry().x() );
    config.writeEntry( keybase + "Geometry/SourcewindowY", dw->geometry().y() );
    config.writeEntry( keybase + "Geometry/SourcewindowWidth", dw->width() );
    config.writeEntry( keybase + "Geometry/SourcewindowHeight", dw->height() );

    dw =(QDockWindow *) plv->parent()->parent();
    config.writeEntry( keybase + "Geometry/PhrasewindowInDock",
                       dw->place() );
    config.writeEntry( keybase + "Geometry/PhrasewindowX", dw->geometry().x() );
    config.writeEntry( keybase + "Geometry/PhrasewindowY", dw->geometry().y() );
    config.writeEntry( keybase + "Geometry/PhrasewindowWidth", dw->width() );
    config.writeEntry( keybase + "Geometry/PhrasewindowHeight", dw->height() );*/
}

void TrWindow::setupRecentFilesMenu()
{
    recentFilesMenu->clear();
    QStringList::Iterator it = recentFiles.begin();
    for (; it != recentFiles.end(); ++it) {
        recentFilesMenu->addAction(*it);
    }
}

void TrWindow::recentFileActivated(QAction *action)
{
    if (!action->text().isEmpty()) {
        if (maybeSave())
            openFile(action->text());
    }
}

void TrWindow::addRecentlyOpenedFile(const QString &fn, QStringList &lst)
{
    if (lst.contains(fn))
        return;
    if ( lst.count() >= 10 )
        lst.removeAt(0);
    lst << fn;
}

void TrWindow::toggleGuessing()
{
    me->toggleGuessing();
}

void TrWindow::focusSourceList()
{
    stv->setFocus();
}

void TrWindow::focusPhraseList()
{
    ptv->setFocus();
}

void TrWindow::toggleStatistics()
{
    if (toggleStats->isChecked()) {
        if (!stats) {
            stats = new Statistics(this);
            connect(this, SIGNAL(statsChanged(int,int,int,int,int,int)), stats,
                SLOT(updateStats(int,int,int,int,int,int)));
        }
        stats->show();
        updateStatistics();
    } 
    else if (stats) {
        stats->close();
    }
}

void TrWindow::updateStatistics()
{
    // don't call this if stats dialog is not open
    // because this can be slow...
    if (!stats || !stats->isVisible())
        return;
    
    QList<ContextItem *> ctxtList;
    QList<MessageItem *> msgList;
    const MessageItem *mi;
    int trW = 0;
    int trC = 0;
    int trCS = 0;

    ctxtList = cmdl->contextList();

    for (int i=0; i<ctxtList.count(); i++) {
        msgList = ctxtList.at(i)->messageItemList();
        for (int j=0; j<msgList.count(); j++) {
            mi = msgList.at(j);
            if (mi->finished() && !(mi->message().type() == MetaTranslatorMessage::Obsolete))
                doCharCounting(mi->translation(), trW, trC, trCS);
        }
    }

    emit statsChanged(srcWords, srcChars, srcCharsSpc, trW, trC, trCS);
}

void TrWindow::doCharCounting(const QString& text, int& trW, int& trC, int& trCS)
{
    trCS += text.length();
    bool inWord = false;
    for (int i=0; i<(int)text.length(); ++i) {
        if (text[i].isLetterOrNumber() || text[i] == QChar('_')) {
            if (!inWord) {
                ++trW;
                inWord = true;
            }
        } else {
            inWord = false;
        }
        if (!text[i].isSpace())
            trC++;
    }
}

QIcon TrWindow::loadPixmap(const QString &imageName)
{
    if (!imageName.isEmpty()) {
        QPixmap enabledPix(":/images/" + imageName);

        QIcon s(enabledPix);
        if (imageName != QLatin1String("whatsthis.xpm")) {
            QPixmap disabledPix(":/images/d_" + imageName);
            s.setPixmap(disabledPix, Qt::SmallIconSize, QIcon::Disabled);
        }
        return s;
	}

    return QIcon();
}

