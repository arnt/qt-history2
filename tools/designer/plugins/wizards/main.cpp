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

#include <designerinterface.h>
#include <templatewizardiface.h>
#ifndef QT_NO_SQL
#include "sqlformwizardimpl.h"
#endif
#include "mainwindowwizard.h"
#include <qfeatures.h>
#include <qwidget.h>
#include <qapplication.h>

class StandardTemplateWizardInterface : public TemplateWizardInterface, public QLibraryInterface
{
public:
    StandardTemplateWizardInterface();
    virtual ~StandardTemplateWizardInterface();

    QRESULT queryInterface( const QUuid&, QUnknownInterface** );
    Q_REFCOUNT;

    QStringList featureList() const;

    void setup( const QString &templ, QWidget *widget, DesignerFormWindow *fw, QUnknownInterface *aIface );

    bool init();
    void cleanup();
    bool canUnload() const;

private:
    bool inUse;
};

StandardTemplateWizardInterface::StandardTemplateWizardInterface()
    : inUse( false )
{
}

StandardTemplateWizardInterface::~StandardTemplateWizardInterface()
{
}

bool StandardTemplateWizardInterface::init()
{
    return true;
}

void StandardTemplateWizardInterface::cleanup()
{
}

bool StandardTemplateWizardInterface::canUnload() const
{
    return !inUse;
}

QStringList StandardTemplateWizardInterface::featureList() const
{
    QStringList list;
#ifndef QT_NO_SQL
    list << "QDataBrowser" << "QDesignerDataBrowser" << "QDataView" << \
	"QDesignerDataView" << "QDataTable";
#endif
    list << "QMainWindow";

    return list;
}

void StandardTemplateWizardInterface::setup( const QString &templ, QWidget *widget, DesignerFormWindow *fw, QUnknownInterface *aIface )
{
    inUse = true;
#ifndef QT_NO_SQL
    if ( templ == "QDesignerDataView" ||
	 templ == "QDesignerDataBrowser" ||
	 templ == "QDataView" ||
	 templ == "QDataBrowser" ||
	 templ == "QDataTable" ) {
	SqlFormWizard wizard( aIface, widget, qApp->mainWidget(), fw, 0, true );
	wizard.exec();
    }
#endif
    if ( templ == "QMainWindow" ) {
	MainWindowWizardBase wizard( qApp->mainWidget(), 0, true );
	wizard.setAppInterface( aIface, fw, widget );
	wizard.exec();
    }
    inUse = false;
}

QRESULT StandardTemplateWizardInterface::queryInterface( const QUuid& uuid, QUnknownInterface** iface )
{
    *iface = 0;
    if ( uuid == IID_QUnknown )
	*iface = (QUnknownInterface*)(TemplateWizardInterface*) this;
    else if ( uuid == IID_QFeatureList )
	*iface = (QFeatureListInterface*)this;
    else if ( uuid == IID_TemplateWizard )
	*iface = (TemplateWizardInterface*)this;
    else if ( uuid == IID_QLibrary )
	*iface = (QLibraryInterface*)this;
    else
	return QE_NOINTERFACE;

    (*iface)->addRef();
    return QS_OK;
}

Q_EXPORT_COMPONENT()
{
    Q_CREATE_INSTANCE( StandardTemplateWizardInterface )
}
