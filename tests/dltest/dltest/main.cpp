#include <qlibrary.h>
#include <qapplication.h>
#include <iostream>
#include <dlfcn.h>

typedef void (*InitMe)();

// Either use QLibrary (which currently uses "RTLD_LAZY|RTLD_GLOBAL")
// or use dl* directly with RTDL_LAZY only
#define QLIB 1

int main( int argc, char **argv )
{
    QApplication a( argc, argv );

#ifdef QLIB
    std::cerr << "Using QLibrary" << std::endl;
    QLibrary libp1( "./p1/libp1.so" );
    InitMe p1Init = (InitMe)libp1.resolve( "init_p1" );
    if ( p1Init )
	p1Init();

    // Unloading before loading p2 (works)
    //libp1.unload();

    QLibrary libp2( "./p2/libp2.so" );
    InitMe p2Init = (InitMe)libp2.resolve( "init_p2" );
    if ( p2Init )
	p2Init();

    if ( p1Init )
	p1Init();

    // Unloading after loading p2 (doesn't change anything)
    libp1.unload();

    if ( p2Init )
	p2Init();

#else
    std::cerr << "Using dlopen directly" << std::endl;
    void* libp1 = dlopen( "./p1/libp1.so", RTLD_LAZY );
    if ( !libp1 )
	return 1;
    InitMe p1Init = (InitMe)dlsym( libp1, "init_p1" );
    if ( p1Init )
	p1Init();

    //std::cerr << "Directly pulling in the foo symbol from p1" << std::endl;
    //InitMe p1Foo = (InitMe)dlsym( libp1, "foo__Fv" );
    //if ( p1Foo )
    //    p1Foo();

    void* libp2 = dlopen( "./p2/libp2.so", RTLD_LAZY );
    if ( !libp2 )
	return 2;
    InitMe p2Init = (InitMe)dlsym( libp2, "init_p2" );
    if ( p2Init )
	p2Init();

    //std::cerr << "Directly pulling in the foo symbol from p2" << std::endl;
    //InitMe p2Foo = (InitMe)dlsym( libp2, "foo__Fv" );
    //if ( p2Foo )
    //	  p2Foo();

    dlclose( libp1 );
    dlclose( libp2 );
#endif

    return 0;
}
