#ifndef TEMPLATEWIZARDIFACE_H
#define TEMPLATEWIZARDIFACE_H

#include <qcomponentinterface.h>
#include <qstringlist.h>

class QWidget;
struct DesignerFormWindow;

// {983d3eab-fea3-49cc-97ad-d8cc89b7c17b}
#ifndef IID_TemplateWizardInterface
#define IID_TemplateWizardInterface QUuid( 0x983d3eab, 0xfea3, 0x49cc, 0x97, 0xad, 0xd8, 0xcc, 0x89, 0xb7, 0xc1, 0x7b )
#endif

class TemplateWizardInterface : public QUnknownInterface
{
public:
    virtual QStringList featureList() const = 0;
    virtual void setup( const QString &templ, QWidget *widget, DesignerFormWindow *fw, QUnknownInterface *appIface ) = 0;

};

#endif
