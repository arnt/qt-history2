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

#ifndef TOPICCHOOSERIMPL_H
#define TOPICCHOOSERIMPL_H

#include "topicchooser.h"

#include <qstringlist.h>

class TopicChooser : public TopicChooserBase
{
    Q_OBJECT
    
public:
    TopicChooser( QWidget *parent, const QStringList &lnkNames,
		  const QStringList &lnks, const QString &title );
    
    QString link() const;
    
    static QString getLink( QWidget *parent, const QStringList &lnkNames,
			    const QStringList &lnks, const QString &title );
    
private:
    QString theLink;
    QStringList links, linkNames;

};

#endif
