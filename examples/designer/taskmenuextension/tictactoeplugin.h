/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef TICTACTOEPLUGIN_H
#define TICTACTOEPLUGIN_H

#include <QDesignerCustomWidgetInterface>

class QIcon;
class QWidget;

class TicTacToePlugin : public QObject, public QDesignerCustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
    TicTacToePlugin(QObject *parent = 0);

    QString name() const;
    QString group() const;
    QString toolTip() const;
    QString whatsThis() const;
    QString includeFile() const;
    QIcon icon() const;
    bool isContainer() const;
    QWidget *createWidget(QWidget *parent);
    bool isInitialized() const;
    void initialize(QDesignerFormEditorInterface *formEditor);

private:
    bool initialized;
};

#endif
