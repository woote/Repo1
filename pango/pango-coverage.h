/* Pango
 * pango-coverage.h: Coverage sets for fonts
 *
 * Copyright (C) 2000 Red Hat Software
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __PANGO_COVERAGE_H__
#define __PANGO_COVERAGE_H__

#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct _PangoCoverage PangoCoverage;

typedef enum {
  PANGO_COVERAGE_NONE,
  PANGO_COVERAGE_FALLBACK,
  PANGO_COVERAGE_APPROXIMATE,
  PANGO_COVERAGE_EXACT
} PangoCoverageLevel;

PangoCoverage *    pango_coverage_new     (void);
PangoCoverage *    pango_coverage_copy    (PangoCoverage      *coverage);
void               pango_coverage_destroy (PangoCoverage      *coverage);
PangoCoverageLevel pango_coverage_get     (PangoCoverage      *coverage,
					   int                 index);
void               pango_coverage_set     (PangoCoverage      *coverage,
					   int                 index,
					   PangoCoverageLevel  level);
void               pango_coverage_max     (PangoCoverage      *coverage,
					   PangoCoverage      *other);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif __PANGO_COVERAGE_H__