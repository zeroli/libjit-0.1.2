/*
 * jit-internal.h - Internal definitions for the JIT.
 *
 * Copyright (C) 2004  Southern Storm Software, Pty Ltd.
 *
 * This file is part of the libjit library.
 *
 * The libjit library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * The libjit library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with the libjit library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef	_JIT_INTERNAL_H
#define	_JIT_INTERNAL_H

#include <jit/jit.h>

#ifdef	__cplusplus
extern	"C" {
#endif

/*
#define _JIT_COMPILE_DEBUG	1
*/

/*
 * Determine what kind of Win32 system we are running on.
 */
#if defined(__CYGWIN__) || defined(__CYGWIN32__)
#define	JIT_WIN32_CYGWIN	1
#define	JIT_WIN32_PLATFORM	1
#elif defined(_WIN32) || defined(WIN32)
#define	JIT_WIN32_NATIVE	1
#define	JIT_WIN32_PLATFORM	1
#endif

/*
 * We need the apply rules for "jit_redirector_size".
 */
#include "jit-apply-func.h"

/*
 * Include the thread routines.
 */
#include "jit-thread.h"

/*
 * The following is some macro magic that attempts to detect
 * the best alignment to use on the target platform.  The final
 * value, "JIT_BEST_ALIGNMENT", will be a compile-time constant.
 */

#define	_JIT_ALIGN_CHECK_TYPE(type,name)	\
	struct _JIT_align_##name {		\
		jit_sbyte pad;			\
		type field;			\
	}

#define	_JIT_ALIGN_FOR_TYPE(name)	\
	((jit_nuint)(&(((struct _JIT_align_##name *)0)->field)))

#define	_JIT_ALIGN_MAX(a,b)	\
	((a) > (b) ? (a) : (b))

#define	_JIT_ALIGN_MAX3(a,b,c) \
	(_JIT_ALIGN_MAX((a), _JIT_ALIGN_MAX((b), (c))))

_JIT_ALIGN_CHECK_TYPE(jit_sbyte, sbyte);
_JIT_ALIGN_CHECK_TYPE(jit_short, short);
_JIT_ALIGN_CHECK_TYPE(jit_int, int);
_JIT_ALIGN_CHECK_TYPE(jit_long, long);
_JIT_ALIGN_CHECK_TYPE(jit_ptr, ptr);
_JIT_ALIGN_CHECK_TYPE(jit_float32, float);
_JIT_ALIGN_CHECK_TYPE(jit_float64, double);
_JIT_ALIGN_CHECK_TYPE(jit_nfloat, nfloat);

#if defined(JIT_X86)
/* Sometimes the code below guesses wrong on Win32 platforms */
#define	JIT_BEST_ALIGNMENT	4
#else
#define	JIT_BEST_ALIGNMENT						\
	_JIT_ALIGN_MAX(_JIT_ALIGN_MAX3(_JIT_ALIGN_FOR_TYPE(int),	\
				       _JIT_ALIGN_FOR_TYPE(long),	\
				       _JIT_ALIGN_FOR_TYPE(ptr)),	\
		       _JIT_ALIGN_MAX3(_JIT_ALIGN_FOR_TYPE(float),	\
				       _JIT_ALIGN_FOR_TYPE(double),	\
				       _JIT_ALIGN_FOR_TYPE(nfloat)))
#endif

/*
 * Get the alignment values for various system types.
 * These will also be compile-time constants.
 */
#define	JIT_ALIGN_SBYTE			_JIT_ALIGN_FOR_TYPE(sbyte)
#define	JIT_ALIGN_UBYTE			_JIT_ALIGN_FOR_TYPE(sbyte)
#define	JIT_ALIGN_SHORT			_JIT_ALIGN_FOR_TYPE(short)
#define	JIT_ALIGN_USHORT		_JIT_ALIGN_FOR_TYPE(short)
#define	JIT_ALIGN_CHAR			_JIT_ALIGN_FOR_TYPE(char)
#define	JIT_ALIGN_INT			_JIT_ALIGN_FOR_TYPE(int)
#define	JIT_ALIGN_UINT			_JIT_ALIGN_FOR_TYPE(int)
#define	JIT_ALIGN_NINT			_JIT_ALIGN_FOR_TYPE(ptr)
#define	JIT_ALIGN_NUINT			_JIT_ALIGN_FOR_TYPE(ptr)
#define	JIT_ALIGN_LONG			_JIT_ALIGN_FOR_TYPE(long)
#define	JIT_ALIGN_ULONG			_JIT_ALIGN_FOR_TYPE(long)
#define	JIT_ALIGN_FLOAT32		_JIT_ALIGN_FOR_TYPE(float)
#define	JIT_ALIGN_FLOAT64		_JIT_ALIGN_FOR_TYPE(double)
#define	JIT_ALIGN_NFLOAT		_JIT_ALIGN_FOR_TYPE(nfloat)
#define	JIT_ALIGN_PTR			_JIT_ALIGN_FOR_TYPE(ptr)

/*
 * Structure of a memory pool.
 */
typedef struct jit_pool_block *jit_pool_block_t;
struct jit_pool_block
{
	jit_pool_block_t	next;
	char			data[1];
};
typedef struct
{
	unsigned int		elem_size;
	unsigned int		elems_per_block;
	unsigned int		elems_in_last;
	jit_pool_block_t	blocks;
	void			*free_list;

} jit_memory_pool;

/*
 * Initialize a memory pool.
 */
void _jit_memory_pool_init(jit_memory_pool *pool, unsigned int elem_size);
#define	jit_memory_pool_init(pool,type)	\
			_jit_memory_pool_init((pool), sizeof(type))

/*
 * Free the contents of a memory pool.
 */
void _jit_memory_pool_free(jit_memory_pool *pool, jit_meta_free_func func);
#define	jit_memory_pool_free(pool,func)	_jit_memory_pool_free((pool), (func))

/*
 * Allocate an item from a memory pool.
 */
void *_jit_memory_pool_alloc(jit_memory_pool *pool);
#define	jit_memory_pool_alloc(pool,type)	\
			((type *)_jit_memory_pool_alloc((pool)))

/*
 * Deallocate an item back to a memory pool.
 */
void _jit_memory_pool_dealloc(jit_memory_pool *pool, void *item);
#define	jit_memory_pool_dealloc(pool,item)	\
			(_jit_memory_pool_dealloc((pool), (item)))

/*
 * Storage for metadata.
 */
struct _jit_meta
{
	int			type;
	void			*data;
	jit_meta_free_func	free_data;
	jit_meta_t		next;
	jit_function_t		pool_owner;
};

/*
 * Internal structure of a block.
 */
struct _jit_block
{
	jit_function_t		func;
	jit_label_t		label;
	int			first_insn;
	int			last_insn;
	jit_block_t 		next;
	jit_block_t 		prev;
	jit_meta_t		meta;
	int			entered_via_top : 1;
	int			entered_via_branch : 1;
	int			ends_in_dead : 1;
	void			*address;
	void			*fixup_list;
	void			*fixup_absolute_list;
};

/*
 * Internal structure of a value.
 */
struct _jit_value
{
	jit_block_t		block;
	jit_type_t		type;
	int			is_temporary : 1;
	int			is_local : 1;
	int			is_volatile : 1;
	int			is_addressable : 1;
	int			is_constant : 1;
	int			is_nint_constant : 1;
	int			is_parameter : 1;
	int			is_reg_parameter : 1;
	int			has_address : 1;
	int			free_address : 1;
	int			in_register : 1;
	int			in_frame : 1;
	int			in_global_register : 1;
	int			live : 1;
	int			next_use : 1;
	int			has_frame_offset : 1;
	int			global_candidate : 1;
	int			has_global_register : 1;
	short			reg;
	short			global_reg;
	jit_nint		address;
	jit_nint		frame_offset;
	jit_nuint		usage_count;
	int			index;
};
#define	JIT_INVALID_FRAME_OFFSET	((jit_nint)0x7FFFFFFF)

/*
 * Free the structures that are associated with a value.
 */
void _jit_value_free(void *value);

/*
 * Add references to all of the parameter values in a function.
 * This is used when the initialization block is split during a
 * "jit_insn_move_blocks_to_start" instruction.
 */
void _jit_value_ref_params(jit_function_t func);

/*
 * Internal structure of an instruction.
 */
struct _jit_insn
{
	short			opcode;
	short			flags;
	jit_value_t		dest;
	jit_value_t		value1;
	jit_value_t		value2;
};

/*
 * Instruction flags.
 */
#define	JIT_INSN_DEST_LIVE		0x0001
#define	JIT_INSN_DEST_NEXT_USE		0x0002
#define	JIT_INSN_VALUE1_LIVE		0x0004
#define	JIT_INSN_VALUE1_NEXT_USE	0x0008
#define	JIT_INSN_VALUE2_LIVE		0x0010
#define	JIT_INSN_VALUE2_NEXT_USE	0x0020
#define	JIT_INSN_LIVENESS_FLAGS		0x003F
#define	JIT_INSN_DEST_IS_LABEL		0x0040
#define	JIT_INSN_DEST_IS_FUNCTION	0x0080
#define	JIT_INSN_DEST_IS_NATIVE		0x0100
#define	JIT_INSN_DEST_OTHER_FLAGS	0x01C0
#define	JIT_INSN_VALUE1_IS_NAME		0x0200
#define	JIT_INSN_VALUE1_IS_LABEL	0x0400
#define	JIT_INSN_VALUE1_OTHER_FLAGS	0x0600
#define	JIT_INSN_VALUE2_IS_SIGNATURE	0x0800
#define	JIT_INSN_VALUE2_OTHER_FLAGS	0x0800
#define	JIT_INSN_DEST_IS_VALUE		0x1000

/*
 * Information that is associated with a function for building
 * the instructions and values.  This structure can be discarded
 * once the function has been fully compiled.
 */
typedef struct _jit_builder *jit_builder_t;
struct _jit_builder
{
	/* List of blocks within this function */
	jit_block_t		first_block;
	jit_block_t		last_block;

	/* The next block label to be allocated */
	jit_label_t		next_label;

	/* Mapping from label numbers to blocks */
	jit_block_t		*label_blocks;
	jit_label_t		max_label_blocks;

	/* Entry point for the function */
	jit_block_t		entry;

	/* The current block that is being constructed */
	jit_block_t		current_block;

	/* The position to insert initialization blocks */
	jit_block_t		init_block;
	int			init_insn;

	/* Exception handling definitions for the function */
	jit_value_t		setjmp_value;
	jit_value_t		thrown_exception;
	jit_value_t		thrown_pc;
	jit_label_t		catcher_label;
	jit_value_t		eh_frame_info;

	/* Flag that is set to indicate that this function is not a leaf */
	int			non_leaf : 1;

	/* Flag that indicates if we've seen code that may throw an exception */
	int			may_throw : 1;

	/* Flag that indicates if the function has an ordinary return */
	int			ordinary_return : 1;

	/* Flag that indicates that the current function contains a tail call */
	int			has_tail_call : 1;

	/* Generate position-independent code */
	int			position_independent : 1;

	/* List of all instructions in this function */
	jit_insn_t		*insns;
	int			num_insns;
	int			max_insns;

	/* Memory pools that contain values, instructions, and metadata blocks */
	jit_memory_pool		value_pool;
	jit_memory_pool		insn_pool;
	jit_memory_pool		meta_pool;

	/* Common constants that have been cached */
	jit_value_t		null_constant;
	jit_value_t		zero_constant;

	/* The values for the parameters, structure return, and parent frame */
	jit_value_t		*param_values;
	jit_value_t		struct_return;
	jit_value_t		parent_frame;

	/* Metadata that is stored only while the function is being built */
	jit_meta_t		meta;

	/* Current size of the local variable frame (used by the back end) */
	jit_nint		frame_size;

	/* Number of stack items that are queued for a deferred pop */
	jit_nint		deferred_items;

	/* Size of the outgoing parameter area in the frame */
	jit_nint		param_area_size;

#ifdef _JIT_COMPILE_DEBUG
	int			block_count;
	int			insn_count;
#endif
};

/*
 * Internal structure of a function.
 */
struct _jit_function
{
	/* The context that the function is associated with */
	jit_context_t		context;
	jit_function_t		next;
	jit_function_t		prev;

	/* Containing function in a nested context */
	jit_function_t		nested_parent;

	/* Metadata that survives once the builder is discarded */
	jit_meta_t		meta;

	/* The signature for this function */
	jit_type_t		signature;

	/* The builder information for this function */
	jit_builder_t		builder;

	/* Flag bits for this function */
	int			is_recompilable : 1;
	int			no_throw : 1;
	int			no_return : 1;
	int			has_try : 1;
	int			optimization_level : 8;
	int volatile		is_compiled;

	/* The entry point for the function's compiled code */
	void * volatile		entry_point;

	/* The function to call to perform on-demand compilation */
	jit_on_demand_func	on_demand;

#ifndef JIT_BACKEND_INTERP
# ifdef jit_redirector_size
	/* Buffer that contains the redirector for this function.
	   Redirectors are used to support on-demand compilation */
	unsigned char		*redirector;
# endif

	/* Buffer that contains the indirector for this function.
	   The indirector jumps to the address that is currently
	   stored in the entry_point field. Indirectors are used
	   to support recompilation and on-demand compilation. */
	unsigned char		*indirector;
#endif
};

/*
 * Ensure that there is a builder associated with a function.
 */
int _jit_function_ensure_builder(jit_function_t func);

/*
 * Free the builder associated with a function.
 */
void _jit_function_free_builder(jit_function_t func);

/*
 * Destroy all memory associated with a function.
 */
void _jit_function_destroy(jit_function_t func);

/*
 * Compute value liveness and "next use" information for a function.
 */
void _jit_function_compute_liveness(jit_function_t func);

/*
 * Compile a function on-demand.  Returns the entry point.
 */
void *_jit_function_compile_on_demand(jit_function_t func);

/*
 * Information about a registered external symbol.
 */
typedef struct jit_regsym *jit_regsym_t;
struct jit_regsym
{
	void   *value;
	int		after;
	char	name[1];
};

/*
 * Internal structure of a context.
 */
struct _jit_context
{
	/* Lock that controls access to the building process */
	jit_mutex_t		builder_lock;

	/* Lock that controls access to the function code cache */
	jit_mutex_t		cache_lock;

	/* List of functions that are currently registered with the context */
	jit_function_t		functions;
	jit_function_t		last_function;

	/* Metadata that is associated with the context */
	jit_meta_t		meta;

	/* The context's function code cache */
	struct jit_cache	*cache;

	/* ELF binaries that have been loaded into this context */
	jit_readelf_t		elf_binaries;

	/* Table of symbols that have been registered with this context */
	jit_regsym_t		*registered_symbols;
	int			num_registered_symbols;

	/* Debugger support */
	jit_debugger_hook_func	debug_hook;
	jit_debugger_t		debugger;

	/* On-demand compilation driver */
	jit_on_demand_driver_func	on_demand_driver;
};

/*
 * Backtrace control structure, for managing stack traces.
 * These structures must be allocated on the stack.
 */
typedef struct jit_backtrace *jit_backtrace_t;
struct jit_backtrace
{
	jit_backtrace_t		parent;
	void			*pc;
	void			*security_object;
	jit_meta_free_func	free_security_object;
};

/*
 * Push a new backtrace onto the stack.  The fields in "trace" are filled in.
 */
void _jit_backtrace_push(jit_backtrace_t trace, void *pc);

/*
 * Pop the top-most backtrace item.
 */
void _jit_backtrace_pop(void);

/*
 * Reset the backtrace stack to "trace".  Used in exception catch
 * blocks to fix up the backtrace information.
 */
void _jit_backtrace_set(jit_backtrace_t trace);

/*
 * Control information that is associated with a thread.
 */
struct jit_thread_control
{
	void			*last_exception;
	jit_exception_func	exception_handler;
	jit_backtrace_t		backtrace_head;
	struct jit_jmp_buf	*setjmp_head;
};

/*
 * Get the function code cache for a context, creating it if necessary.
 */
struct jit_cache *_jit_context_get_cache(jit_context_t context);

/*
 * Initialize the block list for a function.
 */
int _jit_block_init(jit_function_t func);

/*
 * Free all blocks that are associated with a function.
 */
void _jit_block_free(jit_function_t func);

/*
 * Create a new block and associate it with a function.
 */
jit_block_t _jit_block_create(jit_function_t func, jit_label_t *label);

/*
 * Record the label mapping for a block.
 */
int _jit_block_record_label(jit_block_t block);

/*
 * Add an instruction to a block.
 */
jit_insn_t _jit_block_add_insn(jit_block_t block);

/*
 * Get the last instruction in a block.  NULL if the block is empty.
 */
jit_insn_t _jit_block_get_last(jit_block_t block);

/*
 * Perform peephole optimization on the branch instruction at the
 * end of a block (if there is a branch).  This will resolve branches
 * to branches, and remove branches to the following block.
 */
void _jit_block_peephole_branch(jit_block_t block);

/*
 * Free one element in a metadata list.
 */
void _jit_meta_free_one(void *meta);

/*
 * Determine if a NULL pointer check is redundant.  The specified
 * iterator is assumed to be positioned one place beyond the
 * "check_null" instruction that we are testing.
 */
int _jit_insn_check_is_redundant(const jit_insn_iter_t *iter);

/*
 * Get the correct opcode to use for a "load" instruction,
 * starting at a particular opcode base.  We assume that the
 * instructions are laid out as "sbyte", "ubyte", "short",
 * "ushort", "int", "long", "float32", "float64", "nfloat",
 * and "struct".
 */
int _jit_load_opcode(int base_opcode, jit_type_t type,
					 jit_value_t value, int no_temps);

/*
 * Get the correct opcode to use for a "store" instruction.
 * We assume that the instructions are laid out as "byte",
 * "short", "int", "long", "float32", "float64", "nfloat",
 * and "struct".
 */
int _jit_store_opcode(int base_opcode, int small_base, jit_type_t type);

/*
 * Function that is called upon each breakpoint location.
 */
void _jit_debugger_hook(jit_function_t func, jit_nint data1, jit_nint data2);

/*
 * Internal structure of a type descriptor.
 */
struct jit_component
{
	jit_type_t		type;
	jit_nuint		offset;
	char			*name;
};
struct _jit_type
{
	unsigned int		ref_count;
	int			kind : 19;
	int			abi : 8;
	int			is_fixed : 1;
	int			layout_flags : 4;
	jit_nuint		size;
	jit_nuint		alignment;
	jit_type_t		sub_type;
	unsigned int		num_components;
	struct jit_component	components[1];
};
struct jit_tagged_type
{
	struct _jit_type	type;
	void			*data;
	jit_meta_free_func	free_func;

};

/*
 * Pre-defined type descriptors.
 */
extern struct _jit_type const _jit_type_void_def;
extern struct _jit_type const _jit_type_sbyte_def;
extern struct _jit_type const _jit_type_ubyte_def;
extern struct _jit_type const _jit_type_short_def;
extern struct _jit_type const _jit_type_ushort_def;
extern struct _jit_type const _jit_type_int_def;
extern struct _jit_type const _jit_type_uint_def;
extern struct _jit_type const _jit_type_nint_def;
extern struct _jit_type const _jit_type_nuint_def;
extern struct _jit_type const _jit_type_long_def;
extern struct _jit_type const _jit_type_ulong_def;
extern struct _jit_type const _jit_type_float32_def;
extern struct _jit_type const _jit_type_float64_def;
extern struct _jit_type const _jit_type_nfloat_def;
extern struct _jit_type const _jit_type_void_ptr_def;

/*
 * Extra call flags for internal use.
 */
#define	JIT_CALL_NATIVE		(1 << 14)

#ifdef JIT_USE_SIGNALS

/*
 * Initialize the signal handlers.
 */
void _jit_signal_init(void);

#endif

#ifdef	__cplusplus
};
#endif

#endif	/* _JIT_INTERNAL_H */
