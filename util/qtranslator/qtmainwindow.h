/****************************************************************************
** $Id: //depot/qt/main/util/qtranslator/qtmainwindow.h#5 $
**
** This is a utility program for translating Qt applications
**
**
** Copyright (C) 1999 by Trolltech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QTMAINWINDOW
#define QTMAINWINDOW

#include <qmainwindow.h>
#include <qstring.h>
#include <qmap.h>
#include <qstringlist.h>
#include <qtextstream.h>

class QTPreferences;
class QTMessageView;

class QListView;
class QListViewItem;
class QSplitter;

/****************************************************************************
 *
 * Class: QTMainWindow
 *
 ****************************************************************************/

class QTMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    QTMainWindow( const char *name );
    ~QTMainWindow();

protected:
    void setupMenu();
    void setupCanvas();
    void fillScopeList();
    QStringList getMessages( const QString &filename, const QString &scope );
    QMap< QString, QString > getTranslatedMessages( const QString &filename, const QString &scope );
    bool findTranslation( const QString &msg, QString &t, int col );
    void saveMessages( const QString &filename, int col, const QString &scope );
    void saveScope();

    QSplitter *splitter;
    QListView *scopes;
    QTMessageView *messages;
    QListViewItem *oldCurrent;

    QTPreferences *preferences;
    bool save;

protected slots:
    void fileNew();
    void fileOpen();
    void fileSave();
    void fileSaveAs();
    void editNewLanguage();
    void editPreferences();
    void toolsPOT();
    void toolsMsgMgr();
    void toolsQM();

    void fillMessageList( QListViewItem * );
    void addNewLanguage( const QString &lang );
    void setupMessageList();
    bool configsOk();
    QString getMessageID( QTextStream &t, QString &line );
    QString getMessageID( QTextStream &t, QString &line, QStringList &out );
    QString getMessageStr( QTextStream &t, QString &line );
    QString parseItem( QTextStream &t, QString &line, QString type, QStringList *lst = 0 );

};

#endif
