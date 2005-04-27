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

#include "qaxbase.h"
#include <qmetaobject.h>
#include <quuid.h>
#include <qt_windows.h>
#include <qtextstream.h>

#include "../shared/qaxtypes.h"

QString qax_docuFromName(ITypeInfo *typeInfo, const QString &name)
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
        QString docString = QString::fromUtf16((const ushort *)docStringBstr);
        QString helpFile = QString::fromUtf16((const ushort *)helpFileBstr);
        SysFreeString(docStringBstr);
        SysFreeString(helpFileBstr);
        if (hres == S_OK) {
            if (!docString.isEmpty())
                docu += docString + "\n";
            if (!helpFile.isEmpty())
                docu += QString("For more information, see help context %1 in %2.").arg((uint)helpContext).arg(helpFile);
        }
    }

    return docu;
}

static inline QString docuFromName(ITypeInfo *typeInfo, const QString &name)
{
    return QString("<p>") + qax_docuFromName(typeInfo, name) + "\n";
}

static QByteArray namedPrototype(const QList<QByteArray> &parameterTypes, const QList<QByteArray> &parameterNames, int numDefArgs = 0)
{
    QByteArray prototype("(");
    for (int p = 0; p < parameterTypes.count(); ++p) {
        QByteArray type(parameterTypes.at(p));
        prototype += type;

        if (p < parameterNames.count())
            prototype += " " + parameterNames.at(p);
         
        if (numDefArgs >= parameterTypes.count() - p)
            prototype += " = 0";
        if (p < parameterTypes.count() - 1)
            prototype += ", ";
    }
    prototype += ")";

    return prototype;
}

static QByteArray toType(const QByteArray &t)
{
    QByteArray type = t;
    int vartype = QVariant::nameToType(type);
    if (vartype == QVariant::Invalid)
        type = "int";
    
    if (type.at(0) == 'Q')
        type = type.mid(1);
    type[0] = toupper(type.at(0));
    if (type == "VariantList")
        type = "List";
    else if (type == "Map<QVariant,QVariant>")
        type = "Map";
    else if (type == "Uint")
        type = "UInt";
    
    return "to" + type + "()";
}

QString qax_generateDocumentation(QAxBase *that)
{
    that->metaObject();

    if (that->isNull())
	return QString();

    ITypeInfo *typeInfo = 0;
    IDispatch *dispatch = 0;
    that->queryInterface(IID_IDispatch, (void**)&dispatch);
    if (dispatch)
	dispatch->GetTypeInfo(0, LOCALE_SYSTEM_DEFAULT, &typeInfo);

    QString docu;
    QTextStream stream(&docu, QIODevice::WriteOnly);

    const QMetaObject *mo = that->metaObject();
    QString coClass  = mo->classInfo(mo->indexOfClassInfo("CoClass")).value();

    stream << "<h1 align=center>" << coClass << " Reference</h1>" << endl;
    stream << "<p>The " << coClass << " COM object is a " << that->qObject()->metaObject()->className();
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

    QList<QString> methodDetails, propDetails;

    const int slotCount = mo->memberCount();
    if (slotCount) {
	stream << "<h2>Public Slots:</h2>" << endl;
	stream << "<ul>" << endl;

        int defArgCount = 0;
	for (int islot = mo->memberOffset(); islot < slotCount; ++islot) {
	    const QMetaMember slot = mo->member(islot);
            if (slot.memberType() != QMetaMember::Slot)
                continue;

            if (slot.attributes() & QMetaMember::Cloned) {
                ++defArgCount;
                continue;
            }

	    QByteArray returntype(slot.typeName());
            if (returntype.isEmpty())
                returntype = "void";
            QByteArray prototype = namedPrototype(slot.parameterTypes(), slot.parameterNames(), defArgCount);
            QByteArray signature = slot.signature();
	    QByteArray name = signature.left(signature.indexOf('('));
	    stream << "<li>" << returntype << " <a href=\"#" << name << "\"><b>" << name << "</b></a>" << prototype << ";</li>" << endl;
            
            prototype = namedPrototype(slot.parameterTypes(), slot.parameterNames());
	    QString detail = "<h3><a name=" + name + "></a>" + returntype + " " + name + " " + prototype + "<tt> [slot]</tt></h3>\n";
            prototype = namedPrototype(slot.parameterTypes(), QList<QByteArray>());
	    detail += docuFromName(typeInfo, name);
	    detail += "<p>Connect a signal to this slot:<pre>\n";
	    detail += "\tQObject::connect(sender, SIGNAL(someSignal" + prototype + "), object, SLOT(" + name + prototype + "));";
	    detail += "</pre>\n";

            if (1) {
                detail += "<p>Or call the function directly:<pre>\n";

                bool hasParams = slot.parameterTypes().count() != 0;
                if (hasParams)
                    detail += "\tQVariantList params = ...\n";
                detail += "\t";
                QByteArray functionToCall = "dynamicCall";
                if (returntype == "IDispatch*" || returntype == "IUnknown*") {
                    functionToCall = "querySubObject";
                    returntype = "QAxObject *";
                }
                if (returntype != "void")
                    detail += returntype + " result = ";
                detail += "object->" + functionToCall + "(\"" + name + prototype + "\"";
                if (hasParams)
                    detail += ", params";
                detail += ")";
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
    int signalCount = mo->memberCount();
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

	for (int isignal = mo->memberOffset(); isignal < signalCount; ++isignal) {
	    const QMetaMember signal(mo->member(isignal));
            if (signal.memberType() != QMetaMember::Signal)
                continue;

            QByteArray prototype = namedPrototype(signal.parameterTypes(), signal.parameterNames());
	    QByteArray signature = signal.signature();
	    QByteArray name = signature.left(signature.indexOf('('));
	    stream << "<li>void <a href=\"#" << name << "\"><b>" << name << "</b></a>" << prototype << ";</li>" << endl;

            QString detail = "<h3><a name=" + name + "></a>void " + name + " " + prototype + "<tt> [signal]</tt></h3>\n";
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
            prototype = namedPrototype(signal.parameterTypes(), QList<QByteArray>());
	    detail += "<p>Connect a slot to this signal:<pre>\n";
	    detail += "\tQObject::connect(object, SIGNAL(" + name + prototype + "), receiver, SLOT(someSlot" + prototype + "));";
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
        if (dispatch)
	    dispatch->GetTypeInfo(0, LOCALE_SYSTEM_DEFAULT, &typeInfo);
	stream << "<h2>Properties:</h2>" << endl;
	stream << "<ul>" << endl;

	for (int iprop = 0; iprop < propCount; ++iprop) {
	    const QMetaProperty prop = mo->property(iprop);
	    QByteArray name(prop.name());
	    QByteArray type(prop.typeName());

	    stream << "<li>" << type << " <a href=\"#" << name << "\"><b>" << name << "</b></a>;</li>" << endl;
	    QString detail = "<h3><a name=" + name + "></a>" + type + " " + name + "</h3>\n";
	    detail += docuFromName(typeInfo, name);
	    QVariant::Type vartype = QVariant::nameToType(type);
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
		QByteArray setterSlot;
                if (isupper(name.at(0))) {
		    setterSlot = "Set" + name;
		} else {
		    QByteArray nameUp = name;
		    nameUp[0] = toupper(nameUp.at(0));
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
	for (int i = 0; i < enumCount; ++i) {
	    const QMetaEnum enumdata = mo->enumerator(i);
	    stream << "<h3><a name=" << enumdata.name() << "></a>" << enumdata.name() << "</h3>" << endl;
	    stream << "<ul>" << endl;
	    for (int e = 0; e < enumdata.keyCount(); ++e) {
		stream << "<li>" << enumdata.key(e) << "\t=" << enumdata.value(e) << "</li>" << endl;
	    }
	    stream << "</ul>" << endl;
	}
    }
    if (methodDetails.count()) {
	stream << "<hr><h2>Member Function Documentation</h2>" << endl;
	for (int i = 0; i < methodDetails.count(); ++i)
	    stream << methodDetails.at(i) << endl;
    }
    if (propDetails.count()) {
	stream << "<hr><h2>Property Documentation</h2>" << endl;
	for (int i = 0; i < propDetails.count(); ++i)
	    stream << propDetails.at(i) << endl;
    }

    if (typeInfo)
        typeInfo->Release();
    if (dispatch)
        dispatch->Release();
    return docu;
}
