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

#ifndef HANDLEBARS_PRIVATE_H
#define HANDLEBARS_PRIVATE_H

#include <assert.h>

#include "handlebars.h"

HBS_EXTERN_C_START

struct handlebars_context;

#define likely handlebars_likely
#define unlikely handlebars_unlikely

#define CONTEXT context
#define MEMCHK_MSG HANDLEBARS_MEMCHECK_MSG
#define MEMCHKEX(cond, ctx) HANDLEBARS_MEMCHECK(cond, ctx)
#define MEMCHK(cond) MEMCHKEX(cond, CONTEXT)
#define MEMCHKF(ptr) (HBS_TYPEOF(ptr)) handlebars_check(CONTEXT, (void *) (ptr), MEMCHK_MSG)
#define MC(ptr) MEMCHKF(ptr)

// Assert wrapper with side effects, yielding the expression
#if !defined(NDEBUG) && defined(HANDLEBARS_HAVE_STATEMENT_EXPRESSIONS)
#define HBS_ASSERT(expr) ({ __typeof__ (expr) _expr = expr; assert(((void) #expr, _expr)); _expr; })
#else
#define HBS_ASSERT(expr) (expr)
#endif

#define YY_NO_UNISTD_H 1
#define YYLTYPE handlebars_locinfo

HBS_EXTERN_C_END

#endif
