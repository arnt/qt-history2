/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of Qt Assistant.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "topicchooserimpl.h"

#include <qlabel.h>
#include <qlistbox.h>
#include <qpushbutton.h>

TopicChooser::TopicChooser( QWidget *parent, const QStringList &lnkNames,
			    const QStringList &lnks, const QString &title )
    : TopicChooserBase( parent, 0, TRUE ), links( lnks ), linkNames( lnkNames )
{
    label->setText( tr( "Choose a topic for <b>%1</b>" ).arg( title ) );
    listbox->insertStringList( linkNames );
    listbox->setCurrentItem( listbox->firstItem() );
    listbox->setFocus();
}

QString TopicChooser::link() const
{
    if ( listbox->currentItem() == -1 )
	return QString::null;
    QString s = listbox->currentText();
    if ( s.isEmpty() )
	return s;
    int i = linkNames.findIndex( s );
    return links[ i ];
}

QString TopicChooser::getLink( QWidget *parent, const QStringList &lnkNames,
				      const QStringList &lnks, const QString &title )
{
    TopicChooser *dlg = new TopicChooser( parent, lnkNames, lnks, title );
    QString lnk;
    if ( dlg->exec() == QDialog::Accepted )
	lnk = dlg->link();
    delete dlg;
    return lnk;
}
