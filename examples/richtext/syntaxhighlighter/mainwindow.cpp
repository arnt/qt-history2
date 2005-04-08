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

#include <QtGui>

#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupFileMenu();
    setupEditor();

    setCentralWidget(editor);
    setWindowTitle(tr("Simple Highlighter"));
}

void MainWindow::fileNew()
{
    editor->clear();
}

void MainWindow::fileOpen()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open File"), "", "*.py");

    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QFile::ReadOnly | QFile::Text))
            editor->setPlainText(file.readAll());
    }
}

void MainWindow::setupEditor()
{
    QTextCharFormat keywordFormat;
    keywordFormat.setForeground(Qt::red);
    keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywordStrings;
    keywordStrings << "\\band\\b" << "\\bdel\\b" << "\\bfor\\b" << "\\bis\\b"
                   << "\\braise\\b" << "\\bassert\\b" << "\\belif\\b"
                   << "\\bfrom\\b" << "\\blambda\\b" << "\\breturn\\b"
                   << "\\bbreak\\b" << "\\belse\\b" << "\\bglobal\\b"
                   << "\\bnot\\b" << "\\btry\\b" << "\\bclass\\b"
                   << "\\bexcept\\b" << "\\bif\\b" << "\\bor\\b"
                   << "\\bwhile\\b" << "\\bcontinue\\b" << "\\bexec\\b"
                   << "\\bimport\\b" << "\\bpass\\b" << "\\byield\\b"
                   << "\\bdef\\b" << "\\bfinally\\b" << "\\bin\\b"
                   << "\\bprint\\b";
    highlighter.addMapping(keywordStrings, keywordFormat);

    QTextCharFormat singleLineCommentFormat;
    singleLineCommentFormat.setBackground(Qt::green);
    singleLineCommentFormat.setFontItalic(true);
    QStringList singleLineCommentStrings;
    singleLineCommentStrings << "#[^\n]*";
    highlighter.addMapping(singleLineCommentStrings, singleLineCommentFormat);

    QTextCharFormat quotationFormat;
    quotationFormat.setBackground(Qt::cyan);
    quotationFormat.setForeground(Qt::blue);
    QStringList quotationStrings;
    quotationStrings << "\".*\"" << "'.*'";
    highlighter.addMapping(quotationStrings, quotationFormat);

    QTextCharFormat functionFormat;
    functionFormat.setForeground(Qt::blue);
    QStringList functionStrings;
    functionStrings << "\\b[A-z0-9_]*\\(.*\\)";
    highlighter.addMapping(functionStrings, functionFormat);

    editor = new QTextEdit(this);
    highlighter.addToDocument(editor->document());
}

void MainWindow::setupFileMenu()
{
    QMenu *fileMenu = new QMenu(tr("&File"), this);
    menuBar()->addMenu(fileMenu);

    fileMenu->addAction(tr("&New..."), this, SLOT(fileNew()),
                        QKeySequence(tr("Ctrl+N", "File|New")));
    fileMenu->addAction(tr("&Open..."), this, SLOT(fileOpen()),
                        QKeySequence(tr("Ctrl+O", "File|Open")));
    fileMenu->addAction(tr("E&xit"), qApp, SLOT(quit()),
                        QKeySequence(tr("Ctrl+Q", "File|Exit")));
}
