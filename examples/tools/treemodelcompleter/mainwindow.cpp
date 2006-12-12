/****************************************************************************
**
** Copyright (C) 2006-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtGui>
#include "treemodelcompleter.h"
#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), completer(0), lineEdit(0)
{
    createMenu();

    completer = new TreeModelCompleter(this);
    completer->setModel(modelFromFile(":/resources/treemodel.txt"));
    completer->setSeparator(QLatin1String("."));

    QWidget *centralWidget = new QWidget;

    QLabel *modelLabel = new QLabel;
    modelLabel->setText(tr("Tree Model<br>(Double click items to edit)"));

    QLabel *modeLabel = new QLabel;
    modeLabel->setText(tr("Completion Mode"));
    modeCombo = new QComboBox;
    modeCombo->addItem(tr("Inline"));
    modeCombo->addItem(tr("Filtered Popup"));
    modeCombo->addItem(tr("Unfiltered Popup"));
    modeCombo->setCurrentIndex(1);

    QLabel *caseLabel = new QLabel;
    caseLabel->setText(tr("Case Sensitivity"));
    caseCombo = new QComboBox;
    caseCombo->addItem(tr("Case Insensitive"));
    caseCombo->addItem(tr("Case Sensitive"));
#ifdef Q_OS_WIN
    caseCombo->setCurrentIndex(0);
#else
    caseCombo->setCurrentIndex(1);
#endif

    wrapCheckBox = new QCheckBox;
    wrapCheckBox->setText(tr("Wrap completions"));
    wrapCheckBox->setChecked(completer->wrapCompletions());
    connect(wrapCheckBox, SIGNAL(clicked(bool)), completer, SLOT(setWrapCompletions(bool)));

    QLabel *contentsLabel = new QLabel;
    contentsLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    contentsLabel->setText(QString("Type paths from the model above with items at each level separated by a '%1'").arg(completer->separator()));

    QTreeView *treeView = new QTreeView;
    treeView->setModel(completer->model());
    treeView->header()->hide();
    treeView->expandAll();

    connect(modeCombo, SIGNAL(activated(int)), this, SLOT(changeMode(int)));
    connect(caseCombo, SIGNAL(activated(int)), this, SLOT(changeCase(int)));

    lineEdit = new QLineEdit;
    lineEdit->setCompleter(completer);

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(modelLabel, 0, 0); layout->addWidget(treeView, 0, 1);
    layout->addWidget(modeLabel, 1, 0);  layout->addWidget(modeCombo, 1, 1);
    layout->addWidget(caseLabel, 2, 0);  layout->addWidget(caseCombo, 2, 1);
    layout->addWidget(wrapCheckBox, 3, 0);
    layout->addWidget(contentsLabel, 4, 0, 1, 2);
    layout->addWidget(lineEdit, 5, 0, 1, 2);
    centralWidget->setLayout(layout);
    setCentralWidget(centralWidget);

    changeCase(caseCombo->currentIndex());
    changeMode(modeCombo->currentIndex());

    setWindowTitle(tr("Tree Model Completer"));
    lineEdit->setFocus();
}

void MainWindow::createMenu()
{
    QAction *exitAction = new QAction(tr("Exit"), this);
    QAction *aboutAct = new QAction(tr("About"), this);
    QAction *aboutQtAct = new QAction(tr("About Qt"), this);

    connect(exitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));
    connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    QMenu* fileMenu = menuBar()->addMenu(tr("File"));
    fileMenu->addAction(exitAction);

    QMenu* helpMenu = menuBar()->addMenu(tr("About"));
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutQtAct);
}

void MainWindow::changeMode(int index)
{
    QCompleter::CompletionMode mode;
    if (index == 0)
        mode = QCompleter::InlineCompletion;
    else if (index == 1)
        mode = QCompleter::PopupCompletion;
    else
        mode = QCompleter::UnfilteredPopupCompletion;

    wrapCheckBox->setEnabled(mode != QCompleter::InlineCompletion);
    completer->setCompletionMode(mode);
}

QAbstractItemModel *MainWindow::modelFromFile(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly))
        return new QStringListModel(completer);

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    QStringList words;

    QStandardItemModel *model = new QStandardItemModel(completer);
    QVector<QStandardItem *> parents(10);
    parents[0] = model->invisibleRootItem();

    while (!file.atEnd()) {
        QString line = file.readLine();
        QString trimmedLine = line.trimmed();
        if (line.isEmpty() || trimmedLine.isEmpty())
            continue;

        QRegExp re("^\\s+");
        int nonws = re.indexIn(line);
        int level = 0;
        if (nonws == -1) {
            level = 0;
        } else {
            if (line.startsWith("\t")) {
                level = re.cap(0).length();
            } else {
                level = re.cap(0).length()/4;
            }
        }

        if (level+1 >= parents.size())
            parents.resize(parents.size()*2);

        QStandardItem *item = new QStandardItem;
        item->setText(trimmedLine);
        parents[level]->appendRow(item);
        parents[level+1] = item;
    }

    QApplication::restoreOverrideCursor();

    return model;
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("About"), tr("This example demonstrates how "
        "to use a QCompleter with a custom tree model."));
}

void MainWindow::changeCase(int cs)
{
    completer->setCaseSensitivity(cs ? Qt::CaseSensitive : Qt::CaseInsensitive);
}

