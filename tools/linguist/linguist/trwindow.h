/**********************************************************************
**   Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
**   trwindow.h
**
**   This file is part of Qt Linguist.
**
**   See the file LICENSE included in the distribution for the usage
**   and distribution terms.
**
**   The file is provided AS IS with NO WARRANTY OF ANY KIND,
**   INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR
**   A PARTICULAR PURPOSE.
**
**********************************************************************/

#ifndef TRWINDOW_H
#define TRWINDOW_H

#include "phrase.h"

#include <qmainwindow.h>
#include <qlist.h>
#include <qdict.h>
#include <qprinter.h>
#include <qstringlist.h>

#include <qlistview.h>

#include <metatranslator.h>

class QDialog;
class QLabel;
class QListView;
class QListViewItem;
class QPrinter;

class Action;
class FindDialog;
class MessageEditor;

class PhraseLV;
class ContextLVI;

class TrWindow : public QMainWindow
{
    Q_OBJECT
public:
    static QPixmap * pxOn;
    static QPixmap * pxOff;
    static QPixmap * pxObsolete;
    static QPixmap * pxDanger;
    static const QPixmap splash();
    static const QPixmap pageCurl();

    TrWindow();
    ~TrWindow();

    void openFile( const QString& name );

protected:
    void readConfig();
    void writeConfig();
    
private slots:
    void doneAndNext();
    void prev();
    void next();
    void recentFileActivated( int );
    void setupRecentFilesMenu();
    void exitApp();
    void open();
    void save();
    void saveAs();
    void release();
    void print();
    void find();
    void findAgain();
    void newPhraseBook();
    void openPhraseBook();
    void closePhraseBook( int id );
    void editPhraseBook( int id );
    void printPhraseBook( int id );
    void overview();
    void revertSorting();
    void about();
    void aboutQt();
    
    void setupPhrase();
    bool maybeSave();
    void updateCaption();
    void showNewScope( QListViewItem *item );
    void showNewCurrent( QListViewItem *item );
    void updateTranslation( const QString& translation );
    void updateFinished( bool finished );
    void toggleFinished( QListViewItem *item, const QPoint& p, int column );
    void prevUnfinished();
    void nextUnfinished();
    void findNext( const QString& text, int where, bool matchCase );
    void revalidate();

private:
    typedef QList<PhraseBook> PBL;
    typedef QDict<PhraseBook> PBD;

    static void setupImageDict();
    static QString friendlyString( const QString& str );

    void addRecentlyOpenedFile( const QString & fn, QStringList & lst );
    void setupMenuBar();
    void setupToolBars();
    void setCurrentContextItem( QListViewItem *item );
    void setCurrentMessageItem( QListViewItem *item );
    QString friendlyPhraseBookName( int k );
    bool openPhraseBook( const QString& name );
    bool savePhraseBook( const QString& name, const PhraseBook& pb );
    void updateProgress();
    void updatePhraseDict();
    PhraseBook getPhrases( const QString& source );
    bool danger( const QString& source, const QString& translation,
		 bool verbose = FALSE );
    bool searchItem( const QString & searchWhere, QListViewItem * j,
		     QListViewItem * k );
    
    int             itemToIndex( QListView * view, QListViewItem * item );
    QListViewItem * indexToItem( QListView * view, int index );
    
    QListView     * plv;
    QListView     * lv;
    QListView     * slv;
    MessageEditor * me;
    QLabel        * progress;
    QLabel        * modified;
    MetaTranslator tor;
    bool dirty;
    bool messageIsShown;
    int  numFinished;
    int  numNonobsolete;
    int  numMessages;
    QStringList recentFiles;
    QString     filename;

    PBD phraseDict;
    PBL phraseBooks;
    QStringList phraseBookNames;

    QPrinter printer;
    QDialog *ov;

    FindDialog *f;
    QString findText;
    int findWhere;
    bool findMatchCase;
    int foundItem;
    QListViewItem *foundScope;
    int foundWhere;
    int foundOffset;

    QPopupMenu * phrasep;
    QPopupMenu * closePhraseBookp;
    QPopupMenu * editPhraseBookp;
    QPopupMenu * printPhraseBookp;
    QPopupMenu * recentFilesMenu;
    int closePhraseBookId;
    int editPhraseBookId;
    int printPhraseBookId;
    Action *openAct;
    Action *saveAct;
    Action *saveAsAct;
    Action *releaseAct;
    Action *printAct;
    Action *exitAct;
    Action *undoAct;
    Action *redoAct;
    Action *cutAct;
    Action *copyAct;
    Action *pasteAct;
    Action *deleteAct;
    Action *selectAllAct;
    Action *findAct;
    Action *findAgainAct;
    Action *newPhraseBookAct;
    Action *openPhraseBookAct;
    Action *acceleratorsAct;
    Action *endingPunctuationAct;
    Action *phraseMatchesAct;
    Action *revertSortingAct;
    Action *bigIconsAct;
    Action *textLabelsAct;
    Action *aboutAct;
    Action *aboutQtAct;
    Action *overviewAct;
    Action *whatsThisAct;
    Action *startFromSourceAct;
    Action *guessAct;
    Action *prevAct;
    Action *nextAct;
    Action *prevUnfinishedAct;
    Action *nextUnfinishedAct;
    Action *finishedAndNextAct;
    
};

#endif
