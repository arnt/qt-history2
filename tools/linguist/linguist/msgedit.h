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

#ifndef MSGEDIT_H
#define MSGEDIT_H

#include "trwindow.h"
#include "phrase.h"

#include <qstring.h>
#include <qpixmap.h>
#include <qbitmap.h>
#include <qtooltip.h>
#include <qevent.h>
#include <qshortcut.h>
#include <qframe.h>
#include <qtextedit.h>
#include <qscrollarea.h>
#include <QTextCharFormat>

template <typename T> class QList;
class QSplitter;
class QDockWidget;
class QLabel;
class QTreeView;
class QVBoxLayout;
class EditorPage;
class MetaTranslator;
class QMenu;
class MessageEditor;

class SourceTextEdit : public QTextEdit
{
    Q_OBJECT
public:
    SourceTextEdit(QWidget *parent = 0);

public slots:
    void copySelection();

protected:
    void contextMenuEvent(QContextMenuEvent *e);

private:
    QAction *actCopy;
    QAction *actSelect;
    QMenu *srcmenu;
};

class GuessShortcut : public QShortcut
{
    Q_OBJECT
public:
    GuessShortcut(int nkey, QWidget *parent, const char *member)
        : QShortcut(parent), nrkey(nkey)
    {
        setKey(Qt::CTRL + (Qt::Key_1 + nrkey));
        connect(this, SIGNAL(activated()), this, SLOT(keyActivated()));
        connect(this, SIGNAL(activated(int)), parent, member);
    }

private slots:
    void keyActivated()
    {
        emit activated(nrkey);
    }

signals:
    void activated(int nkey);

private:
    int nrkey;
};

class PageCurl : public QWidget
{
    Q_OBJECT
public:
    PageCurl(QWidget *parent = 0)
        : QWidget(parent)
    {
        QPixmap px = TrWindow::pageCurl();
        if ( px.hasAlphaChannel() ) {
            setMask( px.mask() );
        }
        QPalette pal = palette();
        pal.setBrush(backgroundRole(), px);
        setPalette(pal);
        setFixedSize( px.size() );
    }

protected:
    bool event(QEvent *e)
    {
        if (e->type() == QEvent::ToolTip && toolTip().isEmpty()) {
            QRect r(34, 0, width()-34, 19);

            QPoint pt = static_cast<QHelpEvent*>(e)->pos();

            if (r.contains(pt)) {
                QToolTip::showText(static_cast<QHelpEvent*>(e)->globalPos(),
                    tr("Next unfinished phrase"), this);
            }

            r.setSize(QSize(width()-34, height()-20));
            r.setX(0);
            r.setY(20);

            if (r.contains(pt)) {
                QToolTip::showText(static_cast<QHelpEvent*>(e)->globalPos(),
                    tr("Previous unfinished phrase"), this);
            }
        }

        return QWidget::event(e);
    }
    void mouseReleaseEvent(QMouseEvent *e)
    {
        int x = e->pos().x()-10;
        int y = e->pos().y();

        if (y < x)
            emit nextPage();
        else
            emit prevPage();
    }

signals:
    void prevPage();
    void nextPage();
};

class ShadowWidget : public QWidget
{
public:
    ShadowWidget(QWidget *parent = 0);
    ShadowWidget(QWidget *child, QWidget *parent = 0);

    void setShadowWidth(int width) {sWidth = width;}
    int  shadowWidth() const {return sWidth;}
    void setMargin(int margin) {wMargin = margin;}
    int  margin() const {return wMargin;}
    void setWidget(QWidget *child);

protected:
    void resizeEvent(QResizeEvent *e);
    void paintEvent(QPaintEvent *e);

private:
    int sWidth;
    int wMargin;
    QWidget *childWgt;
};

class EditorPage : public QFrame
{
    Q_OBJECT
public:
    EditorPage(MessageEditor *parent = 0, const char *name = 0);

protected:
    void resizeEvent(QResizeEvent *);
    void layoutWidgets();
    void updateCommentField();
    void calculateFieldHeight(QTextEdit *field);
    void fontChange(const QFont &);

private:
    PageCurl *pageCurl;
    QLabel *srcTextLbl;
    QLabel *transLbl;
    SourceTextEdit *srcText;
    QTextEdit *cmtText;
    QTextEdit *transText;

    friend class MessageEditor;

private slots:
    void handleTranslationChanges();
    void handleSourceChanges();
    void handleCommentChanges();

signals:
    void pageHeightUpdated(int height);
};

class MessageEditor : public QScrollArea
{
    Q_OBJECT
public:
    MessageEditor(MetaTranslator *t, QMainWindow *parent = 0);
    QTreeView *sourceTextView() const;
    QTreeView *phraseView() const;
    inline QDockWidget *sourceDockWnd() const {return topDockWnd;}
    inline QDockWidget *phraseDockWnd() const {return bottomDockWnd;}

    void showNothing();
    void showMessage(const QString &text, const QString &comment,
        const QString &fullContext, const QString &translation,
        MetaTranslatorMessage::Type type,
        const QList<Phrase> &phrases);
    bool eventFilter(QObject *, QEvent *);

signals:
    void translationChanged(const QString &translation);
    void finished(bool finished);
    void prevUnfinished();
    void nextUnfinished();
    void updateActions(bool enable);

    void undoAvailable(bool avail);
    void redoAvailable(bool avail);
    void cutAvailable(bool avail);
    void copyAvailable(bool avail);
    void pasteAvailable(bool avail);

    void focusSourceList();
    void focusPhraseList();

public slots:
    void undo();
    void redo();
    void cut();
    void copy();
    void paste();
    void selectAll();
    void beginFromSource();
    void toggleGuessing();
    void setEditorFocus();

private slots:
    void emitTranslationChanged();
    void guessActivated(int key);
    void insertPhraseInTranslation(const QModelIndex &index);
    void insertPhraseInTranslationAndLeave(const QModelIndex &index);
    void updateButtons();
    void updateCanPaste();

    void updatePageHeight(int height);

protected:
    void resizeEvent(QResizeEvent *);

private:
    static const char backTab[];
    static const char * const friendlyBackTab[];

    void visualizeBackTabs(const QString &text, QTextEdit *te);
    void setTranslation(const QString &translation, bool emitt);
    void setEditionEnabled(bool enabled);

    QTreeView *srcTextView;
    MessageModel *srcMdl;
    QDockWidget *topDockWnd, *bottomDockWnd;
    EditorPage *editorPage;

    QLabel * phraseLbl;
    QTreeView *phraseTv;
    PhraseModel *phrMdl;
    QTextCharFormat defFormat;

    ShadowWidget *sw;

    MetaTranslator *tor;
    QString sourceText;
    bool mayOverwriteTranslation;
    bool canPaste;
    bool doGuesses;
};

#endif
