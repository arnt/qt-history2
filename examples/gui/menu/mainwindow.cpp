#include <QtGui>

#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QVBoxWidget *vbox = new QVBoxWidget(this);
    vbox->setMargin(11);
    setCentralWidget(vbox);

    QWidget *filler1 = new QWidget(vbox);
    filler1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    infoLabel = new QLabel(tr("<i>Choose a menu option</i>"), vbox);
    infoLabel->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    infoLabel->setAlignment(Qt::AlignCenter);

    QWidget *filler2 = new QWidget(vbox);
    filler2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    usageLabel = new QLabel(tr(
            "A context menu is available if you right-click on the window "
            "or press your keyboard's context menu button"), vbox);
    usageLabel->setWordWrap(true);

    createMenus();

    setWindowTitle(tr("Menus"));
    setMinimumSize(160, 160);
    resize(480, 320);
}

void MainWindow::contextMenuEvent(QContextMenuEvent *event)
{
    editMenu->exec(event->globalPos());
}

void MainWindow::fileNew()
{
    infoLabel->setText("Invoked <b>File|New</b>");
}

void MainWindow::fileOpen()
{
    infoLabel->setText("Invoked <b>File|Open</b>");
}

void MainWindow::fileSave()
{
    infoLabel->setText("Invoked <b>File|Save</b>");
}

void MainWindow::filePrint()
{
    infoLabel->setText("Invoked <b>File|Print</b>");
}

void MainWindow::fileQuit()
{
    close();
}

void MainWindow::editUndo()
{
    infoLabel->setText("Invoked <b>Edit|Undo</b>");
}

void MainWindow::editRedo()
{
    infoLabel->setText("Invoked <b>Edit|Redo</b>");
}

void MainWindow::editCut()
{
    infoLabel->setText("Invoked <b>Edit|Cut</b>");
}

void MainWindow::editCopy()
{
    infoLabel->setText("Invoked <b>Edit|Copy</b>");
}

void MainWindow::editPaste()
{
    infoLabel->setText("Invoked <b>Edit|Paste</b>");
}

void MainWindow::editFormat()
{
    infoLabel->setText("Invoked <b>Edit|Format</b>");
}

void MainWindow::editFormatBold()
{
    infoLabel->setText("Invoked <b>Edit|Format|Bold</b>");
}

void MainWindow::editFormatItalic()
{
    infoLabel->setText("Invoked <b>Edit|Format|Italic</b>");
}

void MainWindow::editFormatLeftAlign()
{
    infoLabel->setText("Invoked <b>Edit|Format|Left Align</b>");
}

void MainWindow::editFormatRightAlign()
{
    infoLabel->setText("Invoked <b>Edit|Format|Right Align</b>");
}

void MainWindow::editFormatJustify()
{
    infoLabel->setText("Invoked <b>Edit|Format|Justify</b>");
}

void MainWindow::editFormatCenter()
{
    infoLabel->setText("Invoked <b>Edit|Format|Center</b>");
}

void MainWindow::editFormatSetLineSpacing()
{
    infoLabel->setText("Invoked <b>Edit|Format|Set Line Spacing</b>");
}

void MainWindow::editFormatSetParagraphSpacing()
{
    infoLabel->setText("Invoked <b>Edit|Format|Set Paragraph Spacing</b>");
}

void MainWindow::helpAbout()
{
    QMessageBox::about(this, tr("About Menu"),
            tr("The <b>Menu</b> example shows how to create "
               "menu-bar menus and context menus."));
}

void MainWindow::helpAboutQt()
{
    QMessageBox::aboutQt(this);
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(tr("&New..."), this, SLOT(fileNew()), tr("Ctrl+N"));
    fileMenu->addAction(tr("&Open..."), this, SLOT(fileOpen()), tr("Ctrl+O"));
    fileMenu->addAction(tr("&Save"), this, SLOT(fileSave()), tr("Ctrl+S"));
    fileMenu->addAction(tr("&Print"), this, SLOT(filePrint()), tr("Ctrl+P"));
    fileMenu->addSeparator();
    fileMenu->addAction(tr("&Quit"), this, SLOT(fileQuit()), tr("Ctrl+Q"));

    editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(tr("&Undo"), this, SLOT(editUndo()), tr("Ctrl+Z"));
    editMenu->addAction(tr("&Redo"), this, SLOT(editRedo()), tr("Ctrl+Y"));
    editMenu->addAction(tr("Cu&t"), this, SLOT(editCut()), tr("Ctrl+X"));
    editMenu->addAction(tr("&Copy"), this, SLOT(editCopy()), tr("Ctrl+C"));
    editMenu->addAction(tr("&Paste"), this, SLOT(editPaste()), tr("Ctrl+V"));

    QList<QAction *> alignmentActions;
    formatMenu = editMenu->addMenu(tr("&Format"));
    QAction *boldAction = formatMenu->addAction(tr("&Bold"), this,
                                SLOT(editFormatBold()), tr("Ctrl+B"));
    QFont font = boldAction->font();
    font.setBold(true);
    boldAction->setFont(font);
    QAction *italicAction = formatMenu->addAction(tr("&Italic"), this,
                                   SLOT(editFormatItalic()), tr("Ctrl+I"));
    font = italicAction->font();
    font.setItalic(true);
    italicAction->setFont(font);
    formatMenu->addSeparator();
    alignmentActions.append(formatMenu->addAction(tr("&Left Align"), this,
                            SLOT(editFormatLeftAlign()), tr("Ctrl+L")));
    alignmentActions.append(formatMenu->addAction(tr("&Right Align"), this,
                            SLOT(editFormatRightAlign()), tr("Ctrl+R")));
    alignmentActions.append(formatMenu->addAction(tr("&Justify"), this,
                            SLOT(editFormatJustify()), tr("Ctrl+J")));
    alignmentActions.append(formatMenu->addAction(tr("&Center"), this,
                            SLOT(editFormatCenter()), tr("Ctrl+E")));
    formatMenu->addSeparator();
    formatMenu->addAction(tr("Set &Line Spacing..."), this,
                          SLOT(editFormatSetLineSpacing()));
    formatMenu->addAction(tr("Set &Paragraph Spacing..."), this,
                          SLOT(editFormatSetParagraphSpacing()));

    QActionGroup *group = new QActionGroup(this);
    foreach (QAction *action, alignmentActions) {
        action->setCheckable(true);
        group->addAction(action);
    }
    alignmentActions[0]->setChecked(true);

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(tr("&About"), this, SLOT(helpAbout()));
    helpMenu->addAction(tr("About &Qt"), this, SLOT(helpAboutQt()));
}
