#ifndef TEMPLATEWIZARDIFACE_H
#define TEMPLATEWIZARDIFACE_H

#include <qcomponentinterface.h>
#include <qstringlist.h>
#include <qmap.h>

class QWidget;
class QObjectList;
class QObject;

// {983d3eab-fea3-49cc-97ad-d8cc89b7c17b}
Q_GUID( IID_TemplateWizardInterface, 0x983d3eab, 0xfea3, 0x49cc, 0x97, 0xad, 0xd8, 0xcc, 0x89, 0xb7, 0xc1, 0x7b );

class TemplateWizardInterface : public QUnknownInterface
{
public:
    struct DatabaseConnection
    {
	QString connection;
	QStringList tables;
	QMap<QString, QStringList> fields;
    };

    virtual QStringList featureList() const = 0;
    virtual void setup( const QString &templ, QWidget *widget, const QValueList<DatabaseConnection> &dbConnections ) = 0;

};

#endif
