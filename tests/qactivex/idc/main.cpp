#include <qfile.h>
#include <qtextstream.h>
#include <qstringlist.h>
#include <qregexp.h>
#include <qmap.h>

static const int ntypes = 32;
static const char* const type_map[ntypes][2] =
{
    { "QMap",		0 },
    { "QValueList",	0 },
    { "QString",	"BSTR" },
    { "QStringList",	0 },
    { "QFont",		0 },
    { "QPixmap",	0 },
    { "QBrush",		0 },
    { "QRect",		0 },
    { "QSize",		0 },
    { "QColor",		"OLE_COLOR" },
    { "QPalette",	0 },
    { "QColorGroup",	0 },
    { "QIconSet",	0 },
    { "QPoint",		0 },
    { "QImage",		0 },
    { "int",		"int" },
    { "uint",		"int" },
    { "bool",		"BOOL" },
    { "double",		"double" },
    { "QCString",	"BSTR" },
    { "QPointArray",	0 },
    { "QRegion",	0 },
    { "QBitmap",	0 },
    { "QCursor",	0 },
    { "QSizePolicy",	0 },
    { "QDate",		"DATE" },
    { "QTime",		"DATE" },
    { "QDateTime",	"DATE" },
    { "QByteArray",	0 },
    { "QBitArray",	0 },
    { "QKeySequence",	0 }
};

static bool convertTypes( QString &f )
{
    for ( int i = 0; i < ntypes; i++ ) {
	QString oldf = f;
	const QCString qtype = type_map[i][0];
	QCString oletype = type_map[i][1];
	if ( !oletype.isEmpty() )
	    oletype += " ";
	f = f.replace( QRegExp("const " + qtype + " &"), oletype );
	f = f.replace( QRegExp("const " + qtype + "&"), oletype );
	f = f.replace( QRegExp( qtype ), oletype );
	if ( oldf != f ) {
	    if ( !type_map[i][1] ) {
		f = qtype;
		return FALSE;
	    }
	}
    }
    return TRUE;
}

static QStringList readFile( QTextStream &in )
{
    QString token;
    int x = 0;
    QStringList tokens;
    bool inCppComment = FALSE;
    while ( !in.atEnd() ) {
	in >> token;
	token = token.stripWhiteSpace();
	token = token.simplifyWhiteSpace();

	do {
	    if ( inCppComment && !token.contains( "*/" ) ) {
		in >> token;
		continue;
	    }
	    if ( inCppComment )
		token = token.right( token.length() - token.find( "*/" )-2 );
	    inCppComment = FALSE;

	    if ( token.contains( "/*" ) && !inCppComment ) {
		QString before = token.left( token.find( "/*" ) );
		if ( !before.isEmpty() )
		    tokens << before;
		inCppComment = TRUE;
	    }
	} while ( inCppComment );
	
	if ( token.contains( "//" ) ) {
	    tokens << token.left( token.find( "//" ) );
	    in.readLine();
	    continue;
	}

	if ( token == "#include" ) {
	    in.readLine();
	    continue;
	} else if ( token == "#define" ) {
	    in.readLine();
	    continue;
	}
	tokens << token;
    }
    return tokens;
}

QString parseFile( QStringList &tokens, QStringList::Iterator &t,
			QString &coclass,
			QString &iid_coclass, QString &iid_iface, QString &iid_events, QString &iid_typelib,
			QStringList &signallist, QStringList &slotlist, QStringList &proplist )
{
    enum State {
	Default,
	Error,
	SearchClass,
	TestClass,
	HaveActiveX,
	HaveProperty,
	HavePropertyRead,
	HavePropertyWrite,
	HavePropertyDesignable,
	HavePropertyScriptable,
	HaveSignals,
	HaveSignal,
	DoneSignal,
	TestSlots,
	HaveSlots,
	HaveSlot,
	DoneSlot,
	HaveIID,
	DoneIID
    } state;

    state = Default;
    QString token;
    QString property;  // type name
    QString propflags; // read,write,designable,scriptable
    QString signal;
    QString slot;
    QString error;
    QString iid;

    int curlybrackets = 0;
    for ( ; t != tokens.end(); ++t ) {
	token = *t;
	curlybrackets += token.contains( '{' );
	curlybrackets -= token.contains( '}' );
	switch ( state ) {
	case Default:
	    if ( token == "class" ) {
		state = SearchClass;
	    } else if ( token.startsWith( "QT_ACTIVEX" ) ) {
		state = HaveIID;
		iid = token;
	    }
	    break;
	case SearchClass:
	    if ( token.length() > 1 && token.right(1) == ":" ) {
		coclass = token.left( token.length()-1 );
		state = TestClass;
	    } else if ( token == ":" ) {
		--t;
		if ( !coclass.isEmpty() && coclass != *(t) ) {
		    state = Error;
		    error = QString("Classnames not matching (%1 != %1)").arg(coclass).arg(*t);
		}
		coclass = *(t);
		state = TestClass;
		++t;
	    } else if ( token.find( ':' ) != -1 ) {
		token = token.left( token.find( ':' ) );
		if ( !coclass.isEmpty() && coclass != token ) {
		    state = Error;
		    error = QString("Classnames not matching (%1 != %1)").arg(coclass).arg(token);
		}
		coclass = token;
		state = TestClass;
	    } else if ( token.find( '{' ) != -1 || token.find( ';' ) != -1 ) {
		state = Default;
	    }
	    break;
	case TestClass:
	    if ( token == "public" || token == "protected" || token == "private" ) {
		if ( t++ == tokens.end() ) {
		    --t;
		    error = QString("Unexpected end of file (%1)").arg(*t);
		    state = Error;
		    break;
		}
		token = *t;
	    }
	    if ( token == "QActiveQt" || token == "QActiveQt," ) {
		state = HaveActiveX;
	    } else if ( token == "{" ) {
		state = Default;
		coclass = QString::null;
	    }
	    break;
	case HaveActiveX:
	    if ( token == "};" ) {
		state = Default;
	    } else if ( curlybrackets != 1 ) {
		break;
	    } else if ( token == "Q_PROPERTY" || token == "Q_PROPERTY(" ) {
		property = QString::null;
		propflags = "0011";
		state = HaveProperty;
	    } else if ( token == "signals:" ) {
		state = HaveSignals;
	    } else if ( token == "public" ) {
		state = TestSlots;
	    }
	    break;

	case TestSlots:
	    if ( token == "slots:" ) {
		state = HaveSlots;
	    } else {
		state = HaveActiveX;
	    }
	    break;
	case HaveProperty:
	    if ( token == ")" ) {
		proplist << propflags + " " + property;
		state = HaveActiveX;
	    } else if ( token == "READ" ) {
		state = HavePropertyRead;
	    } else if ( token == "WRITE" ) {
		state = HavePropertyWrite;
	    } else if ( token == "DESIGNABLE" ) {
		state = HavePropertyDesignable;
	    } else if ( token == "SCRIPTABLE" ) {
		state = HavePropertyScriptable;
	    } else {
		// type conversion to OLE
		if ( property.isEmpty() ) {
		    if ( !convertTypes( token ) ) {
			--t;
			error = QString("Unsupported datatype in property (%1)").arg(*t);
			state = Error;
		    }
		    property = token + " ";
		} else {
		    property += token;
		}
	    }
	    break;
	case HavePropertyRead:
	    propflags[0] = '1';
	    state = HaveProperty;
	    break;
	case HavePropertyWrite:
	    propflags[1] = '1';
	    state = HaveProperty;
	    break;
	case HavePropertyDesignable:
	    if ( token.lower() == "false" )
		propflags[2] = '0';
	    state = HaveProperty;
	    break;
	case HavePropertyScriptable:
	    if ( token.lower() == "false" )
		propflags[3] = '0';
	    state = HaveProperty;
	    break;

	case HaveSlots:
	    if ( token == "void" ) {
		state = HaveSlot;
		slot = QString::null;
	    } else if ( token == "public:" || 
			token == "protected:" || 			
			token == "private:" ) {
		state = HaveActiveX;
	    } else if ( token == "signals:" ) {
		state = HaveSignals;
	    } else if ( token == "public" || 
			token == "protected" || 
			token == "private" ) {
		state = TestSlots;
	    } else if ( token == "};" ) {
		state = Default;
	    }
	    break;
	case HaveSlot:
	    if ( token.right(2) == ");" ) {
		slot += token.left( token.length()-1 );
		state = DoneSlot;
	    } else if ( token.right(2) == "){" ) {
		slot += ")";
    		state = DoneSlot;
	    } else if ( token == "{" ) {
		if ( !convertTypes( slot ) ) {
		    --t;
		    error = QString("Unsupported datatype in slot (%1)").arg(*t);
		    state = Error;
		}
		slotlist << slot;
		state = HaveSlots;
	    } else {
		slot += token + " ";
	    }
	    break;
	case DoneSlot:
	    if ( !convertTypes( slot ) ) {
		--t;
		error = QString("Unsupported datatype in slot (%1)").arg(*t);
		state = Error;
	    }
	    slotlist << slot;
	    state = HaveSlots;
	    --t;
	    break;

	case HaveSignals:
	    if ( token == "void" ) {
		state = HaveSignal;
		signal = QString::null;
	    } else if ( token == "signals:" || 
		        token == "public:" || 
			token == "protected:" || 			
			token == "private:" ) {
		state = HaveActiveX;
	    } else if ( token == "public" || 
			token == "protected" || 
			token == "private" ) {
		state = TestSlots;
	    } else if( token == "};" ) {
		state = Default;
	    }
	    break;
	case HaveSignal:
	    if ( token.right(2) == ");" ) {
		signal += token.left( token.length()-1 );
		state = DoneSignal;
	    } else if ( token.right(2) == "){" ) {
		signal += signal+")";
    		state = DoneSignal;
	    } else if ( token == "{" ) {
		if ( !convertTypes( signal ) ) {
		    --t;
		    error = QString("Unsupported datatype in signal (%1)").arg(*t);
		    state = Error;
		}
		signallist << signal;
    		state = HaveSignals;
	    } else {
		signal += token + " ";
	    }
	    break;
	case DoneSignal:
	    if ( !convertTypes( signal ) ) {
		--t;
		error = QString("Unsupported datatype in signal (%1)").arg(*t);
		state = Error;
	    }
	    signallist << signal;
	    --t;
	    state = HaveSignals;
	    break;

	case HaveIID:
	    if ( iid.contains( ")" ) ) {
		state = DoneIID;
		--t;
	    } else {
		iid += token;
	    }
	    break;
	case DoneIID:
	    {
		iid = iid.mid( iid.find( "(" )+1 );
		iid = iid.left( iid.findRev( ")" )+1 );
		QStringList iids = QStringList::split( ",", iid );
		if ( !coclass.isEmpty() && iids[0] != coclass ) {
		    error = QString("Classnames not matching (%1 != %1)").arg(coclass).arg(iids[0]);
		    return error;
		}
		coclass = iids[0];
		iid_coclass = iids[1].mid( 2, iids[1].length()-4 );;
		iid_iface = iids[2].mid( 2, iids[2].length()-4 );;
		iid_events = iids[3].mid( 2, iids[3].length()-4 );;
		iid_typelib = iids[4].mid( 2, iids[4].length()-4 );;
		--t;

		if ( state == DoneIID )
		    return error;
	    }
	    break;

	case Error:
	    return error;
	}
    }

    return error;
}

int main( int argc, char **argv )
{
    QStringList input;
    QString output;
    QString resource;
    QString error;
    int i = 1;
    while ( i < argc ) {
	QCString p = QCString(argv[i]).lower();

	if ( p == "/o" || p == "-o" ) {
	    if ( !!output ) {
		error = "Too many output files specified!";
		break;
	    }
	    ++i;
	    if ( i >= argc ) {
		error = "Missing name for output file!";
		break;
	    }
	    output = argv[i];
	    output = output.stripWhiteSpace();
	} else if ( p == "/r" || p == "-r" || p == "/rc" || p == "-rc" ) {
	    if ( !!resource ) {
		error = "Too many resource files specified!";
		break;
	    }
	    ++i;
	    if ( i >= argc ) {
		error = "Missing name for resource file!";
		break;
	    }
	    resource = argv[i];
	    resource = resource.stripWhiteSpace();
	} else if ( p == "/v" || p == "-v" ) {
	    qWarning( "Qt interface definition compiler version 1.0" );
	    return 0;
	} else if ( p[0] == '/' || p[0] == '-' ) {
	    error = "Unknown option \"" + p + "\"";
	    break;
	} else {
	    QString in = argv[i];
	    input << in.stripWhiteSpace();
	}
	i++;
    }
    if ( input.isEmpty() )
	error = "No input file specified!";
    if ( !!error ) {
	qFatal( "Qt interface definition compiler\n"
		"Invalid argument %d: %s\n"
		"Usage:\tidc [options] <files>\n"
		      "\t-o file\t\tWrite output to file rather than stdout\n"
		      "\t-r[c] file\tWrite resource file\n"
		      "\t-v\t\tDisplay version of idc", i, error.latin1() );
	return 1;
    }

    QString filebase;

    filebase = output;
    filebase = filebase.left( filebase.findRev( "." ) );

    if ( !!resource ) {
	QFile file( resource );
	if ( !file.open( IO_WriteOnly ) )
	    qFatal( "Couldn't open %s for write", resource.latin1() );
	QTextStream out( &file );
	out << "1 TYPELIB \"" << filebase.replace( QRegExp("\\\\"), "\\\\" ) << ".tlb\"" << endl;
    }

    QStringList::Iterator it;

    QStringList coclasses;
    QMap<QString, QString> coclass_clsid;
    QMap<QString, QString> coclass_iface;
    QMap<QString, QString> coclass_events;
    QMap<QString, QStringList> coclass_signals;
    QMap<QString, QStringList> coclass_slots;
    QMap<QString, QStringList> coclass_properties;
    QString typelib_iid;
    QString typelib = filebase.right( filebase.length() - filebase.findRev( "\\" )-1 );
    for ( it = input.begin(); it != input.end(); ++it ) {
	QFile infile( *it );
	if ( !infile.open( IO_ReadOnly ) )
	    qFatal( "Couldn't open %s for reading", (*it).latin1() );
	
	QTextStream in( &infile );
	{
	    QStringList tokens = readFile( in );

	    QStringList::Iterator t = tokens.begin();
	    while ( t != tokens.end() ) {
		QString coclass;
		QString iid_coclass;
		QString iid_iface;
		QString iid_events;
		QString iid_typelib;
		QStringList signallist;
		QStringList slotlist;
		QStringList proplist;

		QString error = parseFile( tokens, t, coclass, iid_coclass, iid_iface, iid_events, iid_typelib, signallist, slotlist, proplist );
		if ( coclass.isEmpty() )
		    break;
		if ( t != tokens.end() )
		    ++t;
		if ( !error.isEmpty() ) {
		    qFatal( "%s: Error parsing file: %s", (*it).latin1(), error.latin1() );
		}
		if ( !!typelib_iid && typelib_iid != iid_typelib ) {
		    qFatal( "%s: Multiple typelibraries are not supported", (*it).latin1() );
		}
		typelib_iid = iid_typelib;
		coclasses.append( coclass );
		coclass_clsid.insert( coclass, iid_coclass );
		coclass_iface.insert( coclass, iid_iface );
		coclass_events.insert( coclass, iid_events );
		coclass_signals.insert( coclass, signallist );
		coclass_slots.insert( coclass, slotlist );
		coclass_properties.insert( coclass, proplist );
	    }
	}
    }
    QTextStream out;
    QFile file;
    if ( !!output ) {
	file.setName( output );
	if ( !file.open( IO_WriteOnly ) )
	    qFatal( "Couldn't open %s for writing", output.latin1() );
	out.setDevice( &file );
    } else {
	//### stdout
    }

    out << "import \"oaidl.idl\";" << endl;
    out << "import \"ocidl.idl\";" << endl;
    out << "#include \"olectl.h\"" << endl << endl << endl;

#ifndef QDISPATCH
    for ( it = coclasses.begin(); it != coclasses.end(); ++it ) {
	QString coclass = *it;
	QString iid_iface = coclass_iface[coclass];
	QStringList slotlist = coclass_slots[coclass];
	QStringList proplist = coclass_properties[coclass];

	out << "\t[" << endl;
	out << "\t\tobject," << endl;
	out << "\t\tuuid(" << iid_iface << ")," << endl;
	out << "\t\tdual," << endl;
	out << "\t\thelpstring(\"" << coclass << " Interface\")," << endl;
	out << "\t\tpointer_default(unique)" << endl;
	out << "\t]" << endl;
	out << "\tinterface I" << coclass << " : IDispatch" << endl;
	out << "\t{" << endl;
	int ifacemethod = 1;
	QStringList::iterator it2;
	for ( it2 = slotlist.begin(); it2 != slotlist.end(); ++it2 ) {
	    QString slot = *it2;
	    out << "\t\t[id(" << ifacemethod << "), helpstring(\"method " << slot.left(slot.find("(")) << "\")] ";
	    out << "HRESULT " << slot << ";" << endl;
	    ++ifacemethod;
	}
	for ( it2 = proplist.begin(); it2 != proplist.end(); ++it2 ) {
	    QString prop = *it2;
	    bool read = prop[0] != '0';
	    bool write = prop[1] != '0';
	    bool designable = prop[2] != '0';
	    bool scriptable = prop[3] != '0';
	    prop = prop.mid( 5 );
	    QString type = prop.left( prop.find(" ") );
	    QString name = prop.mid( prop.find( " " ) + 1 );
	    if ( read ) {
		out << "\t\t[propget, id(" << ifacemethod << "), helpstring(\"property " << name << "\")";
		if ( scriptable )
		    out << ", bindable";
		if ( !designable )
		    out << ", nonbrowsable";
		out << ", requestedit] HRESULT " << name << "([out, retval] " << type << " *pVal);" << endl;
	    }
	    if ( write ) {
		out << "\t\t[propput, id(" << ifacemethod << "), helpstring(\"property " << name << "\")";
		if ( scriptable )
		    out << ", bindable";
		if ( !designable )
		    out << ", nonbrowsable";
		out << ", requestedit] HRESULT " << name << "([in] " << type << " newVal);" << endl;
	    }
	    ++ifacemethod;
	}
	out << "\t};" << endl << endl;
    }
#endif

    out << "[" << endl;
    out << "\tuuid(" << typelib_iid << ")," << endl;
    out << "\tversion(1.0)," << endl;
    out << "\thelpstring(\"" << typelib << " 1.0 Type Library\")" << endl;
    out << "]" << endl;
    out << "library " << typelib << "Lib" << endl;
    out << "{" << endl;
    out << "\timportlib(\"stdole32.tlb\");" << endl;
    out << "\timportlib(\"stdole2.tlb\");" << endl << endl;

    for ( it = coclasses.begin(); it != coclasses.end(); ++it ) {
	QString coclass = *it;
	
	QString iid_coclass = coclass_clsid[coclass];
	QString iid_iface = coclass_iface[coclass];
	QString	iid_events = coclass_events[coclass];
	QStringList slotlist = coclass_slots[coclass];
	QStringList proplist = coclass_properties[coclass];
	QStringList signallist = coclass_signals[coclass];
	QStringList::Iterator it2;

#ifdef QDISPATCH

	out << "\t[" << endl;
	out << "\t\tuuid(" << iid_iface << ")," << endl;
	out << "\t\thelpstring(\"" << coclass << " Interface\")," << endl;
	out << "\t]" << endl;
	out << "\tdispinterface I" << coclass << endl;
	out << "\t{" << endl;
	out << "\t\tproperties:" << endl;
	out << "\t\tmethods:" << endl;
	int ifacemethod = 1;
	for ( it2 = slotlist.begin(); it2 != slotlist.end(); ++it2 ) {
	    QString slot = *it2;
	    out << "\t\t[id(" << ifacemethod << "), helpstring(\"method " << slot.left(slot.find("(")) << "\")] ";
	    out << "HRESULT " << slot << ";" << endl;
	    ++ifacemethod;
	}
	for ( it2 = proplist.begin(); it2 != proplist.end(); ++it2 ) {
	    QString prop = *it2;
	    bool read = prop[0] != '0';
	    bool write = prop[1] != '0';
	    bool designable = prop[2] != '0';
	    bool scriptable = prop[3] != '0';
	    prop = prop.mid( 5 );
	    QString type = prop.left( prop.find(" ") );
	    QString name = prop.mid( prop.find( " " ) + 1 );
	    if ( write ) {
		out << "\t\t[propput, id(" << ifacemethod << "), helpstring(\"property " << name << "\")";
		if ( scriptable )
		    out << ", bindable";
		if ( !designable )
		    out << ", nonbrowsable";
		out << ", requestedit] HRESULT " << name << "([in] " << type << " newVal);" << endl;
	    }
	    if ( read ) {
		out << "\t\t[propget, id(" << ifacemethod << "), helpstring(\"property " << name << "\")";
		if ( scriptable )
		    out << ", bindable";
		if ( !designable )
		    out << ", nonbrowsable";
		out << ", requestedit] HRESULT " << name << "([out, retval] " << type << " *pVal);" << endl;
	    }
	    ++ifacemethod;
	}
	out << "\t};" << endl << endl;
#endif
	
	out << "\t[" << endl;
	out << "\t\tuuid(" << iid_events << ")," << endl;
	out << "\t\thelpstring(\"" << coclass << " Events Interface\")" << endl;
	out << "\t]" << endl;
	out << "\tdispinterface I" << coclass << "Events" << endl;
	out << "\t{" << endl;
	out << "\t\tproperties:" << endl;
	out << "\t\tmethods:" << endl;
	int eventmethod = 1;
	for ( it2 = signallist.begin(); it2 != signallist.end(); ++it2 ) {
	    QString signal = *it2;
	    out << "\t\t[id(" << eventmethod << ")] void " << signal << ";" << endl;
	    ++eventmethod;
	}
	out << "\t};" << endl << endl;
	
	out << "\t[" << endl;
	out << "\t\tuuid(" << iid_coclass << ")," << endl;
	out << "\t\thelpstring(\"" << coclass << " Class\")" << endl;
	out << "\t]" << endl;
	out << "\tcoclass " << coclass << endl;
	out << "\t{" << endl;
#ifdef QDISPATCH
	out << "\t\t[default] dispinterface I" << coclass << ";" << endl;
#else
	out << "\t\t[default] interface I" << coclass << ";" << endl;
#endif
	out << "\t\t[default, source] dispinterface I" << coclass << "Events;" << endl;
	out << "\t};" << endl;
    }
    out << "};" << endl;

    return 0;
}
