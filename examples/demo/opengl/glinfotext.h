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

#include <qlayout.h>
#include <qtextview.h>
#include <qfont.h>
#include "glinfo.h"

class GLInfoText : public QWidget
{
public:
    GLInfoText(QWidget *parent) 
	: QWidget(parent)
    {
	view = new QTextView(this);
	view->setFont(QFont("courier", 10));
	view->setWordWrap(QTextEdit::NoWrap);
	QHBoxLayout *l = new QHBoxLayout(this);
	l->addWidget(view);
	GLInfo info;
	view->setText(info.info());
    }
    
private:
    QTextView *view;
};
