/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

void ArchiveDialog::init()
{
    connect(&articleSearcher, SIGNAL(done(bool)), this, SLOT(searchDone(bool)));
    connect(&articleFetcher, SIGNAL(done(bool)), this, SLOT(fetchDone(bool)));
    connect(myListView, SIGNAL(selectionChanged(Q3ListViewItem*)), this, SLOT(fetch(Q3ListViewItem*)));
    connect(myLineEdit, SIGNAL(returnPressed()), this, SLOT(search()));
    connect(myListView, SIGNAL(returnPressed(Q3ListViewItem*)), this, SLOT(fetch(Q3ListViewItem*)));
    connect(myPushButton, SIGNAL(clicked()), this, SLOT(close()));
}

void ArchiveDialog::fetch( Q3ListViewItem *it )
{
    QUrl u(it->text(1));
    articleFetcher.setHost(u.host());
    articleFetcher.get(it->text(1));
}

void ArchiveDialog::fetchDone( bool error )
{
    if (error) {
	QMessageBox::critical(this, "Error fetching",
			      "An error occurred when fetching this document: "
			      + articleFetcher.errorString(),	
			      QMessageBox::Ok, QMessageBox::NoButton);
    } else {
	myTextBrowser->setText(articleFetcher.readAll());
    }
}

void ArchiveDialog::search()
{
    if (articleSearcher.state() == QHttp::HostLookup
	|| articleSearcher.state() == QHttp::Connecting
	|| articleSearcher.state() == QHttp::Sending
	|| articleSearcher.state() == QHttp::Reading) {
	articleSearcher.abort();
    }
    
    if (myLineEdit->text() == "") {
	QMessageBox::critical(this, "Empty query",
			      "Please type a search string.",
			      QMessageBox::Ok, QMessageBox::NoButton);
    } else {
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    
	articleSearcher.setHost("www.trolltech.com");
    
	QHttpRequestHeader header("POST", "/search.html");
	header.setValue("Host", "www.trolltech.com");
	header.setContentType("application/x-www-form-urlencoded");
    
	QString encodedTopic = myLineEdit->text();
	QUrl::encode(encodedTopic);
	QString searchString = "qt-interest=on&search=" + encodedTopic;

	articleSearcher.request(header, searchString.utf8());
    }

}

void ArchiveDialog::searchDone( bool error )
{
    if (error) {
	QMessageBox::critical(this, "Error searching",
			      "An error occurred when searching: "
			      + articleSearcher.errorString(),
			      QMessageBox::Ok, QMessageBox::NoButton);
    } else {
	QString result(articleSearcher.readAll());
	
	QRegExp rx("<a href=\"(http://lists\\.trolltech\\.com/qt-interest/.*)\">(.*)</a>");
	rx.setMinimal(TRUE);
	int pos = 0;
	while (pos >= 0) {
	    pos = rx.search(result, pos);
	    if (pos > -1) {
		pos += rx.matchedLength();
		new Q3ListViewItem(myListView, rx.cap(2), rx.cap(1));
	    }
	}
    }
    
    QApplication::restoreOverrideCursor();
}
