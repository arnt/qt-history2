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

#ifndef TRWINDOW_H
#define TRWINDOW_H

#include "phrase.h"

#include <metatranslator.h>
#include <qmainwindow.h>
#include <qhash.h>
#include <qprinter.h>

class QModelIndex;
class QStringList;
class QPixmap;
class QAction;
class QDialog;
class QLabel;
class QMenu;
class QAssistantClient;
template <typename T> class QList;
class QIcon;

class QTreeView;
class PhraseModel;
class PhraseItem;
class MessageModel;
class MessageItem;
class ContextModel;
class ContextItem;
class FindDialog;
class MessageEditor;
class Statistics;

#define TREEVIEW_ODD_COLOR QColor(235,245,255)

class TrWindow : public QMainWindow
{
    Q_OBJECT
public:
    enum {PhraseCloseMenu, PhraseEditMenu, PhrasePrintMenu};

    static QPixmap *pxOn;
    static QPixmap *pxOff;
    static QPixmap *pxObsolete;
    static QPixmap *pxDanger;
    static QPixmap *pxObs;
    static QPixmap *pxEmpty;
    static const QPixmap pageCurl();

    TrWindow();
    ~TrWindow();

    void openFile(const QString &name);

protected:
    void readConfig();
    void writeConfig();
    void closeEvent(QCloseEvent *);

signals:
    void statsChanged(int w, int c, int cs, int w2, int c2, int cs2);

private slots:
    void sortContexts(int section, Qt::MouseButton state);
    void sortMessages(int section, Qt::MouseButton state);
    void sortPhrases(int section, Qt::MouseButton state);
    void doneAndNext();
    void prev();
    void next();
    void recentFileActivated(QAction *action);
    void setupRecentFilesMenu();
    void open();
    void save();
    void saveAs();
    void release();
    void print();
    void find();
    void findAgain();
    void newPhraseBook();
    void openPhraseBook();
    void closePhraseBook(QAction *action);
    void editPhraseBook(QAction *action);
    void printPhraseBook(QAction *action);
    void manual();
    void revertSorting();
    void about();
    void aboutQt();
    void updateViewMenu();

    void setupPhrase();
    bool maybeSave();
    void updateCaption();
    void showNewScope(const QModelIndex &current, const QModelIndex &old);
    void showNewCurrent(const QModelIndex &current, const QModelIndex &old);
    void updateTranslation(const QString &translation);
    void updateFinished(bool finished);
    void toggleFinished(const QModelIndex &index, Qt::MouseButton);
    void prevUnfinished();
    void nextUnfinished();
    void findNext(const QString &text, int where, bool matchCase);
    void revalidate();
    void toggleGuessing();
    void focusSourceList();
    void focusPhraseList();
    void toggleStatistics();
    void updateStatistics();
    void onWhatsThis();

private:

    typedef QHash<QString, PhraseBook> PBD;

    static QString friendlyString(const QString &str);

    int findCurrentContextRow();
    int findCurrentMessageRow();
    bool setNextMessage(int *currentrow, bool checkUnfinished);
    bool setPrevMessage(int *currentrow, bool checkUnfinished);
    bool setNextContext(int *currentrow, bool checkUnfinished);
    bool setPrevContext(int *currentrow, bool checkUnfinished);
    bool next(bool checkUnfinished);
    bool prev(bool checkUnfinished);

    void addRecentlyOpenedFile(const QString &fn, QStringList &lst);
    void setupMenuBar();
    void setupToolBars();
    void setCurrentContextRow(int row);
    void setCurrentMessageRow(int row);
    void setCurrentContext(const QModelIndex &indx);
    void setCurrentMessage(const QModelIndex &indx);
    QString friendlyPhraseBookName(const PhraseBook &pb) const;
    PhraseBook phraseBookFromFileName(QString name) const;
    bool openPhraseBook(const QString &name);
    bool phraseBooksContains(QString name);
    bool savePhraseBook(QString &name, const PhraseBook &pb);
    void updateProgress();
    void updatePhraseDict();
    PhraseBook getPhrases(const QString &source);
    bool danger(const QString &source, const QString &translation,
        bool verbose = false);

    void insertMessage(MessageItem *m);
    void printDanger(MessageItem *m);
    bool updateDanger(MessageItem *m, bool verbose = false);

    bool searchItem(const QString &searchWhat, int c, int m);
    void doCharCounting( const QString& text, int& trW, int& trC, int& trCS );

    QAssistantClient *ac;
    QTreeView *tv;
    ContextModel *cmdl;
    QTreeView *stv;
    MessageModel *mmdl;
    QTreeView *ptv;
    PhraseModel *pmdl;
    MessageEditor * me;
    QLabel        * progress;
    QLabel        * modified;
    MetaTranslator tor;
    bool dirty;
    int  numFinished;
    int  numNonobsolete;
    int  numMessages;
    QStringList recentFiles;
    QString     filename;
    PBD phraseDict;
    QMap<QAction *, PhraseBook> phraseBooks[3];
    QPrinter printer;

    FindDialog *finddlg;
    QString findText;
    int findWhere;
    bool findMatchCase;
    int foundWhere;
    int foundOffset;

    QDockWidget *dwScope;

    QMenu *phrasep;
    QMenu *closePhraseBookp;
    QMenu *editPhraseBookp;
    QMenu *printPhraseBookp;
    QMenu *recentFilesMenu;
    QMenu *tbMenu;

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
    Statistics *stats;
    int  srcWords;
    int  srcChars;
    int  srcCharsSpc;
};

#endif
