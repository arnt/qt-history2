#ifndef SYNTAXCHECKINTERFACE_H
#define SYNTAXCHECKINTERFACE_H

#include <qcomponentinterface.h>
#include <qstringlist.h>

// {25e8412e-4a4f-4265-a370-b29ebb7a537d}
Q_UUID( IID_SyntaxCheckInterface,
	0x25e8412e, 0x4a4f, 0x4265, 0xa3, 0x70, 0xb2, 0x9e, 0xbb, 0x7a, 0x53, 0x7d );

class SyntaxCheckInterface : public QUnknownInterface
{
public:
    virtual QStringList featureList() const = 0;
    virtual void checkSyntax( const QString &code, QString &error, int &line ) = 0;

};

#endif
