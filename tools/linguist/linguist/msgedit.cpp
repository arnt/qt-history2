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

/*  TRANSLATOR MsgEdit

  This is the right panel of the main window.
*/

#include "msgedit.h"

#include "trwindow.h"
#include "simtexth.h"
#include "messagemodel.h"
#include "phrasemodel.h"

#include <qmenu.h>
#include <qaction.h>
#include <qapplication.h>
#include <qclipboard.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qtextedit.h>
#include <qpalette.h>
#include <qstring.h>
#include <qpainter.h>
#include <qscrollbar.h>
#include <qheaderview.h>
#include <qshortcut.h>
#include <qregexp.h>
#include <qdockwidget.h>
#include <qfont.h>
#include <qtreeview.h>
#include <qscrollarea.h>
#include <qtextdocumentfragment.h>
#include <qtextcursor.h>
#include <QTextBlock>
#include <QAbstractTextDocumentLayout>

static const int MaxCandidates = 5;

const char MessageEditor::backTab[] = "\a\b\f\n\r\t";
const char * const MessageEditor::friendlyBackTab[] = {
        QT_TRANSLATE_NOOP("MessageEditor", "bell"),
        QT_TRANSLATE_NOOP("MessageEditor", "backspace"),
        QT_TRANSLATE_NOOP("MessageEditor", "new page"),
        QT_TRANSLATE_NOOP("MessageEditor", "new line"),
        QT_TRANSLATE_NOOP("MessageEditor", "carriage return"),
        QT_TRANSLATE_NOOP("MessageEditor", "tab")
    };

void MessageEditor::visualizeBackTabs(const QString &text, QTextEdit *te)
{
    te->clear();
    QTextCursor tc(te->textCursor());
    QTextCharFormat blueFormat = defFormat;
    blueFormat.setForeground(QBrush(Qt::blue));
    blueFormat.setFontItalic(true);
    blueFormat.setProperty(QTextFormat::UserProperty, -1);

    QString plainText;
    for (int i = 0; i < (int) text.length(); ++i)
    {
        int ch = text[i].unicode();
        if (ch < 0x20)
        {
            if (!plainText.isEmpty())
            {
                tc.insertText(plainText, defFormat);
                plainText.clear();
            }
            const char *p = strchr(backTab, ch);
            // store the character in the user format property
            // in the first '(' in the phrase
            blueFormat.setProperty(QTextFormat::UserProperty, ch);
            tc.insertText(QString("("), blueFormat);
            blueFormat.setProperty(QTextFormat::UserProperty, -1);
            if (p == 0)
            {
                tc.insertText(QString::number(ch, 16) + ")", blueFormat);
            }
            else
            {
                tc.insertText(MessageEditor::tr(friendlyBackTab[p - backTab]) + ")",
                    blueFormat);
                if (backTab[p - backTab] == '\n')
                    tc.insertBlock();
            }
        }
        // if a space is by itself, at the end, or beside other spaces
        else if (ch == ' ')
        {
            if (i == 0 || i == text.length() - 1 || text[i - 1].isSpace() ||
                text[i + 1].isSpace())
            {
                blueFormat.setProperty(QTextFormat::UserProperty, ch);
                tc.insertText(QString("("), blueFormat);
                blueFormat.setProperty(QTextFormat::UserProperty, -1);
                tc.insertText(MessageEditor::tr("sp)"), blueFormat);
            }
            else
            {
                plainText += ' ';
            }
        }
        else
        {
            plainText += QString(ch);
        }
    }
    tc.insertText(plainText, defFormat);
}

SourceTextEdit::SourceTextEdit(QWidget *parent) : QTextEdit(parent)
{
    srcmenu = 0;
    actCopy = new QAction(tr("&Copy"), this);
    actCopy->setShortcut(QKeySequence(tr("Ctrl+C")));
    actSelect = new QAction(tr("Select &All"), this);
    actSelect->setShortcut(QKeySequence(tr("Ctrl+A")));
    connect(actCopy, SIGNAL(triggered()), this, SLOT(copySelection()));
    connect(actSelect, SIGNAL(triggered()), this, SLOT(selectAll()));
}

void SourceTextEdit::copySelection()
{
    QTextDocumentFragment tdf = textCursor().selection();
    QTextDocument td;
    QTextCursor tc(&td);
    tc.insertFragment(tdf);
    int ch;

    tc.movePosition(QTextCursor::Start);
    while(!tc.atEnd())
    {
        tc.movePosition(QTextCursor::NextCharacter);
        ch = tc.charFormat().intProperty(QTextFormat::UserProperty);
        if (ch != 0) // if wrong format
        {
            // delete char
            tc.deletePreviousChar();
            if (ch != -1) // insert backtab
                tc.insertText(QString(ch));
        }
    }

    QApplication::clipboard()->setText(td.toPlainText());
}

void SourceTextEdit::contextMenuEvent(QContextMenuEvent *e)
{
    if (!srcmenu)
    {
        srcmenu = new QMenu(this);
        srcmenu->addAction(actCopy);
        srcmenu->addAction(actSelect);
    }

    actCopy->setEnabled(textCursor().hasSelection());
    actSelect->setEnabled(!document()->isEmpty());

    srcmenu->popup(e->globalPos());
}

/*
   ShadowWidget class impl.

   Used to create a shadow like effect for a widget
*/
ShadowWidget::ShadowWidget(QWidget *parent)
    : QWidget(parent), sWidth(10), wMargin(3), childWgt(0)
{

}

ShadowWidget::ShadowWidget(QWidget *child, QWidget *parent)
    : QWidget(parent), sWidth(10), wMargin(3), childWgt(0)
{
    setWidget(child);
}

void ShadowWidget::setWidget(QWidget *child)
{
    childWgt = child;
    if (childWgt && childWgt->parent() != this) {
        childWgt->setParent(this);
        childWgt->move(0,0);
        childWgt->show();
    }
}

void ShadowWidget::resizeEvent(QResizeEvent *)
{
    if(childWgt) {
        childWgt->move(wMargin, wMargin);
        childWgt->resize(width() - sWidth - wMargin, height() - sWidth -
            wMargin);
    }
}

void ShadowWidget::paintEvent(QPaintEvent *e)
{
    QPainter p;
    int w = width() - sWidth;
    int h = height() - sWidth;


    if (!((w > 0) && (h > 0)))
        return;

    if (p.begin(this)) {
        p.setPen(palette().color(QPalette::Shadow));

        p.drawPoint(w + 5, 6);
        p.drawLine(w + 3, 6, w + 5, 8);
        p.drawLine(w + 1, 6, w + 5, 10);
        int i;
        for (i=7; i < h; i += 2)
            p.drawLine( w, i, w + 5, i + 5);
        for (i = w - i + h; i > 6; i -= 2)
            p.drawLine( i, h, i + 5, h + 5);
        for (; i > 0 ; i -= 2)
            p.drawLine( 6, h + 6 - i, i + 5, h + 5);

        p.end();
    }
    QWidget::paintEvent(e);
}

/*
   EditorPage class impl.

   A frame that contains the source text, translated text and any
   source code comments and hints.
*/
EditorPage::EditorPage(MessageEditor *parent, const char *name)
    : QFrame(parent)
{
    setObjectName(name);
    setLineWidth(1);
    setFrameStyle(QFrame::Box | QFrame::Plain);

    // Use white explicitly as the background color for the editor page.
    QPalette p = palette();
    p.setColor(QPalette::Active, QPalette::Base, QColor(Qt::white));
    p.setColor(QPalette::Inactive, QPalette::Base, QColor(Qt::white));
    p.setColor(QPalette::Disabled, QPalette::Base, QColor(Qt::white));
    p.setColor(QPalette::Active, QPalette::Background,
                p.color(QPalette::Active, QPalette::Base));
    p.setColor(QPalette::Inactive, QPalette::Background,
                p.color(QPalette::Inactive, QPalette::Base));
    p.setColor(QPalette::Disabled, QPalette::Background,
                p.color(QPalette::Disabled, QPalette::Base));

    parent->setPalette(p);

    srcTextLbl = new QLabel(tr("Source text"), this);
    transLbl   = new QLabel(tr("Translation"), this);

    QFont fnt = font();
    fnt.setBold(true);
    srcTextLbl->setFont(fnt);
    transLbl->setFont(fnt);

    srcText = new SourceTextEdit(this);
    srcText->setFrameStyle(QFrame::NoFrame);
    srcText->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding,
        QSizePolicy::Minimum));
    srcText->setAutoFormatting(QTextEdit::AutoNone);
    srcText->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    srcText->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    p = srcText->palette();
    p.setColor(QPalette::Disabled, QPalette::Base, p.color(QPalette::Active, QPalette::Base));
    srcText->setPalette( p );
	srcText->setReadOnly(true);
    connect(srcText->document(), SIGNAL(contentsChanged()), SLOT(handleSourceChanges()));

    cmtText = new QTextEdit(this);
    cmtText->setObjectName("comment/context view");
    cmtText->setFrameStyle( QFrame::NoFrame );
    cmtText->setSizePolicy( QSizePolicy( QSizePolicy::MinimumExpanding,
                                         QSizePolicy::Minimum ) );
    cmtText->setAutoFormatting(QTextEdit::AutoNone);
    cmtText->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    cmtText->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    p = cmtText->palette();
    p.setColor(QPalette::Active, QPalette::Base, QColor(236,245,255));
    p.setColor(QPalette::Inactive, QPalette::Base, QColor(236,245,255));
    cmtText->setPalette(p);
	cmtText->setReadOnly(true);
    connect(cmtText->document(), SIGNAL(contentsChanged()), SLOT(handleCommentChanges()));

    transText = new QTextEdit(this);
    transText->setObjectName("translation editor");
    transText->setFrameStyle(QFrame::NoFrame);
    transText->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding,
                                             QSizePolicy::MinimumExpanding));
    transText->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    transText->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    transText->setAutoFormatting(QTextEdit::AutoNone);
    transText->setLineWrapMode(QTextEdit::WidgetWidth);
    p = transText->palette();
    p.setColor(QPalette::Disabled, QPalette::Base, p.color(QPalette::Active, QPalette::Base));
    transText->setPalette(p);
    connect(transText->document(), SIGNAL(contentsChanged()),
             SLOT(handleTranslationChanges()));

    pageCurl = new PageCurl(this);

    // Focus
    setFocusPolicy(Qt::StrongFocus);
    transLbl->setFocusProxy(transText);
    srcTextLbl->setFocusProxy(transText);
    srcText->setFocusProxy(transText);
    cmtText->setFocusProxy(transText);
    setFocusProxy(transText);

    updateCommentField();
}

/*
   Don't show the comment field if there are no comments.
*/
void EditorPage::updateCommentField()
{
    if(cmtText->toPlainText().isEmpty())
        cmtText->hide();
    else
        cmtText->show();

    layoutWidgets();
}

/*
   Handle the widget layout manually
*/
void EditorPage::layoutWidgets()
 {
    int margin = 6;
    int space  = 2;
    int w = width();

    pageCurl->move(width() - pageCurl->width(), 0);

    QFontMetrics fm(srcTextLbl->font());
    srcTextLbl->move(margin, margin);
    srcTextLbl->resize(fm.width(srcTextLbl->text()), srcTextLbl->height());

    srcText->move(margin, srcTextLbl->y() + srcTextLbl->height() + space);
    srcText->resize(w - margin*2, srcText->height());

    cmtText->move(margin, srcText->y() + srcText->height() + space);
    cmtText->resize(w - margin*2, cmtText->height());

    if (cmtText->isHidden())
        transLbl->move(margin, srcText->y() + srcText->height() + space);
    else
        transLbl->move(margin, cmtText->y() + cmtText->height() + space);
    transLbl->resize( w - margin*2, transLbl->height() );

    transText->move(margin, transLbl->y() + transLbl->height() + space);
    transText->resize(w - margin*2, transText->height());

    // Calculate the total height for the editor page - emit a signal
    // if the actual page size is larger/smaller
    int totHeight = margin + srcTextLbl->height() +
                    srcText->height() + space +
                    transLbl->height() + space +
                    transText->height() + space +
                    frameWidth()*lineWidth()*2 + space * 3;

    if (!cmtText->isHidden())
        totHeight += cmtText->height() + space;

     if (height() != totHeight)
         emit pageHeightUpdated(totHeight);
}

void EditorPage::resizeEvent(QResizeEvent *)
{
    handleTranslationChanges();
    handleSourceChanges();
    handleCommentChanges();
    layoutWidgets();
}

void EditorPage::handleTranslationChanges()
{
    calculateFieldHeight(transText);
}

void EditorPage::handleSourceChanges()
{
    calculateFieldHeight(srcText);
}

void EditorPage::handleCommentChanges()
{
    calculateFieldHeight(cmtText);
}

/*
   Check if the translation text field is big enough to show all text
   that has been entered. If it isn't, resize it.
*/
void EditorPage::calculateFieldHeight(QTextEdit *field)
{
    int contentsHeight = qRound(field->document()->documentLayout()->documentSize().height());

    if (contentsHeight != field->height()) {
        int oldHeight = field->height();
        if(contentsHeight < 30)
            contentsHeight = 30;
        field->resize(field->width(), contentsHeight);
        emit pageHeightUpdated(height() + (field->height() - oldHeight));
    }
}

void EditorPage::fontChange(const QFont &)
{
    //keep the labels bold...
    QFont fnt = font();

    fnt.setBold(true);
    QFontMetrics fm(fnt);
    srcTextLbl->setFont(fnt);
    srcTextLbl->resize(fm.width(srcTextLbl->text()), srcTextLbl->height());
    transLbl->setFont(fnt);
    transLbl->resize(fm.width(transLbl->text()), transLbl->height());
    update();
}

/*
   MessageEditor class impl.

   Handle layout of dock windows and the editor page.
*/
MessageEditor::MessageEditor(MetaTranslator *t, QMainWindow *parent)
    : QScrollArea(parent), tor(t)
{
    doGuesses = true;
    canPaste = false;
    topDockWnd = new QDockWidget(parent);
    topDockWnd->setAllowedAreas(Qt::AllDockWidgetAreas);
    topDockWnd->setFeatures(QDockWidget::AllDockWidgetFeatures);
    topDockWnd->setWindowTitle(tr("Source text"));

    srcTextView = new QTreeView(topDockWnd);
    srcMdl = new MessageModel(topDockWnd);
    srcTextView->setModel(srcMdl);
    srcTextView->setAlternatingRowColors(true);
    srcTextView->setSelectionBehavior(QAbstractItemView::SelectRows);
    srcTextView->setSelectionMode(QAbstractItemView::SingleSelection);
    srcTextView->setRootIsDecorated(false);
    srcTextView->setUniformRowHeights(true);
    QPalette pal = srcTextView->palette();
    pal.setColor(QPalette::AlternateBase, TREEVIEW_ODD_COLOR);
    srcTextView->setPalette(pal);

    QFontMetrics fm(font());
    srcTextView->header()->setResizeMode(1, QHeaderView::Stretch);
    srcTextView->header()->resizeSection(0, fm.width(MessageModel::tr("Done")) + 20);
    srcTextView->header()->resizeSection(2, 300);
    srcTextView->header()->setClickable(true);

    topDockWnd->setWidget(srcTextView);
    parent->addDockWidget(Qt::TopDockWidgetArea, topDockWnd);

    bottomDockWnd = new QDockWidget(parent);
    bottomDockWnd->setAllowedAreas(Qt::AllDockWidgetAreas);
    bottomDockWnd->setFeatures(QDockWidget::AllDockWidgetFeatures);
    bottomDockWnd->setWindowTitle(tr("Phrases"));

    QWidget *w = new QWidget(bottomDockWnd);
    w->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));
    QVBoxLayout *vl = new QVBoxLayout(w);
    vl->setSpacing(6);

    phraseLbl = new QLabel( tr("Phrases and guesses:"), w );

    phraseTv = new QTreeView(w);
    phraseTv->setObjectName("phrase list view");
    phrMdl = new PhraseModel(w);
    phraseTv->setModel(phrMdl);
    phraseTv->setAlternatingRowColors(true);
    phraseTv->setSelectionBehavior(QAbstractItemView::SelectRows);
    phraseTv->setSelectionMode(QAbstractItemView::SingleSelection);
    phraseTv->setRootIsDecorated(false);
    pal = phraseTv->palette();
    pal.setColor(QPalette::AlternateBase, TREEVIEW_ODD_COLOR);
    phraseTv->setPalette(pal);

    phraseTv->header()->setResizeMode(QHeaderView::Stretch);
    phraseTv->header()->setClickable(true);

    vl->addWidget(phraseLbl);
    vl->addWidget(phraseTv);

    for (int i = 0; i < 9; ++i) {
        (void) new GuessShortcut(i, this, SLOT(guessActivated(int)));
    }

    bottomDockWnd->setWidget(w);
    parent->addDockWidget(Qt::BottomDockWidgetArea, bottomDockWnd);

    setObjectName("scroll area");
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameStyle(QFrame::NoFrame);

    editorPage = new EditorPage(this, "editor page");
    connect(editorPage, SIGNAL(pageHeightUpdated(int)),
             SLOT(updatePageHeight(int)));

    sw = new ShadowWidget(editorPage, this);
    sw->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    sw->setMinimumSize(QSize(100, 150));

    setWidget(sw);
    defFormat = editorPage->srcText->currentCharFormat();
    editorPage->transText->installEventFilter(this);

    // Signals
    connect(editorPage->pageCurl, SIGNAL(nextPage()),
        SIGNAL(nextUnfinished()));
    connect(editorPage->pageCurl, SIGNAL(prevPage()),
        SIGNAL(prevUnfinished()));

    connect(editorPage->transText->document(), SIGNAL(contentsChanged()),
        this, SLOT(emitTranslationChanged()));
    connect(editorPage->transText->document(), SIGNAL(contentsChanged()),
        this, SLOT(updateButtons()));
    connect(editorPage->transText->document(), SIGNAL(undoAvailable(bool)),
        this, SIGNAL(undoAvailable(bool)));
    connect(editorPage->transText->document(), SIGNAL(redoAvailable(bool)),
        this, SIGNAL(redoAvailable(bool)));
    connect(editorPage->transText, SIGNAL(copyAvailable(bool)),
        this, SIGNAL(cutAvailable(bool)));
    connect(editorPage->transText, SIGNAL(copyAvailable(bool)),
        this, SIGNAL(copyAvailable(bool)));
    connect(qApp->clipboard(), SIGNAL(dataChanged()),
        this, SLOT(updateCanPaste()));
    connect(phraseTv, SIGNAL(doubleClicked(QModelIndex)),
        this, SLOT(insertPhraseInTranslation(QModelIndex)));

    phraseTv->installEventFilter(this);

    connect(srcTextView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
             parent, SLOT(showNewCurrent(QModelIndex,QModelIndex)));

    // What's this
    this->setWhatsThis(tr("This whole panel allows you to view and edit "
                              "the translation of some source text.") );
    editorPage->srcText->setWhatsThis(tr("This area shows the source text.") );
    editorPage->cmtText->setWhatsThis(tr("This area shows a comment that"
                        " may guide you, and the context in which the text"
                        " occurs.") );
    editorPage->transText->setWhatsThis(tr("This is where you can enter or modify"
                        " the translation of some source text.") );

    showNothing();
}

bool MessageEditor::eventFilter(QObject *o, QEvent *e)
{
    // handle copying from the source
    if ((e->type() == QEvent::KeyPress) ||
        (e->type() == QEvent::ShortcutOverride))
    {
        QKeyEvent *ke = static_cast<QKeyEvent *>(e);

        // handle return key in phrase list
        if (o == phraseTv
            && e->type() == QEvent::KeyPress
            && ke->modifiers() == Qt::NoModifier
            && ke->key() == Qt::Key_Return
            && phraseTv->currentIndex().isValid())
        {
            insertPhraseInTranslationAndLeave(phraseTv->currentIndex());
            return false;
        }

        if (ke->modifiers() & Qt::ControlModifier)
        {
            if ((ke->key() == Qt::Key_A) &&
                editorPage->srcText->underMouse())
            {
                editorPage->srcText->selectAll();
                return true;
            }
            if ((ke->key() == Qt::Key_C) &&
                editorPage->srcText->textCursor().hasSelection() &&
                editorPage->srcText->underMouse())
            {
                editorPage->srcText->copySelection();
                return true;
            }
        }
    }

    return QWidget::eventFilter(o, e);
}

void MessageEditor::updatePageHeight(int height)
{
    sw->resize(sw->width(), height + sw->margin() + sw->shadowWidth());
}

void MessageEditor::resizeEvent(QResizeEvent *e)
{
    sw->resize(viewport()->width(), sw->height());
    QScrollArea::resizeEvent(e);
}

QTreeView *MessageEditor::sourceTextView() const
{
    return srcTextView;
}

QTreeView *MessageEditor::phraseView() const
{
    return phraseTv;
}

void MessageEditor::showNothing()
{
    editorPage->srcText->clear();

    setEditionEnabled(false);
    sourceText.clear();
    editorPage->cmtText->clear();
    setTranslation(QString::null, false);
    editorPage->handleSourceChanges();
    editorPage->handleCommentChanges();
    editorPage->handleTranslationChanges();
    editorPage->updateCommentField();
}

void MessageEditor::showMessage(const QString &text,
                                const QString &comment,
                                const QString &fullContext,
                                const QString &translation,
                                MetaTranslatorMessage::Type type,
                                const QList<Phrase> &phrases)
{
    phraseTv->clearSelection();

    bool obsolete = (type == MetaTranslatorMessage::Obsolete);
    setEditionEnabled(!obsolete);
    sourceText = text;

    visualizeBackTabs(text, editorPage->srcText);

    if (!fullContext.isEmpty() && !comment.isEmpty())
        visualizeBackTabs(fullContext.simplified() + "\n" +
            comment.simplified(), editorPage->cmtText);
    else if (!fullContext.isEmpty() && comment.isEmpty())
        visualizeBackTabs(fullContext.simplified(), editorPage->cmtText);
    else if (fullContext.isEmpty() && !comment.isEmpty())
        visualizeBackTabs(comment.simplified(), editorPage->cmtText);
    else
        editorPage->cmtText->clear();

    setTranslation(translation, false);
    phrMdl->removePhrases();

    foreach(Phrase p, phrases) {
        phrMdl->addPhrase(p);
    }

    if (doGuesses && !sourceText.isEmpty()) {
        CandidateList cl = similarTextHeuristicCandidates(tor,
            sourceText.toLatin1(), MaxCandidates);
        int n = 0;
        QList<Candidate>::Iterator it = cl.begin();
        while (it != cl.end()) {
            QString def;
            if (n < 9)
                def = tr("Guess (%1)").arg(QString(QKeySequence(Qt::CTRL | (Qt::Key_0 + (n + 1)))));
            else
                def = tr("Guess");
            phrMdl->addPhrase(Phrase((*it).source, (*it).target, def, n));
            ++n;
            ++it;
        }
    }
    phrMdl->resort();
    editorPage->handleSourceChanges();
    editorPage->handleCommentChanges();
    editorPage->handleTranslationChanges();
    editorPage->updateCommentField();
}

void MessageEditor::setTranslation(const QString &translation, bool emitt)
{
    // Block signals so that a signal is not emitted when
    // for example a new source text item is selected and *not*
    // the actual translation.
    if (!emitt)
        editorPage->transText->document()->blockSignals(true);

    if (translation.isNull())
        editorPage->transText->clear();
    else
        editorPage->transText->setPlainText(translation);

    if (!emitt)
    {
        editorPage->transText->document()->blockSignals(false);

        //don't undo the change
        emit undoAvailable(false);
        emit redoAvailable(false);
        updateButtons();
    }
    emit cutAvailable(false);
    emit copyAvailable(false);
}

void MessageEditor::setEditionEnabled(bool enabled)
{
    editorPage->transLbl->setEnabled(enabled);
    editorPage->transText->setReadOnly(!enabled);

    phraseLbl->setEnabled(enabled);
    phraseTv->setEnabled(enabled);
    updateCanPaste();
}

void MessageEditor::undo()
{
    editorPage->transText->document()->undo();
}

void MessageEditor::redo()
{
    editorPage->transText->document()->redo();
}

void MessageEditor::cut()
{
    editorPage->transText->cut();
}

void MessageEditor::copy()
{
    editorPage->transText->copy();
}

void MessageEditor::paste()
{
    editorPage->transText->paste();
}

void MessageEditor::selectAll()
{
    editorPage->transText->selectAll();
}

void MessageEditor::emitTranslationChanged()
{
    emit translationChanged(editorPage->transText->toPlainText());
}

void MessageEditor::guessActivated(int key)
{
    QModelIndex mi;
    Phrase p;

    for (int i=0; i<phrMdl->phraseList().count(); ++i) {
        mi = phrMdl->QAbstractTableModel::index(i, 0);
        p = phrMdl->phrase(mi);
        if (p.shortcut() == key) {
            insertPhraseInTranslation(mi);
            break;
        }
    }
}

void MessageEditor::insertPhraseInTranslation(const QModelIndex &index)
{
    if (!editorPage->transText->isReadOnly())
    {
        editorPage->transText->textCursor().insertText(phrMdl->phrase(index).target());
        emit translationChanged(editorPage->transText->toPlainText());
    }
}

void MessageEditor::insertPhraseInTranslationAndLeave(const QModelIndex &index)
{
    if (!editorPage->transText->isReadOnly())
    {
        editorPage->transText->textCursor().insertText(phrMdl->phrase(index).target());
        emit translationChanged(editorPage->transText->toPlainText());
        editorPage->transText->setFocus();
    }
}

void MessageEditor::updateButtons()
{
    bool overwrite = (!editorPage->transText->isReadOnly() &&
             (editorPage->transText->toPlainText().trimmed().isEmpty() ||
              mayOverwriteTranslation));
    mayOverwriteTranslation = false;
    emit updateActions(overwrite);
}

void MessageEditor::beginFromSource()
{
    mayOverwriteTranslation = true;
    setTranslation(sourceText, true);
    setEditorFocus();
}

void MessageEditor::setEditorFocus()
{
    if (!editorPage->hasFocus())
        editorPage->setFocus();
}

void MessageEditor::updateCanPaste()
{
    bool oldCanPaste = canPaste;
    canPaste = (!editorPage->transText->isReadOnly() &&
        !qApp->clipboard()->text().isNull());
    if (canPaste != oldCanPaste)
        emit pasteAvailable(canPaste);
}

void MessageEditor::toggleGuessing()
{
    doGuesses = !doGuesses;
    if (!doGuesses) {
        phrMdl->removePhrases();
    }
}
