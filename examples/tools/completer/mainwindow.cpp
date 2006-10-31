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
#include "dirmodel.h"
#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), completer(0), comboBox(0), lineEdit(0)
{
    createMenu();

    QWidget *centralWidget = new QWidget;
    
    QLabel *modelLabel = new QLabel;
    modelLabel->setText(tr("Model"));

    modelCombo = new QComboBox;
    modelCombo->addItem(tr("QDirModel"));
    modelCombo->addItem(tr("QDirModel that shows full path"));
    modelCombo->addItem(tr("Country list"));
    modelCombo->addItem(tr("Word list"));
    modelCombo->setCurrentIndex(0);

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

    contentsLabel = new QLabel;
    contentsLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    connect(modelCombo, SIGNAL(activated(int)), this, SLOT(updateModel()));
    connect(modeCombo, SIGNAL(activated(int)), this, SLOT(changeMode(int)));
    connect(caseCombo, SIGNAL(activated(int)), this, SLOT(changeCase(int)));
    
    QGridLayout *layout = new QGridLayout; 
    layout->addWidget(modelLabel, 0, 0); layout->addWidget(modelCombo, 0, 1);
    layout->addWidget(modeLabel, 1, 0);  layout->addWidget(modeCombo, 1, 1);
    layout->addWidget(caseLabel, 2, 0);  layout->addWidget(caseCombo, 2, 1);
    layout->addWidget(contentsLabel, 3, 0, 1, 2);
    centralWidget->setLayout(layout);
    setCentralWidget(centralWidget);

    useComboBox(false);
    updateModel();
    changeCase(caseCombo->currentIndex());
    changeMode(modeCombo->currentIndex());

    setWindowTitle(tr("Completer"));
}

void MainWindow::createMenu()
{
    QAction *comboAction = new QAction(tr("Show QComboBox"), this);
    comboAction->setCheckable(true);
    QAction *exitAction = new QAction(tr("Exit"), this);
    QAction *aboutAct = new QAction(tr("About"), this);
    QAction *aboutQtAct = new QAction(tr("About Qt"), this);

    connect(comboAction, SIGNAL(toggled(bool)), this, SLOT(useComboBox(bool)));
    connect(exitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));
    connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    QMenu* fileMenu = menuBar()->addMenu(tr("File"));
    fileMenu->addAction(comboAction);
    fileMenu->addSeparator();
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

    completer->setCompletionMode(mode);
}

QAbstractItemModel *MainWindow::modelFromFile(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly))
        return new QStringListModel(completer);

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    QStringList words;
    
    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        if (!line.isEmpty())
            words << line.trimmed();
    }

    QApplication::restoreOverrideCursor();

    if (!fileName.contains(QLatin1String("countries.txt")))
        return new QStringListModel(words, completer);

    // The last two chars of the countries.txt file indicate the country
    // symbol. We put that in column 2 of a standard item model
    QStandardItemModel *m = new QStandardItemModel(words.count(), 2, completer);
    for (int i = 0; i < words.count(); ++i) {
        QModelIndex countryIdx = m->index(i, 0);
        QModelIndex symbolIdx = m->index(i, 1);
        QString country = words[i].mid(0, words[i].length() - 2).trimmed();
        QString symbol = words[i].right(2);
        m->setData(countryIdx, country);
        m->setData(symbolIdx, symbol);
    }
    
    return m;
}

void MainWindow::updateModel()
{
    if (completer) {
        comboBox ? comboBox->setCompleter(0) : lineEdit->setCompleter(0);
        delete completer;
    }
    completer = new QCompleter(this);
    comboBox ? comboBox->setCompleter(completer) : lineEdit->setCompleter(completer);

    switch (modelCombo->currentIndex()) {
    default:
    case 0:
        { // Unsorted QDirModel
            QDirModel *dirModel = new QDirModel(completer);
            completer->setModel(dirModel);
            contentsLabel->setText(tr("Enter file path"));
        }
        break;
    case 1:
        { // DirModel that shows full paths
            DirModel *dirModel = new DirModel(completer);
            completer->setModel(dirModel);
            contentsLabel->setText(tr("Enter file path"));
        }
        break;
    case 2:
        { // Country List
            completer->setModel(modelFromFile(":/resources/countries.txt"));
            QTreeView *treeView = new QTreeView;
            completer->setPopup(treeView);
            treeView->setRootIsDecorated(false);
            treeView->header()->hide();
            treeView->header()->setStretchLastSection(false);
            treeView->header()->setResizeMode(0, QHeaderView::Stretch);
            treeView->header()->setResizeMode(1, QHeaderView::ResizeToContents);
            contentsLabel->setText(tr("Enter name of your country"));
        }
        break;
    case 3:
        { // Word list
            completer->setModel(modelFromFile(":/resources/wordlist.txt"));
            completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
            contentsLabel->setText(tr("Enter a word"));
        }
        break;
    }

    changeCase(caseCombo->currentIndex());
    changeMode(modeCombo->currentIndex());
}

void MainWindow::useComboBox(bool combo)
{
    if ((comboBox && combo) || (lineEdit && !combo))
        return;
    if (combo) {
        delete lineEdit;
        lineEdit = 0;
        comboBox = new QComboBox;
        comboBox->setEditable(true);
        comboBox->setCompleter(completer);
        (static_cast<QGridLayout *>(centralWidget()->layout()))->addWidget(comboBox, 4, 0, 1, 2);
        comboBox->setFocus();
    } else {
        delete comboBox;
        comboBox = 0;
        lineEdit = new QLineEdit;
        lineEdit->setCompleter(completer);
        (static_cast<QGridLayout *>(centralWidget()->layout()))->addWidget(lineEdit, 4, 0, 1, 2);
        lineEdit->setFocus();
    }
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("About"), tr("This example demonstrates the "
        "different features of the QCompleter class."));
}

void MainWindow::changeCase(int cs)
{
    completer->setCaseSensitivity(cs ? Qt::CaseSensitive : Qt::CaseInsensitive);
}
