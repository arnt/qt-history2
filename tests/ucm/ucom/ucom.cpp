#include "ucom.h"

void UType::clear( UObject* )
{
}

void UType::copy( UObject *dst, const UObject *src )
{
    //ASSERT( isEqual( src->type, this ) );
    memcpy( dst, src, sizeof(UObject) );
}

// {261D70EB-047D-40B1-BEDE-DAF1EEC273BF}
const UUid TID_UType_Null = { 0x261d70eb, 0x047d, 0x40b1, { 0xbe, 0xde, 0xda, 0xf1, 0xee, 0xc2, 0x73, 0xbf } };
struct UType_Null : public UType
{
    const UUid *uuid() const { return &TID_UType_Null; }
    const char *desc() const { return "null"; }

    bool convertFrom( UObject *, UType * ) { return false; }
    bool convertTo( UObject *, UType * ) { return false; }
};
static UType_Null static_UType_Null;
UType *pUType_Null = &static_UType_Null;


