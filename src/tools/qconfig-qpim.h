#ifndef QT_H
#endif // QT_H

// Empty leaves all features enabled.  See doc/html/features.html for choices.

// Note that disabling some features will produce a libqt that is not
// compatible with other libqt builds. Such modifications are only
// supported on Qt/Embedded where reducing the library size is important
// and where the application-suite is often a fixed set.

//#define QT_DEMO_LINUX
//#define QT_DEMO_SINGLE_FLOPPY
//#define QT_QWS_IPAQ
#define QT_QWS_CASSIOPEIA

#ifndef QT_DLL
#define QT_DLL // Internal
#endif

#define QT_NO_PROCESS
#if defined(QT_QWS_IPAQ) || defined(QT_QWS_CASSIOPEIA)
# define QT_NO_QWS_CURSOR
#endif
#define QT_NO_CODECS
#define QT_NO_UNICODETABLES
#define QT_NO_IMAGEIO_BMP
#define QT_NO_IMAGEIO_PPM
// defined by build environment
//#define QT_NO_IMAGEIO_JPEG
//#define QT_NO_IMAGEIO_MNG
#define QT_NO_ASYNC_IO
#define QT_NO_ASYNC_IMAGE_IO
#define QT_NO_TRUETYPE
#define QT_NO_BDF
#define QT_NO_FONTDATABASE
#define QT_NO_TRANSLATION
#define QT_NO_DRAGANDDROP
#define QT_NO_CLIPBOARD
#define QT_NO_SOUND
#define QT_NO_PROPERTIES
#define QT_NO_DNS

#define QT_NO_COLORNAMES
#define QT_NO_TRANSFORMATIONS
#define QT_NO_PRINTER
#define QT_NO_PICTURE
//#define QT_NO_ICONVIEW
#define QT_NO_DIAL
#define QT_NO_SIZEGRIP
#define QT_NO_WORKSPACE
//#define QT_NO_TABLE
//#define QT_NO_ACTION
#define QT_NO_STYLE_MOTIF
#define QT_NO_STYLE_PLATINUM
//#define QT_NO_FILEDIALOG
#define QT_NO_FONTDIALOG
#define QT_NO_PRINTDIALOG
//#define QT_NO_COLORDIALOG
//#define QT_NO_INPUTDIALOG
//#define QT_NO_MESSAGEBOX
#define QT_NO_PROGRESSDIALOG
//#define QT_NO_TABDIALOG
#define QT_NO_WIZARD
#define QT_NO_EFFECTS
#define QT_NO_COMPONENT
#define QT_NO_DOM

#define QT_NO_QWS_SAVEFONTS

#if defined(QT_DEMO_LINUX) || defined(QT_DEMO_SINGLE_FLOPPY)
#define QT_NO_QWS_VFB
#define QT_NO_CHECK
#endif

//The new richtext stuff requires networkprotocol

#ifdef QT_DEMO_SINGLE_FLOPPY 
#define QT_NO_QWS_TRANSFORMED
#define QT_NO_NETWORK
#define QT_NO_NETWORKPROTOCOL
#else
#define QT_NO_QWS_MULTIPROCESS
#define QT_NO_NETWORKPROTOCOL_FTP
#define QT_NO_NETWORKPROTOCOL_HTTP
#endif

#ifdef QT_DEMO_SINGLE_FLOPPY // VGA16 is all we need
#define QT_NO_QWS_MACH64
#define QT_NO_QWS_VOODOO3
#define QT_NO_QWS_MATROX
#define QT_NO_QWS_DEPTH_16
#define QT_NO_QWS_DEPTH_32
#else
#define QT_NO_QWS_VGA_16
#endif

// Doesn't compile
#ifndef QT_NO_QWS_VOODOO3
#define QT_NO_QWS_VOODOO3
#endif
