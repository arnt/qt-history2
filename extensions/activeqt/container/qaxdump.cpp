/****************************************************************************
**
** Implementation of the qax_generateDocumentation function.
**
** Copyright (C) 2001-2003 Trolltech AS. All rights reserved.
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

static void docuFromName( ITypeInfo *typeInfo, const QString &name, QString &docu )
{
    if ( !typeInfo )
	return;

    MEMBERID memId;
    BSTR names = QStringToBSTR(name);
    typeInfo->GetIDsOfNames( (BSTR*)&names, 1, &memId );
    SysFreeString(names);
    if (memId != DISPID_UNKNOWN ) {
	BSTR docStringBstr, helpFileBstr;
	ulong helpContext;
	HRESULT hres = typeInfo->GetDocumentation( memId, 0, &docStringBstr, &helpContext, &helpFileBstr );
	QString docString = BSTRToQString( docStringBstr );
	QString helpFile = BSTRToQString( helpFileBstr );
	SysFreeString( docStringBstr );
	SysFreeString( helpFileBstr );
	if ( hres == S_OK ) {
	    docu += "<p>";
	    if ( !!docString )
		docu += docString + "\n";
	    if ( !!helpFile )
		docu += QString("For more information, see help context %1 in %2.\n").arg(helpContext).arg(helpFile);
	}
    }
}

static QString toType( const QString &t )
{
    QString type = t;
    int vartype = QVariant::nameToType(type.latin1());
    if ( vartype == QVariant::Invalid )
	type = "int";

    if ( type.startsWith("Q") )
	type = type.mid(1);
    type[0] = type[0].toUpper();
    if ( type == "ValueList<QVariant>" )
	type = "List";
    else if ( type == "Map<QVariant,QVariant>" )
	type = "Map";
    else if ( type == "Uint" )
	type = "UInt";
    
    return "to" + type + "()";
}

QString qax_generateDocumentation(QAxBase *that, QAxBasePrivate *d)
{
    that->metaObject();

    if ( that->isNull() )
	return QString::null;

    ITypeInfo *typeInfo = 0;
    if ( d->dispatch() )
	d->dispatch()->GetTypeInfo( 0, LOCALE_SYSTEM_DEFAULT, &typeInfo );

    QString docu;
    QTextStream stream( &docu, IO_WriteOnly );

    const QMetaObject *mo = that->metaObject();
    QString coClass  = mo->classInfo(mo->indexOfClassInfo("CoClass")).value();

    stream << "<h1 align=center>" << coClass << " Reference</h1>" << endl;
    stream << "<p>The " << coClass << " COM object is a " << that->qObject()->className();
    stream << " with the CLSID " <<  that->control() << ".</p>";
/*
    stream << "<h3>Interfaces</h3>" << endl;
    stream << "<ul>" << endl;
    const char *inter = 0;
    int interCount = 1;  
    while ((inter = mo->classInfo(mo->indexOfClassInfo(QString("Interface %1").arg(interCount))).value())) {
	stream << "<li>" << inter << endl;
	interCount++;
    }
    stream << "</ul>" << endl;

    stream << "<h3>Event Interfaces</h3>" << endl;
    stream << "<ul>" << endl;
    interCount = 1;  
    while ((inter = mo->classInfo(mo->indexOfClassInfo(QString("Event Interface %1").arg(interCount))).value())) {
	stream << "<li>" << inter << endl;
	interCount++;
    }
    stream << "</ul>" << endl;

    QStringList methodDetails, propDetails;

    const int slotCount = mo->slotCount();

    if ( slotCount ) {
	stream << "<h2>Public Slots:</h2>" << endl;
	stream << "<ul>" << endl;

	QMap<QString,QString> slotMap;
	for ( int islot = 0; islot < slotCount; ++islot ) {
	    const QMetaMember slot = mo->slot( islot );
	    const QUMethod *method = slot->method;

	    QString returntype;
	    if ( !method->count ) {
		returntype = "void";
	    } else {
		const QUParameter *param = method->parameters;
		bool returnType = param->inOut == QUParameter::Out;
		if ( !returnType )
		    returntype = "void";
		else if ( QUType::isEqual( &static_QUType_ptr, param->type ) )
		    returntype = (const char*)param->typeExtra;
		else if ( QUType::isEqual( &static_QUType_enum, param->type ) && param->typeExtra )
		    returntype = ((QUEnum*)param->typeExtra)->name;
		else if ( QUType::isEqual( &static_QUType_varptr, param->type ) && param->typeExtra ) {
		    QVariant::Type vartype = (QVariant::Type)*(char*)param->typeExtra;
		    returntype = QVariant::typeToName( vartype );
		} else {
		    returntype = param->type->desc();
		}
	    }
	    slotMap[slot->name] = returntype;
	}
	QMapConstIterator<QString,QString> it;
	for ( it = slotMap.begin(); it != slotMap.end(); ++it ) {
	    QString slot = it.key();
	    int iname = slot.find( '(' );
	    QString name = slot.left( iname );
	    QString params = slot.mid( iname );
	    stream << "<li>" << it.data() << " <a href=\"#" << name << "\"><b>" << name << "</b></a>" << params << ";</li>" << endl;

	    QString detail = "<h3><a name=" + name + "></a>" + it.data() + " " + slot + "<tt> [slot]</tt></h3>\n";
	    docuFromName( typeInfo, name, detail );
	    detail += "<p>Connect a signal to this slot:<pre>\n";
	    detail += "\tQObject::connect( sender, SIGNAL(someSignal" + params + "), object, SLOT(" + name + params + ") );";
	    detail += "</pre>\n";
	    const QMetaData *slotdata = mo->slot( mo->findSlot( slot.latin1(), TRUE ), TRUE );
	    if ( !slotdata )
		continue;
	    const QUMethod *slotmethod = slotdata->method;
	    int pcount = slotmethod->count;
	    QVariant::Type rettype = QVariant::Invalid;
	    bool retval = FALSE;
	    bool outparams = FALSE;
	    bool allVariants = TRUE;
	    for ( int p = 0; p < slotmethod->count; ++p ) {
		if ( !p && slotmethod->parameters->inOut == QUParameter::Out ) {
		    pcount--;
		    retval = TRUE;
		    rettype = QVariant::nameToType( it.data() );
		    if ( rettype == QVariant::Invalid && QUType::isEqual(slotmethod->parameters->type, &static_QUType_enum ) )
			rettype = QVariant::Int;
		}
		if ( p && slotmethod->parameters->inOut & QUParameter::Out )
		    outparams = TRUE;
		if ( allVariants && QUType::isEqual( slotmethod->parameters[p].type, &static_QUType_ptr ) ) {
		    const char *typeExtra = (const char*)slotmethod->parameters[p].typeExtra;
		    allVariants = !p && (!qstrcmp(typeExtra, "IDispatch*") || !qstrcmp(typeExtra, "IUnknown*"));
		}
	    }
	    if ( allVariants ) {
		detail += "<p>Or call the function directly:<pre>\n";
		if ( retval && rettype != QVariant::Invalid ) {
		    if ( outparams ) {
			detail += "\tQValueList<QVariant> params;\n";
			for ( int p = 0; p < pcount; ++p )
			    detail += "\tparams &lt;&lt; var" + QString::number(p+1) + ";\n";
			detail += "\t" + QCString(QVariant::typeToName(rettype)) + " res = ";
			detail += "object->dynamicCall( \"" + name + params + "\", params ).";
			detail += toType(QVariant::typeToName(rettype)) + ";\n";
		    } else {
			detail += "\t" + QCString(QVariant::typeToName(rettype)) + " res = ";
			detail += "object->dynamicCall( \"" + name + params + "\"";
			for ( int p = 0; p < pcount; ++p )
			    detail += ", var" + QString::number(p+1);
			detail += " )." + toType(QVariant::typeToName(rettype)) + ";\n";
		    }
		} else if ( retval ) {
		    detail += "\tQAxObject *res = object->querySubObject( \"" + name + params + "\"";
		    for ( int p = 0; p < pcount; ++p )
			detail += ", var" + QString::number(p+1);
		    detail += " );";
		} else { // no return value
		    if ( outparams ) {
			detail += "\tQValueList<QVariant> params;\n";
			for ( int p = 0; p < pcount; ++p )
			    detail += "\tparams &lt;&lt; var" + QString::number(p+1) + ";\n";
			detail += "\tobject->dynamicCall( \"" + name + params + "\", params );\n";
		    } else {
			detail += "\tobject->dynamicCall( \"" + name + params + "\"";
			for ( int p = 0; p < pcount; ++p )
			    detail += ", var" + QString::number(p+1);
			detail += " );\n";
		    }
		}
		detail += "</pre>\n";
	    } else {
		detail += "<p>This function has parameters of unsupported types and cannot be called directly.";
	    }

	    methodDetails << detail;
	}
	stream << "</ul>" << endl;
    }
    int signalCount = mo->numSignals();
    if ( signalCount ) {
	stream << "<h2>Signals:</h2>" << endl;
	stream << "<ul>" << endl;

	QMap<QString, QString> signalMap;
	for ( int isignal = 0; isignal < signalCount; ++isignal ) {
	    const QMetaData *signal = mo->signal( isignal );
	    signalMap[signal->name] = "void";
	}
	QMapConstIterator<QString,QString> it;
	for ( it = signalMap.begin(); it != signalMap.end(); ++it ) {
	    QString signal = it.key();
	    int iname = signal.find( '(' );
	    QString name = signal.left( iname );
	    QString params = signal.mid( iname );

	    stream << "<li>" << it.data() << " <a href=\"#" << name << "\"><b>" << name << "</b></a>" << params << ";</li>" << endl;
	    QString detail = "<h3><a name=" + name + "></a>" + it.data() + " " + signal + "<tt> [signal]</tt></h3>\n";
	    docuFromName( typeInfo, name, detail );
	    detail += "<p>Connect a slot to this signal:<pre>\n";
	    detail += "\tQObject::connect( object, SIGNAL(" + name + params + "), receiver, SLOT(someSlot" + params + ") );";
	    detail += "</pre>\n";

	    methodDetails << detail;
	}
	stream << "</ul>" << endl;
    }

    const int propCount = mo->numProperties();
    if ( propCount ) {
	stream << "<h2>Properties:</h2>" << endl;
	stream << "<ul>" << endl;

	QMap<QString, QString> propMap;
	for ( int iprop = 0; iprop < propCount; ++iprop ) {
	    const QMetaProperty *prop = mo->property( iprop );
	    propMap[prop->name()] = prop->type();
	}
	QMapConstIterator<QString,QString> it;
	for ( it = propMap.begin(); it != propMap.end(); ++it ) {
	    QString name = it.key();
	    QString type = it.data();

	    stream << "<li>" << type << " <a href=\"#" << name << "\"><b>" << name << "</b></a>;</li>" << endl;
	    QString detail = "<h3><a name=" + name + "></a>" + type + " " + name + "</h3>\n";
	    docuFromName( typeInfo, name, detail );
	    QVariant::Type vartype = QVariant::nameToType( type );
	    const QMetaProperty *prop = mo->property( mo->findProperty( name.latin1() ) );
	    if ( !prop )
		continue;

	    bool castToType = FALSE;
	    if ( vartype == QVariant::Invalid && prop->isEnumType() ) {
		vartype = QVariant::Int;
		castToType = TRUE;
	    }
	    if ( vartype != QVariant::Invalid ) {
		detail += "<p>Read this property's value using QObject::property:<pre>\n";
		detail += "\t" + type + " val = ";
		if ( castToType ) {
		    detail += "(" + type + ")";
		}
		detail += "object->property( \"" + name + "\" )." + toType(type) + ";\n";
		detail += "</pre>\n";
	    } else if ( type == "IDispatch*" || type == "IUnkonwn*" ) {
		detail += "<p>Get the subobject using querySubObject:<pre>\n";
		detail += "\tQAxObject *" + name + " = object->querySubObject( \"" + name + "\" );\n";
		detail += "</pre>\n";
	    } else {
		detail += "<p>This property is of an unsupported type.\n";
	    }
	    if ( prop->writable() ) {
		detail += "Set this property' value using QObject::setProperty:<pre>\n";
		detail += "\t" + type + " newValue = ...\n";
		detail += "\tobject->setProperty( \"" + name + "\", newValue );\n";
		detail += "</pre>\n";
		detail += "Or using the ";
		QString setterSlot;
		if ( name[0].upper() == name[0] ) {
		    setterSlot = "Set" + name;
		} else {
		    QString nameUp = name;
		    nameUp[0] = nameUp[0].upper();
		    setterSlot = "set" + nameUp;
		}
		detail += "<a href=\"#" + setterSlot + "\">" + setterSlot + "</a> slot.\n";
	    }
	    if ( prop->isEnumType() ) {
		QCString enumName = prop->enumData->name;
		detail += "<p>See also <a href=\"#" + enumName + "\">" + enumName + "</a>.\n";
	    }

	    propDetails << detail;
	}
	stream << "</ul>" << endl;
    }
    QStrList enumerators = mo->enumeratorNames();
    if ( enumerators.count() ) {
	stream << "<hr><h2>Member Type Documentation</h2>" << endl;
	for ( uint i = 0; i < enumerators.count(); ++i ) {
	    const QMetaEnum *enumdata = mo->enumerator( enumerators.at(i) );
	    stream << "<h3><a name=" << enumdata->name << "></a>" << enumdata->name << "</h3>" << endl;
	    stream << "<ul>" << endl;
	    for ( uint e = 0; e < enumdata->count; ++e ) {
		const QMetaEnum::Item *item = enumdata->items+e;
		stream << "<li>" << item->key << "\t=" << item->value << "</li>" << endl;
	    }
	    stream << "</ul>" << endl;
	}
    }
    if ( methodDetails.count() ) {
	stream << "<hr><h2>Member Function Documentation</h2>" << endl;
	for ( QStringList::Iterator it = methodDetails.begin(); it != methodDetails.end(); ++it ) {
	    stream << (*it) << endl;
	}
    }
    if ( propDetails.count() ) {
	stream << "<hr><h2>Property Documentation</h2>" << endl;
	for ( QStringList::Iterator it = propDetails.begin(); it != propDetails.end(); ++it ) {
	    stream << (*it) << endl;
	}
    }
*/
    if ( typeInfo ) typeInfo->Release();
    return docu;
}
