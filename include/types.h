#ifndef _TYPES_H_
#define _TYPES_H_

#ifndef NULL
#define NULL ((void*) 0)
#endif

typedef enum{false, true} bool;

// Explicitly-sized versions of integer types
typedef	signed char			int8_t;
typedef unsigned char		uint8_t;

typedef short				int16_t;
typedef unsigned short		uint16_t;

typedef int					int32_t;
typedef unsigned int		uint32_t;

typedef long long			int64_t;
typedef unsigned long long	uint64_t;

// size_t is used for memory object sizes.
typedef	uint32_t			size_t;


// Pointers and addresses are 32 bits long
// Uintptr_t is to represent the numerical values of virtual addresses.
// physaddr_t is to represent physical addresses.
typedef int32_t				intptr_t;
typedef uint32_t			uintptr_t;
typedef uint32_t			physaddr_t;

// size_t is used for memory object sizes
typedef uint32_t			size_t;

// ssize_t is a signed version of ssize_t, used in case there might be an
// error return.
typedef int32_t				ssize_t;

// off_t is used for file offsets and lengths.
typedef int32_t				off_t;

// Round up to the nearest multiple of n
// typeof declares y with the type of what x points to.
#define ROUNDDOWN(a, n)					\
({										\
	uint32_t __a = (uint32_t) (a);		\
	(typeof(a)) (__a - __a % (n));		\
})

// Round up to the nearest multiple of n
#define ROUNDUP(a, n)					\
({										\
	uint32_t __n = (uint32_t) (n);		\
	(typeof (a)) (ROUNDDOWN((uint32_t) (a) + __n - 1, __n));	\
})
#endif
