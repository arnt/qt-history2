// Empty leaves all features enabled.  See doc/html/features.html for choices.

// Note that disabling some features will produce a libqt that is not
// compatible with other libqt builds. Such modifications are only
// supported on Qt/Embedded where reducing the library size is important
// and where the application-suite is often a fixed set.


#if !defined(QT_DLL) && !defined(QT_NODLL)
#define QT_DLL // Internal
#endif

#define QT_NO_QWS_MACH64
#define QT_NO_QWS_VOODOO3
#define QT_NO_QWS_MATROX

#if !defined(__i386__)
#define QT_NO_QWS_VGA_16
#endif
