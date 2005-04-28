#include <QtGui>

#include "interfaces.h"
#include "mainwindow.h"
#include "paintarea.h"
#include "plugindialog.h"

MainWindow::MainWindow()
{
    paintArea = new PaintArea;

    scrollArea = new QScrollArea;
    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setWidget(paintArea);
    setCentralWidget(scrollArea);

    createActions();
    createMenus();
    loadPlugins();

    setWindowTitle(tr("Plug & Paint"));
}

void MainWindow::open()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                    tr("Open File"), QDir::currentPath());
    if (!fileName.isEmpty()) {
        if (!paintArea->openImage(fileName)) {
            QMessageBox::information(this, tr("Plug & Paint"),
                                     tr("Cannot load %1.").arg(fileName));
            return;
        }
        paintArea->adjustSize();
    }
}

bool MainWindow::saveAs()
{
    QString fileName = QFileDialog::getSaveFileName(this);
    if (fileName.isEmpty()) {
        return false;
    } else {
        return paintArea->saveImage(fileName);
    }
}

void MainWindow::brushColor()
{
    QColor newColor = QColorDialog::getColor(paintArea->brushColor());
    if (newColor.isValid())
        paintArea->setBrushColor(newColor);
}

void MainWindow::brushWidth()
{
    bool ok;
    int newWidth = QInputDialog::getInteger(this, tr("Plug & Paint"),
                                            tr("Select brush width:"),
                                            paintArea->brushWidth(),
                                            1, 50, 1, &ok);
    if (ok)
        paintArea->setBrushWidth(newWidth);
}


void MainWindow::changeBrush()
{
    QAction *action = qobject_cast<QAction *>(sender());
    BrushInterface *iBrush = qobject_cast<BrushInterface *>(action->parent());
    QString brush = action->text();

    paintArea->setBrush(iBrush, brush);
}

void MainWindow::insertShape()
{
    QAction *action = qobject_cast<QAction *>(sender());
    ShapeInterface *iShape = qobject_cast<ShapeInterface *>(action->parent());

    QPainterPath path = iShape->generateShape(action->text(), this);
    if (!path.isEmpty())
        paintArea->insertShape(path);
}

void MainWindow::applyFilter()
{
    QAction *action = qobject_cast<QAction *>(sender());
    FilterInterface *iFilter =
            qobject_cast<FilterInterface *>(action->parent());

    QImage image = iFilter->filterImage(action->text(), paintArea->image());
    paintArea->setImage(image);
}

void MainWindow::about()
{
   QMessageBox::about(this, tr("About Plug & Paint"),
            tr("The <b>Plug & Paint</b> example demonstrates how to write Qt "
               "applications that can be extended through plugins."));
}

void MainWindow::createActions()
{
    openAct = new QAction(tr("&Open..."), this);
    openAct->setShortcut(tr("Ctrl+O"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

    saveAsAct = new QAction(tr("&Save As..."), this);
    saveAsAct->setShortcut(tr("Ctrl+S"));
    connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcut(tr("Ctrl+Q"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    brushColorAct = new QAction(tr("&Brush Color..."), this);
    connect(brushColorAct, SIGNAL(triggered()), this, SLOT(brushColor()));

    brushWidthAct = new QAction(tr("&Brush Width..."), this);
    connect(brushWidthAct, SIGNAL(triggered()), this, SLOT(brushWidth()));

    brushActionGroup = new QActionGroup(this);

    aboutAct = new QAction(tr("&About"), this);
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    aboutQtAct = new QAction(tr("About &Qt"), this);
    connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAsAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    brushMenu = menuBar()->addMenu(tr("&Brush"));
    brushMenu->addAction(brushColorAct);
    brushMenu->addAction(brushWidthAct);
    brushMenu->addSeparator();

    shapesMenu = menuBar()->addMenu(tr("&Shapes"));

    filterMenu = menuBar()->addMenu(tr("&Filter"));

    menuBar()->addSeparator();

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutQtAct);
}

void MainWindow::loadPlugins()
{
    QDir dir(qApp->applicationDirPath());
    dir.cd("plugins");

    QStringList fileFilters;
    fileFilters << "pnp_*" << "libpnp_*";

    QStringList fileNames = dir.entryList(fileFilters);

    foreach (QString fileName, fileNames) {
        QPluginLoader loader(dir.absoluteFilePath(fileName));
        QObject *plugin = loader.instance();
        if (plugin) {
            BrushInterface *iBrush = qobject_cast<BrushInterface *>(plugin);
            if (iBrush)
                addToMenu(plugin, iBrush->brushes(), brushMenu,
                          SLOT(changeBrush()), brushActionGroup);

            ShapeInterface *iShape = qobject_cast<ShapeInterface *>(plugin);
            if (iShape)
                addToMenu(plugin, iShape->shapes(), shapesMenu,
                          SLOT(insertShape()));

            FilterInterface *iFilter = qobject_cast<FilterInterface *>(plugin);
            if (iFilter)
                addToMenu(plugin, iFilter->filters(), filterMenu,
                          SLOT(applyFilter()));
        }
    }

    brushMenu->setEnabled(!brushActionGroup->actions().isEmpty());
    shapesMenu->setEnabled(!shapesMenu->actions().isEmpty());
    filterMenu->setEnabled(!filterMenu->actions().isEmpty());

    PluginDialog dialog(dir.path(), fileNames, this);
    dialog.exec();
}

void MainWindow::addToMenu(QObject *plugin, const QStringList &texts,
                           QMenu *menu, const char *member,
                           QActionGroup *actionGroup)
{
    foreach (QString text, texts) {
        QAction *action = new QAction(text, plugin);
        connect(action, SIGNAL(triggered()), this, member);
        menu->addAction(action);

        if (actionGroup) {
            action->setCheckable(true);
            actionGroup->addAction(action);
            action->setChecked(actionGroup->actions().isEmpty()); // ###
        }
    }
}
