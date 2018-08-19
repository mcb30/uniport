#ifndef _COMPILER_H
#define _COMPILER_H

#include <stddef.h>

/**
 * Get containing structure
 *
 * @v ptr		Pointer to contained field
 * @v type		Containing structure type
 * @v field		Field within containing structure
 * @ret container	Pointer to containing structure
 */
#define container_of( ptr, type, field )				\
	( ( type * )							\
	  ( ( ( const volatile void * ) (ptr) ) -			\
	    offsetof ( type, field ) +					\
	    ( ( &( ( ( type * ) NULL )->field ) == ptr ) ? 0 : 0 ) ) )

#endif /* _COMPILER_H */
