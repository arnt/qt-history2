#ifndef UCOM_H
#define UCOM_H

#include <memory.h>

#ifndef UCOM_EXPORT
#define UCOM_EXPORT Q_EXPORT
#endif
#ifndef UCOM_EXPORT
#error "UCOM_EXPORT undefined"
#endif

struct UObject;
struct UInterfaceDescription;

#if defined( Q_WS_MAC ) && defined(check)
#undef check
#endif


//######### TODO useful error codes, prepare for proper exception
//handling
#define URESULT int
#define URESULT_OK 0
#define URESULT_TYPE_MISMATCH -1
#define URESULT_INVALID_ID 5
#define URESULT_FAILED -2


// Universal unique interface ids
struct UCOM_EXPORT UUid
{
    unsigned int   data1;
    unsigned short data2;
    unsigned short data3;
    unsigned char  data4[ 8 ];

    bool operator==( const UUid &uuid ) const {
	return !memcmp( this, &uuid, sizeof( UUid ) );
    }

    bool operator!=( const UUid &uuid ) const {
	return !( *this == uuid );
    }
};

// the mandatory unknown interface
struct UCOM_EXPORT UUnknownInterface
{
    virtual URESULT queryInterface( const UUid&, UUnknownInterface** ) = 0;
    virtual unsigned long addRef() = 0;
    virtual unsigned long release() = 0;
};



// {DE56511E-4E9F-4b76-A3C2-D1E2EF42F1AC} //### number is fake
extern UCOM_EXPORT const UUid IID_UUnknown;
// {DE56512E-4E9F-4b76-A3C2-D1E2EF42F1AC}//### number is fake
extern UCOM_EXPORT const UUid IID_UDispatch;


// the dispatch interface that inherits the unknown interface.. It is
// used to explore interfaces during runtime and to do dynamic calls.
struct UCOM_EXPORT UDispatchInterface : public UUnknownInterface
{
    // returns the interface description of this dispatch interface.
    virtual const UInterfaceDescription* interfaceDescription() const = 0;

    // returns the event description of this dispatch interface.
    virtual const UInterfaceDescription* eventsDescription() const = 0;

    // invokes method id with parameters V*. Returns some sort of
    // exception code.
    virtual URESULT invoke( int id, UObject* o ) = 0;

    // installs listener as event listener
    virtual void installListener( UDispatchInterface* listener ) = 0;

    // remove listener as event listener
    virtual void removeListener( UDispatchInterface* listener ) = 0;
};


// A type for a UObject
struct UCOM_EXPORT UType
{
    virtual const UUid *uuid() const = 0;
    virtual const char *desc() const = 0;


    virtual bool canConvertFrom( UObject *, UType * ) = 0;
    // virtual private, only called by canConvertFrom
    virtual bool canConvertTo( UObject *, UType * ) = 0;


    virtual bool convertFrom( UObject *, UType * ) = 0;
    // virtual private, only called by convertFrom
    virtual bool convertTo( UObject *, UType * ) = 0;

    virtual void clear( UObject * );

    static bool isEqual( const UType *t1, const UType *t2 );
    static bool check( UObject* o, UType* t );
};


// {DE56510E-4E9F-4b76-A3C2-D1E2EF42F1AC}
extern UCOM_EXPORT const UUid TID_UType_Null;
extern UCOM_EXPORT UType *pUType_Null;



// The magic UObject
struct UCOM_EXPORT UObject
{
public: // scary MSVC bug makes this necessary
    UObject() : type( pUType_Null ) {}
    ~UObject() { type->clear( this ); }

    UType *type;

    // the unavoidable union
    union
    {
	bool b;
	
	char c;
	short s;
	int i;
	long l;
	
	unsigned char uc;
	unsigned short us;
	unsigned int ui;
	unsigned long ul;
	
	float f;
	double d;
	
	char byte[16];
	
	struct {
	   char* data;
	   unsigned long size;
	} bytearray;
	
	void* ptr;
	
	struct {
	    void *ptr;
	    bool owner;
	} voidstar;
	
	struct {
	    char *ptr;
	    bool owner;
	} charstar;

	struct {
	    char *ptr;
	    bool owner;
	} utf8;
	
	struct {
	    char *ptr;
	    bool owner;
	} local8bit;
	
	UUnknownInterface* iface;
	UDispatchInterface* idisp;
	
    } payload;

};


// A parameter description describes one method parameters. A
// parameter has a name, a type and a flag describing whether it's an
// in parameter, an out parameter, or both ways
struct UCOM_EXPORT UParameter
{
    const char* name;
    UType *type;
    const void* typeExtra; //type dependend. Usually 0, but UEnum for UType_enum or const char* for UType_ptr
    enum { In = 1, Out = 2, InOut = In | Out };
    int inOut;
};

// A method description describes one method. A method has a name and
// an array of parameters.
struct UCOM_EXPORT UMethod
{
    const char* name;
    int count;
    const UParameter* parameters;
};

// A Property description. Not used yet in the example.
struct UCOM_EXPORT UProperty
{
    const char* name;
    UType* type;
    const void* typeExtra; //type dependend. Usually 0, but UEnum for UType_enum or const char* for UType_ptr

    int set; // -1 undefined
    int get; // -1 undefined

    int designable; // -1 FALSE, -2 TRUE, else method
    int stored; // -1 FALSE, -2 TRUE, else method
};

// An interface description describes one interface, that is all its
// methods and properties.
struct UCOM_EXPORT UInterfaceDescription
{
    int methodCount;
    const UMethod* methods;
    int propertyCount;
    const UProperty* properties;
};


// A component description describe one component, that is its name,
// vendor, release, info, its component uuid and all its interface
// uuids.
struct UCOM_EXPORT UComponentDescription
{
    const char* name;
    const char* vendor;
    const char* release;
    const char* info;
    UUid cid;
    int count;
    const UUid* interfaces;
};


// A component server description describe one component server, that
// is its name, vendor, release, info and the descriptions of all
// components it can instantiate.
struct UCOM_EXPORT UComponentServerDescription
{
    const char* name;
    const char* vendor;
    const char* release;
    const char* info;
    int count;
    const UComponentDescription* components;
};



struct UCOM_EXPORT UEnumItem 				// - a name/value pair
{
    const char *key;
    int value;
};

struct UCOM_EXPORT UEnum
{			
    const char *name;			// - enumerator name
    unsigned int count;			// - number of values
    const UEnumItem *items;		// - the name/value pairs
    bool set;				// whether enum has to be treated as a set
};

inline bool UType::isEqual( const UType *t1, const UType *t2 ) {
    return t1 == t2 ||
   t1->uuid() == t2->uuid() ||
	       !memcmp( t1->uuid(), t2->uuid(), sizeof(UUid) );
}

inline bool UType::check( UObject* o, UType* t ) {
    return isEqual( o->type, t ) ||
	t->convertFrom( o, o->type );
}






// {7EE17B08-5419-47e2-9776-8EEA112DCAEC}
extern UCOM_EXPORT const UUid TID_UType_enum;
struct UCOM_EXPORT UType_enum : public UType
{
    const UUid *uuid() const;
    const char *desc() const;

    void set( UObject *, int );
    int &get( UObject * o ) { return o->payload.i; }
    bool canConvertFrom( UObject *, UType * );
    bool canConvertTo( UObject *, UType * );
    bool convertFrom( UObject *, UType * );
    bool convertTo( UObject *, UType * );
};
extern UCOM_EXPORT UType_enum *pUType_enum;


// {8AC26448-5AB4-49eb-968C-8F30AB13D732}
extern UCOM_EXPORT const UUid TID_UType_ptr;
struct UCOM_EXPORT UType_ptr : public UType
{
    const UUid *uuid() const;
    const char *desc() const;

    void set( UObject *, const void* );
    void* &get( UObject * o ) { return o->payload.ptr; }
    bool canConvertFrom( UObject *, UType * );
    bool canConvertTo( UObject *, UType * );
    bool convertFrom( UObject *, UType * );
    bool convertTo( UObject *, UType * );
};
extern UCOM_EXPORT UType_ptr * pUType_ptr;

// {97A2594D-6496-4402-A11E-55AEF2D4D25C}
extern UCOM_EXPORT const UUid TID_UType_iface;
struct UCOM_EXPORT UType_iface : public UType
{
    const UUid *uuid() const;
    const char *desc() const;

    void set( UObject *, UUnknownInterface* );
    UUnknownInterface* &get( UObject *o ){ return o->payload.iface; }
    bool canConvertFrom( UObject *, UType * );
    bool canConvertTo( UObject *, UType * );
    bool convertFrom( UObject *, UType * );
    bool convertTo( UObject *, UType * );
};
extern UCOM_EXPORT UType_iface * pUType_iface;

// {2F358164-E28F-4bf4-9FA9-4E0CDCABA50B}
extern UCOM_EXPORT const UUid TID_UType_idisp;
struct UCOM_EXPORT UType_idisp : public UType
{
    const UUid *uuid() const;
    const char *desc() const;

    void set( UObject *, UDispatchInterface* );
    UDispatchInterface* &get( UObject *o ){ return o->payload.idisp; }
    bool canConvertFrom( UObject *, UType * );
    bool canConvertTo( UObject *, UType * );
    bool convertFrom( UObject *, UType * );
    bool convertTo( UObject *, UType * );
};
extern UCOM_EXPORT UType_idisp * pUType_idisp;

// {CA42115D-13D0-456c-82B5-FC10187F313E}
extern UCOM_EXPORT const UUid TID_UType_bool;
struct UCOM_EXPORT UType_bool : public UType
{
    const UUid *uuid() const;
    const char *desc() const;

    void set( UObject *, bool );
    bool &get( UObject *o ) { return o->payload.b; }
    bool canConvertFrom( UObject *, UType * );
    bool canConvertTo( UObject *, UType * );
    bool convertFrom( UObject *, UType * );
    bool convertTo( UObject *, UType * );
};
extern UCOM_EXPORT UType_bool * pUType_bool;


// {53C1F3BE-73C3-4c7d-9E05-CCF09EB676B5}
extern UCOM_EXPORT const UUid TID_UType_int;
struct UCOM_EXPORT UType_int : public UType
{
    const UUid *uuid() const;
    const char *desc() const;

    void set( UObject *, int );
    int &get( UObject *o ) { return o->payload.i; }
    bool canConvertFrom( UObject *, UType * );
    bool canConvertTo( UObject *, UType * );
    bool convertFrom( UObject *, UType * );
    bool convertTo( UObject *, UType * );
};
extern UCOM_EXPORT UType_int * pUType_int;


// {2D0974E5-0BA6-4ec2-8837-C198972CB48C}
extern UCOM_EXPORT const UUid TID_UType_double;
struct UCOM_EXPORT UType_double : public UType
{
    const UUid *uuid() const;
    const char *desc() const;

    void set( UObject *, double );
    double &get( UObject *o ) { return o->payload.d; }
    bool canConvertFrom( UObject *, UType * );
    bool canConvertTo( UObject *, UType * );
    bool convertFrom( UObject *, UType * );
    bool convertTo( UObject *, UType * );
};
extern UCOM_EXPORT UType_double * pUType_double;


// {EFCDD1D4-77A3-4b8e-8D46-DC14B8D393E9}
extern UCOM_EXPORT const UUid TID_UType_charstar;
struct UCOM_EXPORT UType_charstar : public UType
{
    const UUid *uuid() const;
    const char *desc() const;

    void set( UObject *, const char*, bool take = false );
    char* get( UObject *o ){ return o->payload.charstar.ptr; }
    bool canConvertFrom( UObject *, UType * );
    bool canConvertTo( UObject *, UType * );
    bool convertFrom( UObject *, UType * );
    bool convertTo( UObject *, UType * );

    void clear( UObject * );
};
extern UCOM_EXPORT UType_charstar * pUType_charstar;



//##### TODO: UType_Utf8 UType_Local8Bit

#endif // UCOM_H
