#ifndef _UNIPORT_TABLES_H
#define _UNIPORT_TABLES_H

/** @file
 *
 * Linker tables
 *
 * Derived from the implementation in iPXE.  See iPXE documentation
 * for the philosophical background and exciting internal details.
 *
 */

#ifdef DOXYGEN
#define __attribute__( x )
#endif

/**
 * Declare a linker table
 *
 * @v type		Data type
 * @v name		Table name
 * @ret table		Linker table
 */
#define __table( type, name ) ( type, name )

/**
 * Get linker table data type
 *
 * @v table		Linker table
 * @ret type		Data type
 */
#define __table_type( table ) __table_extract_type table
#define __table_extract_type( type, name ) type

/**
 * Get linker table name
 *
 * @v table		Linker table
 * @ret name		Table name
 */
#define __table_name( table ) __table_extract_name table
#define __table_extract_name( type, name ) name

/**
 * Get linker table section name
 *
 * @v table		Linker table
 * @v idx		Sub-table index
 * @ret section		Section name
 */
#define __table_section( table, idx ) \
	".tbl." __table_name ( table ) "." __table_str ( idx )
#define __table_str( x ) #x

/**
 * Get linker table alignment
 *
 * @v table		Linker table
 * @ret align		Alignment
 */
#define __table_alignment( table ) __alignof__ ( __table_type ( table ) )

/**
 * Declare a linker table entry
 *
 * @v table		Linker table
 * @v idx		Sub-table index
 *
 * Example usage:
 *
 * @code
 *
 *   #define FROBNICATORS __table ( struct frobnicator, "frobnicators" )
 *
 *   #define __frobnicator __table_entry ( FROBNICATORS, 01 )
 *
 *   struct frobnicator my_frob __frobnicator = {
 *      ...
 *   };
 *
 * @endcode
 */
#define __table_entry( table, idx )					\
	__attribute__ (( __section__ ( __table_section ( table, idx ) ),\
			 __aligned__ ( __table_alignment ( table ) ) ))

/**
 * Get start of linker table entries
 *
 * @v table		Linker table
 * @v idx		Sub-table index
 * @ret entries		Start of entries
 */
#define __table_entries( table, idx ) ( {				\
	static __table_type ( table ) __table_entries[0]		\
		__table_entry ( table, idx ) 				\
		__attribute__ (( unused ));				\
	__table_entries; } )

/**
 * Get start of linker table
 *
 * @v table		Linker table
 * @ret start		Start of linker table
 *
 * Example usage:
 *
 * @code
 *
 *   #define FROBNICATORS __table ( struct frobnicator, "frobnicators" )
 *
 *   struct frobnicator *frobs = table_start ( FROBNICATORS );
 *
 * @endcode
 */
#define table_start( table ) __table_entries ( table, 00 )

/**
 * Get end of linker table
 *
 * @v table		Linker table
 * @ret end		End of linker table
 *
 * Example usage:
 *
 * @code
 *
 *   #define FROBNICATORS __table ( struct frobnicator, "frobnicators" )
 *
 *   struct frobnicator *frobs_end = table_end ( FROBNICATORS );
 *
 * @endcode
 */
#define table_end( table ) __table_entries ( table, 99 )

/**
 * Get number of entries in linker table
 *
 * @v table		Linker table
 * @ret num_entries	Number of entries in linker table
 *
 * Example usage:
 *
 * @code
 *
 *   #define FROBNICATORS __table ( struct frobnicator, "frobnicators" )
 *
 *   unsigned int num_frobs = table_num_entries ( FROBNICATORS );
 *
 * @endcode
 *
 */
#define table_num_entries( table )					\
	( ( unsigned int ) ( table_end ( table ) -			\
			     table_start ( table ) ) )

/**
 * Get index of entry within linker table
 *
 * @v table		Linker table
 * @v entry		Table entry
 *
 * Example usage:
 *
 * @code
 *
 *   #define FROBNICATORS __table ( struct frobnicator, "frobnicators" )
 *
 *   #define __frobnicator __table_entry ( FROBNICATORS, 01 )
 *
 *   struct frobnicator my_frob __frobnicator = {
 *      ...
 *   };
 *
 *   unsigned int my_frob_idx = table_index ( FROBNICATORS, &my_frob );
 *
 * @endcode
 */
#define table_index( table, entry )					\
	( ( unsigned int ) ( (entry) - table_start ( table ) ) )

/**
 * Iterate through all entries within a linker table
 *
 * @v pointer		Entry pointer
 * @v table		Linker table
 *
 * Example usage:
 *
 * @code
 *
 *   #define FROBNICATORS __table ( struct frobnicator, "frobnicators" )
 *
 *   struct frobnicator *frob;
 *
 *   for_each_table_entry ( frob, FROBNICATORS ) {
 *     ...
 *   }
 *
 * @endcode
 *
 */
#define for_each_table_entry( pointer, table )				\
	for ( pointer = table_start ( table ) ;				\
	      pointer < table_end ( table ) ;				\
	      pointer++ )

/**
 * Iterate through all remaining entries within a linker table
 *
 * @v pointer		Entry pointer, preset to most recent entry
 * @v table		Linker table
 *
 * Example usage:
 *
 * @code
 *
 *   #define FROBNICATORS __table ( struct frobnicator, "frobnicators" )
 *   #define __frobnicator __table_entry ( FROBNICATORS, 01 )
 *
 *   struct frob my_frobnicator __frobnicator;
 *   struct frobnicator *frob;
 *
 *   frob = &my_frobnicator;
 *   for_each_table_entry_continue ( frob, FROBNICATORS ) {
 *     ...
 *   }
 *
 * @endcode
 *
 */
#define for_each_table_entry_continue( pointer, table )			\
	for ( pointer++ ;						\
	      pointer < table_end ( table ) ;				\
	      pointer++ )

/**
 * Iterate through all entries within a linker table in reverse order
 *
 * @v pointer		Entry pointer
 * @v table		Linker table
 *
 * Example usage:
 *
 * @code
 *
 *   #define FROBNICATORS __table ( struct frobnicator, "frobnicators" )
 *
 *   struct frobnicator *frob;
 *
 *   for_each_table_entry_reverse ( frob, FROBNICATORS ) {
 *     ...
 *   }
 *
 * @endcode
 *
 */
#define for_each_table_entry_reverse( pointer, table )			\
	for ( pointer = ( table_end ( table ) - 1 ) ;			\
	      pointer >= table_start ( table ) ;			\
	      pointer-- )

/**
 * Iterate through all remaining entries within a linker table in reverse order
 *
 * @v pointer		Entry pointer, preset to most recent entry
 * @v table		Linker table
 *
 * Example usage:
 *
 * @code
 *
 *   #define FROBNICATORS __table ( struct frobnicator, "frobnicators" )
 *   #define __frobnicator __table_entry ( FROBNICATORS, 01 )
 *
 *   struct frob my_frobnicator __frobnicator;
 *   struct frobnicator *frob;
 *
 *   frob = &my_frobnicator;
 *   for_each_table_entry_continue_reverse ( frob, FROBNICATORS ) {
 *     ...
 *   }
 *
 * @endcode
 *
 */
#define for_each_table_entry_continue_reverse( pointer, table )		\
	for ( pointer-- ;						\
	      pointer >= table_start ( table ) ;			\
	      pointer-- )

#endif /* _UNIPORT_TABLES_H */
