#include "browserwidget.h"

#include <qgenerictableview.h>
#include <qhbox.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qsqldatabase.h>
#include <qsqlerror.h>
#include <qsqlmodel.h>
#include <qsqlquery.h>
#include <qtextedit.h>
#include <qvbox.h>

#include "connectionwidget.h"

BrowserWidget::BrowserWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    QHBox *box = new QHBox(this);
    dbc = new ConnectionWidget(box);

    QVBox *vbox = new QVBox(box);
    view = new QGenericTableView(vbox);
    edit = new QTextEdit(vbox);

    QHBox *hbox = new QHBox(this);
    hbox->layout()->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));

    submitButton = new QPushButton(tr("&Submit"), hbox);
    connect(submitButton, SIGNAL(clicked()), SLOT(exec()));

    layout->addWidget(box);
    layout->addWidget(hbox);

    addConnection();

    emit statusMessage(tr("Ready."));
}

BrowserWidget::~BrowserWidget()
{
}

void BrowserWidget::exec()
{
    QSqlModel *model = new QSqlModel(view);
    model->setQuery(edit->plainText());
    if (model->lastError().type() != QSqlError::NoError)
        emit statusMessage(model->lastError().text());
    view->setModel(model);
}

void BrowserWidget::addConnection()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(":memory:");
    db.open();
}

