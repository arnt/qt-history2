/****************************************************************************
**
** Copyright (C) 2004-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/*
rdflisting.cpp

Provides a widget for displaying news items from RDF news sources.
RDF is an XML-based format for storing items of information (see
http://www.w3.org/RDF/ for details).

The widget itself provides a simple user interface for specifying
the URL of a news source, and controlling the downloading of news.

The widget downloads and parses the XML asynchronously, feeding the
data to an XML reader in pieces. This allows the user to interrupt
its operation, and also allows very large data sources to be read.
*/


#include <qhbox.h>
#include <qlayout.h>
#include <qurl.h>

#include "rdflisting.h"


/*!
    Construct an RDFListing widget with a simple user interface, and set
    up the XML reader to use a custom handler class.

    The user interface consists of a line edit, two push buttons, and a
    list view widget. The line edit is used for entering the URLs of news
    sources; the push buttons start and abort the process of reading the
    news.
*/

RDFListing::RDFListing(QWidget *parent, const char *name, WFlags flags)
    : QWidget(parent, name, flags)
{
    setCaption(tr("RDF listing example"));

    lineEdit = new QLineEdit(this);

    fetchButton = new QPushButton(tr("Fetch"), this);
    abortButton = new QPushButton(tr("Abort"), this);
    abortButton->setEnabled(false);

    listView = new QListView(this);
    listView->addColumn(tr("Title"));
    listView->addColumn(tr("Link"));
    listView->setSorting(-1);

    handler = 0;

    connect(&http, SIGNAL(readyRead(const QHttpResponseHeader &)),
             this, SLOT(readData(const QHttpResponseHeader &)));

    connect(&http, SIGNAL(requestFinished(int, bool)),
             this, SLOT(finished(int, bool)));

    connect(fetchButton, SIGNAL(clicked()), this, SLOT(fetch()));
    connect(abortButton, SIGNAL(clicked()), &http, SLOT(abort()));

    QVBoxLayout *layout = new QVBoxLayout(this);

    QHBoxLayout *hboxLayout = new QHBoxLayout;

    hboxLayout->addWidget(lineEdit);
    hboxLayout->addWidget(fetchButton);
    hboxLayout->addWidget(abortButton);

    layout->addLayout(hboxLayout);
    layout->addWidget(listView);
}

/*!
    Starts fetching data from a news source specified in the line
    edit widget.

    The line edit is made read only to prevent the user from modifying its
    contents during the fetch; this is only for cosmetic purposes.
    The fetch button is disabled, and the abort button is enabled to allow
    the user to interrupt processing. The list view is cleared, and we
    define the last list view item to be 0, meaning that there are no
    existing items in the list.

    We reset the flag used to determine whether parsing should begin again
    or continue. A new handler is created, if required, and made available
    to the reader.

    The HTTP handler is supplied with the raw contents of the line edit and
    a fetch is initiated. We keep the ID value returned by the HTTP handler
    for future reference.
*/

void RDFListing::fetch()
{
    lineEdit->setReadOnly(true);
    fetchButton->setDisabled(true);
    abortButton->setEnabled(true);
    listView->clear();

    lastItemCreated = 0;

    newInformation = true;

    if (handler != 0)
        delete handler;
    handler = new Handler;

    xmlReader.setContentHandler(handler);
    xmlReader.setErrorHandler(handler);

    connect(handler, SIGNAL(newItem(QString &, QString &)),
             this, SLOT(addItem(QString &, QString &)));

    QUrl url(lineEdit->text());

    http.setHost(url.host());
    connectionId = http.get(url.path(true));
}

/*!
    Reads data received from the RDF source.

    We read all the available data, and pass it to the XML
    input source. The first time we receive new information,
    the reader is set up for a new incremental parse;
    we continue parsing using a different function on
    subsequent calls involving the same data source.

    If parsing fails for any reason, we abort the fetch.
*/

void RDFListing::readData(const QHttpResponseHeader &)
{
    bool ok;

    xmlInput.setData(http.readAll());

    if (newInformation) {
        ok = xmlReader.parse(&xmlInput, true);
        newInformation = false;
    }
    else
        ok = xmlReader.parseContinue();

    if (!ok) http.abort();
}

/*!
    Finish processing an HTTP request.

    The default behavior is to keep the text edit read only.

    If an error has occurred, the user interface is made available
    to the user for further input, allowing a new fetch to be
    started.

    If the HTTP get request has finished, we perform a final
    parsing operation on the data returned to ensure that it was
    well-formed. Whether this is successful or not, we make the
    user interface available to the user for further input.
*/

void RDFListing::finished(int id, bool error)
{
    if (error) {
        qWarning("Received error during HTTP fetch.");
        lineEdit->setReadOnly(false);
        abortButton->setDisabled(true);
        fetchButton->setEnabled(true);
    }
    else if (id == connectionId) {

        bool ok = xmlReader.parseContinue();
        if (!ok)
            qWarning("Parse error at the end of input.");

        lineEdit->setReadOnly(false);
        abortButton->setDisabled(true);
        fetchButton->setEnabled(true);
    }
}

/*!
    Add an item to the list view as it is reported by the handler.

    We keep a record of the last item created to ensure that the
    items are created in sequence.
*/

void RDFListing::addItem(QString &title, QString &link)
{
    QListViewItem *item;

    item = new QListViewItem(listView, lastItemCreated);
    item->setText(0, title);
    item->setText(1, link);

    lastItemCreated = item;
}

