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

#ifndef TRWINDOW_H
#define TRWINDOW_H

#include "phrase.h"

#include <metatranslator.h>

#include <qmainwindow.h>
#include <qlist.h>
#include <qhash.h>
#include <qstringlist.h>
#include <qpixmap.h>
#include <q3listview.h>

#if 0 /// ### enable me
#include <qprinter.h>
#endif


class QAction;
class QDialog;
class QLabel;
class Q3ListView;
class Q3ListViewItem;
class QMenu;

class FindDialog;
class MessageEditor;

class PhraseLV;
class ContextLVI;
class Statistics;

class TrWindow : public QMainWindow
{
    Q_OBJECT
public:
    static QPixmap * pxOn;
    static QPixmap * pxOff;
    static QPixmap * pxObsolete;
    static QPixmap * pxDanger;
    static const QPixmap pageCurl();

    TrWindow();
    ~TrWindow();

    void openFile( const QString& name );

protected:
    void readConfig();
    void writeConfig();
    void closeEvent( QCloseEvent * );

signals:
    void statsChanged( int w, int c, int cs, int w2, int c2, int cs2 );

private slots:
    void doneAndNext();
    void prev();
    void next();
    void recentFileActivated( int );
    void setupRecentFilesMenu();
    void open();
    void save();
    void saveAs();
    void release();
    void print();
    void find();
    void findAgain();
    void replace();
    void newPhraseBook();
    void openPhraseBook();
    void closePhraseBook( int id );
    void editPhraseBook( int id );
    void printPhraseBook( int id );
    void manual();
    void revertSorting();
    void about();
    void aboutQt();

    void setupPhrase();
    bool maybeSave();
    void updateCaption();
    void showNewScope( Q3ListViewItem *item );
    void showNewCurrent( Q3ListViewItem *item );
    void updateTranslation( const QString& translation );
    void updateFinished( bool finished );
    void toggleFinished( Q3ListViewItem *item, const QPoint& p, int column );
    void prevUnfinished();
    void nextUnfinished();
    void findNext( const QString& text, int where, bool matchCase );
    void revalidate();
    void toggleGuessing();
    void focusSourceList();
    void focusPhraseList();
    void updateClosePhraseBook();
    void toggleStatistics();
    void updateStatistics();

private:
    static QIconSet loadPixmap(const QString &imageName);

    typedef QList<PhraseBook> PBL;
    typedef QHash<QString, PhraseBook> PBD;

    static QString friendlyString( const QString& str );

    void addRecentlyOpenedFile( const QString & fn, QStringList & lst );
    void setupMenuBar();
    void setupToolBars();
    void setCurrentContextItem( Q3ListViewItem *item );
    void setCurrentMessageItem( Q3ListViewItem *item );
    QString friendlyPhraseBookName( int k );
    bool openPhraseBook( const QString& name );
    bool savePhraseBook( QString& name, const PhraseBook& pb );
    void updateProgress();
    void updatePhraseDict();
    PhraseBook getPhrases( const QString& source );
    bool danger( const QString& source, const QString& translation,
                 bool verbose = FALSE );

    int itemToIndex( Q3ListView * view, Q3ListViewItem * item );
    Q3ListViewItem * indexToItem( Q3ListView * view, int index );
    bool searchItem( const QString & searchWhat, Q3ListViewItem * j,
                     Q3ListViewItem * k );
    void doCharCounting( const QString& text, int& trW, int& trC, int& trCS );

    Q3ListView     * plv;
    Q3ListView     * lv;
    Q3ListView     * slv;
    MessageEditor * me;
    QLabel        * progress;
    QLabel        * modified;
    MetaTranslator tor;
    bool dirty;
    bool messageIsShown;
    int  numFinished;
    int  numNonobsolete;
    int  numMessages;
    int  dirtyItem;
    QStringList recentFiles;
    QString     filename;

    PBD phraseDict;
    PBL phraseBooks;
    QStringList phraseBookNames;

#if 0 /// ### enable me
    QPrinter printer;
#endif

    FindDialog *f;
    FindDialog *h;
    QString findText;
    int findWhere;
    bool findMatchCase;
    int foundItem;
    Q3ListViewItem *foundScope;
    int foundWhere;
    int foundOffset;

    QMenu *phrasep;
    QMenu *closePhraseBookp;
    QMenu *editPhraseBookp;
    QMenu *printPhraseBookp;
    QMenu *recentFilesMenu;
    QAction *closePhraseBookId;
    QAction *editPhraseBookId;
    QAction *printPhraseBookId;
    QAction *openAct;
    QAction *saveAct;
    QAction *saveAsAct;
    QAction *releaseAct;
    QAction *printAct;
    QAction *exitAct;
    QAction *undoAct;
    QAction *redoAct;
    QAction *cutAct;
    QAction *copyAct;
    QAction *pasteAct;
    QAction *selectAllAct;
    QAction *findAct;
    QAction *findAgainAct;
    QAction *replaceAct;
    QAction *newPhraseBookAct;
    QAction *openPhraseBookAct;
    QAction *acceleratorsAct;
    QAction *endingPunctuationAct;
    QAction *phraseMatchesAct;
    QAction *revertSortingAct;
    QAction *aboutAct;
    QAction *aboutQtAct;
    QAction *manualAct;
    QAction *whatsThisAct;
    QAction *beginFromSourceAct;
    QAction *prevAct;
    QAction *nextAct;
    QAction *prevUnfinishedAct;
    QAction *nextUnfinishedAct;
    QAction *doneAndNextAct;
    QAction *doneAndNextAlt;
    QAction *doGuessesAct;
    QAction *toggleStats;
    Statistics * stats;
    int  srcWords;
    int  srcChars;
    int  srcCharsSpc;
};

#endif
