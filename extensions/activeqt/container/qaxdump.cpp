/****************************************************************************
**
** Implementation of the qax_generateDocumentation function.
**
** Copyright (C) 2001-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the Active Qt integration.
** EDITIONS: ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#define QAXPRIVATE_DECL
#include "qaxbase.cpp"

static QString docuFromName(ITypeInfo *typeInfo, const QString &name)
{
    QString docu;
    if (!typeInfo)
        return docu;

    MEMBERID memId;
    BSTR names = QStringToBSTR(name);
    typeInfo->GetIDsOfNames((BSTR*)&names, 1, &memId);
    SysFreeString(names);
    if (memId != DISPID_UNKNOWN) {
        BSTR docStringBstr, helpFileBstr;
        ulong helpContext;
        HRESULT hres = typeInfo->GetDocumentation(memId, 0, &docStringBstr, &helpContext, &helpFileBstr);
        QString docString = BSTRToQString(docStringBstr);
        QString helpFile = BSTRToQString(helpFileBstr);
        SysFreeString(docStringBstr);
        SysFreeString(helpFileBstr);
        if (hres == S_OK) {
            docu += "<p>";
            if (!docString.isEmpty())
                docu += docString + "\n";
            if (!helpFile.isEmpty())
                docu += QString("For more information, see help context %1 in %2.\n").arg(helpContext).arg(helpFile);
        }
    }

    return docu;
}

static QString namedPrototype(const QString &signature, const QString &names, int numDefArgs = 0)
{
    QString prototype;
    if (names.isEmpty()) {
        prototype = signature;
        prototype.replace(',', ", ");
    } else {
        prototype = signature.left(signature.indexOf('(') + 1);

        QString sigTypes(signature);
        sigTypes = sigTypes.mid(prototype.length());
        sigTypes.truncate(sigTypes.length() - 1);

        QString paramNames(names);

        while (!paramNames.isEmpty()) {
            QString paramName = paramNames.left(paramNames.indexOf(','));
            if (paramName.isEmpty())
                paramName = paramNames;
            paramNames = paramNames.mid(paramName.length() + 1);
            QString sigType = sigTypes.left(sigTypes.indexOf(','));
            if (sigType.isEmpty())
                sigType = sigTypes;
            sigTypes = sigTypes.mid(sigType.length() + 1);

            prototype += sigType + " " + paramName;
            if (!paramNames.isEmpty())
                prototype += ", ";
        }
        prototype += ")";
    }

    // no default arguments
    if (!numDefArgs)
        return prototype;

    int numArgs = prototype.count(',') + 1;
    // default arguments
    int comma = -1;
    while (numDefArgs > 1) {
        comma = prototype.lastIndexOf(',', comma);
        prototype.replace(comma, 1, " = 0,");
        --numDefArgs;
    }

    // last argument
    prototype.replace(')', " = 0)");

    return prototype;
}

static QString toType(const QString &t)
{
    QString type = t;
    int vartype = QVariant::nameToType(type.latin1());
    if (vartype == QVariant::Invalid)
        type = "int";
    
    if (type.startsWith("Q"))
        type = type.mid(1);
    type[0] = type[0].toUpper();
    if (type == "VariantList")
        type = "List";
    else if (type == "Map<QVariant,QVariant>")
        type = "Map";
    else if (type == "Uint")
        type = "UInt";
    
    return "to" + type + "()";
}

QString qax_generateDocumentation(QAxBase *that, QAxBasePrivate *d)
{
    that->metaObject();

    if (that->isNull())
	return QString::null;

    ITypeInfo *typeInfo = 0;
    if (d->dispatch())
	d->dispatch()->GetTypeInfo(0, LOCALE_SYSTEM_DEFAULT, &typeInfo);

    QString docu;
    QTextStream stream(&docu, IO_WriteOnly);

    const QMetaObject *mo = that->metaObject();
    QString coClass  = mo->classInfo(mo->indexOfClassInfo("CoClass")).value();

    stream << "<h1 align=center>" << coClass << " Reference</h1>" << endl;
    stream << "<p>The " << coClass << " COM object is a " << that->qObject()->className();
    stream << " with the CLSID " <<  that->control() << ".</p>" << endl;

    stream << "<h3>Interfaces</h3>" << endl;
    stream << "<ul>" << endl;
    const char *inter = 0;
    int interCount = 1;
    while ((inter = mo->classInfo(mo->indexOfClassInfo("Interface " + QByteArray::number(interCount))).value())) {
	stream << "<li>" << inter << endl;
	interCount++;
    }
    stream << "</ul>" << endl;

    stream << "<h3>Event Interfaces</h3>" << endl;
    stream << "<ul>" << endl;
    interCount = 1;  
    while ((inter = mo->classInfo(mo->indexOfClassInfo("Event Interface " + QByteArray::number(interCount))).value())) {
	stream << "<li>" << inter << endl;
	interCount++;
    }
    stream << "</ul>" << endl;

    QStringList methodDetails, propDetails;

    const int slotCount = mo->slotCount();
    if (slotCount) {
	stream << "<h2>Public Slots:</h2>" << endl;
	stream << "<ul>" << endl;

        int defArgCount = 0;
	for (int islot = mo->slotOffset(); islot < slotCount; ++islot) {
	    const QMetaMember slot = mo->slot(islot);

            if (slot.isCloned()) {
                ++defArgCount;
                continue;
            }

	    QString returntype(slot.type());
            if (returntype.isEmpty())
                returntype = "void";
            QString prototype = namedPrototype(slot.signature(), slot.parameters(), defArgCount);
            QString signature = slot.signature();
	    QString name = signature.left(signature.indexOf('('));
	    QString params = prototype.mid(name.length());
	    stream << "<li>" << returntype << " <a href=\"#" << name << "\"><b>" << name << "</b></a>" << params << ";</li>" << endl;
            
            params = signature.mid(name.length());
	    QString detail = "<h3><a name=" + name + "></a>" + returntype + " " + prototype + "<tt> [slot]</tt></h3>\n";
	    detail += docuFromName(typeInfo, name);
	    detail += "<p>Connect a signal to this slot:<pre>\n";
	    detail += "\tQObject::connect(sender, SIGNAL(someSignal" + params + "), object, SLOT(" + name + params + "));";
	    detail += "</pre>\n";

            if (1) {
                detail += "<p>Or call the function directly:<pre>\n";

                if (params != "()")
                    detail += "\tQVariantList params = ...\n";
                detail += "\t";
                QString functionToCall = "dynamicCall";
                if (returntype == "IDispatch*" || returntype == "IUnknown*") {
                    functionToCall = "querySubObject(";
                    returntype = "QAxObject *";
                }
                if (returntype != "void")
                    detail += returntype + " result = ";
                detail += "object->" + functionToCall + "(\"" + name + params + "\")";
                if (returntype != "void" && returntype != "QAxObject *" && returntype != "QVariant")
                    detail += "." + toType(returntype);
	        detail += ";</pre>\n";
	    } else {
		detail += "<p>This function has parameters of unsupported types and cannot be called directly.";
	    }

	    methodDetails << detail;
            defArgCount = 0;
	}

	stream << "</ul>" << endl;
    }
    int signalCount = mo->signalCount();
    if (signalCount) {
        ITypeLib *typeLib = 0;
        if (typeInfo) {
            UINT index = 0;
            typeInfo->GetContainingTypeLib(&typeLib, &index);
            typeInfo->Release();
        }
        typeInfo = 0;

	stream << "<h2>Signals:</h2>" << endl;
	stream << "<ul>" << endl;

	for (int isignal = mo->signalOffset(); isignal < signalCount; ++isignal) {
	    const QMetaMember signal = mo->signal(isignal);

            QString prototype = namedPrototype(signal.signature(), signal.parameters());
	    QString signature = signal.signature();
	    QString name = signature.left(signature.indexOf('('));
	    QString params = prototype.mid(name.length());
	    stream << "<li>void <a href=\"#" << name << "\"><b>" << name << "</b></a>" << params << ";</li>" << endl;

            params = signature.mid(name.length());
	    QString detail = "<h3><a name=" + name + "></a>void " + prototype + "<tt> [signal]</tt></h3>\n";
            if (typeLib) {
                interCount = 0;
                do {
                    if (typeInfo)
                        typeInfo->Release();
                    typeInfo = 0;
                    typeLib->GetTypeInfo(++interCount, &typeInfo);
                    QString typeLibDocu = docuFromName(typeInfo, name);
                    if (!typeLibDocu.isEmpty()) {
                        detail += typeLibDocu;
                        break;
                    }
                } while (typeInfo);
            }
	    detail += "<p>Connect a slot to this signal:<pre>\n";
	    detail += "\tQObject::connect(object, SIGNAL(" + name + params + "), receiver, SLOT(someSlot" + params + "));";
	    detail += "</pre>\n";

	    methodDetails << detail;
            if (typeInfo)
                typeInfo->Release();
            typeInfo = 0;
	}
	stream << "</ul>" << endl;

        if (typeLib)
            typeLib->Release();
    }

    const int propCount = mo->propertyCount();
    if (propCount) {
        if (d->dispatch())
	    d->dispatch()->GetTypeInfo(0, LOCALE_SYSTEM_DEFAULT, &typeInfo);
	stream << "<h2>Properties:</h2>" << endl;
	stream << "<ul>" << endl;

	for (int iprop = 0; iprop < propCount; ++iprop) {
	    const QMetaProperty prop = mo->property(iprop);
	    QString name(prop.name());
	    QString type(prop.type());

	    stream << "<li>" << type << " <a href=\"#" << name << "\"><b>" << name << "</b></a>;</li>" << endl;
	    QString detail = "<h3><a name=" + name + "></a>" + type + " " + name + "</h3>\n";
	    detail += docuFromName(typeInfo, name);
	    QVariant::Type vartype = QVariant::nameToType(type.latin1());
	    if (!prop.isReadable())
		continue;

	    if (prop.isEnumType())
		vartype = QVariant::Int;

            if (vartype != QVariant::Invalid) {
		detail += "<p>Read this property's value using QObject::property:<pre>\n";
                if (prop.isEnumType())
		    detail += "\tint val = ";
                else
                    detail += "\t" + type + " val = ";
		detail += "object->property(\"" + name + "\")." + toType(type) + ";\n";
		detail += "</pre>\n";
	    } else if (type == "IDispatch*" || type == "IUnknown*") {
		detail += "<p>Get the subobject using querySubObject:<pre>\n";
		detail += "\tQAxObject *" + name + " = object->querySubObject(\"" + name + "\");\n";
		detail += "</pre>\n";
	    } else {
		detail += "<p>This property is of an unsupported type.\n";
	    }
	    if (prop.isWritable()) {
		detail += "Set this property' value using QObject::setProperty:<pre>\n";
                if (prop.isEnumType()) {
                    detail += "\tint newValue = ... // string representation of values also supported\n";
                } else {
		    detail += "\t" + type + " newValue = ...\n";
                }
		detail += "\tobject->setProperty(\"" + name + "\", newValue);\n";
		detail += "</pre>\n";
		detail += "Or using the ";
		QString setterSlot;
                if (name[0].category() & QChar::Letter_Uppercase) {
		    setterSlot = "Set" + name;
		} else {
		    QString nameUp = name;
		    nameUp[0] = nameUp[0].toUpper();
		    setterSlot = "set" + nameUp;
		}
		detail += "<a href=\"#" + setterSlot + "\">" + setterSlot + "</a> slot.\n";
	    }
	    if (prop.isEnumType()) {
		detail += "<p>See also <a href=\"#" + type + "\">" + type + "</a>.\n";
	    }

	    propDetails << detail;
	}
	stream << "</ul>" << endl;
    }

    const int enumCount = mo->enumeratorCount();
    if (enumCount) {
	stream << "<hr><h2>Member Type Documentation</h2>" << endl;
	for (uint i = 0; i < enumCount; ++i) {
	    const QMetaEnum enumdata = mo->enumerator(i);
	    stream << "<h3><a name=" << enumdata.name() << "></a>" << enumdata.name() << "</h3>" << endl;
	    stream << "<ul>" << endl;
	    for (uint e = 0; e < enumdata.numKeys(); ++e) {
		stream << "<li>" << enumdata.key(e) << "\t=" << enumdata.value(e) << "</li>" << endl;
	    }
	    stream << "</ul>" << endl;
	}
    }
    if (methodDetails.count()) {
	stream << "<hr><h2>Member Function Documentation</h2>" << endl;
	for (QStringList::Iterator it = methodDetails.begin(); it != methodDetails.end(); ++it) {
	    stream << (*it) << endl;
	}
    }
    if (propDetails.count()) {
	stream << "<hr><h2>Property Documentation</h2>" << endl;
	for (QStringList::Iterator it = propDetails.begin(); it != propDetails.end(); ++it) {
	    stream << (*it) << endl;
	}
    }

    if (typeInfo)
        typeInfo->Release();
    return docu;
}
