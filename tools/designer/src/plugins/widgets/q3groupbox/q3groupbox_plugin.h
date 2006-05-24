/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef Q3GROUPBOX_PLUGIN_H
#define Q3GROUPBOX_PLUGIN_H

#include <QtDesigner/QDesignerCustomWidgetInterface>

#include <Qt3Support/Q3GroupBox>
#include <QtGui/QLayout>
#include <QtCore/qplugin.h>

class Q3GroupBoxPlugin: public QObject, public QDesignerCustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)
public:
    Q3GroupBoxPlugin(QObject *parent = 0);
    virtual ~Q3GroupBoxPlugin();

    virtual QString name() const;
    virtual QString group() const;
    virtual QString toolTip() const;
    virtual QString whatsThis() const;
    virtual QString includeFile() const;
    virtual QIcon icon() const;

    virtual bool isContainer() const;

    virtual QWidget *createWidget(QWidget *parent);

    virtual bool isInitialized() const;
    virtual void initialize(QDesignerFormEditorInterface *core);

    virtual QString domXml() const
    { return QLatin1String("\
        <widget class=\"Q3GroupBox\" name=\"groupBox\">\
            <property name=\"geometry\">\
                <rect>\
                    <x>0</x>\
                    <y>0</y>\
                    <width>100</width>\
                    <height>80</height>\
                </rect>\
            </property>\
        </widget>\
      "); }

private:
    bool m_initialized;
};

#endif
