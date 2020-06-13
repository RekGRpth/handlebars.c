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

#ifndef HANDLEBARS_CLOSURE_H
#define HANDLEBARS_CLOSURE_H

#include "handlebars.h"

HBS_EXTERN_C_START

struct handlebars_context;
struct handlebars_module;

// {{{ Reference Counting
void handlebars_closure_addref(struct handlebars_closure * closure)
    HBS_ATTR_NONNULL_ALL;
void handlebars_closure_delref(struct handlebars_closure * closure)
    HBS_ATTR_NONNULL_ALL;
// }}} Reference Counting

struct handlebars_closure * handlebars_closure_ctor(
    struct handlebars_vm * vm,
    struct handlebars_module * module,
    long program,
    long partial_block_depth
) HBS_ATTR_NONNULL_ALL HBS_ATTR_RETURNS_NONNULL HBS_ATTR_WARN_UNUSED_RESULT;

struct handlebars_value * handlebars_closure_call(
    struct handlebars_closure * closure,
    struct handlebars_value * input,
    struct handlebars_value * data,
    struct handlebars_value * block_params,
    struct handlebars_value * rv
) HBS_ATTR_NONNULL(1, 2, 5) HBS_ATTR_RETURNS_NONNULL HBS_ATTR_WARN_UNUSED_RESULT;

long handlebars_closure_get_partial_block_depth(
    struct handlebars_closure * closure
) HBS_ATTR_NONNULL_ALL;

HBS_EXTERN_C_END

#endif /* HANDLEBARS_COMPILER_H */
