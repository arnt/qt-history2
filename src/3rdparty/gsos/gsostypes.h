/*
 *              Copyright (C) 2000  Sony Computer Entertainment Inc.
 *                              All Rights Reserved.
 */

#ifndef GSOSTYPES_H
#define GSOSTYPES_H

#ifdef __cplusplus
extern "C" {
#endif

typedef int                GSOSint ;
typedef unsigned int       GSOSuint ;
typedef short              GSOSshort ;
typedef unsigned short     GSOSushort ;
typedef char               GSOSchar ;
typedef unsigned char      GSOSuchar ;
typedef long               GSOSlong ;
typedef unsigned long      GSOSulong ;
typedef float              GSOSfloat ;
typedef double             GSOSdouble ;
typedef unsigned long long GSOSbit64 ;

typedef union {
    GSOSbit64 ul64[2];
    GSOSuint  ui32[4];
} GSOSpacket ;

#ifdef __cplusplus
}
#endif

#endif /* GSOSTYPES_H */
