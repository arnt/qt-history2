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

#include <container.h>
#include <customwidget.h>
#include <abstractformeditor.h>
#include <qextensionmanager.h>

#include <qplugin.h>

#include <QIcon>

class WorldTimeClockPlugin : public QObject, public ICustomWidget
{
    Q_OBJECT
    Q_INTERFACES(ICustomWidget)

public:
    WorldTimeClockPlugin(QObject *parent = 0);
    bool isContainer() const;
    bool isForm() const;
    bool isInitialized() const;
    QIcon icon() const;
    QString codeTemplate() const;
    QString domXml() const;
    QString group() const;
    QString includeFile() const;
    QString name() const;
    QString toolTip() const;
    QString whatsThis() const;
    QWidget *createWidget(QWidget *parent);
    void initialize(AbstractFormEditor *core);

private:
    bool initialized;
};
