#include <stdio.h>
#define UCOM_EXPORT
#include "ucom.cpp"


struct MyInterface : public UDispatchInterface
{
    virtual void getXY( int& x, int& y ) = 0;
    virtual int stringLength( const char* s ) = 0;
    virtual void increment( int& i ) = 0;
};


struct MyInterfaceImpl : public MyInterface
{
    MyInterfaceImpl():ref(0){}
    int ref;
    
    UUnknownInterface* queryInterface( const UUid& u )  { 
	if ( u == IID_UUnknown || u == IID_UDispach ) {
	    addRef();
	    return this;
	}
	return 0; 
    }
    
    unsigned long addRef() { return ++ref; }
    unsigned long release() { if ( --ref == 0 ) {delete this; return 0;} return ref; }
    
    const UInterfaceDescription* interfaceDescription() const {
	static const UParameter ParamGetXY[] = {
	    { "x", pUType_int, 0, UParameter::InOut },
	    { "y", pUType_int, 0, UParameter::InOut }
	};
	static const UParameter ParamStringLength[] = {
	    { 0, pUType_int, 0, UParameter::Out },
	    { "s", pUType_charstar, 0, UParameter::In }
	};
	static const UParameter ParamIncrement[] = {
	    { "i", pUType_int, 0, UParameter::InOut }
	};
	static const UMethod methods[] = {
	    {"getXY", 2,  ParamGetXY},
	    {"stringLength", 2,  ParamStringLength}, 
	    {"increment", 1,  ParamIncrement} 
	};
	static const UInterfaceDescription interface = {
	    3, methods, 
	    0, 0
	};
	return &interface;
    }

    const UInterfaceDescription* eventsDescription() const {
	return 0;
    }

    URESULT invoke( int id, UObject* o ) {
	switch ( id ) {
	case 0: 
	    getXY( pUType_int->get(o+1), pUType_int->get(o+2) ); 
	    break;
	case 1: 
	    pUType_int->set( o, stringLength(pUType_charstar->get(o+1)) ); 
	    break;
	case 2: 
	    increment( pUType_int->get(o+1) ); 
	    break;
	default:
	    return URESULT_INVALID_ID;
	};
	return URESULT_OK;
    }

    void installListener( UDispatchInterface* listener ) {}
    
    void removeListener( UDispatchInterface* listener ) {}
    
    void getXY( int& x, int& y ) {
	x = 42;
	y = 21;
    }
    
    int stringLength( const char* s ) {
	int l = 0;
	while ( *s++ )
	    l++;
	return l;
    }
    
    void increment( int& i ) {
	i++;
    }
};

static const UUid CID_ExampleComponent = // ### bogus fake number
{ 0xde56519e, 0x4e9f, 0x4b76, { 0xa3, 0xc2, 0xd1, 0xe2, 0xef, 0x42, 0xf1, 0xac } };

UComponentServerDescription* ucom_describe()
{
    
    static UUid Interfaces_ExampleComponent[] = {
	IID_UUnknown,
	IID_UDispach
    };
    
    static UComponentDescription components[] = {
	{ "Example Component", "Trolltech AS", "1.0", "An example component", 
	  CID_ExampleComponent, 1, Interfaces_ExampleComponent }
    };
    
    static UComponentServerDescription server = {
	"Example Component Server",
	"Trolltech AS",
	"1.0",
	"Just a little example",
	1,
	components,
    };
    
    return &server;
}

UUnknownInterface* ucom_instantiate( const UUid& cid, const UUid& iid, UUnknownInterface* /*outerUnknown*/ ) 
{
    if ( cid == CID_ExampleComponent ) {
	MyInterfaceImpl* impl = new MyInterfaceImpl;
	return impl->queryInterface( iid );
    }
    return 0;
}



int main( int /*argc*/, char** /*argv*/ )
{
    MyInterfaceImpl impl;
    MyInterface* iface = &impl;
    { 
	UObject o[3];
	iface->invoke( 0, o );
	printf( "Invoke %s returns types %s/%s with value %d/%d\n", 
		iface->interfaceDescription()->methods[0].name,
		iface->interfaceDescription()->methods[0].parameters[0].type->desc(),
		iface->interfaceDescription()->methods[0].parameters[1].type->desc(),
		pUType_int->get( o+1 ),
		pUType_int->get( o+2 ) );
    }
    
    const char* string = "13 characters";
    {
	UObject o[2];
	pUType_charstar->set( o+1, string );
	iface->invoke( 1, o );
	printf("The string '%s' has %d characters\n", string, pUType_int->get(o) );
	
    }
    
    { 
	UObject o[2];
	pUType_int->set( o+1, 41 );
	iface->invoke( 2, o );
	printf( "Invoke %s returns incremented value %d\n",
		iface->interfaceDescription()->methods[2].name,
		pUType_int->get( o+1 ) );
    }

    { 
	UObject o[2];
	pUType_double->set( o+1, 41.1 );
	UType::check( o+1, iface->interfaceDescription()->methods[2].parameters[0].type );
	iface->invoke( 2, o );
	UType::check( o+1, pUType_double );
	printf( "Invoke %s with conversion returns incremented value %f\n",
		iface->interfaceDescription()->methods[2].name,
		pUType_double->get( o+1 ) );
    }

    return 0;
}
