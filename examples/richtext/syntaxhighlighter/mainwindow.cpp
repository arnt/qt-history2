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
    setWindowTitle(tr("Syntax Highlighter"));
}

void MainWindow::fileNew()
{
    editor->clear();
}

void MainWindow::fileOpen()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open File"), "", "qmake files (*.pro *.prf *.pri)");

    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QFile::ReadOnly | QFile::Text))
            editor->setPlainText(file.readAll());
    }
}

void MainWindow::setupEditor()
{
    QTextCharFormat defaultFormat;
    defaultFormat.setFontFamily("Courier");
    defaultFormat.setFontPointSize(10);
    
    QTextCharFormat variableFormat = defaultFormat;
    variableFormat.setFontWeight(QFont::Bold);
    variableFormat.setForeground(Qt::blue);
    highlighter.addMapping("\\b[A-Z_]+\\b", variableFormat);

    QTextCharFormat singleLineCommentFormat = defaultFormat;
    singleLineCommentFormat.setBackground(QColor("#77ff77"));
    highlighter.addMapping("#[^\n]*", singleLineCommentFormat);

    QTextCharFormat quotationFormat = defaultFormat;
    quotationFormat.setBackground(Qt::cyan);
    quotationFormat.setForeground(Qt::blue);
    highlighter.addMapping("\".*\"", quotationFormat);

    QTextCharFormat functionFormat = defaultFormat;
    functionFormat.setFontItalic(true);
    functionFormat.setForeground(Qt::blue);
    highlighter.addMapping("\\b[a-z0-9_]+\\(.*\\)", functionFormat);

    editor = new QTextEdit(this);
    editor->setFont(defaultFormat.font());
    editor->document()->setDefaultFont(defaultFormat.font());
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
