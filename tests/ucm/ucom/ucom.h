#ifndef UCOM_H
#define UCOM_H

#include <memory.h>
#include <qstring.h>

struct UObject;

struct UUid
{
    unsigned int   data1;
    unsigned short data2;
    unsigned short data3;
    unsigned char  data4[ 8 ];
};

struct UType
{
    virtual const UUid *uuid() const = 0;
    virtual const char *desc() const = 0;
    
    virtual bool convertFrom( UObject *, UType * ) = 0;
    // virtual private, only called by convertFrom
    virtual bool convertTo( UObject *, UType * ) = 0;

    virtual void clear( UObject * ) {}
    virtual void copy( UObject *, const UObject * );

    static bool isEqual( const UType *t1, const UType *t2 )
    {
	return t1 == t2 || 
	       t1->uuid() == t2->uuid() ||
	       !memcmp( t1->uuid(), t2->uuid(), sizeof(UUid) );
    }
};

struct UType_Int;
struct UType_Double;

// {261D70EB-047D-40B1-BEDE-DAF1EEC273BF} 
extern const UUid TID_UType_Null;

struct UType_Null : public UType
{
    const UUid *uuid() const { return &TID_UType_Null; }
    const char *desc() const;

    bool convertFrom( UObject *, UType * ) { return false; }
    bool convertTo( UObject *, UType * ) { return false; }
};

extern UType_Null *pUType_Null;

// {E1D3BE80-2F2F-44F7-AB11-E8A0CEC84B82} 
extern const UUid TID_UType_Int;

struct UType_Int : public UType
{
    const UUid *uuid() const { return &TID_UType_Int; }
    const char *desc() const;

    void set( UObject *, int );
    int get( UObject *, bool * = 0 );
    bool convertFrom( UObject *, UType * );
    bool convertTo( UObject *, UType * );
};

extern UType_Int * pUType_Int;

// {A1D3BE80-2F2F-44F7-AB11-E8A0CEC84B82} 
extern const UUid TID_UType_Double;

struct UType_Double : public UType
{
    const UUid *uuid() const { return &TID_UType_Double; }
    const char *desc() const;

    void set( UObject *, double );
    double get( UObject *, bool * = 0 );
    bool convertFrom( UObject *, UType * );
    bool convertTo( UObject *, UType * );
};

extern UType_Double * pUType_Double;

// {C1D3BE80-2F2F-44F7-AB11-E8A0CEC84B82} 
extern const UUid TID_UType_CharStar;

struct UType_CharStar : public UType
{
    const UUid *uuid() const { return &TID_UType_CharStar; }
    const char *desc() const;

    void set( UObject *, const char* );
    char* get( UObject *, bool * = 0 );
    bool convertFrom( UObject *, UType * );
    bool convertTo( UObject *, UType * );

    void clear( UObject * );
};

extern UType_CharStar * pUType_CharStar;

// {B1D3BE80-2F2F-44F7-AB11-E8A0CEC84B82} 
extern const UUid TID_UType_QString;

struct UType_QString : public UType
{
    const UUid *uuid() const { return &TID_UType_QString; }
    const char *desc() const;

    void set( UObject *, const QString & );
    QString get( UObject *, bool * = 0 );

    bool convertFrom( UObject *, UType * );
    bool convertTo( UObject *, UType * );

    void clear( UObject * );
};

extern UType_QString * pUType_QString;

///////////////////////////////////////////
///////////////////////////////////////////

struct UObject
{
    UObject() { type = pUType_Null; }
    ~UObject() { type->clear( this ); }

    UType *type;

    // the unavoidable union
    union
    {
	char b[16]; 
	long l;
	unsigned long ul;
	double d;
	void *ptr;
	struct {
	   unsigned long size;
	   void* ptr;
	} data;
    } payload;
};

#endif // UCOM_H
