#include <QtGui>

#include "mainwindow.h"
#include "addbuttondialog.h"

MainWindow::MainWindow()
{
    setupUi(this);
    myAddButton = deleteAddDialogBox->addButton(tr("Add"), 
				      QDialogButtonBox::AcceptRole);
    myDeleteButton = deleteAddDialogBox->addButton(tr("Delete"),
				      QDialogButtonBox::RejectRole);

    connectActions();

    mdiArea = new QMdiArea;
    mdiArea->setScrollBarsEnabled(true);
    connect(mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow *)),
	    this, SLOT(subWindowActivated(QMdiSubWindow *)));
    currentWindow =
	mdiArea->addSubWindow(createDialogButtonBox(ReallyQuit));

    resolveButtons();

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(mdiArea);
    layout->setMargin(0);
    myCentralWidget->setLayout(layout);

    tableWidget->verticalHeader()->setVisible(false);
    tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);

    setWindowTitle(tr("Dialog Button Box Example"));
}

void MainWindow::addButton()
{
    if (!currentWindow)
	return;

    QDialogButtonBox *box = currentWindow->findChild<QDialogButtonBox *>();

    if (box) {
	AddButtonDialog *dialog = new AddButtonDialog(box ,this);
	dialog->setModal(true);
	QDialog::DialogCode code = QDialog::DialogCode(dialog->exec());

	if (code == QDialog::Accepted)
	    dialog->addButton(); 
	box->adjustSize();
	resolveButtons();
    }    
    resizeActiveWindow();
}

void MainWindow::deleteButton()
{
    QList<QTableWidgetItem *> list = tableWidget->selectedItems();

    if (!list.isEmpty()) {
	int row = list.first()->row();
	QString buttonText = tableWidget->item(row, 0)->text();
	tableWidget->removeRow(row);
	QDialogButtonBox *box = 
	    currentWindow->findChild<QDialogButtonBox *>();

	if (box) {
	    foreach (QAbstractButton *button, box->buttons()) {
		if (button->text().remove('&') == buttonText) {
		    box->removeButton(button);
		    resizeActiveWindow();
		}
	    }
	}
    }	
}

void MainWindow::loadPresetBox()
{   
    QAction *action = qobject_cast<QAction *>(sender());

    currentWindow =
	mdiArea->addSubWindow(
	    createDialogButtonBox(Presets(action->data().value<int>())));
    currentWindow->show();

    resolveButtons();
}

void MainWindow::newStyle(QAction *action)
{
    if (currentWindow)	{
	QDialogButtonBox *box = currentWindow->findChild<QDialogButtonBox *>();
	QStyle *newStyle = QStyleFactory::create(action->text());
	setStyle(box, newStyle);
	currentWindow->adjustSize();
    }
}

void MainWindow::newOrientation(QAction *action)
{
    if (currentWindow) {
	QDialogButtonBox *box = currentWindow->findChild<QDialogButtonBox *>();
	box->setOrientation(Qt::Orientation(action->data().value<int>()));	
	box->adjustSize();
	resizeActiveWindow();
    }
}

void MainWindow::subWindowActivated(QMdiSubWindow *window)
{
    currentWindow = window;
    
    QDialogButtonBox *box = currentWindow->findChild<QDialogButtonBox *>();

    if (box) {
	if (box->orientation() == Qt::Vertical)
	    verticalAction->setChecked(true);
	else
	    horizontalAction->setChecked(true);

	foreach (QAction *action, styleGroup->actions()) {
	    if (QStyleFactory::create(action->text())->metaObject()->className()
		== box->style()->metaObject()->className()) {
		action->setChecked(true);
	    }
	}

	resolveButtons();
    } else
	tableWidget->setRowCount(0);
}

void MainWindow::connectActions()
{
    loadSaveChangesAction->setData(int(SaveChanges));
    loadReallyQuitAction->setData(int(ReallyQuit));
    loadEmptyAction->setData(int(Empty));
    loadFileErrorAction->setData(int(FileError));

    verticalAction->setData(int(Qt::Vertical));
    horizontalAction->setData(int(Qt::Horizontal));

    styleGroup = new QActionGroup(this);
    foreach (QString style, QStyleFactory::keys()) {
	QAction *action = new QAction(style, this);
	action->setCheckable(true);
	action->setData(QStyleFactory::create(style));
	stylesMenu->addAction(action);
	styleGroup->addAction(action);
    }
    styleGroup->actions().first()->setChecked(true);

    orientationGroup = new QActionGroup(this);
    orientationGroup->addAction(horizontalAction);
    orientationGroup->addAction(verticalAction);

    connect(loadSaveChangesAction, SIGNAL(triggered()),
	    this, SLOT(loadPresetBox()));
    connect(loadReallyQuitAction, SIGNAL(triggered()),
	    this, SLOT(loadPresetBox()));
    connect(loadEmptyAction, SIGNAL(triggered()),
	    this, SLOT(loadPresetBox()));
    connect(loadFileErrorAction, SIGNAL(triggered()),
	    this, SLOT(loadPresetBox()));
    connect(addButtonAction, SIGNAL(triggered()),
	    this, SLOT(addButton()));
    connect(myAddButton, SIGNAL(clicked()),
	    this, SLOT(addButton()));    
    connect(myDeleteButton, SIGNAL(clicked()),
	    this, SLOT(deleteButton()));

    connect(styleGroup, SIGNAL(triggered(QAction *)),
	    this, SLOT(newStyle(QAction *)));
    connect(orientationGroup, SIGNAL(triggered(QAction *)),
	    this, SLOT(newOrientation(QAction *)));
}

QWidget *MainWindow::createDialogButtonBox(Presets preset)
{
    QDialogButtonBox *box;
    QWidget *widget = new QWidget;

     switch (preset) {
        case SaveChanges:
            box = new QDialogButtonBox(QDialogButtonBox::Yes |
                                       QDialogButtonBox::YesToAll |
                                       QDialogButtonBox::No |
                                       QDialogButtonBox::NoToAll |
                                       QDialogButtonBox::Help);
	    widget->setWindowTitle(tr("Save Changes"));
            break;
        case ReallyQuit:
            box = new QDialogButtonBox(QDialogButtonBox::Cancel |
                                       QDialogButtonBox::Yes);
	    widget->setWindowTitle(tr("Really Quit"));
            break;
        case FileError:
            box = new QDialogButtonBox(QDialogButtonBox::Retry |
                                       QDialogButtonBox::Abort |
                                       QDialogButtonBox::Ignore);
	    widget->setWindowTitle(tr("File Error"));
            break;
        default:
            box = new QDialogButtonBox;
	    widget->setWindowTitle(tr("Magic Box"));
            box->resize(180, 27);
    }
    setStyle(box,
	QStyleFactory::create(styleGroup->checkedAction()->text()));
    box->adjustSize();

    QGridLayout *layout = new QGridLayout;
    layout->addItem(new QSpacerItem(box->width() + 50, 75), 0, 0);
    layout->addWidget(box, 1, 0);
    widget->setLayout(layout);
    widget->adjustSize();
    horizontalAction->setChecked(true);

    return widget;
}

void MainWindow::setStyle(QDialogButtonBox *box, QStyle *style)
{	
    box->setStyle(style);
    box->setPalette(style->standardPalette());

    foreach (QObject *child, box->children()) {
	QWidget *widget = qobject_cast<QWidget *>(child);

	if (widget) {
	    widget->setStyle(style);
	    widget->setPalette(style->standardPalette());
	}
    }
}

void MainWindow::resolveButtons()
{
    QDialogButtonBox *box =
	currentWindow->findChild<QDialogButtonBox *>();

    if (box) {
	int i = 0;
	tableWidget->clearContents();
	tableWidget->setRowCount(box->buttons().count());

	foreach (QAbstractButton *button, box->buttons()) {
	    QTableWidgetItem *textItem = new QTableWidgetItem(
		button->text().remove('&'));
	    textItem->setFlags(textItem->flags() ^ Qt::ItemIsEditable);
	    tableWidget->setItem(i, 0, textItem);

	    QDialogButtonBox::ButtonRole role = box->buttonRole(button);
	    QTableWidgetItem *roleItem = 
		new QTableWidgetItem(AddButtonDialog::roleToString(role));
	    roleItem->setFlags(roleItem->flags() ^ Qt::ItemIsEditable);
	    tableWidget->setItem(i, 1, roleItem);
	    ++i;
	}
    }
}

void MainWindow::resizeActiveWindow()
{
    QDialogButtonBox *box = currentWindow->findChild<QDialogButtonBox *>();
    delete currentWindow->layout();
    QGridLayout *layout = new QGridLayout;

    box->adjustSize(); 

    if (horizontalAction->isChecked()) {
	layout->addItem(new QSpacerItem(box->width() + 50, 75), 0, 0);
	layout->addWidget(box, 1, 0);
	layout->setRowStretch(2, 10);
    } else {
	layout->addItem(new QSpacerItem(75, box->height() + 50), 0, 0);
	layout->setColumnStretch(1, 10);
	layout->addWidget(box, 0, 2);	
    }
    currentWindow->setLayout(layout);
    currentWindow->adjustSize();
}
