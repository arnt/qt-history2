#include "activeqt.h"

#include <qsettings.h>
#include <qapplication.h>
#include <qmetaobject.h>
#include <private/qucom_p.h>
#include <private/qucomextra_p.h>
#include <qintdict.h>

#define PropDesignable	1001
#define PropScriptable	1002
#define PropStored	1004

static QIntDict<QMetaData>* slotlist = 0;
static QIntDict<QMetaData>* signallist = 0;
static QIntDict<QMetaProperty>* proplist = 0;

static inline QString vartypeToQt( VARTYPE vt )
{
    QString str;
    switch ( vt ) {
    case VT_EMPTY:
	// str = "[Empty]";
	break;
    case VT_NULL:
	// str = "[Null]";
	break;
    case VT_I2:
    case VT_I4:
	str = "int";
	break;
    case VT_R4:
    case VT_R8:
	str = "double";
	break;
    case VT_CY:
	str = "long long"; // ### 64bit struct CY { ulong lo, long hi };
	break;
    case VT_DATE:
	str = "QDateTime";
	break;
    case VT_BSTR:
	str = "QString";
	break;
    case VT_DISPATCH:
	str = "IDispatch*";
	break;
    case VT_ERROR:
	str = "long";
	break;
    case VT_BOOL:
	str = "bool";
	break;
    case VT_VARIANT:
	str = "QVariant";
	break;
    case VT_DECIMAL:
	// str = "[DECIMAL]";
	break;
    case VT_RECORD:
	// str = "[Usertype]";
	break;
    case VT_UNKNOWN:
	str = "IUnknown*";
	break;
    case VT_I1:
	str = "char";
	break;
    case VT_UI1:
	str = "unsigned char";
	break;
    case VT_UI2:
	str = "unsigned short";
	break;
    case VT_UI4:
	str = "unsigned int";
	break;
    case VT_INT:
	str = "int";
	break;
    case VT_UINT:
	str = "unsigned int";
	break;
    case VT_VOID:
	str = "void";
	break;
    case VT_HRESULT:
	str = "long";
	break;

    case VT_PTR:
	// str = "[Pointer]";
	break;
    case VT_SAFEARRAY:
	// str = "VT_ARRAY";
	break;
    case VT_CARRAY:
	// str = "[C array]";
	break;
    case VT_USERDEFINED:
	str = "USERDEFINED";
	break;
    case VT_LPSTR:
	str = "const char*";
	break;
    case VT_LPWSTR:
	str = "const unsigned short*";
	break;

    case VT_FILETIME:
	// str = "[FILETIME]";
	break;
    case VT_BLOB:
	// str = "[Blob]";
	break;
    case VT_STREAM:
	// str = "[Stream]";
	break;
    case VT_STORAGE:
	// str = "[Storage]";
	break;
    case VT_STREAMED_OBJECT:
	// str = "[Streamed object]";
	break;
    case VT_STORED_OBJECT:
	// str = "[Stored object]";
	break;
    case VT_BLOB_OBJECT:
	// str = "[Blob object]";
	break;
    case VT_CF:
	// str = "[Clipboard]";
	break;
    case VT_CLSID:
	// str = "GUID";
	break;
    case VT_VECTOR:
	// str = "[Vector]";
	break;

    case VT_ARRAY:
	// str = "SAFEARRAY*";
	break;
    case VT_RESERVED:
	// str = "[Reserved]";
	break;

    default:
	// str = "[Unknown]";
	break;
    }

    if ( vt & VT_BYREF )
	str += "*";

    return str;
}

static inline QString typedescToQString( TYPEDESC typedesc )
{
    QString ptype;

    VARTYPE vt = typedesc.vt;
    if ( vt == VT_PTR ) {
	vt = typedesc.lptdesc->vt;
	ptype = vartypeToQt( vt );
	if ( !!ptype ) 
	    ptype += "*";
    } else if ( vt == VT_SAFEARRAY ) {
	vt = typedesc.lpadesc->tdescElem.vt;
	ptype = vartypeToQt( vt );
	if ( !!ptype ) 
	    ptype = ptype + "[" + QString::number( typedesc.lpadesc->cDims ) + "]";
    } else {
	ptype = vartypeToQt( vt );
    }
    if ( ptype.isEmpty() )
	ptype = "UNSUPPORTED";
    else if ( ptype == "USERDEFINED" ) // most USERDEFINED types are long or ints, or interfaces
	ptype = "int";
    else if ( ptype == "USERDEFINED*" )
	ptype = "IUnknown*";
	
    return ptype;
}

QString BSTRToQString( BSTR bstr )
{
    QString str;
    if ( !bstr )
	return str;

    int len = wcslen( bstr );
    str.setUnicode( (QChar*)bstr, len );
    return str;
}

static inline QString constRefify( const QString& type )
{
    QString crtype;

    if ( type == "QString" )
	crtype = "const QString&";
    else if ( type == "QDateTime" )
	crtype = "const QDateTime&";
    else if ( type == "QVariant" )
	crtype = "const QVariant&";
    else 
	crtype = type;

    return crtype;
}

static inline void QStringToQUType( const QString& type, QUParameter *param )
{
    param->typeExtra = 0;
    if ( type == "int" || type == "long" ) {
	param->type = &static_QUType_int;
    } else if ( type == "bool" ) {
	param->type = &static_QUType_bool;
    } else if ( type == "QString" || type == "const QString&" ) {
	param->type = &static_QUType_QString;
    } else if ( type == "double" ) {
	param->type = &static_QUType_double;
    } else if ( type == "QVariant" || type == "const QVariant&" ) {
	param->type = &static_QUType_QVariant;
    } else if ( type == "IUnknown*" ) {
	param->type = &static_QUType_iface;
	param->typeExtra = "QUnknownInterface";
    } else if ( type == "IDispatch*" ) {
	param->type = &static_QUType_idisp;
	param->typeExtra = "QDispatchInterface";
    } else {
	param->type = &static_QUType_ptr;
	QString ptype = type;
	if ( ptype.right(1) == "*" )
	    ptype.remove( ptype.length()-1, 1 );
	param->typeExtra = new char[ ptype.length() + 1 ];
	param->typeExtra = qstrcpy( (char*)param->typeExtra, ptype );
    }
}


QActiveX::QActiveX()
    : m_pTypeInfo( 0 )
{
}

QActiveX::~QActiveX()
{
    if ( m_pTypeInfo ) {
	m_pTypeInfo->setObject( 0 );
	m_pTypeInfo->Release();
	m_pTypeInfo = 0;
    }
    if ( object ) {
	delete object;
    }
    if ( m_pWidget ) {
	delete m_pWidget;
    }
}

LRESULT QActiveX::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    m_pWidget = new QVBox( 0, 0, Qt::WStyle_Customize );
    ::SetWindowLong( m_pWidget->winId(), GWL_STYLE, WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS );
#if 0
    QUnknownInterface *unknown = ucm_instantiate();
    QInterfacePtr<QWidgetFactoryInterface> iface = 0;
    if ( unknown->queryInterface( IID_QWidgetFactory, (QUnknownInterface**)&iface ) == QS_OK ) {
	QString key = iface->featureList()[0];
	object = iface->create( key );
	iface->release();
    }
    unknown->release();
#else
    object = axmain( m_pWidget );
#endif

    if ( !slotlist ) {
	slotlist = new QIntDict<QMetaData>;
	signallist = new QIntDict<QMetaData>;
	proplist = new QIntDict<QMetaProperty>;

	CComPtr<ITypeInfo> info;
	GetTypeInfo( 0, LOCALE_SYSTEM_DEFAULT, &info );
	// read type information
	while ( info ) {
	    ushort nFuncs = 0;
	    ushort nVars = 0;
	    ushort nImpl = 0;
	    // get information about type
	    TYPEATTR *typeattr;
	    info->GetTypeAttr( &typeattr );
	    if ( typeattr ) {
		if ( ( typeattr->typekind != TKIND_DISPATCH && typeattr->typekind != TKIND_INTERFACE ) ||
		     ( typeattr->guid == IID_IDispatch || typeattr->guid == IID_IUnknown ) ) {
		    info->ReleaseTypeAttr( typeattr );
		    break;
		}
		// get number of functions, variables, and implemented interfaces
		nFuncs = typeattr->cFuncs;
		nVars = typeattr->cVars;
		nImpl = typeattr->cImplTypes;

		info->ReleaseTypeAttr( typeattr );
	    }

	    // get information about all functions
	    for ( ushort fd = 0; fd < nFuncs ; ++fd ) {
		FUNCDESC *funcdesc;
		info->GetFuncDesc( fd, &funcdesc );
		if ( !funcdesc )
		    break;

		// get function prototype
		QString function;
		QString prototype;
		QStringList parameters;
		QStringList paramTypes;

		// get return value
		TYPEDESC typedesc = funcdesc->elemdescFunc.tdesc;
		QString returnType = typedescToQString( typedesc );
		if ( funcdesc->invkind == INVOKE_FUNC && returnType != "void" ) {
		    parameters << "return";
		    paramTypes << returnType;
		}

		BSTR bstrNames[256];
		UINT maxNames = 255;
		UINT maxNamesOut;
		info->GetNames( funcdesc->memid, (BSTR*)&bstrNames, maxNames, &maxNamesOut );
		for ( int p = 0; p < (int)maxNamesOut; ++ p ) {
		    QString paramName = BSTRToQString( bstrNames[p] );
		    SysFreeString( bstrNames[p] );

		    // function name
		    if ( !p ) {
			function = paramName;
			prototype = function + "(";
			continue;
		    }

		    // parameter
		    bool optional = p > funcdesc->cParams - funcdesc->cParamsOpt;
		    QString ptype;
		    TYPEDESC tdesc = funcdesc->lprgelemdescParam[p - (funcdesc->invkind == INVOKE_FUNC) ? 1 : 0 ].tdesc;
		    ptype = typedescToQString( tdesc );
		    if ( funcdesc->invkind == INVOKE_FUNC )
			ptype = constRefify( ptype );

		    prototype += ptype;
		    if ( optional )
			ptype += "=0";
		    paramTypes << ptype;
		    parameters << paramName;
		    if ( p < funcdesc->cParams )
			prototype += ",";
		}
		if ( !!prototype )
		    prototype += ")";

		bool bindable = FALSE;
		if ( funcdesc->wFuncFlags & FUNCFLAG_FBINDABLE )
		    bindable = TRUE;
		if ( funcdesc->wFuncFlags & FUNCFLAG_FREQUESTEDIT )
		    bindable = TRUE;

		// get type of function
		if ( !(funcdesc->wFuncFlags & FUNCFLAG_FHIDDEN) ) switch( funcdesc->invkind ) {
		case INVOKE_PROPERTYGET: // property
		case INVOKE_PROPERTYPUT:
		    {
			if ( funcdesc->cParams > 1 ) {
			    qWarning( "%s: Too many parameters in property", function.latin1() );
			    break;
			}
			QMetaProperty *prop = proplist->find( funcdesc->memid );
			if ( !prop ) {
			    if ( bindable ) {
#if 0
				if ( !eventSink )
				    that->eventSink = new QAxEventSink( that );
#endif
				// generate changed signal
				QString signalName = function + "Changed";
				QString signalParam = constRefify( paramTypes[0] );
				QString signalProto = signalName + "(" + signalParam + ")";
				QString paramName = "value";
				if ( !signallist->find( funcdesc->memid ) ) {
				    QUMethod *signal = new QUMethod;
				    signal->name = new char[signalName.length()+1];
				    signal->name = qstrcpy( (char*)signal->name, signalName );
				    signal->count = 1;
				    QUParameter *param = new QUParameter;
				    param->name = new char[paramName.length()+1];
				    param->name = qstrcpy( (char*)param->name, paramName );
				    param->inOut = QUParameter::In;
				    QStringToQUType( signalParam, param );
				    signal->parameters = param;

				    QMetaData *data = new QMetaData;
				    data->name = new char[prototype.length()+1];
				    data->name = qstrcpy( (char*)data->name, prototype );
				    data->access = QMetaData::Public;
				    data->method = signal;
				    signallist->insert( funcdesc->memid, data );
#if 0
				    eventSink->addProperty( funcdesc->memid, function, signalProto );
#endif
				}
			    }

			    prop = new QMetaProperty;
			    proplist->insert( funcdesc->memid, prop );
			    prop->meta = 0;
			    prop->_id = -1;
			    prop->enumData = 0;
			    prop->flags = PropStored;
			    if ( !(funcdesc->wFuncFlags & FUNCFLAG_FNONBROWSABLE) )
				prop->flags |= PropDesignable;
			    if ( !(funcdesc->wFuncFlags & FUNCFLAG_FRESTRICTED) )
				prop->flags |= PropScriptable;

			    QString ptype = paramTypes[0];
			    if ( ptype.isEmpty() )
				ptype = returnType;
			    if ( ptype != "void" ) {
				prop->t = new char[ptype.length()+1];
				prop->t = qstrcpy( (char*)prop->t, ptype );
			    } else {
				prop->t = 0;
			    }
			    prop->n = new char[function.length()+1];
			    prop->n = qstrcpy( (char*)prop->n, function );
			} else if ( !prop->t ) {
			    QString ptype = paramTypes[0];
			    if ( ptype.isEmpty() )
				ptype = returnType;
			    if ( paramTypes.isEmpty() )
				paramTypes.append( ptype );
			    else
				paramTypes[0] = ptype;
			    prop->t = new char[ptype.length()+1];
			    prop->t = qstrcpy( (char*)prop->t, ptype );
			}
			if ( funcdesc->invkind == INVOKE_PROPERTYGET ) {
			    prop->flags |= QMetaProperty::Readable;
			} else {
			    prop->flags |= QMetaProperty::Writable;
			}
			if ( !prop->t )
			    break;
			// fall through to generate put function as slot
		    }

		case INVOKE_FUNC: // method
		    {
			if ( funcdesc->invkind != INVOKE_FUNC ) {
			    function = "set" + function;
			    if ( prototype.right( 2 ) == "()" ) {
				QString ptype = paramTypes[0];
				if ( ptype.isEmpty() )
				    ptype = returnType;
				prototype = function + "(" + constRefify(ptype) + ")";
			    } else {
				prototype = "set" + prototype;
			    }
			    if ( slotlist->find( funcdesc->memid ) )
				break;
			}
			bool defargs;
			QString defprototype = prototype;
			do {
			    defargs = FALSE;
			    QUMethod *slot = new QUMethod;
			    slot->name = new char[function.length()+1];
			    slot->name = qstrcpy( (char*)slot->name, function );
			    slot->count = parameters.count();
			    QUParameter *params = slot->count ? new QUParameter[slot->count] : 0;
			    int offset = parameters[0] == "return" ? 1 : 0;
			    for ( int p = 0; p< slot->count; ++p ) {
				QString paramName = parameters[p];
				QString paramType = paramTypes[p];
				if ( paramType.right( 2 ) == "=0" ) {
				    paramType.truncate( paramType.length()-2 );
				    paramTypes[p] = paramType;
				    defargs = TRUE;
				    slot->count = p;
				    prototype = function + "(";
				    for ( int pp = offset; pp < p; ++pp ) {
					prototype += paramTypes[pp];
					if ( pp < p-1 )
					    prototype += ",";
				    }
				    prototype += ")";
				    break;
				}
				params[p].name = new char[paramName.length()+1];
				params[p].name = qstrcpy( (char*)params[p].name, paramName );
				params[p].inOut = 0;
				if ( !p && paramName == "return" ) {
				    params[p].inOut = QUParameter::Out;
				} else if ( funcdesc->lprgelemdescParam + p - offset ) {
				    ushort inout = funcdesc->lprgelemdescParam[p-offset].paramdesc.wParamFlags;
				    if ( inout & PARAMFLAG_FIN )
					params[p].inOut |= QUParameter::In;
				    if ( inout & PARAMFLAG_FOUT )
					params[p].inOut |= QUParameter::Out;
				}

				QStringToQUType( paramType, params + p );
			    }

			    slot->parameters = params;

			    QMetaData *data = new QMetaData;
			    data->name = new char[prototype.length()+1];
			    data->name = qstrcpy( (char*)data->name, prototype );
			    data->access = QMetaData::Public;
			    data->method = slot;

			    slotlist->insert( funcdesc->memid, data );
			    prototype = defprototype;
			} while ( defargs );
		    }
		    break;

		default:
		    break;
		}

#if 0 // documentation in metaobject would be cool?
		// get function documentation
		BSTR bstrDocu;
		info->GetDocumentation( funcdesc->memid, 0, &bstrDocu, 0, 0 );
		QString strDocu = BSTRToQString( bstrDocu );
		if ( !!strDocu )
		    desc += "[" + strDocu + "]";
		desc += "\n";
		SysFreeString( bstrDocu );
#endif

		info->ReleaseFuncDesc( funcdesc );
	    }
	    
	    // get information about all variables
	    for ( ushort vd = 0; vd < nVars; ++vd ) {
		VARDESC *vardesc;
		info->GetVarDesc( vd, &vardesc );
		if ( !vardesc )
		    break;

		// no use if it's not a dispatched variable
		if ( vardesc->varkind != VAR_DISPATCH ) {
		    info->ReleaseVarDesc( vardesc );
		    continue;
		}

		// get variable type
		TYPEDESC typedesc = vardesc->elemdescVar.tdesc;
		QString variableType = typedescToQString( typedesc );

		// get variable name
		QString variableName;

		BSTR bstrNames[256];
		UINT maxNames = 255;
		UINT maxNamesOut;
		info->GetNames( vardesc->memid, (BSTR*)&bstrNames, maxNames, &maxNamesOut );
		for ( int v = 0; v < (int)maxNamesOut; ++v ) {
		    QString varName = BSTRToQString( bstrNames[v] );
		    SysFreeString( bstrNames[v] );

		    if ( !v ) {
			variableName = varName;
			continue;
		    }
		}

		bool bindable = FALSE;
		if ( vardesc->wVarFlags & VARFLAG_FBINDABLE )
		    bindable = TRUE;
		if ( vardesc->wVarFlags & VARFLAG_FREQUESTEDIT )
		    bindable = TRUE;

		if ( !(vardesc->wVarFlags & VARFLAG_FHIDDEN) ) {
		    // generate meta property
		    QMetaProperty *prop = proplist->find( vardesc->memid );
		    if ( !prop ) {
			if ( bindable ) {
#if 0
			    if ( !eventSink )
				that->eventSink = new QAxEventSink( that );
#endif
			    // generate changed signal
			    QString signalName = variableName + "Changed";
			    QString signalParam = constRefify( variableType );
			    QString signalProto = signalName + "(" + signalParam + ")";
			    QString paramName = "value";
			    if ( !signallist->find( vardesc->memid ) ) {
				QUMethod *signal = new QUMethod;
				signal->name = new char[signalName.length()+1];
				signal->name = qstrcpy( (char*)signal->name, signalName );
				signal->count = 1;
				QUParameter *param = new QUParameter;
				param->name = new char[paramName.length()+1];
				param->name = qstrcpy( (char*)param->name, paramName );
				param->inOut = QUParameter::In;
				QStringToQUType( signalParam, param );
				signal->parameters = param;

				QMetaData *data = new QMetaData;
				data->name = new char[signalProto.length()+1];
				data->name = qstrcpy( (char*)data->name, signalProto );
				data->access = QMetaData::Public;
				data->method = signal;

				signallist->insert( vardesc->memid, data );
#if 0
				eventSink->addProperty( vardesc->memid, variableName, signalProto );
#endif
			    }
			}

			prop = new QMetaProperty;
			proplist->insert( vardesc->memid, prop );
			prop->meta = 0;
			prop->_id = -1;
			prop->enumData = 0;
			prop->flags = QMetaProperty::Readable | PropStored;
			if ( !(vardesc->wVarFlags & VARFLAG_FREADONLY) )
			    prop->flags |= QMetaProperty::Writable;;
			if ( !(vardesc->wVarFlags & VARFLAG_FNONBROWSABLE) )
			    prop->flags |= PropDesignable;
			if ( !(vardesc->wVarFlags & VARFLAG_FRESTRICTED) )
			    prop->flags |= PropScriptable;

			prop->t = new char[variableType.length()+1];
			prop->t = qstrcpy( (char*)prop->t, variableType );
			prop->n = new char[variableName.length()+1];
			prop->n = qstrcpy( (char*)prop->n, variableName );
		    }

		    // generate a set slot
		    if ( !(vardesc->wVarFlags & VARFLAG_FREADONLY) ) {
			variableType = constRefify( variableType );

			QString function = "set" + variableName;
			QString prototype = function + "(" + variableType + ")";

			if ( !slotlist->find( vardesc->memid ) ) {
			    QUMethod *slot = new QUMethod;
			    slot->name = new char[ function.length() + 1 ];
			    slot->name = qstrcpy( (char*)slot->name, function );
			    slot->count = 1;
			    QUParameter *params = new QUParameter;
			    params->inOut = QUParameter::In;
			    params->name = new char[ variableName.length() + 1 ];
			    params->name = qstrcpy( (char*)params->name, variableName );

			    QStringToQUType( variableType, params );

			    slot->parameters = params;

			    QMetaData *data = new QMetaData;
			    data->name = new char[prototype.length()+1];
			    data->name = qstrcpy( (char*)data->name, prototype );
			    data->access = QMetaData::Public;
			    data->method = slot;

			    slotlist->insert( vardesc->memid, data );
			}
		    }

#if 0 // documentation in metaobject would be cool?
		    // get function documentation
		    BSTR bstrDocu;
		    info->GetDocumentation( vardesc->memid, 0, &bstrDocu, 0, 0 );
		    QString strDocu = BSTRToQString( bstrDocu );
		    if ( !!strDocu )
			desc += "[" + strDocu + "]";
		    desc += "\n";
		    SysFreeString( bstrDocu );
#endif
		}
		info->ReleaseVarDesc( vardesc );
	    }

	    if ( !nImpl )
		break;

	    // go up one base class
	    HREFTYPE pRefType;
	    info->GetRefTypeOfImplType( 0, &pRefType );
	    CComPtr<ITypeInfo> baseInfo;
	    info->GetRefTypeInfo( pRefType, &baseInfo );
	    if ( info == baseInfo ) // IUnknown inherits IUnknown ???
		break;
	    info = baseInfo;
	}

	CComPtr<IConnectionPointContainer> cpoints;
	QueryInterface( IID_IConnectionPointContainer, (void**)&cpoints );
	if ( cpoints ) {
	    CComPtr<IProvideClassInfo> classinfo;
	    cpoints->QueryInterface( IID_IProvideClassInfo, (void**)&classinfo );

	    CComPtr<IEnumConnectionPoints> epoints;
	    cpoints->EnumConnectionPoints( &epoints );
	    if ( epoints ) {
		ULONG c = 1;
		epoints->Reset();
		do {
		    CComPtr<IConnectionPoint> cpoint;
		    epoints->Next( c, &cpoint, &c );
		    if ( !c )
			break;

		    IID iid;
		    cpoint->GetConnectionInterface( &iid );
#if 0
		    if ( !eventSink )
			that->eventSink = new QAxEventSink( that );
		    eventSink->addConnection( cpoint, iid );
#endif

		    if ( classinfo ) {
			CComPtr<ITypeInfo> info;
			CComPtr<ITypeInfo> eventinfo;
			classinfo->GetClassInfo( &info );
			if ( info ) { // this is the type info of the component, not the event interface
			    // get information about type
			    TYPEATTR *typeattr;
			    info->GetTypeAttr( &typeattr );
			    if ( typeattr ) {
				// test if one of the interfaces implemented is the one we're looking for
				for ( int impl = 0; impl < typeattr->cImplTypes; ++impl ) {
				    // get the ITypeInfo for the interface
				    HREFTYPE reftype;
				    info->GetRefTypeOfImplType( impl, &reftype );
				    CComPtr<ITypeInfo> eventtype;
				    info->GetRefTypeInfo( reftype, &eventtype );
				    if ( eventtype ) {
					TYPEATTR *eventattr;
					eventtype->GetTypeAttr( &eventattr );
					// this is it
					if ( eventattr && eventattr->guid == iid ) {
					    eventinfo = eventtype;
					    break;
					}
					eventtype->ReleaseTypeAttr( eventattr );
				    }
				}
				info->ReleaseTypeAttr( typeattr );

				// what about other event interfaces?
				if ( eventinfo ) {
				    TYPEATTR *eventattr;
				    eventinfo->GetTypeAttr( &eventattr );
				    // Number of functions
				    ushort nEvents = eventattr->cFuncs;

				    // get information about all event functions
				    for ( UINT fd = 0; fd < nEvents; ++fd ) {
					FUNCDESC *funcdesc;
					eventinfo->GetFuncDesc( fd, &funcdesc );
					if ( !funcdesc )
					    break;
					if ( funcdesc->invkind != INVOKE_FUNC ||
					     funcdesc->funckind != FUNC_DISPATCH ) {
					    info->ReleaseFuncDesc( funcdesc );
					    continue;
					}

					// get return value
					TYPEDESC typedesc = funcdesc->elemdescFunc.tdesc;
					QString returnType = typedescToQString( typedesc );

					// get event function prototype
					QString function;
					QString prototype;
					QStringList parameters;
					QStringList paramTypes;

					BSTR bstrNames[256];
					UINT maxNames = 255;
					UINT maxNamesOut;
					eventinfo->GetNames( funcdesc->memid, (BSTR*)&bstrNames, maxNames, &maxNamesOut );
					for ( int p = 0; p < (int)maxNamesOut; ++ p ) {
					    QString paramName = BSTRToQString( bstrNames[p] );
					    SysFreeString( bstrNames[p] );

					    // function name
					    if ( !p ) {
						function = paramName;
						prototype = function + "(";
						continue;
					    }

					    // parameter
					    bool optional = p > funcdesc->cParams - funcdesc->cParamsOpt;
					    
					    TYPEDESC tdesc = funcdesc->lprgelemdescParam[p - (funcdesc->invkind == INVOKE_FUNC) ? 1 : 0 ].tdesc;
					    QString ptype = typedescToQString( tdesc );
					    if ( funcdesc->invkind == INVOKE_FUNC )
						ptype = constRefify( ptype );

					    paramTypes << ptype;
					    parameters << paramName;
					    prototype += ptype;
					    if ( optional )
						prototype += "=0";
					    if ( p < funcdesc->cParams )
						prototype += ",";
					}
					if ( !!prototype )
					    prototype += ")";

					if ( !signallist->find( funcdesc->memid ) ) {
					    QUMethod *signal = new QUMethod;
					    signal->name = new char[function.length()+1];
					    signal->name = qstrcpy( (char*)signal->name, function );
					    signal->count = parameters.count();
					    QUParameter *params = signal->count ? new QUParameter[signal->count] : 0;
					    for ( p = 0; p< signal->count; ++p ) {
						QString paramName = parameters[p];
						QString paramType = paramTypes[p];
						params[p].name = new char[paramName.length()+1];
						params[p].name = qstrcpy( (char*)params[p].name, paramName );
						params[p].inOut = 0;
						if ( funcdesc->lprgelemdescParam + p ) {
						    ushort inout = funcdesc->lprgelemdescParam[p].paramdesc.wParamFlags;
						    if ( inout & PARAMFLAG_FIN )
							params[p].inOut |= QUParameter::In;
						    if ( inout & PARAMFLAG_FOUT )
							params[p].inOut |= QUParameter::Out;
						}

						QStringToQUType( paramType, params + p );
					    }
					    signal->parameters = params;

					    QMetaData *data = new QMetaData;
					    data->name = new char[prototype.length()+1];
					    data->name = qstrcpy( (char*)data->name, prototype );
					    data->access = QMetaData::Public;
					    data->method = signal;

					    signallist->insert( funcdesc->memid, data );
#if 0
					    eventSink->addSignal( funcdesc->memid, prototype );
#endif
					}

#if 0 // documentation in metaobject would be cool?
					// get function documentation
					BSTR bstrDocu;
					eventinfo->GetDocumentation( funcdesc->memid, 0, &bstrDocu, 0, 0 );
					QString strDocu = BSTRToQString( bstrDocu );
					if ( !!strDocu )
					    desc += "[" + strDocu + "]";
					desc += "\n";
					SysFreeString( bstrDocu );
#endif
					eventinfo->ReleaseFuncDesc( funcdesc );
				    }
				}
			    }
			}
		    }
		} while ( c );
	    }
	}
    }

    Q_ASSERT(object);
    
    return 0;
}

LRESULT QActiveX::OnShowWindow( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
    if( m_pWidget ) {
	::SetParent( m_pWidget->winId(), m_hWnd );
	m_pWidget->raise();
	m_pWidget->move( 0, 0 );
	if( wParam )
	    m_pWidget->show();
	else
	    m_pWidget->hide();
    }
    return 0;
}

LRESULT QActiveX::OnPaint( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
    if ( m_pWidget )
	m_pWidget->update();
    return 0;
}

LRESULT QActiveX::ForwardMessage( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
    if ( uMsg == WM_SIZE && m_pWidget ) {
	m_pWidget->resize( LOWORD(lParam), HIWORD(lParam) );
    }
    if( m_pWidget )
	return ::SendMessage( m_pWidget->winId(), uMsg, wParam, lParam );
    return 0;
}

LRESULT QActiveX::ForwardFocusMessage( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
    if( m_pWidget ) {
	if ( uMsg == WM_SETFOCUS )
	    ::SendMessage( m_pWidget->winId(), WM_ACTIVATE, MAKEWPARAM( WA_ACTIVE, 0 ), 0 );
	return ::SendMessage( m_pWidget->winId(), uMsg, wParam, lParam );
    }
    return 0;
}

HRESULT WINAPI QActiveX::UpdateRegistry(BOOL bRegister)
{
    char filename[MAX_PATH];
    GetModuleFileNameA( 0, filename, MAX_PATH-1 );
    QString file = QString::fromLocal8Bit(filename );
    QString path = file.left( file.findRev( "\\" )+1 );
    QString module = file.right( file.length() - path.length() );
    module = module.left( module.findRev( "." ) );
    
    //return _Module.UpdateRegistryFromResource(IDR_QEXETEST, bRegister);
    QSettings settings;
    settings.insertSearchPath( QSettings::Windows, "/Classes" );
    const QString appID = QUuid( IID_ActiveQtApp ).toString().upper();
    const QString classID = QUuid( CLSID_QActiveX ).toString().upper();
    const QString eventID = QUuid( DIID__IQActiveXEvents ).toString().upper();
    const QString libID = QUuid( LIBID_ACTIVEQTEXELib ).toString().upper();
    const QString ifaceID = QUuid( IID_IQActiveX ).toString().upper();
    
    QApplication *tempapp = 0;
    if ( !qApp ) {
	int argc(0);
	tempapp = new QApplication( argc, 0 );
    }
    QObject *qobject = axmain(0);
    const QString className = QString(qobject->className());
    
    if ( bRegister ) {
	settings.writeEntry( "/AppID/" + appID + "/.", module );
	settings.writeEntry( "/AppID/" + module + ".EXE/AppID", appID );
	
	settings.writeEntry( "/" + module + "." + className + ".1/.", className + " Class" );
	settings.writeEntry( "/" + module + "." + className + ".1/CLSID/.", classID );
	settings.writeEntry( "/" + module + "." + className + ".1/Insertable/.", QString::null );
	
	settings.writeEntry( "/" + module + "." + className + "/.", className + " Class" );
	settings.writeEntry( "/" + module + "." + className + "/CLSID/.", classID );
	settings.writeEntry( "/" + module + "." + className + "/CurVer/.", module + "." + className + ".1" );
	
	settings.writeEntry( "/CLSID/" + classID + "/.", className + " Class" );
	settings.writeEntry( "/CLSID/" + classID + "/AppID", appID );
	settings.writeEntry( "/CLSID/" + classID + "/Control/.", QString::null );
	settings.writeEntry( "/CLSID/" + classID + "/Insertable/.", QString::null );
	settings.writeEntry( "/CLSID/" + classID + "/LocalServer32/.", file );
	settings.writeEntry( "/CLSID/" + classID + "/MiscStatus/.", "0" );
	settings.writeEntry( "/CLSID/" + classID + "/MiscStatus/1/.", "131473" );
	settings.writeEntry( "/CLSID/" + classID + "/Programmable/.", QString::null );
	settings.writeEntry( "/CLSID/" + classID + "/ToolboxBitmap32/.", file + ", 101" );
	settings.writeEntry( "/CLSID/" + classID + "/TypeLib/.", libID );
	settings.writeEntry( "/CLSID/" + classID + "/Version/.", "1.0" );
	settings.writeEntry( "/CLSID/" + classID + "/VersionIndependentProgID/.", module + "." + className );
	settings.writeEntry( "/CLSID/" + classID + "/ProgID/.", module + "." + className + ".1" );
	settings.writeEntry( "/CLSID/" + classID + "/Implemented Categories/.", QString::null );
	//### TODO: write some list of categories
	
	settings.writeEntry( "/Interface/" + ifaceID + "/.", "IQActiveX" );
	settings.writeEntry( "/Interface/" + ifaceID + "/ProxyStubClsid/.", "{00020424-0000-0000-C000-000000000046}" );
	settings.writeEntry( "/Interface/" + ifaceID + "/ProxyStubClsid32/.", "{00020424-0000-0000-C000-000000000046}" );
	settings.writeEntry( "/Interface/" + ifaceID + "/TypeLib/.", libID );
	settings.writeEntry( "/Interface/" + ifaceID + "/TypeLib/Version", "1.0" );
	
	settings.writeEntry( "/Interface/" + eventID + "/.", "_IQActiveXEvents" );
	settings.writeEntry( "/Interface/" + eventID + "/ProxyStubClsid/.", "{00020420-0000-0000-C000-000000000046}" );
	settings.writeEntry( "/Interface/" + eventID + "/ProxyStubClsid32/.", "{00020420-0000-0000-C000-000000000046}" );
	settings.writeEntry( "/Interface/" + eventID + "/TypeLib/.", libID );
	settings.writeEntry( "/Interface/" + eventID + "/TypeLib/Version", "1.0" );
	
	settings.writeEntry( "/TypeLib/" + libID + "/1.0/0/win32/.", file );
	settings.writeEntry( "/TypeLib/" + libID + "/1.0/FLAGS/.", "0" );
	settings.writeEntry( "/TypeLib/" + libID + "/1.0/HELPDIR/.", path );
	settings.writeEntry( "/TypeLib/" + libID + "/1.0/.", "ActiveQtEXE 1.0 Type Library" );
    } else {
	settings.removeEntry( "/AppID/" + module + ".EXE/AppID" );
	settings.removeEntry( "/AppID/" + appID + "/." );
	
	settings.removeEntry( "/" + module + "." + className + ".1/CLSID/." );
	settings.removeEntry( "/" + module + "." + className + ".1/Insertable/." );
	settings.removeEntry( "/" + module + "." + className + ".1/." );
	
	settings.removeEntry( "/" + module + "." + className + "/CLSID/." );
	settings.removeEntry( "/" + module + "." + className + "/CurVer/." );
	settings.removeEntry( "/" + module + "." + className + "/." );
	
	settings.removeEntry( "/CLSID/" + classID + "/AppID" );
	settings.removeEntry( "/CLSID/" + classID + "/Control/." );
	settings.removeEntry( "/CLSID/" + classID + "/Insertable/." );
	settings.removeEntry( "/CLSID/" + classID + "/LocalServer32/." );
	settings.removeEntry( "/CLSID/" + classID + "/MiscStatus/1/." );
	settings.removeEntry( "/CLSID/" + classID + "/MiscStatus/." );	    
	settings.removeEntry( "/CLSID/" + classID + "/Programmable/." );
	settings.removeEntry( "/CLSID/" + classID + "/ToolboxBitmap32/." );
	settings.removeEntry( "/CLSID/" + classID + "/TypeLib/." );
	settings.removeEntry( "/CLSID/" + classID + "/Version/." );
	settings.removeEntry( "/CLSID/" + classID + "/VersionIndependentProgID/." );
	settings.removeEntry( "/CLSID/" + classID + "/ProgID/." );
	//### TODO: remove some list of categories
	settings.removeEntry( "/CLSID/" + classID + "/Implemented Categories/." );
	settings.removeEntry( "/CLSID/" + classID + "/." );
	
	settings.removeEntry( "/Interface/" + ifaceID + "/ProxyStubClsid/." );
	settings.removeEntry( "/Interface/" + ifaceID + "/ProxyStubClsid32/." );
	settings.removeEntry( "/Interface/" + ifaceID + "/TypeLib/Version" );
	settings.removeEntry( "/Interface/" + ifaceID + "/TypeLib/." );
	settings.removeEntry( "/Interface/" + ifaceID + "/." );
	
	settings.removeEntry( "/Interface/" + eventID + "/ProxyStubClsid/." );
	settings.removeEntry( "/Interface/" + eventID + "/ProxyStubClsid32/." );
	settings.removeEntry( "/Interface/" + eventID + "/TypeLib/Version" );
	settings.removeEntry( "/Interface/" + eventID + "/TypeLib/." );
	settings.removeEntry( "/Interface/" + eventID + "/." );
	
	settings.removeEntry( "/TypeLib/" + libID + "/1.0/0/win32/." );
	settings.removeEntry( "/TypeLib/" + libID + "/1.0/0/." );
	settings.removeEntry( "/TypeLib/" + libID + "/1.0/FLAGS/." );
	settings.removeEntry( "/TypeLib/" + libID + "/1.0/HELPDIR/." );
	settings.removeEntry( "/TypeLib/" + libID + "/1.0/." );
    }
    
    delete qobject;
    if ( tempapp )
	delete tempapp;
    return S_OK;
}

HRESULT WINAPI QActiveX::Invoke(DISPID dispidMember, REFIID riid,
		  LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult,
		  EXCEPINFO* pexcepinfo, UINT* puArgErr)
{
    QMetaData *slot = slotlist->find( dispidMember );
    if ( slot ) {
	int index = object->metaObject()->findSlot( slot->name );
	if ( index != -1 ) {
	    object->qt_invoke( index, 0 );
	}
	return S_OK;
    }

    return IDispatchImpl<IQActiveX, &IID_IQActiveX, &LIBID_ACTIVEQTEXELib >::
	Invoke( dispidMember, riid, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr );
}
