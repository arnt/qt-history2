#ifndef UCOM_H
#define UCOM_H

#include <memory.h>

struct UObject;
struct UInterfaceDescription;


//######### TODO useful error codes, prepare for proper exception
//handling
#define URESULT int
#define URESULT_OK 0
#define URESULT_TYPE_MISMATCH -1
#define URESULT_INVALID_ID 5


// Universal unique interface ids
struct UUid
{
    unsigned int   data1;
    unsigned short data2;
    unsigned short data3;
    unsigned char  data4[ 8 ];
};

// the mandatory unknown interface
struct UUnknownInterface
{
    virtual UUnknownInterface* queryInterface( const UUid& ) = 0;
    virtual unsigned long addRef() = 0;
    virtual unsigned long release() = 0;
};

//##### TODO: UUid for dispatch interface. UUid for unknown interface


// the dispatch interface that inherits the unknown interface.. It is
// used to explore interfaces during runtime and to do dynamic calls.
struct UDispatchInterface : public UUnknownInterface
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
struct UType
{
    virtual const UUid *uuid() const = 0;
    virtual const char *desc() const = 0;

    virtual bool convertFrom( UObject *, UType * ) = 0;
    // virtual private, only called by convertFrom
    virtual bool convertTo( UObject *, UType * ) = 0;

    virtual void clear( UObject * );

    static bool isEqual( const UType *t1, const UType *t2 );
    static bool check( UObject* o, UType* t );
};


// {261D70EB-047D-40B1-BEDE-DAF1EEC273BF}
extern const UUid TID_UType_Null;
extern UType *pUType_Null;



// The magic UObject
struct UObject
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
struct UParameter
{
    const char* name;
    UType *type;
    const void* typeExtra; //type dependend. Usually 0, but UEnum for UType_enum or const char* for UType_ptr
    enum { In = 1, Out = 2, InOut = In | Out };
    int inOut;
};

// A method description describes one method. A method has a name and
// an array of parameters.
struct UMethod
{
    const char* name;
    int count;
    const UParameter* parameter;
};

// A Property description. Not used yet in the example.
struct UProperty
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
struct UInterfaceDescription
{
    int methodCount;
    const UMethod* methods;
    int propertyCount;
    const UProperty* properties;
};


struct UEnumItem 				// - a name/value pair
{
    const char *key;
    int value;
};

struct UEnum
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







// {261D70EB-047D-40B1-BEDE-DAF1EEC273BF}
extern const UUid TID_UType_enum;
struct UType_enum : public UType
{
    const UUid *uuid() const;
    const char *desc() const;

    void set( UObject *, int );
    int &get( UObject * o ) { return o->payload.i; }
    bool convertFrom( UObject *, UType * );
    bool convertTo( UObject *, UType * );
};
extern UType_enum *pUType_enum;


// {F1D3BE80-2F2F-44F7-AB11-E8A0CEC84B82}
extern const UUid TID_UType_ptr;
struct UType_ptr : public UType
{
    const UUid *uuid() const;
    const char *desc() const;

    void set( UObject *, const void* );
    void* &get( UObject * o ) { return o->payload.ptr; }
    bool convertFrom( UObject *, UType * );
    bool convertTo( UObject *, UType * );
};
extern UType_ptr * pUType_ptr;

// {F2D3BE80-2F2F-44F7-AB11-E8A0CEC84B82}
extern const UUid TID_UType_iface;
struct UType_iface : public UType
{
    const UUid *uuid() const;
    const char *desc() const;

    void set( UObject *, UUnknownInterface* );
    UUnknownInterface* &get( UObject *o ){ return o->payload.iface; }
    bool convertFrom( UObject *, UType * );
    bool convertTo( UObject *, UType * );
};
extern UType_iface * pUType_iface;

// {F3D3BE80-2F2F-44F7-AB11-E8A0CEC84B82}
extern const UUid TID_UType_idisp;
struct UType_idisp : public UType
{
    const UUid *uuid() const;
    const char *desc() const;

    void set( UObject *, UDispatchInterface* );
    UDispatchInterface* &get( UObject *o ){ return o->payload.idisp; }
    bool convertFrom( UObject *, UType * );
    bool convertTo( UObject *, UType * );
};
extern UType_idisp * pUType_idisp;

// {E1D3BE80-2F2F-45F7-AB11-E8A0CEC84B82}
extern const UUid TID_UType_bool;
struct UType_bool : public UType
{
    const UUid *uuid() const;
    const char *desc() const;

    void set( UObject *, bool );
    bool &get( UObject *o ) { return o->payload.b; }
    bool convertFrom( UObject *, UType * );
    bool convertTo( UObject *, UType * );
};
extern UType_bool * pUType_bool;


// {E1D3BE80-2F2F-44F7-AB11-E8A0CEC84B82}
extern const UUid TID_UType_int;
struct UType_int : public UType
{
    const UUid *uuid() const;
    const char *desc() const;

    void set( UObject *, int );
    int &get( UObject *o ) { return o->payload.i; }
    bool convertFrom( UObject *, UType * );
    bool convertTo( UObject *, UType * );
};
extern UType_int * pUType_int;


// {A1D3BE80-2F2F-44F7-AB11-E8A0CEC84B82}
extern const UUid TID_UType_double;
struct UType_double : public UType
{
    const UUid *uuid() const;
    const char *desc() const;

    void set( UObject *, double );
    double &get( UObject *o ) { return o->payload.d; }
    bool convertFrom( UObject *, UType * );
    bool convertTo( UObject *, UType * );
};
extern UType_double * pUType_double;


// {C1D3BE80-2F2F-44F7-AB11-E8A0CEC84B82}
extern const UUid TID_UType_charstar;
struct UType_charstar : public UType
{
    const UUid *uuid() const;
    const char *desc() const;

    void set( UObject *, const char*, bool take = false );
    char* get( UObject *o ){ return o->payload.charstar.ptr; }
    bool convertFrom( UObject *, UType * );
    bool convertTo( UObject *, UType * );

    void clear( UObject * );
};
extern UType_charstar * pUType_charstar;



//##### TODO: UType_Utf8 UType_Local8Bit

#endif // UCOM_H
