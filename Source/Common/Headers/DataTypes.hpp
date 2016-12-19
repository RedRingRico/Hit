#ifndef __HIT_DATATYPES_HPP__
#define __HIT_DATATYPES_HPP__

#if defined( __GNUC__ )
#include <CompilerGCC.hpp>
#else
#error Unknown compiler
#endif // __GNUC__

#if defined( HIT_PLATFORM_LINUX )
#include <DataTypesLinux.hpp>
#else
#error Unknown platform
#endif // HIT_LINUX_X86

typedef size_t	HIT_BOOL;
typedef float	HIT_FLOAT32;
typedef double	HIT_FLOAT64;

typedef size_t	HIT_MEMSIZE;

const HIT_BOOL HIT_TRUE		= 1;
const HIT_BOOL HIT_FALSE	= 0;

#endif // __HIT_DATATYPES_HPP__

