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

    workspace = new QWorkspace;
    connect(workspace, SIGNAL(windowActivated(QWidget *)),
	    this, SLOT(windowActivated(QWidget *)));
    workspace->addWindow(createDialogButtonBox(ReallyQuit));
    resolveButtons();

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(workspace);
    layout->setMargin(0);
    myCentralWidget->setLayout(layout);

    tableWidget->verticalHeader()->setVisible(false);
    tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);

    setWindowTitle(tr("Dialog Button Box Example"));
}

void MainWindow::addButton()
{
    QWidget *window = workspace->activeWindow();
    QDialogButtonBox *box = window->findChild<QDialogButtonBox *>();

    if (box) {
	AddButtonDialog *window = new AddButtonDialog(box ,this);
	window->setModal(true);
	QDialog::DialogCode code = QDialog::DialogCode(window->exec());
	if (code == QDialog::Accepted)
	    window->addButton(); 
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
	    workspace->activeWindow()->findChild<QDialogButtonBox *>();

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

    workspace->addWindow(createDialogButtonBox(
			 Presets(action->data().value<int>())))->show();
    resolveButtons();
}

void MainWindow::newStyle(QAction *action)
{
    QWidget *window = workspace->activeWindow();

    if (window)	{
	QDialogButtonBox *box = window->findChild<QDialogButtonBox *>();
	QStyle *newStyle = QStyleFactory::create(action->text());
	setStyle(box, newStyle);
	window->adjustSize();
    }
}

void MainWindow::newOrientation(QAction *action)
{
    QWidget *window = workspace->activeWindow();

    if (window) {
	QDialogButtonBox *box = window->findChild<QDialogButtonBox *>();
	box->setOrientation(Qt::Orientation(action->data().value<int>()));	
	box->adjustSize();
	resizeActiveWindow();
    }
}

void MainWindow::windowActivated(QWidget *)
{
    QWidget *window = (workspace->activeWindow());
    QDialogButtonBox *box = window->findChild<QDialogButtonBox *>();

    if (box->orientation() == Qt::Vertical)
	verticalAction->setChecked(true);
    else
	horizontalAction->setChecked(true);

    resolveButtons();
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
    QWidget *window = new QWidget;

     switch (preset) {
        case SaveChanges:
            box = new QDialogButtonBox(QDialogButtonBox::Yes |
                                       QDialogButtonBox::YesToAll |
                                       QDialogButtonBox::No |
                                       QDialogButtonBox::NoToAll |
                                       QDialogButtonBox::Help);
	    window->setWindowTitle(tr("Save Changes"));
            break;
        case ReallyQuit:
            box = new QDialogButtonBox(QDialogButtonBox::Cancel |
                                       QDialogButtonBox::Yes);
	    window->setWindowTitle(tr("Really Quit"));
            break;
        case FileError:
            box = new QDialogButtonBox(QDialogButtonBox::Retry |
                                       QDialogButtonBox::Abort |
                                       QDialogButtonBox::Ignore);
	    window->setWindowTitle(tr("File Error"));
            break;
        default:
            box = new QDialogButtonBox;
	    window->setWindowTitle(tr("Magic Box"));
            box->resize(180, 27);
    }
    setStyle(box,
	QStyleFactory::create(styleGroup->checkedAction()->text()));
    box->adjustSize();

    QGridLayout *layout = new QGridLayout;
    layout->addItem(new QSpacerItem(box->width() + 50, 75), 0, 0);
    layout->addWidget(box, 1, 0);
    window->setLayout(layout);
    window->adjustSize();
    horizontalAction->setChecked(true);

    return window;
}

void MainWindow::setStyle(QDialogButtonBox *box, QStyle *style)
{	
    box->setStyle(style);
    foreach (QObject *child, box->children()) {
	QWidget *widget = qobject_cast<QWidget *>(child);
	if (widget) {
	    widget->setStyle(style);
	}
    }
}

void MainWindow::resolveButtons()
{
    QWidget *window = (workspace->activeWindow());
    QDialogButtonBox *box = window->findChild<QDialogButtonBox *>();

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
    QWidget *window = workspace->activeWindow();
    Q_ASSERT(window);
    QDialogButtonBox *box = window->findChild<QDialogButtonBox *>();
    delete window->layout();
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
    window->setLayout(layout);
    window->adjustSize();
}
