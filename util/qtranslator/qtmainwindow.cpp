/****************************************************************************
** $Id: //depot/qt/main/util/qtranslator/qtmainwindow.cpp#8 $
**
** This is a utility program for translating Qt applications
**
**
** Copyright (C) 1999 by Trolltech AS.  All rights reserved.
**
*****************************************************************************/

#include "qtmainwindow.h"
#include "qtprefdia.h"
#include "qtpreferences.h"
#include "qtmessageview.h"
#include "qtaddlangdia.h"
#include "mergetr.h"
#include "msg2qm.h"

#include <qapplication.h>
#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qfiledialog.h>
#include <qsplitter.h>
#include <qlistview.h>
#include <qfileinfo.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qheader.h>
#include <qmessagebox.h>

#include <stdlib.h>

/****************************************************************************
 *
 * Class: QTMainWindow
 *
 ****************************************************************************/

QTMainWindow::QTMainWindow( const char *name )
    : QMainWindow( 0, name ), splitter( 0L ),
      oldCurrent( 0 ), preferences( new QTPreferences ),
      save( FALSE )
{
    setupMenu();
    setupCanvas();
    setCaption( tr( "QTranslator [%1]" ).arg( tr( "No Project File" ) ) );
}

QTMainWindow::~QTMainWindow()
{
    if ( messages->firstChild() )
        saveScope();
    if ( preferences->sources.directories.count() > 0 ||
         preferences->sources.extensions.count() > 0 ||
         !preferences->translation.directory.isEmpty() )
        fileSave();
    delete preferences;
}

void QTMainWindow::setupMenu()
{
    QPopupMenu *file = new QPopupMenu( this );
    menuBar()->insertItem( tr( "&File" ), file );

    file->insertItem( tr( "&New Project..." ), this,
                      SLOT( fileNew() ), CTRL + Key_N );
    file->insertItem( tr( "&Open Project File..." ), this,
                      SLOT( fileOpen() ), CTRL + Key_O );

    file->insertSeparator();

    file->insertItem( tr( "&Save Project File" ), this,
                      SLOT( fileSave() ), CTRL + Key_S );
    file->insertItem( tr( "Save Project File &As..." ), this,
                      SLOT( fileSaveAs() ), CTRL + Key_A );

    file->insertSeparator();

    file->insertItem( tr( "&Exit" ), qApp,
                      SLOT( quit() ), CTRL + Key_Q );

    QPopupMenu *edit = new QPopupMenu( this );
    menuBar()->insertItem( tr( "&Edit" ), edit );

    edit->insertItem( tr( "Add new language..." ), this,
                      SLOT( editNewLanguage() ) );

    edit->insertSeparator();

    edit->insertItem( tr( "Preferences..." ), this,
                      SLOT( editPreferences() ) );

    QPopupMenu *tools = new QPopupMenu( this );
    menuBar()->insertItem( tr( "&Tools" ), tools );

    tools->insertItem( tr( "&Create Translation Template" ), this,
                       SLOT( toolsPOT() ) );
    tools->insertItem( tr( "&Merge Transtaions with Template" ), this,
                       SLOT( toolsMsgMgr() ) );
    tools->insertItem( tr( "Create &Qt Message Files" ), this,
                       SLOT( toolsQM() ) );

}

void QTMainWindow::setupCanvas()
{
    bool setCentral = FALSE;
    if ( !splitter ) {
        splitter = new QSplitter( this );
        setCentral = TRUE;
    } else {
        if ( scopes )
            delete scopes;
        if ( messages )
            delete messages;
    }


    scopes = new QListView( splitter );
    scopes->addColumn( tr( "Scope            " ) );
    scopes->show();

    messages = new QTMessageView( splitter );
    messages->addColumn( tr( "Original Message" ) );
    messages->setAllColumnsShowFocus( TRUE );
    messages->header()->setMovingEnabled( FALSE );
    messages->show();

    splitter->setResizeMode( scopes, QSplitter::KeepSize );

    connect( scopes, SIGNAL( currentChanged( QListViewItem * ) ),
             this, SLOT ( fillMessageList( QListViewItem * ) ) );

    if ( setCentral )
        setCentralWidget( splitter );
}

void QTMainWindow::fillScopeList()
{
    QString filename = preferences->translation.directory + "/" +
                       preferences->translation.prefix + "orig.pot";
    if ( filename.isEmpty() || !QFileInfo( filename ).exists() )
        return;

    QFile f( filename );
    if ( !f.open( IO_ReadOnly ) )
        return;

    scopes->clear();

    QTextStream s( &f );

    QString line;
    QStringList lst;
    while ( !s.atEnd() ) {
        line = s.readLine();
        if ( line.left( 5 ) == "msgid" ) {
            int start = line.find( '\"' );
            int end = line.find( "::" );
            if ( start < end && start != -1 && end != -1 ) {
                if ( !lst.contains( line.mid( start + 1, end - start - 1 ) ) &&
                     line.mid( start + 1, end - start - 1 ).length() > 0 ) {
                    ( void )new QListViewItem( scopes, line.mid( start + 1, end - start - 1 ) );
                    lst.append( line.mid( start + 1, end - start - 1 ) );
                }
            }
        }
    }

    f.close();

    oldCurrent = 0;
    scopes->setCurrentItem( scopes->firstChild() );
    scopes->setSelected( scopes->firstChild(), TRUE );
}

QStringList QTMainWindow::getMessages( const QString &filename, const QString &scope )
{
    if ( filename.isEmpty() || !QFileInfo( filename ).exists() )
        return QStringList();

    QFile f( filename );
    if ( !f.open( IO_ReadOnly ) )
        return QStringList();

    QTextStream s( &f );

    QString line;
    QStringList lst;
    while ( !s.atEnd() ) {
        line = s.readLine();
        if ( line.left( 5 ) == "msgid" && line.contains( scope ) ) {
            QString msgid = getMessageID( s, line );
            lst.append( msgid );
        }
    }

    f.close();

    return lst;
}

QMap< QString, QString > QTMainWindow::getTranslatedMessages( const QString &filename, const QString &scope )
{
    if ( filename.isEmpty() || !QFileInfo( filename ).exists() )
        return QMap< QString, QString>();

    QFile f( filename );
    if ( !f.open( IO_ReadOnly ) )
        return QMap< QString, QString>();

    QTextStream s( &f );

    QString line;
    QMap< QString, QString > map;
    while ( !s.atEnd() ) {
        line = s.readLine();
        if ( line.left( 5 ) == "msgid" && line.contains( scope ) ) {
            QString msgid = getMessageID( s, line );
            while ( line.left( 6 ) != "msgstr" )
                line = s.readLine();
            QString msgstr = getMessageStr( s, line );
            map.insert( msgid, msgstr );
        }
    }

    f.close();

    return map;
}

bool QTMainWindow::findTranslation( const QString &msg, QString &t, int col )
{
    QListViewItemIterator it( messages );
    for ( ; it.current(); ++it ) {
        if ( it.current()->text( 0 ) == msg ) {
            t = it.current()->text( col );
            return TRUE;
        }
    }
    return FALSE;
}

void QTMainWindow::saveMessages( const QString &filename, int col, const QString &scope )
{
    if ( filename.isEmpty() || !QFileInfo( filename ).exists() )
        return;

    QFile f( filename );
    if ( !f.open( IO_ReadOnly ) )
        return;

    QTextStream s( &f );

    QStringList out;
    QString line;
    bool justSaved = FALSE;
    while ( !s.atEnd() ) {
        line = s.readLine();
        if ( line.left( 5 ) == "msgid" && line.contains( scope ) ) {
            justSaved = FALSE;
            out.append( line );
            QString msgid = getMessageID( s, line, out );
            QString t;
            if ( findTranslation( msgid, t, col ) ) {
                while ( line.left( 6 ) != "msgstr" ) {
                    line = s.readLine();
                    if ( !line.isEmpty() )
                        out.append( line );
                }
                line = "msgstr \"";
                line += t + "\"";
                out.append( line );
                justSaved = TRUE;
            }
        } else {
            if ( !justSaved || justSaved && line.stripWhiteSpace()[ 0 ] != '\"' ) {
                out.append( line );
                justSaved = FALSE;
            }
        }
    }

    f.close();

    if ( !f.open( IO_WriteOnly ) )
        return;

    s.setDevice( &f );
    QStringList::Iterator it = out.begin();
    for ( ; it != out.end(); ++it )
        s << *it << "\n";

    f.close();
}

void QTMainWindow::saveScope()
{
    if ( !oldCurrent )
        return;

    int col = 1;
    QStringList::Iterator it = preferences->languages.begin();
    for ( ; it != preferences->languages.end(); ++it, ++col ) {
        QString filename;
        if ( !preferences->translation.folders ) {
            filename = preferences->translation.directory + "/" +
                       preferences->translation.prefix + *it + ".po";
        } else {
            bool suffix = preferences->translation.prefix.isEmpty();
            filename = preferences->translation.directory + "/" + *it + "/" +
                       preferences->translation.prefix + ( suffix ? *it : QString::null ) + ".po";
        }

        saveMessages( filename, col, oldCurrent->text( 0 ) );
    }
}

void QTMainWindow::fileNew()
{
    if ( preferences->sources.directories.count() > 0 ||
         preferences->sources.extensions.count() > 0 ||
         !preferences->translation.directory.isEmpty() )
        fileSave();
    if ( messages->firstChild() )
        saveScope();

    preferences->sources.directories.clear();
    preferences->sources.extensions.clear();
    preferences->translation.directory = QString::null;
    preferences->translation.prefix = QString::null;
    preferences->translation.folders = FALSE;
    preferences->languages.clear();
    preferences->projectFile = QString::null;
    setupCanvas();

    setCaption( tr( "QTranslator [%1]" ).arg( tr( "No Project File" ) ) );
}

void QTMainWindow::fileOpen()
{
    if ( preferences->sources.directories.count() > 0 ||
         preferences->sources.extensions.count() > 0 ||
         !preferences->translation.directory.isEmpty() )
        fileSave();
    if ( messages->firstChild() )
        saveScope();

    QString file = QFileDialog::getOpenFileName( preferences->projectFile );
    if ( !file.isEmpty() ) {
        preferences->sources.directories.clear();
        preferences->sources.extensions.clear();
        preferences->translation.directory = QString::null;
        preferences->translation.prefix = QString::null;
        preferences->translation.folders = FALSE;
        preferences->languages.clear();
        preferences->projectFile = QString::null;

        setCaption( tr( "QTranslator [%1]" ).arg( file ) );
        preferences->projectFile = file;
        preferences->readProjectConfig();
        setupCanvas();
        fillScopeList();
        setupMessageList();
    }
}

void QTMainWindow::fileSave()
{
    save = TRUE;
    if ( preferences->projectFile.isEmpty() ) {
        fileSaveAs();
        return;
    }
    preferences->createProjectConfig();
    preferences->saveProjectConfig();
    save = FALSE;
}

void QTMainWindow::fileSaveAs()
{
    if ( save )
        QMessageBox::information( this, tr( "Information" ),
                                  tr( "You haven't saved the configuration of the current\n"
                                      "Project. Please choose now a Project File, into which\n"
                                      "the configuration can be saved" ) );

    QString file = QFileDialog::getSaveFileName( preferences->projectFile );
    if ( !file.isEmpty() ) {
        setCaption( tr( "QTranslator [%1]" ).arg( file ) );
        preferences->projectFile = file;
        fileSave();
    }
}

void QTMainWindow::editNewLanguage()
{
    QTAddLangDia dia( this, "addlangdia" );
    connect( &dia, SIGNAL( newLangChosen( const QString & ) ),
             this, SLOT( addNewLanguage( const QString & ) ) );
    dia.resize( 360, 100 );
    dia.show();
}

void QTMainWindow::editPreferences()
{
    QTPrefDia dia( this, "prefdia", preferences );
    dia.resize( 400, 400 );
    dia.show();
}

void QTMainWindow::toolsPOT()
{
    if ( !configsOk() )
        return;

    QApplication::setOverrideCursor( waitCursor );

    QString findtr = "findtr";
    QString cmd = findtr + " ";

    QStringList::Iterator it1 = preferences->sources.directories.begin();
    for ( ; it1 != preferences->sources.directories.end(); ++it1 ) {
        QStringList::Iterator it2 = preferences->sources.extensions.begin();
        for ( ; it2 != preferences->sources.extensions.end(); ++it2 )
            cmd += *( it1 ) + "/*." + ( *it2 ) + " ";
    }
    cmd += ">" + preferences->translation.directory + "/" +
           preferences->translation.prefix + "orig.pot";

    system( cmd.latin1() );

    QApplication::restoreOverrideCursor();
    if ( QMessageBox::information( this, tr( "Message Merge" ),
                                   tr( "Do you want to merge the existing translations\n"
                                       "with the new translation template?" ), tr( "&Yes" ),
                                   tr ( "&No" ) ) == 0 )
        toolsMsgMgr();

    setupMessageList();
    fillScopeList();
}

void QTMainWindow::toolsMsgMgr()
{
    if ( !configsOk() )
        return;

    QString newname = preferences->translation.directory + "/" +
                      preferences->translation.prefix + "orig.pot";

    QStringList::Iterator it = preferences->languages.begin();
    for ( ; it != preferences->languages.end(); ++it ) {
        QString filename;
        if ( !preferences->translation.folders ) {
            filename = preferences->translation.directory + "/" +
                       preferences->translation.prefix + *it + ".po";
        } else {
            bool suffix = preferences->translation.prefix.isEmpty();
            filename = preferences->translation.directory + "/" + *it + "/" +
                       preferences->translation.prefix + ( suffix ? *it : QString::null ) + ".po";
        }

        QString out = filename + "tmp";
        merge( newname, filename, out );
        QDir dir = QFileInfo( filename ).dir( TRUE );
        dir.rename( out, filename );
    }

}

void QTMainWindow::toolsQM()
{
    if ( !configsOk() )
        return;

    if ( messages->firstChild() )
        saveScope();

    QStringList::Iterator it = preferences->languages.begin();
    for ( ; it != preferences->languages.end(); ++it ) {
        QString filename;
        if ( !preferences->translation.folders ) {
            filename = preferences->translation.directory + "/" +
                       preferences->translation.prefix + *it;
        } else {
            bool suffix = preferences->translation.prefix.isEmpty();
            filename = preferences->translation.directory + "/" + *it + "/" +
                       preferences->translation.prefix + ( suffix ? *it : QString::null );
        }

        translate( filename + ".po", filename + ".qm" );
    }
    QString filename = preferences->translation.directory + "/" +
                       preferences->translation.prefix + "orig";
    translate( filename + ".pot", filename + ".qm" );

}

void QTMainWindow::fillMessageList( QListViewItem *item )
{
    if ( messages->firstChild() )
        saveScope();

    messages->clear();
    oldCurrent = item;

    if ( !item )
        return;

    QString filename = preferences->translation.directory + "/" +
                       preferences->translation.prefix + "orig.pot";

    QStringList lst = getMessages( filename, item->text( 0 ) );
    QStringList::Iterator it = lst.begin();
    for ( ; it != lst.end(); ++it ) {
        QTMessageViewItem *i = new QTMessageViewItem( messages );
        i->setText( 0, *it );
    }

    int col = 1;
    QStringList::Iterator it2 = preferences->languages.begin();
    for ( ; it2 != preferences->languages.end(); ++it2, ++col ) {
        QString filename;
        if ( !preferences->translation.folders ) {
            filename = preferences->translation.directory + "/" +
                       preferences->translation.prefix + *it2 + ".po";
        } else {
            bool suffix = preferences->translation.prefix.isEmpty();
            filename = preferences->translation.directory + "/" + *it2 + "/" +
                       preferences->translation.prefix + ( suffix ? *it2 : QString::null ) + ".po";
        }

        QMap< QString, QString > map = getTranslatedMessages( filename, item->text( 0 ) );
        QListViewItemIterator lit( messages );
        for ( ; lit.current(); ++lit )
            lit.current()->setText( col, *( map.find( lit.current()->text( 0 ) ) ) );
    }

    messages->setCurrentItem( messages->firstChild() );
    messages->setSelected( messages->firstChild(), TRUE );
}

void QTMainWindow::addNewLanguage( const QString &lang )
{
    if ( preferences->languages.find( lang ) != preferences->languages.end() ) {
        QMessageBox::warning( this, tr( "Add Language" ),
                              tr( "The language `%1' already exists." ).arg( lang ),
                              tr( "&OK" ) );
        return;
    }


    if ( !configsOk() )
        return;

    QString filename = preferences->translation.directory + "/" +
                       preferences->translation.prefix + "orig.pot";
    if ( filename.isEmpty() || !QFileInfo( filename ).exists() )
        return;

    QFile f( filename );
    if ( !f.open( IO_ReadOnly ) )
        return;

    QTextStream s( &f );
    QString cont = s.read();

    f.close();

    if ( !preferences->translation.folders ) {
        filename = preferences->translation.directory + "/" +
                   preferences->translation.prefix + lang + ".po";
    } else {
        bool suffix = preferences->translation.prefix.isEmpty();
        filename = preferences->translation.directory + "/";
        QFileInfo( filename ).dir().mkdir( lang );
        filename += lang + "/" +
                    preferences->translation.prefix + ( suffix ? lang : QString::null ) + ".po";
    }

    QFile out( filename );
    if ( !out.open( IO_WriteOnly ) )
        return;

    s.setDevice( &out );
    s << cont;
    out.close();

    messages->addColumn( lang );
    preferences->languages.append( lang );
    fillMessageList( scopes->currentItem() );
}

void QTMainWindow::setupMessageList()
{
    QStringList lst;
    for ( int i = 0;i < messages->header()->count(); ++i )
        lst.append( messages->header()->label( i ) );

    QStringList::Iterator it = preferences->languages.begin();
    for ( ; it != preferences->languages.end(); ++it )
        if ( lst.find( *it ) == lst.end() )
            messages->addColumn( *it );
}

bool QTMainWindow::configsOk()
{
    bool src = FALSE, ext = FALSE, tdir = FALSE;

    if ( preferences->sources.directories.count() > 0 ) {
        src = TRUE;
        QStringList::Iterator it = preferences->sources.directories.begin();
        for ( ; it != preferences->sources.directories.end() && src ; ++it )
            if ( !QFileInfo( *it ).exists() )
                src = FALSE;
    }
    if ( preferences->sources.extensions.count() > 0 )
        ext = TRUE;
    if ( QFileInfo( preferences->translation.directory ).exists() )
        tdir = TRUE;

    bool ok = src && ext && tdir;
    bool openPrefs = FALSE;

    if ( !src )
        openPrefs = QMessageBox::warning( this, tr( "Error"),
                                          tr( "You have specified no Source Directories or one\n"
                                              "or some of them doesn't exist. Do you want to correct\n"
                                              "this now?" ), tr( "&Yes" ), tr( "&No" ) ) == 0;
    else if ( !ext )
        openPrefs = QMessageBox::warning( this, tr( "Error"),
                                          tr( "You have not specified Extensions of Source files which\n"
                                              "should get translated. Do you want to correct this now?" ),
                                          tr( "&Yes" ), tr( "&No" ) ) == 0;
    else if ( !tdir )
        openPrefs = QMessageBox::warning( this, tr( "Error"),
                                          tr( "You have specified the directory in which the Translations\n"
                                              "should be stored.. Do you want to correct this now?" ),
                                          tr( "&Yes" ), tr( "&No" ) ) == 0;

    if ( openPrefs )
        editPreferences();

    return ok;
}

QString QTMainWindow::parseItem( QTextStream &t, QString &line, QString type, QStringList *lst )
{
    QString item;

    QString tmp;
    if ( line.left( type.length() ) == type ) {
        int start = line.find( '\"' );
        int end = line.findRev( '\"', line.length() - 1 );
        item = line.mid( start + 1, end - start - 1 );


        line = t.readLine();
        if ( lst && line.left( 6 ) != "msgstr" &&
             line.left( 5 ) != "msgid" )
            lst->append( line );
        tmp = line.stripWhiteSpace();
        while ( tmp[ 0 ] == '\"' ) {
            start = line.find( '\"' );
            end = line.findRev( '\"', line.length() - 1 );
            item += line.mid( start + 1, end - start - 1 );

            line = t.readLine();
            if ( lst && line.left( 6 ) != "msgstr" &&
                 line.left( 5 ) != "msgid" )
                lst->append( line );
            tmp = line.stripWhiteSpace();
        }
    }

    return item;
}

QString QTMainWindow::getMessageID( QTextStream &t, QString &line )
{
    QString msgid = parseItem( t, line, "msgid" );
    int cc = msgid.find( "::" );
    msgid.remove( 0, cc + 2 );

    return msgid;
}

QString QTMainWindow::getMessageID( QTextStream &t, QString &line, QStringList &out )
{
    QString msgid = parseItem( t, line, "msgid", &out );
    int cc = msgid.find( "::" );
    msgid.remove( 0, cc + 2 );

    return msgid;
}

QString QTMainWindow::getMessageStr( QTextStream &t, QString &line )
{
    return parseItem( t, line, "msgstr" );
}

