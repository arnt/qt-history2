/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef MULTIPAGEWIDGETTASKMENUEXTENSION_H
#define MULTIPAGEWIDGETTASKMENUEXTENSION_H

#include <QtDesigner/QDesignerTaskMenuExtension>

class QAction;
class MultiPageWidget;

class MultiPageWidgetTaskMenuExtension: public QObject, public QDesignerTaskMenuExtension
{
    Q_OBJECT
    Q_INTERFACES(QDesignerTaskMenuExtension)

public:
    MultiPageWidgetTaskMenuExtension(MultiPageWidget *widget, QObject *parent);

    QList<QAction *> taskActions() const;

private slots:
    void addPage();
    void removePage();

private:
    QAction *addPageAction;
    QAction *removePageAction;
    MultiPageWidget *myWidget;
};

#endif
