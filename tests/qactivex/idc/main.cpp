#include <qfile.h>
#include <qtextstream.h>
#include <qstringlist.h>
#include <qregexp.h>

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
    { "bool",		"boolean" },
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

static QString parseFile( QTextStream &in,
			QString &coclass,
			QString &iid_coclass, QString &iid_iface, QString &iid_events, QString &iid_typelib,
			QStringList &signallist, QStringList &slotlist, QStringList &proplist )
{
    QString token;
    int x = 0;
    QStringList tokens;
    while ( !in.atEnd() ) {
	in >> token;
	token = token.stripWhiteSpace();
	token = token.simplifyWhiteSpace();
	if ( token == "#include" ) {
	    in.readLine();
	    continue;
	} else if ( token == "#define" ) {
	    in.readLine();
	    continue;
	}
	tokens << token;
    }

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
    QString property;  // type name
    QString propflags; // read,write,designable,scriptable
    QString signal;
    QString slot;
    QString error;
    QString iid;

    int curlybrackets = 0;
    QStringList::Iterator t;
    for ( t = tokens.begin(); t != tokens.end(); ++t ) {
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
		if ( token != "QActiveQt" ) {
		    state = Default;
		    coclass = QString::null;
		} else {
		    state = HaveActiveX;
		}
	    } else if ( token == "QActiveQt" || token == "QActiveQt," ) {
		state = HaveActiveX;
	    } else if ( token == "{" ) {
		state = Default;
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
	    } else if ( token == "signals:" || 
		        token == "public:" || 
			token == "protected:" || 			
			token == "private:" ) {
		state = HaveActiveX;
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
		    state = Error;
		}
		coclass = iids[0];
		iid_coclass = iids[1].mid( 2, iids[1].length()-4 );;
		iid_iface = iids[2].mid( 2, iids[2].length()-4 );;
		iid_events = iids[3].mid( 2, iids[3].length()-4 );;
		iid_typelib = iids[4].mid( 2, iids[4].length()-4 );;
		--t;

		state = Default;
	    }
	    break;

	case Error:
	    return FALSE;
	}
    }

    if ( coclass.isEmpty() ) {
	error = "No relevant classes found.";
    }
    return error;
}

int main( int argc, char **argv )
{
    QString input;
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
	    if ( !!input ) {
		error = "Too many input files specified!";	
		break;
	    }
	    input = argv[i];
	    input = input.stripWhiteSpace();
	}
	i++;
    }
    if ( input.isEmpty() )
	error = "Missing input filename!";
    if ( !!error ) {
	qFatal( "Qt interface definition compiler\n"
		"Invalid argument %d: %s\n"
		"Usage:\tidc [options] <header-file>\n"
		      "\t-o file\t\tWrite output to file rather than stdout\n"
		      "\t-r[c] file\tWrite resource file\n"
		      "\t-v\t\tDisplay version of idc", i, error.latin1() );
	return 1;
    }

    QString filebase;

    if ( !!output )
	filebase = output;//.right( output.length() - output.findRev( "\\" )-1 );
    else
	filebase = input;//.right( input.length() - input.findRev( "\\" )-1 );

    filebase = filebase.left( filebase.findRev( "." ) );

    QFile infile( input );
    if ( !infile.open( IO_ReadOnly ) )
	qFatal( "Couldn't open %s for reading", input.latin1() );

    QTextStream in( &infile );

    if ( !!resource ) {
	QFile file( resource );
	if ( !file.open( IO_WriteOnly ) )
	    qFatal( "Couldn't open %s for write", resource.latin1() );
	QTextStream out( &file );
	out << "1 TYPELIB \"" << filebase.replace( QRegExp("\\\\"), "\\\\" ) << ".tlb\"" << endl;
    }
    {
	QTextStream out;
	QFile file;
	if ( !!output ) {
	    file.setName( output );
	    if ( !file.open( IO_WriteOnly ) )
		qFatal( "Couldn't open %s for writing", output.latin1() );
	    out.setDevice( &file );
	} else {
	}

	QString coclass;
	QString iid_coclass;
	QString iid_iface;
	QString iid_events;
	QString iid_typelib;
	QStringList signallist;
	QStringList slotlist;
	QStringList proplist;
	QString error = parseFile( in, coclass, iid_coclass, iid_iface, iid_events, iid_typelib, signallist, slotlist, proplist );
	if ( !error.isEmpty() ) {
	    qFatal( "%s: Error parsing file: %s", input.latin1(), error.latin1() );
	}

	QStringList::Iterator it;

	out << "import \"oaidl.idl\";" << endl;
	out << "import \"ocidl.idl\";" << endl;
	out << "#include \"olectl.h\"" << endl << endl << endl;

	out << "\t[" << endl;
	out << "\t\tobject," << endl;
	out << "\t\tuuid(" << iid_iface << ")," << endl;
	out << "\t\tdual," << endl;
	out << "\t\thelpstring(\"I" << coclass << " Interface\")," << endl;
	out << "\t\tpointer_default(unique)" << endl;
	out << "\t]" << endl;
	out << "\tinterface I" << coclass << " : IDispatch" << endl;
	out << "\t{" << endl;
	int ifacemethod = 1;
	for ( it = slotlist.begin(); it != slotlist.end(); ++it ) {
	    QString slot = *it;
	    out << "\t\t[id(" << ifacemethod << "), helpstring(\"method " << slot.left(slot.find("(")) << "\")] ";
	    out << "HRESULT " << slot << ";" << endl;
	    ++ifacemethod;
	}
	for ( it = proplist.begin(); it != proplist.end(); ++it ) {
	    QString prop = *it;
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

	out << "[" << endl;
	out << "\tuuid(" << iid_typelib << ")," << endl;
	out << "\tversion(1.0)," << endl;
	out << "\thelpstring(\"" << coclass << " 1.0 Type Library\")" << endl;
	out << "]" << endl;
	out << "library " << coclass << "Lib" << endl;
	out << "{" << endl;
	out << "\timportlib(\"stdole32.tlb\");" << endl;
	out << "\timportlib(\"stdole2.tlb\");" << endl << endl;
	out << "\t[" << endl;
	out << "\t\tuuid(" << iid_events << ")," << endl;
	out << "\t\thelpstring(\"_I" << coclass << "Events Interface\")" << endl;
	out << "\t]" << endl;
	out << "\tdispinterface _I" << coclass << "Events" << endl;
	out << "\t{" << endl;
	out << "\t\tproperties:" << endl;
	out << "\t\tmethods:" << endl;
	int eventmethod = 1;
	for ( it = signallist.begin(); it != signallist.end(); ++it ) {
	    QString signal = *it;
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
	out << "\t\t[default] interface I" << coclass << ";" << endl;
	out << "\t\t[default, source] dispinterface _I" << coclass << "Events;" << endl;
	out << "\t};" << endl;
	out << "};" << endl;
    }

    return 0;
}
