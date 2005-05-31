/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ Trolltech AS. All rights reserved.
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

void MainWindow::newFile()
{
    editor->clear();
}

void MainWindow::openFile(const QString &path)
{
    QString fileName = path;

    if (fileName.isNull())
        fileName = QFileDialog::getOpenFileName(this,
            tr("Open File"), "", "qmake Files (*.pro *.prf *.pri)");

    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QFile::ReadOnly | QFile::Text))
            editor->setPlainText(file.readAll());
    }
}

void MainWindow::setupEditor()
{
    QTextCharFormat variableFormat;
    variableFormat.setFontWeight(QFont::Bold);
    variableFormat.setForeground(Qt::blue);
    highlighter.addMapping("\\b[A-Z_]+\\b", variableFormat);

    QTextCharFormat singleLineCommentFormat;
    singleLineCommentFormat.setBackground(QColor("#77ff77"));
    highlighter.addMapping("#[^\n]*", singleLineCommentFormat);

    QTextCharFormat quotationFormat;
    quotationFormat.setBackground(Qt::cyan);
    quotationFormat.setForeground(Qt::blue);
    highlighter.addMapping("\".*\"", quotationFormat);

    QTextCharFormat functionFormat;
    functionFormat.setFontItalic(true);
    functionFormat.setForeground(Qt::blue);
    highlighter.addMapping("\\b[a-z0-9_]+\\(.*\\)", functionFormat);

    QFont font;
    font.setFamily("Courier");
    font.setFixedPitch(true);
    font.setPointSize(10);

    editor = new QTextEdit;
    editor->setFont(font);
    highlighter.addToDocument(editor->document());
}

void MainWindow::setupFileMenu()
{
    QMenu *fileMenu = new QMenu(tr("&File"), this);
    menuBar()->addMenu(fileMenu);

    fileMenu->addAction(tr("&New..."), this, SLOT(newFile()),
                        QKeySequence(tr("Ctrl+N", "File|New")));
    fileMenu->addAction(tr("&Open..."), this, SLOT(openFile()),
                        QKeySequence(tr("Ctrl+O", "File|Open")));
    fileMenu->addAction(tr("E&xit"), qApp, SLOT(quit()),
                        QKeySequence(tr("Ctrl+Q", "File|Exit")));
}
