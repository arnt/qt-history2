#include <qlayout.h>
#include <qpushbutton.h>
#include <qsignalmapper.h>

#include "buttonwidget.h"

ButtonWidget::ButtonWidget(QStringList captions,
                           QWidget *parent, const char *name)
    : QWidget(parent, name)
{
    int row = 0;
    int col = 0;
    signalMapper = new QSignalMapper(this);
    QGridLayout *grid = new QGridLayout(this);
    QPushButton *button = 0;
    for (uint i = 0; i < captions.count(); i++) {
	button = new QPushButton(captions[i], this);
	connect(button, SIGNAL(clicked()), signalMapper, SLOT(map()));
	signalMapper->setMapping(button, captions[i]);
	grid->addWidget(button, row, col++);
	if (col == MAXCOLS) {
	    ++row;
	    col = 0;
	}
    }
    connect(signalMapper, SIGNAL(mapped(const QString&)),
            this, SIGNAL(clicked(const QString&)));
}
