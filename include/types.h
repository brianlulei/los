#ifndef _TYPES_H_
#define _TYPES_H_

#ifndef NULL
#define NULL ((void*) 0)
#endif


// Explicitly-sized versions of integer types
typedef	signed char			int8_t;
typedef unsigned char		uint8_t;

typedef short				int16_t;
typedef unsigned short		uint16_t;

typedef int					int32_t;
typedef unsigned int		uint32_t;

typedef long long			int64_t;
typedef unsigned long long	uint64_t;

// Pointers and addresses are 32 bits long
// Uintptr_t is to represent the numerical values of virtual addresses.
// physaddr_t is to represent physical addresses.
typedef int32_t				intptr_t;
typedef uint32_t			uintptr_t;
typedef uint32_t			physaddr_t;


#endif
