#include "qfiledialog.h"

#ifndef QT_NO_FILEDIALOG

#include "qapplication.h"
#include "../kernel/qapplication_p.h"
#include "qt_mac.h"
#include "qregexp.h"
#include "qbuffer.h"
#include "qstringlist.h"
#include "qtextcodec.h"


static Boolean qt_mac_nav_filter(AEDesc *, void *, 
				 void *, NavFilterModes)
{
    return TRUE;
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
    memset(&options, '\0', sizeof(options));
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
    NavGetFile(use_initial ? &initial : NULL, &ret, &options, NULL, NULL, 
	       qt_mac_nav_filter, NULL, (void *)&filter);

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
    memset(&options, '\0', sizeof(options));
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
    NavPutFile(use_initial ? &initial : NULL, &ret, &options, NULL, 
	       'cute', kNavGenericSignature, (void *)&filter);

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
