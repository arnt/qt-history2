/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of Qt Linguist.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/*  TRANSLATOR TrWindow

  This is the application's main window.
*/

#include "trwindow.h"
#include "listviews.h"
#include "finddialog.h"
#include "msgedit.h"
#include "phrasebookbox.h"
#include "printout.h"
#include "about.h"
#include "phraselv.h"
#include "statistics.h"

#include <qaccel.h>
#include <qaction.h>
#include <qapplication.h>
#include <qbitmap.h>
#include <qhash.h>
#include <qdockarea.h>
#include <qdockwindow.h>
#include <qfile.h>
#include <qfiledialog.h>
#include <qfileinfo.h>
#include <qheader.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qmenu.h>
#include <qregexp.h>
#include <qsettings.h>
#include <qstatusbar.h>
#include <qtoolbar.h>
#include <qwhatsthis.h>
#include <qassistantclient.h>
#include <qdesktopwidget.h>
#include <qassistantclient.h>

#include <stdlib.h>

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

QPixmap * TrWindow::pxOn = 0;
QPixmap * TrWindow::pxOff = 0;
QPixmap * TrWindow::pxObsolete = 0;
QPixmap * TrWindow::pxDanger = 0;

enum Ending { End_None, End_FullStop, End_Interrobang, End_Colon,
              End_Ellipsis };

static Ending ending( QString str )
{
    str = str.simplified();
    int ch = 0;
    if ( !str.isEmpty() )
        ch = str.right( 1 )[0].unicode();

    switch ( ch ) {
    case 0x002e: // full stop
        if ( str.endsWith(QString("...")) )
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
    pixmap = QPixmap::fromMimeSource( "pagecurl.png" );
    if ( !pixmap.isNull() ) {
        QBitmap pageCurlMask( pagecurl_mask_width, pagecurl_mask_height,
                        pagecurl_mask_bits, TRUE );
        pixmap.setMask( pageCurlMask );
    }

    return pixmap;
}

TrWindow::TrWindow()
    : QMainWindow( 0, "translation window", WType_TopLevel | WDestructiveClose )
{

#ifndef Q_WS_MAC
    setWindowIcon( QPixmap::fromMimeSource( "appicon.png" ) );
#endif

    // Create the application global listview symbols
    pxOn  = new QPixmap( QPixmap::fromMimeSource( "s_check_on.png" ) );
    pxOff = new QPixmap( QPixmap::fromMimeSource( "s_check_off.png" ) );
    pxObsolete = new QPixmap( QPixmap::fromMimeSource( "d_s_check_obs.png" ) );
    pxDanger = new QPixmap( QPixmap::fromMimeSource( "s_check_danger.png" ) );

    // Set up the Scope dock window
    QDockWindow * dwScope = new QDockWindow( QDockWindow::InDock, this,
                                             "context");
    dwScope->setResizeEnabled( TRUE );
    dwScope->setCloseMode( QDockWindow::Always );
    addDockWindow( dwScope, tr("Context"), Qt::DockLeft );
    dwScope->setWindowTitle( tr("Context") );
    dwScope->setFixedExtentWidth( 200 );
    lv = new QListView( dwScope, "context list view" );
    lv->setShowSortIndicator( TRUE );
    lv->setAllColumnsShowFocus( TRUE );
    lv->header()->setStretchEnabled( TRUE, 1 );
    QFontMetrics fm( font() );
    lv->addColumn( tr("Done"), fm.width( tr("Done") ) + 10 );
    lv->addColumn( tr("Context") );
    lv->addColumn( tr("Items"), 55 );
    lv->setColumnAlignment( 0, Qt::AlignCenter );
    lv->setColumnAlignment( 2, Qt::AlignRight );
    lv->setSorting( 0 );
    lv->setHScrollBarMode( QScrollView::AlwaysOff );
    dwScope->setWidget( lv );

    messageIsShown = FALSE;
    me = new MessageEditor( &tor, this, "message editor" );
    setCentralWidget( me );
    slv = me->sourceTextList();
    plv = me->phraseList();

    setupMenuBar();
    setupToolBars();

    progress = new QLabel( statusBar(), "progress" );
    statusBar()->addWidget( progress, 0, TRUE );
    modified = new QLabel( QString(" %1 ").arg(tr("MOD")), statusBar(),
                           "modified?" );
    statusBar()->addWidget( modified, 0, TRUE );

    dirtyItem = -1;
    numFinished = 0;
    numNonobsolete = 0;
    numMessages = 0;
    updateProgress();

    dirty = FALSE;
    updateCaption();

    f = new FindDialog( FALSE, this, "find", FALSE );
    f->setWindowTitle( tr("Qt Linguist") );
    h = new FindDialog( TRUE, this, "replace", FALSE );
    h->setWindowTitle( tr("Qt Linguist") );
    findMatchCase = FALSE;
    findWhere = 0;
    foundItem = 0;
    foundScope = 0;
    foundWhere = 0;
    foundOffset = 0;

    connect( lv, SIGNAL(selectionChanged(QListViewItem *)),
             this, SLOT(showNewScope(QListViewItem *)) );

    connect( slv, SIGNAL(currentChanged(QListViewItem *)),
             this, SLOT(showNewCurrent(QListViewItem *)) );

    connect( slv, SIGNAL(clicked(QListViewItem *, const QPoint&, int)),
             this, SLOT(showNewCurrent(QListViewItem *)) );

    connect( slv, SIGNAL(clicked(QListViewItem *, const QPoint&, int)),
             this, SLOT(toggleFinished(QListViewItem *, const QPoint&, int)) );

    connect( me, SIGNAL(translationChanged(const QString&)),
             this, SLOT(updateTranslation(const QString&)) );
    connect( me, SIGNAL(finished(bool)), this, SLOT(updateFinished(bool)) );
    connect( me, SIGNAL(prevUnfinished()), this, SLOT(prevUnfinished()) );
    connect( me, SIGNAL(nextUnfinished()), this, SLOT(nextUnfinished()) );
    connect( me, SIGNAL(focusSourceList()), this, SLOT(focusSourceList()) );
    connect( me, SIGNAL(focusPhraseList()), this, SLOT(focusPhraseList()) );
    connect( f, SIGNAL(findNext(const QString&, int, bool)),
             this, SLOT(findNext(const QString&, int, bool)) );

    lv->setWhatsThis(tr("This panel lists the source contexts."));
    slv->setWhatsThis(tr("This panel lists the source texts. "
                         "Items that violate validation rules "
                         "are marked with a warning."));
    showNewCurrent( 0 );

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

void TrWindow::openFile( const QString& name )
{
    if ( !name.isEmpty() ) {
        statusBar()->message( tr("Loading...") );
        qApp->processEvents();
        tor.clear();
        if ( tor.load(name) ) {
            slv->clear();
            slv->repaint();
            slv->viewport()->repaint();
            slv->setUpdatesEnabled( FALSE );
            slv->viewport()->setUpdatesEnabled( FALSE );
            lv->clear();
            lv->repaint();
            lv->viewport()->repaint();
            lv->setUpdatesEnabled( FALSE );
            lv->viewport()->setUpdatesEnabled( FALSE );
            setEnabled( FALSE );
            numFinished = 0;
            numNonobsolete = 0;
            numMessages = 0;
            foundScope = 0;

            TML all = tor.messages();
            QHash<QString,ContextLVI*> contexts;

            srcWords = 0;
            srcChars = 0;
            srcCharsSpc = 0;
            foreach (MetaTranslatorMessage mtm, all) {
                qApp->processEvents();
                ContextLVI *c = 0;
                if (contexts.contains(QString(mtm.context())))
                    c = contexts.value( QString(mtm.context()) );
                else {
                    c = new ContextLVI( lv, tor.toUnicode(mtm.context(),
                                                          mtm.utf8()) );
                    contexts.insert( QString(mtm.context()), c );
                }
                if ( QByteArray(mtm.sourceText()) == ContextComment ) {
                    c->appendToComment( tor.toUnicode(mtm.comment(),
                                                      mtm.utf8()) );
                } else {
                    MessageLVI *tmp = new MessageLVI( slv, mtm,
                                           tor.toUnicode(mtm.sourceText(),
                                                         mtm.utf8()),
                                           tor.toUnicode(mtm.comment(),
                                                         mtm.utf8()), c );
                    tmp->setDanger( danger(tmp->sourceText(),
                                           tmp->translation()) &&
                                    tmp->message().type() ==
                                    MetaTranslatorMessage::Finished );
                    c->instantiateMessageItem( slv, tmp );
                    if ( mtm.type() != MetaTranslatorMessage::Obsolete ) {
                        numNonobsolete++;
                        if ( mtm.type() == MetaTranslatorMessage::Finished )
                            numFinished++;
                        doCharCounting( tmp->sourceText(), srcWords, srcChars, srcCharsSpc );
                    } else {
                        c->incrementObsoleteCount();
                    }
                    numMessages++;
                }
                c->updateStatus();
            }
            slv->viewport()->setUpdatesEnabled( TRUE );
            slv->setUpdatesEnabled( TRUE );
            lv->viewport()->setUpdatesEnabled( TRUE );
            lv->setUpdatesEnabled( TRUE );
            setEnabled( TRUE );
            slv->repaint();
            slv->viewport()->repaint();
            lv->triggerUpdate();
            updateProgress();
            filename = name;
            dirty = FALSE;
            updateCaption();
            me->showNothing();
            doneAndNextAct->setEnabled( FALSE );
            doneAndNextAlt->setEnabled( FALSE );
            messageIsShown = FALSE;
            statusBar()->message(
                    tr("%1 source phrase(s) loaded.").arg(numMessages),
                    MessageMS );

            foundItem = 0;
            foundWhere = 0;
            foundOffset = 0;
            if ( lv->childCount() > 0 ) {
                findAct->setEnabled( TRUE );
                findAgainAct->setEnabled( FALSE );
#ifdef notyet
                replaceAct->setEnabled( TRUE );
#endif
        lv->setCurrentItem( lv->firstChild() );
            }
            addRecentlyOpenedFile( name, recentFiles );
            updateStatistics();
        } else {
            statusBar()->clear();
            QMessageBox::warning( this, tr("Qt Linguist"),
                                  tr("Cannot open '%1'.").arg(name) );
        }
    }
}

void TrWindow::open()
{
    if ( maybeSave() ) {
        QString newFilename = QFileDialog::getOpenFileName( filename,
                tr("Qt translation source (*.ts)\n"
                   "All files (*)"),
                this, "open" );
        openFile( newFilename );
    }
}

void TrWindow::save()
{
    if ( filename.isEmpty() )
        return;

    if ( tor.save(filename) ) {
        dirty = FALSE;
        updateCaption();
        statusBar()->message( tr("File saved."), MessageMS );
    } else {
        QMessageBox::warning( this, tr("Qt Linguist"), tr("Cannot save '%1'.")
                              .arg(filename) );
    }
}

void TrWindow::saveAs()
{
    QString newFilename = QFileDialog::getSaveFileName( filename,
            tr( "Qt translation source (*.ts)\n"
                "All files (*)"),
            this, "save_as" );
    if ( !newFilename.isEmpty() ) {
        filename = newFilename;
        save();
        updateCaption();
    }
}

void TrWindow::release()
{
    QString newFilename = filename;
    newFilename.replace( QRegExp(".ts$"), "" );
    newFilename += QString( ".qm" );

    newFilename = QFileDialog::getSaveFileName( newFilename,
            tr("Qt message files for released applications (*.qm)\n"
               "All files (*)"),
            this, "release",
            tr("Release") );
    if ( !newFilename.isEmpty() ) {
        if ( tor.release(newFilename) )
            statusBar()->message( tr("File created."), MessageMS );
        else
            QMessageBox::warning( this, tr("Qt Linguist"),
                                  tr("Cannot save '%1'.").arg(newFilename) );
    }
}

void TrWindow::print()
{
    int pageNum = 0;

    if ( printer.setup(this) ) {
        QApplication::setOverrideCursor( WaitCursor );
        printer.setDocName( filename );
        statusBar()->message( tr("Printing...") );
        PrintOut pout( &printer );
        ContextLVI *c = (ContextLVI *) lv->firstChild();
        while ( c != 0 ) {
            setCurrentContextItem( c );
            pout.vskip();
            pout.setRule( PrintOut::ThickRule );
            pout.setGuide( c->context() );
            pout.addBox( 100, tr("Context: %1").arg(c->context()),
                         PrintOut::Strong );
            pout.flushLine();
            pout.addBox( 4 );
            pout.addBox( 92, c->comment(), PrintOut::Emphasis );
            pout.flushLine();
            pout.setRule( PrintOut::ThickRule );

            MessageLVI *m = (MessageLVI *) slv->firstChild();
            while ( m != 0 ) {
                pout.setRule( PrintOut::ThinRule );

                QString type;
                switch ( m->message().type() ) {
                case MetaTranslatorMessage::Finished:
                    type = tr( "finished" );
                    break;
                case MetaTranslatorMessage::Unfinished:
                    type = m->danger() ? tr( "unresolved" ) : QString( "unfinished" );
                    break;
                case MetaTranslatorMessage::Obsolete:
                    type = tr( "obsolete" );
                    break;
                default:
                    type = QString( "" );
                }
                pout.addBox( 40, m->sourceText() );
                pout.addBox( 4 );
                pout.addBox( 40, m->translation() );
                pout.addBox( 4 );
                pout.addBox( 12, type, PrintOut::Normal, Qt::AlignRight );
                if ( !m->comment().isEmpty() ) {
                    pout.flushLine();
                    pout.addBox( 4 );
                    pout.addBox( 92, m->comment(), PrintOut::Emphasis );
                }
                pout.flushLine( TRUE );

                if ( pout.pageNum() != pageNum ) {
                    pageNum = pout.pageNum();
                    statusBar()->message( tr("Printing... (page %1)")
                                          .arg(pageNum) );
                }
                m = (MessageLVI *) m->nextSibling();
            }
            c = (ContextLVI *) c->nextSibling();
        }
        pout.flushLine( TRUE );
        QApplication::restoreOverrideCursor();
        statusBar()->message( tr("Printing completed"), MessageMS );
    } else {
        statusBar()->message( tr("Printing aborted"), MessageMS );
    }
}

void TrWindow::find()
{
    h->hide();
    f->show();
    f->setActiveWindow();
    f->raise();
}

void TrWindow::findAgain()
{
    int pass = 0;
    int oldItemNo = itemToIndex( slv, slv->currentItem() );
    QString delayedMsg;
    QListViewItem * j = foundScope;
    QListViewItem * k = indexToItem( slv, foundItem );
    QListViewItem * oldScope = lv->currentItem();

    if ( lv->childCount() == 0 )
        return;
#if 1
    /*
      As long as we don't implement highlighting of the text in the QTextView,
      we may have only one match per message.
    */
    foundOffset = (int) 0x7fffffff;
#else
    foundOffset++;
#endif
    slv->setUpdatesEnabled( FALSE );
    do {
        // Iterate through every item in all contexts
        if ( j == 0 ) {
            j = lv->firstChild();
            setCurrentContextItem( j );
            if ( foundScope != 0 )
                delayedMsg = tr("Search wrapped.");
        }
        if ( k == 0 )
            k = slv->firstChild();

        while ( k ) {
            MessageLVI * m = (MessageLVI *) k;
            switch ( foundWhere ) {
                case 0:
                    foundWhere  = FindDialog::SourceText;
                    foundOffset = 0;
                    // fall-through
                case FindDialog::SourceText:
                    if ( searchItem( m->sourceText(), j, k ) ) {
                        f->hide();
                        if ( !delayedMsg.isEmpty() )
                            statusBar()->message( delayedMsg, MessageMS );
                         return;
                    }
                    foundWhere  = FindDialog::Translations;
                    foundOffset = 0;
                    // fall-through
                case FindDialog::Translations:
                    if ( searchItem( m->translation(), j, k ) ) {
                        f->hide();
                        if ( !delayedMsg.isEmpty() )
                            statusBar()->message( delayedMsg, MessageMS );
                        return;
                    }
                    foundWhere  = FindDialog::Comments;
                    foundOffset = 0;
                    // fall-through
                case FindDialog::Comments:
                    if ( searchItem( ((ContextLVI *) j)->fullContext(), j, k) ) {
                        f->hide();
                        if ( !delayedMsg.isEmpty() )
                            statusBar()->message( delayedMsg, MessageMS );
                        return;
                    }
                    foundWhere  = 0;
                    foundOffset = 0;
            }
            k = k->nextSibling();
        }

        j = j->nextSibling();
        if ( j ) {
            setCurrentContextItem( j );
            k = slv->firstChild();
        }
    } while ( pass++ != lv->childCount() );

    // This is just to keep the current scope and source text item
    // selected after a search failed.
    if ( oldScope ) {
        setCurrentContextItem( oldScope );
        QListViewItem * tmp = indexToItem( slv, oldItemNo );
        if( tmp )
            setCurrentMessageItem( tmp );
    } else {
        if( lv->firstChild() )
            setCurrentContextItem( lv->firstChild() );
        if( slv->firstChild() )
            setCurrentMessageItem( slv->firstChild() );
    }

    slv->setUpdatesEnabled( TRUE );
    slv->triggerUpdate();
    qApp->beep();
    QMessageBox::warning( this, tr("Qt Linguist"),
                          QString( tr("Cannot find the string '%1'.") ).arg(findText) );
    foundItem   = 0;
    foundWhere  = 0;
    foundOffset = 0;
}

void TrWindow::replace()
{
    f->hide();
    h->show();
    h->setActiveWindow();
    h->raise();
}

int TrWindow::itemToIndex( QListView * view, QListViewItem * item )
{
    int no = 0;
    QListViewItem * tmp;

    if( view && item ){
        if( (tmp = view->firstChild()) != 0 )
            do {
                no++;
                tmp = tmp->nextSibling();
            } while( tmp && (tmp != item) );
    }
    return no;
}

QListViewItem * TrWindow::indexToItem( QListView * view, int index )
{
    QListViewItem * item = 0;

    if ( view && index > 0 ) {
        item = view->firstChild();
        while( item && index-- > 0 )
            item = item->nextSibling();
    }
    return item;
}

bool TrWindow::searchItem( const QString & searchWhat, QListViewItem * j,
                           QListViewItem * k )
{
    if ( (findWhere & foundWhere) != 0 ) {
        foundOffset = searchWhat.find( findText, foundOffset, findMatchCase );
        if ( foundOffset >= 0 ) {
            foundItem = itemToIndex( slv, k );
            foundScope = j;
            setCurrentMessageItem( k );
            slv->setUpdatesEnabled( TRUE );
            slv->triggerUpdate();
            return TRUE;
        }
    }
    foundOffset = 0;
    return FALSE;
}

void TrWindow::newPhraseBook()
{
    QString name;
    for (;;) {
        name = QFileDialog::getSaveFileName( QString::null,
            tr("Qt phrase books (*.qph)\n"
               "All files (*)"),
            this, "new_phrasebook",
            tr("Create New Phrase Book") );
        if ( !QFile::exists(name) )
            break;
        QMessageBox::warning( this, tr("Qt Linguist"),
                              tr("A file called '%1' already exists."
                                 "  Please choose another name.").arg(name) );
    }
    if ( !name.isEmpty() ) {
        PhraseBook pb;
        if ( savePhraseBook(name, pb) ) {
            if ( openPhraseBook(name) )
                statusBar()->message( tr("Phrase book created."), MessageMS );
        }
    }
}

void TrWindow::openPhraseBook()
{
    QString phrasebooks( qInstallPathData() );
    QString name = QFileDialog::getOpenFileName( phrasebooks + "/phrasebooks",
        tr("Qt phrase books (*.qph)\n"
           "All files (*)"),
        this, "open_phrasebook",
        tr("Open Phrase Book") );
    if ( !name.isEmpty() && !phraseBookNames.contains(name) ) {
        if ( openPhraseBook(name) ) {
            int n = phraseBooks.at( phraseBooks.count() - 1 ).count();
            statusBar()->message( tr("%1 phrase(s) loaded.").arg(n),
                                  MessageMS );
        }
    }
}

void TrWindow::closePhraseBook( int id )
{
    int index = closePhraseBookp->indexOf( id );
    phraseBooks.removeAt(index);
    phraseBookNames.removeAt(index);
    updatePhraseDict();

    dirtyItem = index; // remove the item next time the menu is opened
    editPhraseBookp->removeItem( editPhraseBookp->idAt(index) );
    printPhraseBookp->removeItem( printPhraseBookp->idAt(index) );
}

void TrWindow::editPhraseBook( int id )
{
    int index = editPhraseBookp->indexOf( id );
    PhraseBookBox box( phraseBookNames.at(index), phraseBooks.at(index), this,
                       "phrase book box", TRUE );
    box.setWindowTitle( tr("%1 - %2").arg(tr("Qt Linguist"))
                                 .arg(friendlyPhraseBookName(index)) );
    box.resize( 500, 300 );
    box.exec();
    phraseBooks.replace(index, box.phraseBook());
    updatePhraseDict();
}

void TrWindow::printPhraseBook( int id )
{
    int index = printPhraseBookp->indexOf( id );
    int pageNum = 0;

    if ( printer.setup(this) ) {
        printer.setDocName(phraseBookNames.at(index));
        statusBar()->message( tr("Printing...") );
        PrintOut pout( &printer );
        PhraseBook phraseBook = phraseBooks.at(index);
        pout.setRule( PrintOut::ThinRule );
        foreach (Phrase p, phraseBook) {
            pout.setGuide( p.source() );
            pout.addBox( 29, p.source() );
            pout.addBox( 4 );
            pout.addBox( 29, p.target() );
            pout.addBox( 4 );
            pout.addBox( 34, p.definition(), PrintOut::Emphasis );

            if ( pout.pageNum() != pageNum ) {
                pageNum = pout.pageNum();
                statusBar()->message( tr("Printing... (page %1)")
                                      .arg(pageNum) );
            }
            pout.setRule( PrintOut::NoRule );
            pout.flushLine( TRUE );
        }
        pout.flushLine( TRUE );
        statusBar()->message( tr("Printing completed"), MessageMS );
    } else {
        statusBar()->message( tr("Printing aborted"), MessageMS );
    }
}

void TrWindow::revertSorting()
{
    lv->setSorting( 0 );
    slv->setSorting( 0 );
}

void TrWindow::manual()
{
    QString path = QDir::cleanDirPath( QString( qInstallPath() ) +
                                       QDir::separator() + "bin/" );
#if defined(Q_OS_MAC)
    path += QDir::separator() + ".app/Contents/MacOS/";
#endif
    QAssistantClient *ac = new QAssistantClient( path, this );
    ac->showPage( QString( qInstallPath() ) + "/doc/html/linguist-manual.html" );
}

void TrWindow::about()
{
    AboutDialog about( this, 0, TRUE );
    about.versionLabel->setText( tr("Version %1").arg(QT_VERSION_STR) );
    about.exec();
}

void TrWindow::aboutQt()
{
    QMessageBox::aboutQt( this, tr("Qt Linguist") );
}

void TrWindow::setupPhrase()
{
    bool enabled = !phraseBooks.isEmpty();
    closePhraseBookId->setEnabled(enabled);
    editPhraseBookId->setEnabled(enabled);
    printPhraseBookId->setEnabled(enabled);
}

void TrWindow::closeEvent( QCloseEvent *e )
{
    if ( maybeSave() )
        e->accept();
    else
        e->ignore();
}

bool TrWindow::maybeSave()
{
    if ( dirty ) {
        switch ( QMessageBox::information(this, tr("Qt Linguist"),
                                  tr("Do you want to save '%1'?")
                                  .arg(filename),
                                  QMessageBox::Yes | QMessageBox::Default,
                                  QMessageBox::No,
                                  QMessageBox::Cancel | QMessageBox::Escape ) )
        {
            case QMessageBox::Cancel:
                return FALSE;
            case QMessageBox::Yes:
                save();
                return !dirty;
            case QMessageBox::No:
                break;
        }
    }
    return TRUE;
}

void TrWindow::updateCaption()
{
    QString cap;
    bool enable = !filename.isEmpty();
    saveAct->setEnabled( enable );
    saveAsAct->setEnabled( enable );
    releaseAct->setEnabled( enable );
    printAct->setEnabled( enable );
    acceleratorsAct->setEnabled( enable );
    endingPunctuationAct->setEnabled( enable );
    phraseMatchesAct->setEnabled( enable );
    revertSortingAct->setEnabled( enable );

    if ( filename.isEmpty() )
        cap = tr( "Qt Linguist by Trolltech" );
    else
        cap = tr( "%1 - %2" ).arg( tr("Qt Linguist by Trolltech") )
                             .arg( filename );
    setWindowTitle( cap );
    modified->setEnabled( dirty );
}

//
// New scope selected - build a new list of source text items
// for that scope.
//
void TrWindow::showNewScope( QListViewItem *item )
{
    static ContextLVI *oldContext = 0;

    if( item != 0 ) {
        ContextLVI *c = (ContextLVI *) item;
        bool upe = slv->isUpdatesEnabled();
        slv->setUpdatesEnabled( FALSE );
        slv->viewport()->setUpdatesEnabled( FALSE );
        if ( oldContext != 0 ) {
            MessageLVI * tmp;
            slv->blockSignals( TRUE );
            while ( (tmp = (MessageLVI*) slv->firstChild()) != 0 )
                oldContext->appendMessageItem( slv, tmp );
            slv->blockSignals( FALSE );
        }
        MessageLVI * tmp;
        while ( c->messageItemsInList() ) {
            tmp = c->takeMessageItem( c->messageItemsInList() - 1);
            slv->insertItem( tmp );
            tmp->updateTranslationText();
        }

        slv->viewport()->setUpdatesEnabled( upe );
        slv->setUpdatesEnabled( upe );
        if( upe )
            slv->triggerUpdate();
        oldContext = (ContextLVI *) item;
        statusBar()->clear();
    }
}

void TrWindow::showNewCurrent( QListViewItem *item )
{
    messageIsShown = (item != 0);
    MessageLVI *m = (MessageLVI *) item;
    ContextLVI *c = (ContextLVI *) m ? m->contextLVI() : 0;

    if ( messageIsShown ) {
        me->showMessage( m->sourceText(), m->comment(), c->fullContext(),
                         m->translation(), m->message().type(),
                         getPhrases(m->sourceText()) );
        if ( (m->message().type() != MetaTranslatorMessage::Finished) &&
             m->danger() )
            danger( m->sourceText(), m->translation(), TRUE );
        else
            statusBar()->clear();

        doneAndNextAct->setEnabled( m->message().type() !=
                                    MetaTranslatorMessage::Obsolete );
    } else {
        if ( item == 0 )
            me->showNothing();
        else
            me->showContext( c->fullContext(), c->finished() );
        doneAndNextAct->setEnabled( FALSE );
    }
    doneAndNextAlt->setEnabled( doneAndNextAct->isEnabled() );

    selectAllAct->setEnabled( messageIsShown );
}

void TrWindow::updateTranslation( const QString& translation )
{
    QListViewItem *item = slv->currentItem();
    if ( item != 0 ) {
        MessageLVI *m = (MessageLVI *) item;
        if ( translation != m->translation() ) {
            bool dngr;
            m->setTranslation( translation );
            if ( m->finished() &&
                 (dngr = danger( m->sourceText(), m->translation(), TRUE )) ) {
                numFinished -= 1;
                m->setDanger( dngr );
                m->setFinished( FALSE );
                m->contextLVI()->updateStatus();
                updateProgress();
            }
            //tor.insert( m->message() );
            if ( !dirty ) {
                dirty = TRUE;
                updateCaption();
            }
            m->updateTranslationText();
            tor.insert(m->message());
        }
    }
}

void TrWindow::updateFinished( bool finished )
{
    QListViewItem *item = slv->currentItem();
    if ( item != 0 ) {
        MessageLVI *m = (MessageLVI *) item;
        if ( finished != m->finished() ) {
            numFinished += finished ? +1 : -1;
            updateProgress();
            m->setFinished( finished );
            bool oldDanger = m->danger();
            m->setDanger( /*m->finished() &&*/
                          danger(m->sourceText(), m->translation(),
                          !oldDanger) );
            if ( !oldDanger && m->danger() )
                qApp->beep();
            tor.insert( m->message() );
            if ( !dirty ) {
                dirty = TRUE;
                updateCaption();
            }
        }
    }
}

void TrWindow::doneAndNext()
{
    MessageLVI *m = (MessageLVI *) slv->currentItem();
    bool dngr = FALSE;

    if ( !m ) return;
    dngr = danger( m->sourceText(), m->translation(), TRUE );
    if ( !dngr ) {
        me->finishAndNext();
        m->contextLVI()->updateStatus();
    } else {
        if ( m->danger() != dngr )
            m->setDanger( dngr );
        tor.insert( m->message() );
        if ( !dirty ) {
            dirty = TRUE;
            updateCaption();
        }
        qApp->beep();
    }
    updateStatistics();
}

void TrWindow::toggleFinished( QListViewItem *item, const QPoint& /* p */,
                               int column )
{
    if ( item != 0 && column == 0 ) {
        MessageLVI *m = (MessageLVI *) item;
        bool dngr = FALSE;

        if ( m->message().type() == MetaTranslatorMessage::Unfinished ) {
            dngr = danger( m->sourceText(), m->translation(), TRUE );
        }
        if ( !dngr && m->message().type() != MetaTranslatorMessage::Obsolete) {
            setCurrentMessageItem( m );
            me->setFinished( !m->finished() );
            m->contextLVI()->updateStatus();
        } else {
            bool oldDanger = m->danger();
            m->setDanger( danger(m->sourceText(), m->translation(),
                                 !oldDanger) );
            if ( !oldDanger && m->danger() )
                qApp->beep();
            tor.insert( m->message() );
            if ( !dirty ) {
                dirty = TRUE;
                updateCaption();
            }
        }
        updateStatistics();
    }
}

void TrWindow::nextUnfinished()
{
    if ( nextUnfinishedAct->isEnabled() ) {
        // Select a message to translate, grab the first available if
        // there are no current selection.
        QListViewItem * cItem = lv->currentItem(); // context item
        QListViewItem * mItem = slv->currentItem(); // message item

        // Make sure an item is selected from both the context and the
        // message list.
        if( (mItem == 0) && !(mItem = slv->firstChild()) ) {
            if( (cItem == 0) && !(cItem = lv->firstChild()) ) {
                statusBar()->message( tr("No phrase to translate."),
                                      MessageMS );
                qApp->beep();
                return;
            } else {
                showNewScope( cItem );
                while( cItem && !(mItem = slv->firstChild()) ) {
                    // no children in this node - try next one
                    cItem = cItem->nextSibling();
                    showNewScope( cItem );
                }
                setCurrentContextItem( cItem );
                if( mItem ) {
                    setCurrentMessageItem( mItem );
                } else {
                    statusBar()->message( tr("No phrase to translate."),
                                          MessageMS );
                    qApp->beep();
                    return;
                }
            }
        } else {
            setCurrentMessageItem( mItem );
        }

        MessageLVI * m = (MessageLVI *) mItem;
        MessageLVI * n;
        ContextLVI * p = (ContextLVI *) cItem;
        ContextLVI * q;

        // Find the next Unfinished sibling within the same context.
        m = (MessageLVI *) mItem->nextSibling();
        n = m;
        do {
            if ( n == 0 )
                break;
            if ( n && !n->finished() && n != mItem ) {
                setCurrentMessageItem( n );
                return;
            }
            n = (MessageLVI *) n->nextSibling();
        } while ( n != m );

        // If all siblings are Finished or Obsolete, look in the first
        // Unfinished context.
        p = (ContextLVI *) p->nextSibling();
        q = p;
        do {
            if ( q == 0 )
                q = (ContextLVI *) lv->firstChild();
            if ( q && !q->finished() ) {
                showNewScope( q );
                setCurrentContextItem( q );
                n = (MessageLVI *) slv->firstChild();
                while ( n && n->finished() )
                    n = (MessageLVI *) n->nextSibling();
                if ( n && q ) {
                    setCurrentMessageItem( n );
                    showNewCurrent( n );
                    return;
                }
            }
            q = (ContextLVI *) q->nextSibling();
        } while ( q != p );
    }

    // If no Unfinished message is left, the user has finished the job.  We
    // congratulate on a job well done with this ringing bell.
    statusBar()->message( tr("No untranslated phrases left."), MessageMS );
    qApp->beep();
}

static QListViewItem * lastChild( QListView * view )
{
    if ( view ) {
        QListViewItem * ret, * tmp;
        ret = view->firstChild();
        while ( ret ) {
            tmp = ret->nextSibling();
            if ( tmp == 0 )
                return ret;
            ret = tmp;
        }
    }
    return 0;
}

void TrWindow::prevUnfinished()
{
    if ( prevUnfinishedAct->isEnabled() ) {
        // Select a message to translate, grab the first available if
        // there are no current selection.
        QListViewItem * cItem = lv->currentItem();  // context item
        QListViewItem * mItem = slv->currentItem(); // message item

        // Make sure an item is selected from both the context and the
        // message list.
        if( (mItem == 0) && !(mItem = slv->firstChild()) ) {
            if( (cItem == 0) && !(cItem = lv->firstChild()) ) {
                statusBar()->message( tr("No phrase to translate."),
                                      MessageMS );
                qApp->beep();
                return;
            } else {
                showNewScope( cItem );
                while( cItem && !(mItem = slv->firstChild()) ) {
                    // no children in this node - try next one
                    cItem = cItem->nextSibling();
                    showNewScope( cItem );
                }
                setCurrentContextItem( cItem );
                if( mItem ) {
                    setCurrentMessageItem( cItem );
                } else {
                    statusBar()->message( tr("No phrase to translate."),
                                          MessageMS );
                    qApp->beep();
                    return;
                }
            }
        } else {
            setCurrentMessageItem( mItem );
        }

        MessageLVI * m = (MessageLVI *) mItem;
        MessageLVI * n;
        ContextLVI * p = (ContextLVI *) cItem;
        ContextLVI * q;

        // Find the next Unfinished sibling within the same context.
        n = m;
        do {
            n = (MessageLVI * ) n->itemAbove();
            if ( n == 0 )
                break;
            if ( n && !n->finished() ) {
                setCurrentMessageItem( n );
                return;
            }
        } while ( !((ContextLVI *) cItem)->finished() && n != 0 );

        // If all siblings are Finished or Obsolete, look in the prev
        // Unfinished context.
        q = p;
        do {
            q = (ContextLVI *) q->itemAbove();
            if ( q == 0 )
                q = (ContextLVI *) lastChild( lv );
            if ( q && !q->finished() ) {
                showNewScope( q );
                setCurrentContextItem( q );
                n = (MessageLVI *) lastChild( slv );
                while ( n && n->finished() )
                    n = (MessageLVI *) n->itemAbove();
                if ( n && q ) {
                    setCurrentMessageItem( n );
                    return;
                }
            }
        } while ( q != 0 );
    }
    statusBar()->message( tr("No untranslated phrases left."), MessageMS );
    qApp->beep();
}

void TrWindow::prev()
{
    QListViewItem * cItem = lv->currentItem();  // context item
    QListViewItem * mItem = slv->currentItem(); // message item
    QListViewItem * tmp;

    if ( !cItem ) {
        cItem = lv->firstChild();
        if ( !cItem ) return;
        setCurrentContextItem( cItem );
    }

    if ( !mItem ) {
        mItem = lastChild( slv );
        if ( !mItem ) return;
        setCurrentMessageItem( mItem );
    } else {
        if ( (tmp = mItem->itemAbove()) != 0 ) {
            setCurrentMessageItem( tmp );
            return;
        } else {
            if ( (tmp = cItem->itemAbove()) == 0 ) {
                tmp = lastChild( lv );
            }
            if ( !tmp ) return;
            setCurrentContextItem( tmp );
            setCurrentMessageItem( lastChild( slv ) );
        }
    }
}

void TrWindow::next()
{
    QListViewItem * cItem = lv->currentItem();  // context item
    QListViewItem * mItem = slv->currentItem(); // message item
    QListViewItem * tmp;

    if ( !cItem ) {
        cItem = lv->firstChild();
        if ( !cItem ) return;
        setCurrentContextItem( cItem );
    }

    if ( !mItem ) {
        mItem = slv->firstChild();
        if ( !mItem ) return;
        setCurrentMessageItem( mItem );
    } else {
        if ( (tmp = mItem->nextSibling()) != 0 ) {
            setCurrentMessageItem( tmp );
            return;
        } else {
            if ( (tmp = cItem->nextSibling()) == 0 ) {
                tmp = lv->firstChild();
            }
            if ( !tmp ) return;
            setCurrentContextItem( tmp );
            setCurrentMessageItem( slv->firstChild() );
        }
    }
}


void TrWindow::findNext( const QString& text, int where, bool matchCase )
{
    findText = text;
    if ( findText.isEmpty() )
        findText = QString( "magicwordthatyoushouldavoid" );
    findWhere = where;
    findMatchCase = matchCase;
    findAgainAct->setEnabled( TRUE );
    findAgain();
}

void TrWindow::revalidate()
{
    ContextLVI *c = (ContextLVI *) lv->firstChild();
    QListViewItem * oldScope = lv->currentItem();
    int oldItemNo = itemToIndex( slv, slv->currentItem() );
    slv->setUpdatesEnabled( FALSE );

    while ( c != 0 ) {
        showNewScope( c );
        MessageLVI *m = (MessageLVI *) slv->firstChild();
        while ( m != 0 ) {
            m->setDanger( danger(m->sourceText(), m->translation()) &&
                    m->message().type() == MetaTranslatorMessage::Finished );
            m = (MessageLVI *) m->nextSibling();
        }
        c = (ContextLVI *) c->nextSibling();
    }

    if ( oldScope ){
        showNewScope( oldScope );
        QListViewItem * tmp = indexToItem( slv, oldItemNo );
        if( tmp )
            setCurrentMessageItem( tmp );
    }
    slv->setUpdatesEnabled( TRUE );
    slv->triggerUpdate();
}

QString TrWindow::friendlyString( const QString& str )
{
    QString f = str.toLower();
    f.replace( QRegExp(QString("[.,:;!?()-]")), QString(" ") );
    f.replace( "&", QString("") );
    f = f.simplified();
    f = f.toLower();
    return f;
}

void TrWindow::setupMenuBar()
{
    QMenuBar * m = menuBar();
    QMenu * filep = new QMenu( this );
    QMenu * editp  = new QMenu( this );
    QMenu * translationp = new QMenu( this );
    QMenu * validationp = new QMenu( this );
    validationp->setCheckable( TRUE );
    phrasep = new QMenu( this );
    closePhraseBookp = new QMenu( this );
    editPhraseBookp = new QMenu( this );
    printPhraseBookp = new QMenu( this );
    QMenu * viewp = new QMenu( this );
    viewp->setCheckable( TRUE );
    QMenu * helpp = new QMenu( this );

    m->addMenu( tr("&File"), filep );
    m->addMenu( tr("&Edit"), editp );
    m->addMenu( tr("&Translation"), translationp );
    m->addMenu( tr("V&alidation"), validationp );
    m->addMenu( tr("&Phrases"), phrasep );
    m->addMenu( tr("&View"), viewp );
    m->addMenu( tr("&Help"), helpp );

    connect( closePhraseBookp, SIGNAL(activated(int)),
             this, SLOT(closePhraseBook(int)) );
    connect( closePhraseBookp, SIGNAL(aboutToShow()),
             this, SLOT(updateClosePhraseBook()) );
    connect( editPhraseBookp, SIGNAL(activated(int)),
             this, SLOT(editPhraseBook(int)) );
    connect( printPhraseBookp, SIGNAL(activated(int)),
             this, SLOT(printPhraseBook(int)) );
    // File menu
    openAct = filep->addAction(loadPixmap("fileopen.png"), tr("&Open..."), this, SLOT(open()), QKeySequence("Ctrl+O") );
    filep->addSeparator();
    saveAct = filep->addAction(loadPixmap("filesave.png"), tr("&Save"), this, SLOT(save()), QKeySequence("Ctrl+S") );
    saveAsAct = filep->addAction(tr("Save &As..."), this, SLOT(saveAs()) );
    releaseAct = filep->addAction(tr("&Release..."), this, SLOT(release()) );
    filep->addSeparator();
    printAct = filep->addAction(loadPixmap("print.png"), tr("&Print..."), this, SLOT(print()), QKeySequence("Ctrl+P") );
    filep->addSeparator();

    recentFilesMenu = new QMenu( this );
    filep->addMenu( tr("Re&cently opened files"), recentFilesMenu );
    connect( recentFilesMenu, SIGNAL(aboutToShow()), this,
             SLOT(setupRecentFilesMenu()) );
    connect( recentFilesMenu, SIGNAL(activated( int )), this,
             SLOT(recentFileActivated( int )) );

    filep->addSeparator();

    exitAct = filep->addAction(tr("E&xit"), this, SLOT(close()), QKeySequence("Ctrl+Q") );
    // Edit menu
    undoAct = editp->addAction(loadPixmap("undo.png"), tr("&Undo"), me, SLOT(undo()), QKeySequence("Ctrl+Z") );
    undoAct->setEnabled( FALSE );
    connect( me, SIGNAL(undoAvailable(bool)), undoAct, SLOT(setEnabled(bool)) );
    redoAct = editp->addAction(loadPixmap("redo.png"), tr("&Redo"), me, SLOT(redo()), QKeySequence("Ctrl+Y") );
    redoAct->setEnabled( FALSE );
    connect( me, SIGNAL(redoAvailable(bool)), redoAct, SLOT(setEnabled(bool)) );
    editp->addSeparator();
    cutAct = editp->addAction(loadPixmap("editcut.png"), tr("Cu&t"), me, SLOT(cut()), QKeySequence("Ctrl+X") );
    cutAct->setEnabled( FALSE );
    connect( me, SIGNAL(cutAvailable(bool)), cutAct, SLOT(setEnabled(bool)) );
    copyAct = editp->addAction(loadPixmap("editcopy.png"), tr("&Copy"), me, SLOT(copy()), QKeySequence("Ctrl+C") );
    copyAct->setEnabled( FALSE );
    connect( me, SIGNAL(copyAvailable(bool)), copyAct, SLOT(setEnabled(bool)) );
    pasteAct = editp->addAction(loadPixmap("editpaste.png"), tr("&Paste"), me, SLOT(paste()), QKeySequence("Ctrl+V") );
    pasteAct->setEnabled( FALSE );
    connect( me, SIGNAL(pasteAvailable(bool)),
             pasteAct, SLOT(setEnabled(bool)) );
    selectAllAct = editp->addAction(tr("Select &All"), me, SLOT(selectAll()), QKeySequence("Ctrl+A") );
    selectAllAct->setEnabled( FALSE );
    editp->addSeparator();
    findAct = editp->addAction(loadPixmap("searchfind.png"), tr("&Find..."), this, SLOT(find()), QKeySequence("Ctrl+F") );
    findAct->setEnabled( FALSE );
    findAgainAct = editp->addAction(tr("Find &Next"), this, SLOT(findAgain()), Key_F3 );
    findAgainAct->setEnabled( FALSE );
#ifdef notyet
    replaceAct = editp->addAction(tr("&Replace..."), this, SLOT(replace()), QKeySequence("Ctrl+H") );
    replaceAct->setEnabled( FALSE );
#endif

    // Translation menu
    // when updating the accelerators, remember the status bar
    prevUnfinishedAct = translationp->addAction(loadPixmap("prevunfinished.png"), tr("&Prev Unfinished"),
                                    this, SLOT(prevUnfinished()), QKeySequence("Ctrl+K") );
    nextUnfinishedAct = translationp->addAction(loadPixmap("nextunfinished.png"), tr("&Next Unfinished"),
                                    this, SLOT(nextUnfinished()), QKeySequence("Ctrl+L") );

    prevAct = translationp->addAction(loadPixmap("prev.png"), tr("P&rev"),
                          this, SLOT(prev()),
                          QKeySequence("Ctrl+Shift+K") );
    nextAct = translationp->addAction(loadPixmap("next.png"), tr("Ne&xt"),
                          this, SLOT(next()),
                          QKeySequence("Ctrl+Shift+L") );
    doneAndNextAct = translationp->addAction(loadPixmap("doneandnext.png"), tr("Done and &Next"),
                                 this, SLOT(doneAndNext()),
                                 QKeySequence("Ctrl+Enter") );
    doneAndNextAlt = new QAction( this );
    doneAndNextAlt->setShortcut( QKeySequence("Ctrl+Return") );
    connect( doneAndNextAlt, SIGNAL(activated()), this, SLOT(doneAndNext()) );
    beginFromSourceAct = translationp->addAction(tr("&Begin from Source"),
                                     me, SLOT(beginFromSource()),
                                     QKeySequence("Ctrl+B") );
    connect( me, SIGNAL(updateActions(bool)), beginFromSourceAct,
             SLOT(setEnabled(bool)) );

    // Phrasebook menu
    newPhraseBookAct = phrasep->addAction(tr("&New Phrase Book..."),
                                   this, SLOT(newPhraseBook()),
                                   QKeySequence("Ctrl+N") );
    openPhraseBookAct = phrasep->addAction(loadPixmap("book.png"), tr("&Open Phrase Book..."),
                                    this, SLOT(openPhraseBook()),
                                    QKeySequence("Ctrl+H") );
    closePhraseBookId = phrasep->addMenu( tr("&Close Phrase Book"),
                                             closePhraseBookp );
    phrasep->addSeparator();
    editPhraseBookId = phrasep->addMenu( tr("&Edit Phrase Book..."),
                                            editPhraseBookp );
    printPhraseBookId = phrasep->addMenu( tr("&Print Phrase Book..."),
                                             printPhraseBookp );
    connect( phrasep, SIGNAL(aboutToShow()), this, SLOT(setupPhrase()) );

    // Validation menu
    acceleratorsAct = validationp->addAction(loadPixmap("accelerator.png"), tr("&Accelerators"),
                                  this, SLOT(revalidate()));
    acceleratorsAct->setCheckable(true);
    acceleratorsAct->setChecked( TRUE );
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
                                   this, SLOT(revertSorting()) );
    doGuessesAct = viewp->addAction(tr("&Display guesses"),
                               this, SLOT(toggleGuessing()) );
    doGuessesAct->setCheckable(true);
    doGuessesAct->setChecked(true);
    toggleStats = viewp->addAction(tr("&Statistics"), this, SLOT(toggleStatistics()) );
    toggleStats->setCheckable(true);
    viewp->addSeparator();
    viewp->addMenu( tr("Vie&ws"), createDockWindowMenu( NoToolBars ) );
    viewp->addMenu( tr("&Toolbars"), createDockWindowMenu( OnlyToolBars ) );

    // Help
    manualAct = helpp->addAction(tr("&Manual"), this, SLOT(manual()), Key_F1 );
    helpp->addSeparator();
    aboutAct = helpp->addAction(tr("&About"), this, SLOT(about()) );
    aboutQtAct = helpp->addAction(tr("About &Qt"), this, SLOT(aboutQt()) );
    helpp->addSeparator();
    whatsThisAct = helpp->addAction(loadPixmap("whatsthis.xpm"), tr("&What's This?"),
                               this, SLOT(whatsThis()), SHIFT + Key_F1 );

    openAct->setWhatsThis( tr("Open a Qt translation source file (TS file) for"
                              " editing.") );
    saveAct->setWhatsThis( tr("Save changes made to this Qt translation "
                                "source file.") );
    saveAsAct->setWhatsThis( tr("Save changes made to this Qt translation"
                                "source file into a new file.") );
    releaseAct->setWhatsThis( tr("Create a Qt message file suitable for"
                                 " released applications"
                                 " from the current message file.") );
    printAct->setWhatsThis( tr("Print a list of all the phrases in the current"
                               " Qt translation source file.") );
    exitAct->setWhatsThis( tr("Close this window and exit.") );

    undoAct->setWhatsThis( tr("Undo the last editing operation performed on the"
                              " translation.") );
    redoAct->setWhatsThis( tr("Redo an undone editing operation performed on"
                              " the translation.") );
    cutAct->setWhatsThis( tr("Copy the selected translation text to the"
                             " clipboard and deletes it.") );
    copyAct->setWhatsThis( tr("Copy the selected translation text to the"
                              " clipboard.") );
    pasteAct->setWhatsThis( tr("Paste the clipboard text into the"
                               " translation.") );
    selectAllAct->setWhatsThis( tr("Select the whole translation text.") );
    findAct->setWhatsThis( tr("Search for some text in the translation "
                                "source file.") );
    findAgainAct->setWhatsThis( tr("Continue the search where it was left.") );
#ifdef notyet
    replaceAct->setWhatsThis( tr("Search for some text in the translation"
                                 " source file and replace it by another"
                                 " text.") );
#endif

    newPhraseBookAct->setWhatsThis( tr("Create a new phrase book.") );
    openPhraseBookAct->setWhatsThis( tr("Open a phrase book to assist"
                                        " translation.") );
    acceleratorsAct->setWhatsThis( tr("Toggle validity checks of"
                                      " accelerators.") );
    endingPunctuationAct->setWhatsThis( tr("Toggle validity checks"
                                           " of ending punctuation.") );
    phraseMatchesAct->setWhatsThis( tr("Toggle checking that phrase"
                                       " suggestions are used.") );

    revertSortingAct->setWhatsThis( tr("Sort the items back in the same order"
                                       " as in the message file.") );

    doGuessesAct->setWhatsThis( tr("Set whether or not to display translation guesses.") );
    manualAct->setWhatsThis( tr("Display the manual for %1.")
                               .arg(tr("Qt Linguist")) );
    aboutAct->setWhatsThis( tr("Display information about %1.")
                            .arg(tr("Qt Linguist")) );
    aboutQtAct->setWhatsThis( tr("Display information about the Qt toolkit by"
                                 " Trolltech.") );
    whatsThisAct->setWhatsThis( tr("Enter What's This? mode.") );

    beginFromSourceAct->setWhatsThis( tr("Copies the source text into"
                                         " the translation field.") );
    nextAct->setWhatsThis( tr("Moves to the next item.") );
    prevAct->setWhatsThis( tr("Moves to the previous item.") );
    nextUnfinishedAct->setWhatsThis( tr("Moves to the next unfinished item.") );
    prevUnfinishedAct->setWhatsThis( tr("Moves to the previous unfinished item.") );
    doneAndNextAct->setWhatsThis( tr("Marks this item as done and moves to the"
                                     " next unfinished item.") );
    doneAndNextAlt->setWhatsThis( doneAndNextAct->whatsThis() );
}

void TrWindow::setupToolBars()
{
    QToolBar *filet = new QToolBar( tr("File"), this );
    QToolBar *editt = new QToolBar( tr("Edit"), this );
    QToolBar *translationst = new QToolBar( tr("Translation"), this );
    QToolBar *validationt   = new QToolBar( tr("Validation"), this );
    QToolBar *helpt = new QToolBar( tr("Help"), this );

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
#ifdef notyet
    editt->addAction(replaceAct);
#endif

    // beginFromSourceAct->addToToolbar( translationst,
    //                                tr("Begin from Source"), "searchfind" );
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

void TrWindow::setCurrentContextItem( QListViewItem *item )
{
    lv->ensureItemVisible( item );
    lv->setSelected( item, TRUE );
}

void TrWindow::setCurrentMessageItem( QListViewItem *item )
{
    slv->ensureItemVisible( item );
    slv->setSelected( item, TRUE );
}

QString TrWindow::friendlyPhraseBookName( int k )
{
    return QFileInfo(phraseBookNames.at(k)).fileName();
}

bool TrWindow::openPhraseBook( const QString& name )
{
    PhraseBook pb;
    if ( !pb.load(name) ) {
        QMessageBox::warning( this, tr("Qt Linguist"),
                              tr("Cannot read from phrase book '%1'.")
                              .arg(name) );
        return FALSE;
    }

    int index = phraseBooks.count();
    phraseBooks.append( pb );
    phraseBookNames.append( name );
    int id = closePhraseBookp->insertItem( friendlyPhraseBookName(index) );
    closePhraseBookp->setWhatsThis( id, tr("Close this phrase book.") );
    id = editPhraseBookp->insertItem( friendlyPhraseBookName(index) );
    editPhraseBookp->setWhatsThis( id, tr("Allow you to add, modify, or delete"
                                          " phrases of this phrase book.") );
    id = printPhraseBookp->insertItem( friendlyPhraseBookName(index) );
    printPhraseBookp->setWhatsThis( id, tr("Print the entries of the phrase"
                                           " book.") );
    updatePhraseDict();
    return TRUE;
}

bool TrWindow::savePhraseBook( QString& name, const PhraseBook& pb )
{
    if ( !name.contains( ".qph" ) && !name.contains(".") )
        name += ".qph";

    if ( !pb.save(name) ) {
        QMessageBox::warning( this, tr("Qt Linguist"),
                              tr("Cannot create phrase book '%1'.")
                              .arg(name) );
        return FALSE;
    }
    return TRUE;
}

void TrWindow::updateProgress()
{
    if ( numNonobsolete == 0 )
        progress->setText( QString("    " "    ") );
    else
        progress->setText( QString(" %1/%2 ").arg(numFinished)
                           .arg(numNonobsolete) );
    prevUnfinishedAct->setEnabled( numFinished != numNonobsolete );
    nextUnfinishedAct->setEnabled( numFinished != numNonobsolete );
    prevAct->setEnabled( lv->firstChild() != 0 );
    nextAct->setEnabled( lv->firstChild() != 0 );
}

void TrWindow::updatePhraseDict()
{
    phraseDict.clear();

    for (int i = 0; i < phraseBooks.size(); ++i) {
        PhraseBook pb = phraseBooks.at(i);
        foreach ( Phrase p, pb ) {
            QString f = friendlyString( p.source() );
            if ( f.length() > 0 ) {
                f = f.split(QChar(' ')).first();
                if (!phraseDict.contains(f)) {
                    PhraseBook pbe;
                    phraseDict.insert( f, pbe );
                }
                phraseDict[f].append(p);
            }
        }
    }
    revalidate();
}

PhraseBook TrWindow::getPhrases( const QString& source )
{
    PhraseBook phrases;
    QString f = friendlyString(source);
    QStringList lookupWords = f.split(QChar(' '));

    foreach (QString s, lookupWords) {
        if (phraseDict.contains(s)) {
            PhraseBook ent = phraseDict.value(s);
            foreach (Phrase p, ent) {
                if ( f.indexOf(friendlyString((p).source())) >= 0 )
                    phrases.append( p );
            }
        }
    }
    return phrases;
}

bool TrWindow::danger( const QString& source, const QString& translation,
                       bool verbose )
{
    if ( acceleratorsAct->isChecked() ) {
        int sk = QAccel::shortcutKey( source );
        int tk = QAccel::shortcutKey( translation );
        if ( sk == 0 && tk != 0 ) {
            if ( verbose )
                statusBar()->message( tr("Accelerator possibly superfluous in"
                                         " translation."), ErrorMS );
            return TRUE;
        } else if ( sk != 0 && tk == 0 ) {
            if ( verbose )
                statusBar()->message( tr("Accelerator possibly missing in"
                                         " translation."), ErrorMS );
            return TRUE;
        }
    }
    if ( endingPunctuationAct->isChecked() ) {
        if ( ending(source) != ending(translation) ) {
            if ( verbose )
                statusBar()->message( tr("Translation does not end with the"
                                         " same punctuation as the source"
                                         " text."), ErrorMS );
            return TRUE;
        }
    }
    if ( phraseMatchesAct->isChecked() ) {
        QString fsource = friendlyString( source );
        QString ftranslation = friendlyString( translation );
        QStringList lookupWords = fsource.split(QChar(' '));

        bool phraseFound;
        foreach (QString s, lookupWords) {
            if (phraseDict.contains(s)) {
                PhraseBook ent = phraseDict.value(s);
                phraseFound = false;
                foreach (Phrase p, ent) {
                    if ( fsource.indexOf(friendlyString(p.source())) < 0 ||
                         ftranslation.indexOf(friendlyString(p.target())) >= 0 ) {
                        phraseFound = true;
                        break;
                    }
                }
                if (!phraseFound) {
                    if ( verbose )
                        statusBar()->message( tr("A phrase book suggestion for"
                                                 " '%1' was ignored.")
                                                 .arg(s), ErrorMS );
                    return TRUE;
                }
            }
        }
    }
    if ( verbose )
        statusBar()->clear();

    return FALSE;
}

void TrWindow::readConfig()
{
    QString keybase( "/Qt Linguist/" +
                     QString::number( (QT_VERSION >> 16) & 0xff ) +
                     "." + QString::number( (QT_VERSION >> 8) & 0xff ) + "/" );
    QSettings config;

    config.insertSearchPath( QSettings::Windows, "/Trolltech" );

    QRect r( pos(), size() );
    recentFiles = config.readListEntry( keybase + "RecentlyOpenedFiles" );
    if ( !config.readBoolEntry( keybase + "Geometry/MainwindowMaximized", FALSE ) ) {
        r.setX( config.readNumEntry( keybase + "Geometry/MainwindowX", r.x() ) );
        r.setY( config.readNumEntry( keybase + "Geometry/MainwindowY", r.y() ) );
        r.setWidth( config.readNumEntry( keybase + "Geometry/MainwindowWidth", r.width() ) );
        r.setHeight( config.readNumEntry( keybase + "Geometry/MainwindowHeight", r.height() ) );

        QRect desk = QApplication::desktop()->geometry();
        QRect inter = desk.intersect( r );
        resize( r.size() );
        if ( inter.width() * inter.height() > ( r.width() * r.height() / 20 ) ) {
            move( r.topLeft() );
        }
    }

    QDockWindow * dw;
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
    dw->setGeometry( r );
    QApplication::sendPostedEvents();
}

void TrWindow::writeConfig()
{
    QString keybase( "/Qt Linguist/" +
                     QString::number( (QT_VERSION >> 16) & 0xff ) +
                     "." + QString::number( (QT_VERSION >> 8) & 0xff ) + "/" );
    QSettings config;

    config.insertSearchPath( QSettings::Windows, "/Trolltech" );
    config.writeEntry( keybase + "RecentlyOpenedFiles", recentFiles );
    config.writeEntry( keybase + "Geometry/MainwindowMaximized", isMaximized() );
    config.writeEntry( keybase + "Geometry/MainwindowX", x() );
    config.writeEntry( keybase + "Geometry/MainwindowY", y() );
    config.writeEntry( keybase + "Geometry/MainwindowWidth", width() );
    config.writeEntry( keybase + "Geometry/MainwindowHeight", height() );

    QDockWindow * dw =(QDockWindow *) lv->parent();
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
    config.writeEntry( keybase + "Geometry/PhrasewindowHeight", dw->height() );
}

void TrWindow::setupRecentFilesMenu()
{
    recentFilesMenu->clear();
    int id = 0;
    QStringList::Iterator it = recentFiles.begin();
    for ( ; it != recentFiles.end(); ++it )
    {
        recentFilesMenu->insertItem( *it, id );
        id++;
    }
}

void TrWindow::recentFileActivated( int id )
{
    if ( id != -1 ) {
        if ( maybeSave() )
            openFile( recentFiles.at( id ) );
    }
}

void TrWindow::addRecentlyOpenedFile( const QString &fn, QStringList &lst )
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
    slv->setFocus();
}

void TrWindow::focusPhraseList()
{
    plv->setFocus();
}

void TrWindow::updateClosePhraseBook()
{
    if ( dirtyItem != -1 ) {
        closePhraseBookp->removeItem( closePhraseBookp->idAt(dirtyItem) );
        dirtyItem = -1;
    }
}

void TrWindow::toggleStatistics()
{
    if ( toggleStats->isChecked() ) {
        if ( !stats ) {
            stats = new Statistics( this, "linguist_stats" );
            connect( this, SIGNAL(statsChanged(int,int,int,int,int,int)), stats,
                     SLOT(updateStats(int,int,int,int,int,int)) );
            connect( stats, SIGNAL(closed()), toggleStats, SLOT(toggle()) );
        }
        updateStatistics();
        stats->show();
    } else if ( stats ) {
        stats->close();
    }
}

void TrWindow::updateStatistics()
{
    QListViewItem * ci = lv->firstChild();
    int trW = 0;
    int trC = 0;
    int trCS = 0;
    while (ci) {
        QList<MessageLVI*> lst = (reinterpret_cast<ContextLVI*>(ci))->messageItemLst();
        foreach (MessageLVI *mi, lst) {
            if (mi->finished() && !(mi->message().type() == MetaTranslatorMessage::Obsolete))
                doCharCounting(mi->translation(), trW, trC, trCS);
        }
        ci = ci->nextSibling();
    }
    // ..and the items in the source list
    if (slv->firstChild()) {
        QListViewItem *lvi = slv->firstChild();
        MessageLVI *mi;
        while (lvi) {
            mi = reinterpret_cast<MessageLVI*>(lvi);
            if (mi->finished() && !(mi->message().type() == MetaTranslatorMessage::Obsolete))
                doCharCounting(mi->translation(), trW, trC, trCS);
            lvi = lvi->nextSibling();
        }

    }
    emit statsChanged(srcWords, srcChars, srcCharsSpc, trW, trC, trCS);
}

void TrWindow::doCharCounting( const QString& text, int& trW, int& trC, int& trCS )
{
    trCS += text.length();
    bool inWord = FALSE;
    for ( int i = 0; i < (int) text.length(); i++ ) {
        if ( text[i].isLetterOrNumber() || text[i] == QChar('_') ) {
            if ( !inWord ) {
                trW++;
                inWord = TRUE;
            }
        } else {
            inWord = FALSE;
        }
        if ( !text[i].isSpace() )
            trC++;
    }
}

QIconSet TrWindow::loadPixmap(const QString &imageName)
{
    if ( !imageName.isEmpty() ) {
        QPixmap enabledPix = QPixmap::fromMimeSource( imageName );
        QIconSet s( enabledPix );
        if ( imageName != QLatin1String("whatsthis.xpm") ) {
            QPixmap disabledPix = QPixmap::fromMimeSource( "d_" + imageName );
            s.setPixmap( disabledPix, QIconSet::Small, QIconSet::Disabled );
        }
        return s;
    }

    return QIconSet();
}

