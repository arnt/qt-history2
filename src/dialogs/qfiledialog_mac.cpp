#include "qfiledialog.h"

#ifndef QT_NO_FILEDIALOG

#include "qapplication.h"
#include "../kernel/qapplication_p.h"
#include "qt_mac.h"
#include "qregexp.h"
#include "qbuffer.h"
#include "qstringlist.h"
#include "qtextcodec.h"

// Returns the wildcard part of a filter.
extern const char qt_file_dialog_filter_reg_exp[]; // defined in qfiledialog.cpp
static QString extractFilter( const QString& rawFilter )
{
    QString result;
    QRegExp r( QString::fromLatin1(qt_file_dialog_filter_reg_exp) );
    int index = r.search( rawFilter );
    if ( index >= 0 )
	result = rawFilter.mid( index + 1, r.matchedLength() - 2 );
    else
	result = rawFilter;
    return result.replace( QRegExp(QString::fromLatin1(" ")), QChar(';') );
}

// Makes a list of filters from ;;-separated text.
static QList<QRegExp> makeFiltersList( const QString &filter )
{
    QString f( filter );

    if ( f.isEmpty( ) )
	f = QFileDialog::tr( "All Files (*.*)" );

    if ( f.isEmpty() )
	return QList<QRegExp>();

    int i = f.find( ";;", 0 );
    QString sep( ";;" );
    if ( i == -1 ) {
	if ( f.find( "\n", 0 ) != -1 ) {
	    sep = "\n";
	    i = f.find( sep, 0 );
	}
    }

    QList<QRegExp> ret;
    QStringList filts = QStringList::split( sep, f);
    for (QStringList::Iterator it = filts.begin(); it != filts.end(); ++it ) {
	qDebug("::%s::", extractFilter((*it)).latin1());
	ret.append(new QRegExp(extractFilter((*it)), TRUE, TRUE));
    }
    return ret;
}

static Boolean qt_mac_nav_filter(AEDesc *theItem, void *info, 
				 void *myd, NavFilterModes)
{	
    QList<QRegExp> *filt = (QList<QRegExp> *)myd;
    if(!filt)
	return true;

    NavFileOrFolderInfo *theInfo = (NavFileOrFolderInfo *)info;
    if(theItem->descriptorType == typeFSS ) {
	if( !theInfo->isFolder ) {
	    AliasHandle alias;
	    Str63 str;
	    char tmp[sizeof(Str63)+2];
	    FSSpec      FSSpec;
	    AliasInfoType x = 0;

	    AEGetDescData( theItem, &FSSpec, sizeof(FSSpec));

	    if(NewAlias( NULL, &FSSpec, &alias ) != noErr) 
		return true;
	    GetAliasInfo(alias, (AliasInfoType)x++, str);
	    if(str[0]) {
		strncpy((char *)tmp, (const char *)str+1, str[0]);
		tmp[str[0]] = '\0';
		
		qDebug("%s", tmp);
		for (QListIterator<QRegExp> it(*filt); it.current(); ++it ) {
		    if(it.current()->match( tmp ))
			return true;
		}
	    }
	    return false;
	}
    }
    return true;
}

const unsigned char * p_str(const char *);
pascal OSErr FSpLocationFromFullPath( short fullPathLength,
				      const void *fullPath,
				      FSSpec *spec);

QString QFileDialog::macGetOpenFileName( const QString &/*initialSelection*/,
					 const QString &filter,
					 QString* initialDirectory,
					 QWidget *parent, const char* name,
					 const QString& caption )
{
    return macGetOpenFileNames(filter, initialDirectory, 
			       parent, name, caption).first();
}

QStringList QFileDialog::macGetOpenFileNames( const QString &filter,
					      QString* initialDirectory,
					      QWidget *parent,
					      const char* /*name*/,
					      const QString& caption )
{
    OSErr err;
    AliasInfoType x = 0;
    QString tmpstr;
    QStringList retstrl;
    NavDialogOptions options;
    NavGetDefaultDialogOptions( &options );
    options.version = kNavDialogOptionsVersion;
    options.location.h = options.location.v = -1;
    if(parent) 
	strcpy((char *)options.clientName, 
	       (const char *) p_str(parent->caption()));
    if(caption.length())
	strcpy((char *)options.windowTitle,
	       (const char *)p_str(caption));

    bool use_initial = FALSE;
    AEDesc initial;
    if(initialDirectory) {
	QString macFilename = initialDirectory->mid( 1 );
	while ( macFilename.find( "/" ) != -1 )
	    macFilename.replace( macFilename.find( "/" ), 1, ":" );
	//FIXME: prepend the volume name to the macFilename 

	FSSpec fileSpec;
	err = FSpLocationFromFullPath( macFilename.length(), 
				       macFilename.latin1(), &fileSpec ); 
	if(err == noErr) {
	    err = AECreateDesc(typeFSS, &fileSpec, sizeof(fileSpec), &initial );
	    if(err == noErr)
		use_initial = TRUE;
	}
    }
    
    NavReplyRecord ret;
    QList<QRegExp> filts = makeFiltersList(filter);
    NavGetFile(use_initial ? &initial : NULL, &ret, &options, NULL, NULL, 
	       qt_mac_nav_filter, NULL, (void *) filts.isEmpty() ? NULL : &filts);
    filts.setAutoDelete(TRUE);
    filts.clear();

    long count;
    err = AECountItems(&(ret.selection), &count);

    if(!ret.validRecord || err != noErr || !count) 
	goto get_name_out;

    AEKeyword  	keyword;
    DescType    type;
    Size        size;
    FSSpec      FSSpec;

    for(long index = 1; index <= count; index++) {
	err = AEGetNthPtr(&(ret.selection), index, typeFSS, &keyword, 
			  &type,&FSSpec, sizeof(FSSpec), &size);
	if(err != noErr) 
	    goto get_name_out;

	AliasHandle alias;
	Str63 str;
	char tmp[sizeof(Str63)+2];
	tmp[0] = '/';
	tmpstr = "";

	if(NewAlias( NULL, &FSSpec, &alias ) != noErr) 
	    goto get_name_out;
	while(1) {
	    GetAliasInfo(alias, (AliasInfoType)x++, str);
	    if(!str[0])
		break;
	    strncpy((char *)tmp+1, (const char *)str+1, str[0]);
	    tmp[str[0]+1] = '\0';
	    tmpstr.prepend((char *)tmp);
	}
	retstrl.append(tmpstr);
    }

 get_name_out:
    if(use_initial)
	AEDisposeDesc(&initial);
    NavDisposeReply(&ret);
    return retstrl;
}

QString QFileDialog::macGetSaveFileName( const QString &initialSelection,
					 const QString &filter,
					 QString* initialDirectory,
					 QWidget *parent, const char* /*name*/,
					 const QString& caption )
{
    OSErr err;
    AliasInfoType x = 0;
    QString retstr;
    NavDialogOptions options;
    NavGetDefaultDialogOptions( &options );
    options.version = kNavDialogOptionsVersion;
    options.location.h = options.location.v = -1;
    if(parent) 
	strcpy((char *)options.clientName, 
	       (const char *) p_str(parent->caption()));
    if(caption.length())
	strcpy((char *)options.windowTitle,
	       (const char *)p_str(caption));

    bool use_initial = FALSE;
    AEDesc initial;
    if(initialDirectory) {
	QString macFilename = initialDirectory->mid( 1 );
	while ( macFilename.find( "/" ) != -1 )
	    macFilename.replace( macFilename.find( "/" ), 1, ":" );
	//FIXME: prepend the volume name to the macFilename 

	FSSpec fileSpec;
	err = FSpLocationFromFullPath( macFilename.length(), 
				       macFilename.latin1(), &fileSpec ); 
	if(err == noErr) {
	    err = AECreateDesc(typeFSS, &fileSpec, sizeof(fileSpec), &initial );
	    if(err == noErr)
		use_initial = TRUE;
	}
    }
    
    NavReplyRecord ret;
    QList<QRegExp> filts = makeFiltersList(filter);
    NavPutFile(use_initial ? &initial : NULL, &ret, &options, NULL, 
	       'cute', kNavGenericSignature, (void *) filts.isEmpty() ? NULL : &filts);
    filts.setAutoDelete(TRUE);
    filts.clear();

    long count;
    err = AECountItems(&(ret.selection), &count);

    if(!ret.validRecord || err != noErr || !count) 
	goto put_name_out;

    AEKeyword  	keyword;
    DescType    type;
    Size        size;
    FSSpec      FSSpec;

    err = AEGetNthPtr(&(ret.selection), 1, typeFSS, &keyword, 
		      &type,&FSSpec, sizeof(FSSpec), &size);
    if(err != noErr) 
	goto put_name_out;

    AliasHandle alias;
    Str63 str;
    char tmp[sizeof(Str63)+2];
    tmp[0] = '/';

    if((err = NewAlias( NULL, &FSSpec, &alias )) != noErr) 
	goto put_name_out;

    while(1) {
	GetAliasInfo(alias, (AliasInfoType)x++, str);
	if(!str[0])
	    break;
	strncpy((char *)tmp+1, (const char *)str+1, str[0]);
	tmp[str[0]+1] = '\0';
	retstr.prepend((char *)tmp);
    }

 put_name_out:
    if(use_initial)
	AEDisposeDesc(&initial);
    NavDisposeReply(&ret);
    return retstr;
}

#endif
