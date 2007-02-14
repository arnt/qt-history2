#include <QtGui>

#include "window.h"
#include "colorlisteditor.h"

Window::Window()
{
    QItemEditorFactory *factory =
    	new QItemEditorFactory(*QItemEditorFactory::defaultFactory());

    QItemEditorCreatorBase *colorListCreator =
	new QStandardItemEditorCreator<ColorListEditor>();

    factory->registerEditor(QVariant::Color, colorListCreator);

    QItemEditorFactory::setDefaultFactory(factory);

    createGUI();
}

void Window::createGUI()
{
    QList<QPair<QString, QColor> > list;
    list << QPair<QString, QColor>(tr("Alice"), QColor("aliceblue")) <<
	    QPair<QString, QColor>(tr("Neptun"), QColor("aquamarine")) <<
	    QPair<QString, QColor>(tr("Ferdinand"), QColor("springgreen"));

    QTableWidget *table = new QTableWidget(3, 2);
    table->setHorizontalHeaderLabels(QStringList() << tr("Name")
				                   << tr("Hair Color"));
    table->verticalHeader()->setVisible(false);
    table->resize(150, 50);

    for (int i = 0; i < 3; ++i) {
	QPair<QString, QColor> pair = list.at(i);

	QTableWidgetItem *nameItem = new QTableWidgetItem(pair.first);
	QTableWidgetItem *colorItem = new QTableWidgetItem;
	colorItem->setData(Qt::DisplayRole, pair.second);

	table->setItem(i, 0, nameItem);
	table->setItem(i, 1, colorItem);
    }
    table->resizeColumnToContents(0);
    table->horizontalHeader()->resizeSection(1, 150);

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(table, 0, 0);

    setLayout(layout);

    setWindowTitle(tr("Color Editor Factory"));
}
