/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#include "textedit.h"

#include <qaction.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qtoolbar.h>
#include <qtabwidget.h>
#include <qapplication.h>
#include <qfontdatabase.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qfileinfo.h>
#include <qfile.h>
#include <qprinter.h>
#include <qpaintdevicemetrics.h>
#include <qcolordialog.h>
#include <qpainter.h>
#include <qlist.h>
#include <qtextcodec.h>
#include <qspinbox.h>
#include <qdebug.h>
#include <qtextdocumentfragment.h>
#include <qfiledialog.h>
#include <qclipboard.h>
#include <qtextedit.h>
#include <qtextcursor.h>
#include <qtextformat.h>
#include <qtextdocument.h>
#include <qtexttable.h>
#include <qprintdialog.h>
#include <private/qtextdocumentlayout_p.h>
#include <qspinbox.h>

#include <limits.h>

TextEdit::TextEdit(QWidget *parent)
    : QMainWindow(parent)
{
    setupFileActions();
    setupEditActions();
    setupTextActions();

    tabWidget = new QTabWidget(this);
    connect(tabWidget, SIGNAL(currentChanged(int)),
            this, SLOT(editorChanged()));
    setCenterWidget(tabWidget);

    connect(QApplication::clipboard(), SIGNAL(dataChanged()), this, SLOT(clipboardDataChanged()));

    if (qApp->argc() == 1) {
        load("example.html");
    } else {
        for (int i = 1; i < qApp->argc(); ++i)
            load(qApp->argv()[i]);
    }
}

void TextEdit::setupFileActions()
{
    QToolBar *tb = new QToolBar(this);
//    tb->setLabel("File Actions");
    QMenu *menu = new QMenu(this);
    menuBar()->addMenu(tr("&File"), menu);

    QAction *a;

    a = new QAction(QPixmap(":/filenew.xpm"), tr("&New..."), this);
    a->setShortcut(Qt::CTRL + Qt::Key_N);
    connect(a, SIGNAL(triggered()), this, SLOT(fileNew()));
    tb->addAction(a);
    menu->addAction(a);

    a = new QAction(QPixmap(":/fileopen.xpm"), tr("&Open..."), this);
    a->setShortcut(Qt::CTRL + Qt::Key_O);
    connect(a, SIGNAL(triggered()), this, SLOT(fileOpen()));
    tb->addAction(a);
    menu->addAction(a);

    menu->addSeparator();

    a = new QAction(QPixmap(":/filesave.xpm"), tr("&Save..."), this);
    a->setShortcut(Qt::CTRL + Qt::Key_S);
    connect(a, SIGNAL(triggered()), this, SLOT(fileSave()));
    tb->addAction(a);
    menu->addAction(a);

    a = new QAction(tr("Save &As..."), 0, this);
    connect(a, SIGNAL(triggered()), this, SLOT(fileSaveAs()));
    menu->addAction(a);
    menu->addSeparator();

    a = new QAction(QPixmap(":/fileprint.xpm"), tr("&Print..."), this);
    a->setShortcut(Qt::CTRL + Qt::Key_P);
    connect(a, SIGNAL(triggered()), this, SLOT(filePrint()));
    tb->addAction(a);
    menu->addAction(a);

    a = new QAction(tr("&Close"), 0, this);
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

    QSpinBox *sb = new QSpinBox;
    tb->addWidget(sb);
//    tb->setLabel("Edit Actions");
    QMenu *menu = new QMenu(this);
    menuBar()->addMenu(tr("&Edit"), menu);

    QAction *a;
    a = actionUndo = new QAction(QPixmap(":/editundo.xpm"), tr("&Undo"), this);
    a->setShortcut(Qt::CTRL + Qt::Key_Z);
    tb->addAction(a);
    menu->addAction(a);
    a = actionRedo = new QAction(QPixmap(":/editredo.xpm"), tr("&Redo"), this);
    a->setShortcut(Qt::CTRL + Qt::Key_Y);
    tb->addAction(a);
    menu->addAction(a);
    menu->addSeparator();
    a = actionCut = new QAction(QPixmap(":/editcut.xpm"), tr("Cu&t"), this);
    a->setShortcut(Qt::CTRL + Qt::Key_X);
    tb->addAction(a);
    menu->addAction(a);
    a = actionCopy = new QAction(QPixmap(":/editcopy.xpm"), tr("&Copy"), this);
    a->setShortcut(Qt::CTRL + Qt::Key_C);
    tb->addAction(a);
    menu->addAction(a);
    a = actionPaste = new QAction(QPixmap(":/editpaste.xpm"), tr("&Paste"), this);
    a->setShortcut(Qt::CTRL + Qt::Key_V);
    tb->addAction(a);
    menu->addAction(a);
    actionPaste->setEnabled(!QApplication::clipboard()->text().isEmpty());
}

void TextEdit::setupTextActions()
{
    QToolBar *tb = new QToolBar(this);
//    tb->setLabel("Format Actions");
    QMenu *menu = new QMenu(this);
    menuBar()->addMenu(tr("F&ormat"), menu);

    comboStyle = new QComboBox;
    tb->addWidget(comboStyle);
    comboStyle->insertItem("Standard");
    comboStyle->insertItem("Bullet List (Disc)");
    comboStyle->insertItem("Bullet List (Circle)");
    comboStyle->insertItem("Bullet List (Square)");
    comboStyle->insertItem("Ordered List (Decimal)");
    comboStyle->insertItem("Ordered List (Alpha lower)");
    comboStyle->insertItem("Ordered List (Alpha upper)");
    connect(comboStyle, SIGNAL(activated(int)),
            this, SLOT(textStyle(int)));

    comboFont = new QComboBox;
    tb->addWidget(comboFont);
    comboFont->setEditable(true);
    QFontDatabase db;
    comboFont->insertStringList(db.families());
    connect(comboFont, SIGNAL(activated(const QString &)),
            this, SLOT(textFamily(const QString &)));
    comboFont->lineEdit()->setText(QApplication::font().family());

    comboSize = new QComboBox;
    tb->addWidget(comboSize);
    comboSize->setEditable(true);

    QList<int> sizes = db.standardSizes();
    for (int i = 0; i < sizes.count(); ++i)
        comboSize->insertItem(QString::number(sizes[i]));

    connect(comboSize, SIGNAL(activated(const QString &)),
            this, SLOT(textSize(const QString &)));
    comboSize->lineEdit()->setText(QString::number(QApplication::font().pointSize()));

    actionTextBold = new QAction(QPixmap(":/textbold.xpm"), tr("&Bold"), this);
    actionTextBold->setShortcut(Qt::CTRL + Qt::Key_B);
    connect(actionTextBold, SIGNAL(triggered()), this, SLOT(textBold()));
    tb->addAction(actionTextBold);
    menu->addAction(actionTextBold);
    actionTextBold->setCheckable(true);

    actionTextItalic = new QAction(QPixmap(":/textitalic.xpm"), tr("&Italic"), this);
    actionTextItalic->setShortcut(Qt::CTRL + Qt::Key_I);
    connect(actionTextItalic, SIGNAL(triggered()), this, SLOT(textItalic()));
    tb->addAction(actionTextItalic);
    menu->addAction(actionTextItalic);
    actionTextItalic->setCheckable(true);

    actionTextUnderline = new QAction(QPixmap(":/textunder.xpm"), tr("&Underline"), this);
    actionTextUnderline->setShortcut(Qt::CTRL + Qt::Key_U);
    connect(actionTextUnderline, SIGNAL(triggered()), this, SLOT(textUnderline()));
    tb->addAction(actionTextUnderline);
    menu->addAction(actionTextUnderline);
    actionTextUnderline->setCheckable(true);

    menu->addSeparator();

    QActionGroup *grp = new QActionGroup(this);
    connect(grp, SIGNAL(triggered(QAction *)), this, SLOT(textAlign(QAction *)));

    actionAlignLeft = new QAction(QPixmap(":/textleft.xpm"), tr("&Left"), grp);
    actionAlignLeft->setShortcut(Qt::CTRL + Qt::Key_L);
    actionAlignLeft->setCheckable(true);
    actionAlignCenter = new QAction(QPixmap(":/textcenter.xpm"), tr("C&enter"), grp);
    actionAlignCenter->setShortcut(Qt::CTRL + Qt::Key_E);
    actionAlignCenter->setCheckable(true);
    actionAlignRight = new QAction(QPixmap(":/textright.xpm"), tr("&Right"), grp);
    actionAlignRight->setShortcut(Qt::CTRL + Qt::Key_R);
    actionAlignRight->setCheckable(true);
    actionAlignJustify = new QAction(QPixmap(":/textjustify.xpm"), tr("&Justify"), grp);
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
}

void TextEdit::load(const QString &f)
{
    if (!QFile::exists(f))
        return;
    QTextEdit *edit = createNewEditor(QFileInfo(f).fileName());
    QFile file(f);
    if (!file.open(IO_ReadOnly))
        return;

    QByteArray data = file.readAll();
    edit->setHtml(data);

    filenames.insert(edit, f);
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
    QString fn;
    if (filenames.find(currentEditor) == filenames.end()) {
        fileSaveAs();
    } else {
        QFile file(*filenames.find(currentEditor));
        if (!file.open(IO_WriteOnly))
            return;
        QTextStream ts(&file);
        ts << currentEditor->document()->plainText();
    }
}

void TextEdit::fileSaveAs()
{
    if (!currentEditor)
        return;
    QString fn = QFileDialog::getSaveFileName(this, tr("Save as..."),
                                              QString::null, tr("HTML-Files (*.htm *.html);;All Files (*)"));
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
    //printer.setFullPage(true);

    QPrintDialog *dlg = new QPrintDialog(&printer, this);
    if (dlg->exec() == QDialog::Accepted) {
//    if (printer.setup(this)) {
        QPainter p(&printer);
        // Check that there is a valid device to print to.
        if (!p.device()) return;
        QPaintDeviceMetrics metrics(p.device());
        int dpiy = metrics.logicalDpiY();
        int margin = (int) ((2/2.54)*dpiy); // 2 cm margins
        QRect body(margin, margin, metrics.width() - 2*margin, metrics.height() - 2*margin);
        QFont font(currentEditor->font());
        font.setPointSize(10); // we define 10pt to be a nice base size for printing

        QTextDocument doc;
        QTextCursor(&doc).insertFragment(QTextDocumentFragment(currentEditor->document()));
        // ###
        QTextDocumentLayout *layout = qt_cast<QTextDocumentLayout *>(doc.documentLayout());
        layout->setPageSize(QSize(body.width(), INT_MAX));

        QRect view(body);
        int page = 1;
        p.translate(body.left(), body.top());
        do {
            QAbstractTextDocumentLayout::PaintContext ctx;
            ctx.palette = palette();
            p.setClipRect(view);
            layout->draw(&p, ctx);
            view.moveBy(0, body.height());
            p.translate(0 , -body.height());
            p.setFont(font);
            p.drawText(view.right() - p.fontMetrics().width(QString::number(page)),
                       view.bottom() + p.fontMetrics().ascent() + 5, QString::number(page));
            if (view.top()  >= layout->totalHeight())
                break;
            printer.newPage();
            page++;
        } while (true);

    }
    delete dlg;
#endif
}

void TextEdit::fileClose()
{
    delete currentEditor;
    currentEditor = qt_cast<QTextEdit *>(tabWidget->currentWidget());
    if (currentEditor)
        currentEditor->viewport()->setFocus();
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
    currentEditor->viewport()->setFocus();
}

void TextEdit::textSize(const QString &p)
{
    if (!currentEditor)
        return;
    currentEditor->setFontPointSize(p.toFloat());
    currentEditor->viewport()->setFocus();
}

void TextEdit::textStyle(int i)
{
    if (!currentEditor)
        return;

    QTextCursor cursor = currentEditor->cursor();

    if (i != 0) {
        QTextListFormat::Style style = QTextListFormat::ListDisc;

        switch (i) {
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
        listFmt.setStyle(style);
        listFmt.setIndent(blockFmt.indent() + 1);

        blockFmt.setIndent(0);
        cursor.setBlockFormat(blockFmt);

        cursor.createList(listFmt);

        cursor.endEditBlock();
    } else {
        // ####
        QTextBlockFormat bfmt;
        bfmt.setObjectIndex(-1);
        cursor.mergeBlockFormat(bfmt);
    }

    currentEditor->viewport()->setFocus();
}

void TextEdit::textColor()
{
    if (!currentEditor)
        return;
    QColor col = QColorDialog::getColor(currentEditor->color(), this);
    if (!col.isValid())
        return;
    currentEditor->setColor(col);
    QPixmap pix(16, 16);
    pix.fill(Qt::black);
    actionTextColor->setIcon(pix);
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
    colorChanged(format.color());

    QTextCursor cursor = currentEditor->cursor();
    alignmentChanged(cursor.blockFormat().alignment());
}

void TextEdit::clipboardDataChanged()
{
    actionPaste->setEnabled(!QApplication::clipboard()->text().isEmpty());
}

void TextEdit::fontChanged(const QFont &f)
{
    comboFont->lineEdit()->setText(f.family());
    comboSize->lineEdit()->setText(QString::number(f.pointSize()));
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
    if ((a == Qt::AlignAuto) || (a & Qt::AlignLeft))
        actionAlignLeft->setChecked(true);
    else if ((a & Qt::AlignHCenter))
        actionAlignCenter->setChecked(true);
    else if ((a & Qt::AlignRight))
        actionAlignRight->setChecked(true);
    else if ((a & Qt::AlignJustify))
        actionAlignJustify->setChecked(true);
}

void TextEdit::editorChanged()
{
    if (currentEditor) {
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

    currentEditor = qt_cast<QTextEdit *>(tabWidget->currentWidget());
    if (!currentEditor)
        return;
    fontChanged(currentEditor->font());
    colorChanged(currentEditor->color());
    alignmentChanged(currentEditor->alignment());

    connect(currentEditor->document(), SIGNAL(undoAvailable(bool)),
            actionUndo, SLOT(setEnabled(bool)));
    connect(currentEditor->document(), SIGNAL(redoAvailable(bool)),
            actionRedo, SLOT(setEnabled(bool)));

    actionUndo->setEnabled(currentEditor->document()->isUndoAvailable());
    actionRedo->setEnabled(currentEditor->document()->isRedoAvailable());

    connect(actionUndo, SIGNAL(triggered()), currentEditor->document(), SLOT(undo()));
    connect(actionRedo, SIGNAL(triggered()), currentEditor->document(), SLOT(redo()));

    const bool selection = currentEditor->cursor().hasSelection();
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
    //edit->setTextFormat(RichText);
    connect(edit, SIGNAL(currentCharFormatChanged(const QTextCharFormat &)),
            this, SLOT(currentCharFormatChanged(const QTextCharFormat &)));

    int tab = tabWidget->addTab(edit, title.isEmpty() ? tr("noname") : title);
    tabWidget->setCurrentIndex(tab);
    edit->viewport()->setFocus();

    return edit;
}

