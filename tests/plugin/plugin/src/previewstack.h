/**********************************************************************
** Copyright (C) 2000 Troll Tech AS.  All rights reserved.
**
** This file is part of Qt GUI Designer.
**
** See the file LICENSE included in the distribution for the usage
** and distribution terms.
**
**********************************************************************/

#ifndef PREVIEWSTACK_H
#define PREVIEWSTACK_H

#include <qwidgetstack.h>

class PreviewStack : public QWidgetStack
{
    Q_OBJECT
public:
    PreviewStack( QWidget* parent = 0, const char* name = 0, WFlags f = 0 );
    ~PreviewStack();

public:
    void setPreviewPalette( QPalette );

public slots:
    void nextWidget();
    void previousWidget();
};

#endif //PREVIEWSTACK_H
