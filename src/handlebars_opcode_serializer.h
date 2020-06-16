/**
 * Copyright (C) 2016 John Boehr
 *
 * This file is part of handlebars.c.
 *
 * handlebars.c is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * handlebars.c is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with handlebars.c.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef HANDLEBARS_OPCODE_SERIALIZER_H
#define HANDLEBARS_OPCODE_SERIALIZER_H

#include <time.h>

#include "handlebars.h"

HBS_EXTERN_C_START

struct handlebars_context;
struct handlebars_program;
struct handlebars_opcode;
struct handlebars_module;

/**
 * @brief Serialize a program into a single buffer. Adds return opcodes and globalizes program IDs.
 * @param[in] context
 * @param[in] program
 * @return The serialized program
 */
struct handlebars_module * handlebars_program_serialize(
    struct handlebars_context * context,
    struct handlebars_program * program
) HBS_ATTR_NONNULL_ALL HBS_ATTR_RETURNS_NONNULL HBS_ATTR_WARN_UNUSED_RESULT;

/**
 * @brief Adjust pointers by the offset between the specified base address and handlebars_module#addr
 * @param[in] module
 * @return void
 */
void handlebars_module_normalize_pointers(
    struct handlebars_module * module,
    void * baseaddr
) HBS_ATTR_NONNULL(1);

/**
 * @brief Fix any pointers by adjusting by the offset between the address of `module` and handlebars_module#addr
 * @param[in] module
 * @return void
 */
void handlebars_module_patch_pointers(
    struct handlebars_module * module
) HBS_ATTR_NONNULL_ALL;

/**
 * @brief Generates, returns, and sets in handlebars_module#hash a hash of the module
 * @param[in] module
 * @return The hash
 */
uint64_t handlebars_module_generate_hash(
    struct handlebars_module * module
) HBS_ATTR_NONNULL_ALL;

/**
 * @brief Verify the module matches its hash and the current handlebars.c version
 * @param[in] ctx
 * @param[in] module
 * @return Whether it matches, unless ctx is set in which it will throw if it does not match
 */
bool handlebars_module_verify(
    struct handlebars_module * module,
    struct handlebars_context * ctx
) HBS_ATTR_NONNULL(1);

size_t handlebars_module_get_size(struct handlebars_module * module) HBS_ATTR_NONNULL_ALL;
int handlebars_module_get_version(struct handlebars_module * module) HBS_ATTR_NONNULL_ALL;
time_t handlebars_module_get_ts(struct handlebars_module * module) HBS_ATTR_NONNULL_ALL;
long handlebars_module_get_flags(struct handlebars_module * module) HBS_ATTR_NONNULL_ALL;
uint64_t handlebars_module_get_hash(struct handlebars_module * module) HBS_ATTR_NONNULL_ALL;

#ifdef HANDLEBARS_OPCODE_SERIALIZER_PRIVATE

/**
 * @brief Program table entry
 */
struct handlebars_module_table_entry
{
    //! The unique ID of this program
    size_t guid;
    //! Number of opcodes
    size_t opcode_count;
    //! Number of child programs
    size_t child_count;
    //! Pointer to opcodes
    struct handlebars_opcode * opcodes;
    //! Pointer to child programs
    struct handlebars_module_table_entry * children;
};

/**
 * @brief Serialized program
 */
struct handlebars_module
{
    //! A hash of everything starting at the following field. May be zero. Set by calling #handlebars_module_generate_hash
    uint64_t hash;

    //! The handlebars version this program was compiled with
    int version;

    //! The original address of the struct used to fix internal pointers
    void * addr;

    //! The size of the struct and all children
    size_t size;

    //! The time at which the struct was created
    time_t ts;

    //! Compiler flags from handlebars_compiler#flags
    long flags;

    //! Number of programs
    size_t program_count;

    //! Array of programs
    struct handlebars_module_table_entry * programs;

    //! Number of opcodes
    size_t opcode_count;

    //! Array of opcodes
    struct handlebars_opcode * opcodes;

    //! Size of data segment
    size_t data_size;

    //! Pointer to the beginning of the data segment
    void * data;
};

#endif /* HANDLEBARS_OPCODE_SERIALIZER_PRIVATE */

HBS_EXTERN_C_END

#endif /* HANDLEBARS_OPCODE_SERIALIZER_H */
