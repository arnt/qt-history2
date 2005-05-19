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

#include "textedit.h"

#include <qaction.h>
#include <qapplication.h>
#include <qclipboard.h>
#include <qcolordialog.h>
#include <qcombobox.h>
#include <qdebug.h>
#include <qfile.h>
#include <qfiledialog.h>
#include <qfileinfo.h>
#include <qfontdatabase.h>
#include <qlineedit.h>
#include <qlist.h>
#include <qmenu.h>
#include <qmenubar.h>
#include <qpainter.h>
#include <qprintdialog.h>
#include <qprinter.h>
#include <qtabwidget.h>
#include <qtextcodec.h>
#include <qtextedit.h>
#include <qtextdocumentfragment.h>
#include <qtextformat.h>
#include <qtoolbar.h>
#include <qtextcursor.h>
#include <qtextlist.h>

#include <limits.h>

#ifdef Q_WS_MAC
const QString rsrcPath = ":/images/mac";
#else
const QString rsrcPath = ":/images/win";
#endif

TextEdit::TextEdit(QWidget *parent)
    : QMainWindow(parent)
{
    setupFileActions();
    setupEditActions();
    setupTextActions();

    tabWidget = new QTabWidget(this);
    connect(tabWidget, SIGNAL(currentChanged(int)),
            this, SLOT(editorChanged()));
    setCentralWidget(tabWidget);

    connect(QApplication::clipboard(), SIGNAL(dataChanged()), this, SLOT(clipboardDataChanged()));

    if (qApp->argc() == 1) {
        if (!load("example.html"))
            fileNew();
    } else {
        for (int i = 1; i < qApp->argc(); ++i)
            load(qApp->argv()[i]);
    }
}

void TextEdit::setupFileActions()
{
    QToolBar *tb = new QToolBar(this);
    tb->setWindowTitle(tr("File Actions"));
    addToolBar(tb);

    QMenu *menu = new QMenu(tr("&File"), this);
    menuBar()->addMenu(menu);

    QAction *a;

    a = new QAction(QIcon(rsrcPath + "/filenew.png"), tr("&New..."), this);
    a->setShortcut(Qt::CTRL + Qt::Key_N);
    connect(a, SIGNAL(triggered()), this, SLOT(fileNew()));
    tb->addAction(a);
    menu->addAction(a);

    a = new QAction(QIcon(rsrcPath + "/fileopen.png"), tr("&Open..."), this);
    a->setShortcut(Qt::CTRL + Qt::Key_O);
    connect(a, SIGNAL(triggered()), this, SLOT(fileOpen()));
    tb->addAction(a);
    menu->addAction(a);

    menu->addSeparator();

    actionSave = a = new QAction(QIcon(rsrcPath + "/filesave.png"), tr("&Save..."), this);
    a->setShortcut(Qt::CTRL + Qt::Key_S);
    connect(a, SIGNAL(triggered()), this, SLOT(fileSave()));
    a->setEnabled(false);
    tb->addAction(a);
    menu->addAction(a);

    a = new QAction(tr("Save &As..."), this);
    connect(a, SIGNAL(triggered()), this, SLOT(fileSaveAs()));
    menu->addAction(a);
    menu->addSeparator();

    a = new QAction(QIcon(rsrcPath + "/fileprint.png"), tr("&Print..."), this);
    a->setShortcut(Qt::CTRL + Qt::Key_P);
    connect(a, SIGNAL(triggered()), this, SLOT(filePrint()));
    tb->addAction(a);
    menu->addAction(a);

    a = new QAction(tr("&Close"), this);
    connect(a, SIGNAL(triggered()), this, SLOT(fileClose()));
    menu->addAction(a);

    a = new QAction(tr("E&xit"), this);
    a->setShortcut(Qt::CTRL + Qt::Key_Q);
    connect(a, SIGNAL(triggered()), this, SLOT(fileExit()));
    menu->addAction(a);
}

void TextEdit::setupEditActions()
{
    QToolBar *tb = new QToolBar(this);
    tb->setWindowTitle(tr("Edit Actions"));
    addToolBar(tb);

    QMenu *menu = new QMenu(tr("&Edit"), this);
    menuBar()->addMenu(menu);

    QAction *a;
    a = actionUndo = new QAction(QIcon(rsrcPath + "/editundo.png"), tr("&Undo"), this);
    a->setShortcut(Qt::CTRL + Qt::Key_Z);
    tb->addAction(a);
    menu->addAction(a);
    a = actionRedo = new QAction(QIcon(rsrcPath + "/editredo.png"), tr("&Redo"), this);
    a->setShortcut(Qt::CTRL + Qt::Key_Y);
    tb->addAction(a);
    menu->addAction(a);
    menu->addSeparator();
    a = actionCut = new QAction(QIcon(rsrcPath + "/editcut.png"), tr("Cu&t"), this);
    a->setShortcut(Qt::CTRL + Qt::Key_X);
    tb->addAction(a);
    menu->addAction(a);
    a = actionCopy = new QAction(QIcon(rsrcPath + "/editcopy.png"), tr("&Copy"), this);
    a->setShortcut(Qt::CTRL + Qt::Key_C);
    tb->addAction(a);
    menu->addAction(a);
    a = actionPaste = new QAction(QIcon(rsrcPath + "/editpaste.png"), tr("&Paste"), this);
    a->setShortcut(Qt::CTRL + Qt::Key_V);
    tb->addAction(a);
    menu->addAction(a);
    actionPaste->setEnabled(!QApplication::clipboard()->text().isEmpty());
}

void TextEdit::setupTextActions()
{
    QToolBar *tb = new QToolBar(this);
    tb->setWindowTitle(tr("Format Actions"));
    addToolBar(tb);

    QMenu *menu = new QMenu(tr("F&ormat"), this);
    menuBar()->addMenu(menu);

    actionTextBold = new QAction(QIcon(rsrcPath + "/textbold.png"), tr("&Bold"), this);
    actionTextBold->setShortcut(Qt::CTRL + Qt::Key_B);
    QFont bold;
    bold.setBold(true);
    actionTextBold->setFont(bold);
    connect(actionTextBold, SIGNAL(triggered()), this, SLOT(textBold()));
    tb->addAction(actionTextBold);
    menu->addAction(actionTextBold);
    actionTextBold->setCheckable(true);

    actionTextItalic = new QAction(QIcon(rsrcPath + "/textitalic.png"), tr("&Italic"), this);
    actionTextItalic->setShortcut(Qt::CTRL + Qt::Key_I);
    QFont italic;
    italic.setItalic(true);
    actionTextItalic->setFont(italic);
    connect(actionTextItalic, SIGNAL(triggered()), this, SLOT(textItalic()));
    tb->addAction(actionTextItalic);
    menu->addAction(actionTextItalic);
    actionTextItalic->setCheckable(true);

    actionTextUnderline = new QAction(QIcon(rsrcPath + "/textunder.png"), tr("&Underline"), this);
    actionTextUnderline->setShortcut(Qt::CTRL + Qt::Key_U);
    QFont underline;
    underline.setUnderline(true);
    actionTextUnderline->setFont(underline);
    connect(actionTextUnderline, SIGNAL(triggered()), this, SLOT(textUnderline()));
    tb->addAction(actionTextUnderline);
    menu->addAction(actionTextUnderline);
    actionTextUnderline->setCheckable(true);

    menu->addSeparator();

    QActionGroup *grp = new QActionGroup(this);
    connect(grp, SIGNAL(triggered(QAction *)), this, SLOT(textAlign(QAction *)));

    actionAlignLeft = new QAction(QIcon(rsrcPath + "/textleft.png"), tr("&Left"), grp);
    actionAlignLeft->setShortcut(Qt::CTRL + Qt::Key_L);
    actionAlignLeft->setCheckable(true);
    actionAlignCenter = new QAction(QIcon(rsrcPath + "/textcenter.png"), tr("C&enter"), grp);
    actionAlignCenter->setShortcut(Qt::CTRL + Qt::Key_E);
    actionAlignCenter->setCheckable(true);
    actionAlignRight = new QAction(QIcon(rsrcPath + "/textright.png"), tr("&Right"), grp);
    actionAlignRight->setShortcut(Qt::CTRL + Qt::Key_R);
    actionAlignRight->setCheckable(true);
    actionAlignJustify = new QAction(QIcon(rsrcPath + "/textjustify.png"), tr("&Justify"), grp);
    actionAlignJustify->setShortcut(Qt::CTRL + Qt::Key_J);
    actionAlignJustify->setCheckable(true);

    tb->addActions(grp->actions());
    menu->addActions(grp->actions());

    menu->addSeparator();

    QPixmap pix(16, 16);
    pix.fill(Qt::black);
    actionTextColor = new QAction(pix, tr("&Color..."), this);
    connect(actionTextColor, SIGNAL(triggered()), this, SLOT(textColor()));
    tb->addAction(actionTextColor);
    menu->addAction(actionTextColor);


    tb = new QToolBar(this);
    tb->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
    tb->setWindowTitle(tr("Format Actions"));
    addToolBarBreak(Qt::TopToolBarArea);
    addToolBar(tb);

    comboStyle = new QComboBox(tb);
    tb->addWidget(comboStyle);
    comboStyle->addItem("Standard");
    comboStyle->addItem("Bullet List (Disc)");
    comboStyle->addItem("Bullet List (Circle)");
    comboStyle->addItem("Bullet List (Square)");
    comboStyle->addItem("Ordered List (Decimal)");
    comboStyle->addItem("Ordered List (Alpha lower)");
    comboStyle->addItem("Ordered List (Alpha upper)");
    connect(comboStyle, SIGNAL(activated(int)),
            this, SLOT(textStyle(int)));

    comboFont = new QComboBox(tb);
    tb->addWidget(comboFont);
    comboFont->setEditable(true);
    QFontDatabase db;
    comboFont->addItems(db.families());
    connect(comboFont, SIGNAL(activated(const QString &)),
            this, SLOT(textFamily(const QString &)));
    comboFont->setCurrentIndex(comboFont->findText(QApplication::font().family()));

    comboSize = new QComboBox(tb);
    comboSize->setObjectName("comboSize");
    tb->addWidget(comboSize);
    comboSize->setEditable(true);

    foreach(int size, db.standardSizes())
        comboSize->addItem(QString::number(size));

    connect(comboSize, SIGNAL(activated(const QString &)),
            this, SLOT(textSize(const QString &)));
    comboSize->setCurrentIndex(comboSize->findText(QString::number(QApplication::font()
                                                                   .pointSize())));
}

bool TextEdit::load(const QString &f)
{
    if (!QFile::exists(f))
        return false;
    QTextEdit *edit = createNewEditor(QFileInfo(f).fileName());
    QFile file(f);
    if (!file.open(QFile::ReadOnly))
        return false;

    QByteArray data = file.readAll();
    QTextCodec *codec = Qt::codecForHtml(data);
    QString str = codec->toUnicode(data);
    if (Qt::mightBeRichText(str)) {
        edit->setHtml(str);
    } else {
        str = QString::fromLocal8Bit(data);
        edit->setPlainText(str);
    }

    filenames.insert(edit, f);
    return true;
}

void TextEdit::fileNew()
{
    createNewEditor();
}

void TextEdit::fileOpen()
{
    QString fn = QFileDialog::getOpenFileName(this, tr("Open File..."),
                                              QString(), tr("HTML-Files (*.htm *.html);;All Files (*)"));
    if (!fn.isEmpty())
        load(fn);
}

void TextEdit::fileSave()
{
    if (!currentEditor)
        return;
    if (filenames.find(currentEditor) == filenames.end()) {
        fileSaveAs();
    } else {
        QFile file(*filenames.find(currentEditor));
        if (!file.open(QFile::WriteOnly))
            return;
        QTextStream ts(&file);
        ts.setCodec(QTextCodec::codecForName("UTF-8"));
        ts << currentEditor->document()->toHtml("UTF-8");
        currentEditor->document()->setModified(false);
    }
}

void TextEdit::fileSaveAs()
{
    if (!currentEditor)
        return;
    QString fn = QFileDialog::getSaveFileName(this, tr("Save as..."),
                                              QString(), tr("HTML-Files (*.htm *.html);;All Files (*)"));
    if (!fn.isEmpty()) {
        filenames.insert(currentEditor, fn);
        fileSave();
        tabWidget->setTabText(tabWidget->indexOf(currentEditor), QFileInfo(fn).fileName());
    }
}

void TextEdit::filePrint()
{
    if (!currentEditor)
        return;
#ifndef QT_NO_PRINTER
    QPrinter printer(QPrinter::HighResolution);
    printer.setFullPage(true);

    QPrintDialog *dlg = new QPrintDialog(&printer, this);
    if (dlg->exec() == QDialog::Accepted) {
        currentEditor->document()->print(&printer);
    }
    delete dlg;
#endif
}

void TextEdit::fileClose()
{
    const bool hadFocus = (currentEditor && currentEditor->hasFocus());
    delete currentEditor;
    currentEditor = qobject_cast<QTextEdit *>(tabWidget->currentWidget());
    if (currentEditor && hadFocus)
        currentEditor->setFocus();
}

void TextEdit::fileExit()
{
    qApp->quit();
}

void TextEdit::textBold()
{
    if (!currentEditor)
        return;
    currentEditor->setFontWeight(actionTextBold->isChecked() ? QFont::Bold : QFont::Normal);
}

void TextEdit::textUnderline()
{
    if (!currentEditor)
        return;
    currentEditor->setFontUnderline(actionTextUnderline->isChecked());
}

void TextEdit::textItalic()
{
    if (!currentEditor)
        return;
    currentEditor->setFontItalic(actionTextItalic->isChecked());
}

void TextEdit::textFamily(const QString &f)
{
    if (!currentEditor)
        return;
    currentEditor->setFontFamily(f);
}

void TextEdit::textSize(const QString &p)
{
    if (!currentEditor)
        return;
    currentEditor->setFontPointSize(p.toFloat());
}

void TextEdit::textStyle(int styleIndex)
{
    if (!currentEditor)
        return;

    QTextCursor cursor = currentEditor->textCursor();

    if (styleIndex != 0) {
        QTextListFormat::Style style = QTextListFormat::ListDisc;

        switch (styleIndex) {
            default:
            case 1:
                style = QTextListFormat::ListDisc;
                break;
            case 2:
                style = QTextListFormat::ListCircle;
                break;
            case 3:
                style = QTextListFormat::ListSquare;
                break;
            case 4:
                style = QTextListFormat::ListDecimal;
                break;
            case 5:
                style = QTextListFormat::ListLowerAlpha;
                break;
            case 6:
                style = QTextListFormat::ListUpperAlpha;
                break;
        }

        cursor.beginEditBlock();

        QTextBlockFormat blockFmt = cursor.blockFormat();

        QTextListFormat listFmt;

        if (cursor.currentList()) {
            listFmt = cursor.currentList()->format();
        } else {
            listFmt.setIndent(blockFmt.indent() + 1);
            blockFmt.setIndent(0);
            cursor.setBlockFormat(blockFmt);
        }

        listFmt.setStyle(style);

        cursor.createList(listFmt);

        cursor.endEditBlock();
    } else {
        // ####
        QTextBlockFormat bfmt;
        bfmt.setObjectIndex(-1);
        cursor.mergeBlockFormat(bfmt);
    }
}

void TextEdit::textColor()
{
    if (!currentEditor)
        return;
    QColor col = QColorDialog::getColor(currentEditor->textColor(), this);
    if (!col.isValid())
        return;
    currentEditor->setTextColor(col);
    colorChanged(col);
}

void TextEdit::textAlign(QAction *a)
{
    if (!currentEditor)
        return;
    if (a == actionAlignLeft)
        currentEditor->setAlignment(Qt::AlignLeft);
    else if (a == actionAlignCenter)
        currentEditor->setAlignment(Qt::AlignHCenter);
    else if (a == actionAlignRight)
        currentEditor->setAlignment(Qt::AlignRight);
    else if (a == actionAlignJustify)
        currentEditor->setAlignment(Qt::AlignJustify);
}

void TextEdit::currentCharFormatChanged(const QTextCharFormat &format)
{
    fontChanged(format.font());
    colorChanged(format.foreground().color());
    alignmentChanged(currentEditor->alignment());
}

void TextEdit::clipboardDataChanged()
{
    actionPaste->setEnabled(!QApplication::clipboard()->text().isEmpty());
}

void TextEdit::fontChanged(const QFont &f)
{
    comboFont->setCurrentIndex(comboFont->findText(f.family()));
    comboSize->setCurrentIndex(comboSize->findText(QString::number(f.pointSize())));
    actionTextBold->setChecked(f.bold());
    actionTextItalic->setChecked(f.italic());
    actionTextUnderline->setChecked(f.underline());
}

void TextEdit::colorChanged(const QColor &c)
{
    QPixmap pix(16, 16);
    pix.fill(c);
    actionTextColor->setIcon(pix);
}

void TextEdit::alignmentChanged(Qt::Alignment a)
{
    if (a & Qt::AlignLeft)
        actionAlignLeft->setChecked(true);
    else if (a & Qt::AlignHCenter)
        actionAlignCenter->setChecked(true);
    else if (a & Qt::AlignRight)
        actionAlignRight->setChecked(true);
    else if (a & Qt::AlignJustify)
        actionAlignJustify->setChecked(true);
}

void TextEdit::editorChanged()
{
    if (currentEditor) {
        disconnect(currentEditor->document(), SIGNAL(modificationChanged(bool)),
                   actionSave, SLOT(setEnabled(bool)));
        disconnect(currentEditor->document(), SIGNAL(modificationChanged(bool)),
                   this, SLOT(setWindowModified(bool)));
        disconnect(currentEditor->document(), SIGNAL(undoAvailable(bool)),
                   actionUndo, SLOT(setEnabled(bool)));
        disconnect(currentEditor->document(), SIGNAL(redoAvailable(bool)),
                   actionRedo, SLOT(setEnabled(bool)));

        disconnect(actionUndo, SIGNAL(triggered()), currentEditor->document(), SLOT(undo()));
        disconnect(actionRedo, SIGNAL(triggered()), currentEditor->document(), SLOT(redo()));

        disconnect(actionCut, SIGNAL(triggered()), currentEditor, SLOT(cut()));
        disconnect(actionCopy, SIGNAL(triggered()), currentEditor, SLOT(copy()));
        disconnect(actionPaste, SIGNAL(triggered()), currentEditor, SLOT(paste()));

        disconnect(currentEditor, SIGNAL(copyAvailable(bool)), actionCut, SLOT(setEnabled(bool)));
        disconnect(currentEditor, SIGNAL(copyAvailable(bool)), actionCopy, SLOT(setEnabled(bool)));

    }

    currentEditor = qobject_cast<QTextEdit *>(tabWidget->currentWidget());
    if (!currentEditor)
        return;

    fontChanged(currentEditor->font());
    colorChanged(currentEditor->textColor());
    alignmentChanged(currentEditor->alignment());

    connect(currentEditor->document(), SIGNAL(modificationChanged(bool)),
            actionSave, SLOT(setEnabled(bool)));
    connect(currentEditor->document(), SIGNAL(modificationChanged(bool)),
            this, SLOT(setWindowModified(bool)));
    connect(currentEditor->document(), SIGNAL(undoAvailable(bool)),
            actionUndo, SLOT(setEnabled(bool)));
    connect(currentEditor->document(), SIGNAL(redoAvailable(bool)),
            actionRedo, SLOT(setEnabled(bool)));

    setWindowModified(currentEditor->document()->isModified());
    actionSave->setEnabled(currentEditor->document()->isModified());
    actionUndo->setEnabled(currentEditor->document()->isUndoAvailable());
    actionRedo->setEnabled(currentEditor->document()->isRedoAvailable());

    connect(actionUndo, SIGNAL(triggered()), currentEditor->document(), SLOT(undo()));
    connect(actionRedo, SIGNAL(triggered()), currentEditor->document(), SLOT(redo()));

    const bool selection = currentEditor->textCursor().hasSelection();
    actionCut->setEnabled(selection);
    actionCopy->setEnabled(selection);

    connect(actionCut, SIGNAL(triggered()), currentEditor, SLOT(cut()));
    connect(actionCopy, SIGNAL(triggered()), currentEditor, SLOT(copy()));
    connect(actionPaste, SIGNAL(triggered()), currentEditor, SLOT(paste()));

    connect(currentEditor, SIGNAL(copyAvailable(bool)), actionCut, SLOT(setEnabled(bool)));
    connect(currentEditor, SIGNAL(copyAvailable(bool)), actionCopy, SLOT(setEnabled(bool)));
}

QTextEdit *TextEdit::createNewEditor(const QString &title)
{
    QTextEdit *edit = new QTextEdit;
    connect(edit, SIGNAL(currentCharFormatChanged(const QTextCharFormat &)),
            this, SLOT(currentCharFormatChanged(const QTextCharFormat &)));

    int tab = tabWidget->addTab(edit, title.isEmpty() ? tr("noname") : title);
    tabWidget->setCurrentIndex(tab);
    edit->setFocus();

    return edit;
}
