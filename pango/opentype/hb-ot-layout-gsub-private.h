/*
 * Copyright (C) 2007,2008,2009  Red Hat, Inc.
 *
 *  This is part of HarfBuzz, an OpenType Layout engine library.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and its documentation for any purpose, provided that the
 * above copyright notice and the following two paragraphs appear in
 * all copies of this software.
 *
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES
 * ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN
 * IF THE COPYRIGHT HOLDER HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 * THE COPYRIGHT HOLDER SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING,
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE COPYRIGHT HOLDER HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 *
 * Red Hat Author(s): Behdad Esfahbod
 */

#ifndef HB_OT_LAYOUT_GSUB_PRIVATE_H
#define HB_OT_LAYOUT_GSUB_PRIVATE_H

#include "hb-ot-layout-gsubgpos-private.h"


struct SingleSubstFormat1
{
  friend struct SingleSubst;

  private:
  inline bool apply (APPLY_ARG_DEF) const
  {
    hb_codepoint_t glyph_id = IN_CURGLYPH ();
    unsigned int index = (this+coverage) (glyph_id);
    if (HB_LIKELY (index == NOT_COVERED))
      return false;

    glyph_id += deltaGlyphID;
    _hb_buffer_replace_glyph (buffer, glyph_id);

    if ( _hb_ot_layout_has_new_glyph_classes (layout) )
    {
      /* we inherit the old glyph class to the substituted glyph */
      _hb_ot_layout_set_glyph_property (layout, glyph_id, property);
    }

    return true;
  }

  private:
  USHORT	format;			/* Format identifier--format = 1 */
  OffsetTo<Coverage>
		coverage;		/* Offset to Coverage table--from
					 * beginning of Substitution table */
  SHORT		deltaGlyphID;		/* Add to original GlyphID to get
					 * substitute GlyphID */
};
ASSERT_SIZE (SingleSubstFormat1, 6);

struct SingleSubstFormat2
{
  friend struct SingleSubst;

  private:
  inline bool apply (APPLY_ARG_DEF) const
  {
    hb_codepoint_t glyph_id = IN_CURGLYPH ();
    unsigned int index = (this+coverage) (glyph_id);
    if (HB_LIKELY (index == NOT_COVERED))
      return false;

    if (HB_UNLIKELY (index >= substitute.len))
      return false;

    glyph_id = substitute[index];
    _hb_buffer_replace_glyph (buffer, glyph_id);

    if ( _hb_ot_layout_has_new_glyph_classes (layout) )
    {
      /* we inherit the old glyph class to the substituted glyph */
      _hb_ot_layout_set_glyph_property (layout, glyph_id, property);
    }

    return true;
  }

  private:
  USHORT	format;			/* Format identifier--format = 2 */
  OffsetTo<Coverage>
		coverage;		/* Offset to Coverage table--from
					 * beginning of Substitution table */
  ArrayOf<GlyphID>
		substitute;		/* Array of substitute
					 * GlyphIDs--ordered by Coverage Index */
};
ASSERT_SIZE (SingleSubstFormat2, 6);

struct SingleSubst
{
  friend struct SubstLookupSubTable;

  private:
  inline bool apply (APPLY_ARG_DEF) const
  {
    switch (u.format) {
    case 1: return u.format1->apply (APPLY_ARG);
    case 2: return u.format2->apply (APPLY_ARG);
    default:return false;
    }
  }

  private:
  union {
  USHORT		format;		/* Format identifier */
  SingleSubstFormat1	format1[];
  SingleSubstFormat2	format2[];
  } u;
};
ASSERT_SIZE (SingleSubst, 2);


struct Sequence
{
  friend struct MultipleSubstFormat1;

  private:
  inline bool apply (APPLY_ARG_DEF) const
  {
    if (HB_UNLIKELY (!substitute.len))
      return false;

    _hb_buffer_add_output_glyphs (buffer, 1,
				  substitute.len, (const uint16_t *) substitute.array,
				  0xFFFF, 0xFFFF);

    if ( _hb_ot_layout_has_new_glyph_classes (layout) )
    {
      /* this is a guess only ... */
      if ( property == HB_OT_LAYOUT_GLYPH_CLASS_LIGATURE )
        property = HB_OT_LAYOUT_GLYPH_CLASS_BASE_GLYPH;

      unsigned int count = substitute.len;
      for (unsigned int n = 0; n < count; n++)
	_hb_ot_layout_set_glyph_property (layout, substitute[n], property);
    }

    return true;
  }

  private:
  ArrayOf<GlyphID>
		substitute;		/* String of GlyphIDs to substitute */
};
ASSERT_SIZE (Sequence, 2);

struct MultipleSubstFormat1
{
  friend struct MultipleSubst;

  private:
  inline bool apply (APPLY_ARG_DEF) const
  {

    unsigned int index = (this+coverage) (IN_CURGLYPH ());
    if (HB_LIKELY (index == NOT_COVERED))
      return false;

    return (this+sequence[index]).apply (APPLY_ARG);
  }

  private:
  USHORT	format;			/* Format identifier--format = 1 */
  OffsetTo<Coverage>
		coverage;		/* Offset to Coverage table--from
					 * beginning of Substitution table */
  OffsetArrayOf<Sequence>
		sequence;		/* Array of Sequence tables
					 * ordered by Coverage Index */
};
ASSERT_SIZE (MultipleSubstFormat1, 6);

struct MultipleSubst
{
  friend struct SubstLookupSubTable;

  private:
  inline bool apply (APPLY_ARG_DEF) const
  {
    switch (u.format) {
    case 1: return u.format1->apply (APPLY_ARG);
    default:return false;
    }
  }

  private:
  union {
  USHORT		format;		/* Format identifier */
  MultipleSubstFormat1	format1[];
  } u;
};
ASSERT_SIZE (MultipleSubst, 2);


typedef ArrayOf<GlyphID> AlternateSet;	/* Array of alternate GlyphIDs--in
					 * arbitrary order */
ASSERT_SIZE (AlternateSet, 2);

struct AlternateSubstFormat1
{
  friend struct AlternateSubst;

  private:
  inline bool apply (APPLY_ARG_DEF) const
  {
    hb_codepoint_t glyph_id = IN_CURGLYPH ();

    unsigned int index = (this+coverage) (glyph_id);
    if (HB_LIKELY (index == NOT_COVERED))
      return false;

    const AlternateSet &alt_set = this+alternateSet[index];

    if (HB_UNLIKELY (!alt_set.len))
      return false;

    unsigned int alt_index = 0;

    /* XXX callback to user to choose alternate
    if (layout->altfunc)
      alt_index = (layout->altfunc)(layout, buffer,
				    buffer->out_pos, glyph_id,
				    alt_set.len, alt_set.array);
				   */

    if (HB_UNLIKELY (alt_index >= alt_set.len))
      return false;

    glyph_id = alt_set[alt_index];

    _hb_buffer_replace_glyph (buffer, glyph_id);

    if ( _hb_ot_layout_has_new_glyph_classes (layout) )
    {
      /* we inherit the old glyph class to the substituted glyph */
      _hb_ot_layout_set_glyph_property (layout, glyph_id, property);
    }

    return true;
  }

  private:
  USHORT	format;			/* Format identifier--format = 1 */
  OffsetTo<Coverage>
		coverage;		/* Offset to Coverage table--from
					 * beginning of Substitution table */
  OffsetArrayOf<AlternateSet>
		alternateSet;		/* Array of AlternateSet tables
					 * ordered by Coverage Index */
};
ASSERT_SIZE (AlternateSubstFormat1, 6);

struct AlternateSubst
{
  friend struct SubstLookupSubTable;

  private:
  inline bool apply (APPLY_ARG_DEF) const
  {
    switch (u.format) {
    case 1: return u.format1->apply (APPLY_ARG);
    default:return false;
    }
  }

  private:
  union {
  USHORT		format;		/* Format identifier */
  AlternateSubstFormat1	format1[];
  } u;
};
ASSERT_SIZE (AlternateSubst, 2);


struct Ligature
{
  friend struct LigatureSet;

  private:
  inline bool apply (APPLY_ARG_DEF, bool is_mark) const
  {
    unsigned int i, j;
    unsigned int count = component.len;
    unsigned int end = MIN (buffer->in_length, buffer->in_pos + context_length);
    if (HB_UNLIKELY (buffer->in_pos + count > end))
      return false;

    for (i = 1, j = buffer->in_pos + 1; i < count; i++, j++)
    {
      while (!_hb_ot_layout_check_glyph_property (layout, IN_INFO (j), lookup_flag, &property))
      {
	if (HB_UNLIKELY (j + count - i == end))
	  return false;
	j++;
      }

      if (!(property & HB_OT_LAYOUT_GLYPH_CLASS_MARK))
	is_mark = FALSE;

      if (HB_LIKELY (IN_GLYPH(j) != component[i]))
        return false;
    }
    if ( _hb_ot_layout_has_new_glyph_classes (layout) )
      /* this is just a guess ... */
      hb_ot_layout_set_glyph_class (layout, ligGlyph,
				    is_mark ? HB_OT_LAYOUT_GLYPH_CLASS_MARK
					    : HB_OT_LAYOUT_GLYPH_CLASS_LIGATURE);

    if (j == buffer->in_pos + i) /* No input glyphs skipped */
      /* We don't use a new ligature ID if there are no skipped
	 glyphs and the ligature already has an ID. */
      _hb_buffer_add_output_glyphs (buffer, i,
				    1, (const uint16_t *) &ligGlyph,
				    0xFFFF,
				    IN_LIGID (buffer->in_pos) ?
				    0xFFFF : _hb_buffer_allocate_lig_id (buffer));
    else
    {
      unsigned int lig_id = _hb_buffer_allocate_lig_id (buffer);
      _hb_buffer_add_output_glyph (buffer, ligGlyph, 0xFFFF, lig_id);

      /* Now we must do a second loop to copy the skipped glyphs to
	 `out' and assign component values to it.  We start with the
	 glyph after the first component.  Glyphs between component
	 i and i+1 belong to component i.  Together with the lig_id
	 value it is later possible to check whether a specific
	 component value really belongs to a given ligature. */

      for ( i = 1; i < count; i++ )
      {
	while (!_hb_ot_layout_check_glyph_property (layout, IN_CURINFO(), lookup_flag, &property))
	  _hb_buffer_add_output_glyph (buffer, IN_CURGLYPH(), i - 1, lig_id);

	(buffer->in_pos)++;
      }

      /* TODO We should possibly reassign lig_id and component for any
       * components of a previous ligature that s now being removed as part of
       * this ligature. */
    }

    return true;
  }

  private:
  GlyphID	ligGlyph;		/* GlyphID of ligature to substitute */
  HeadlessArrayOf<GlyphID>
		component;		/* Array of component GlyphIDs--start
					 * with the second  component--ordered
					 * in writing direction */
};
ASSERT_SIZE (Ligature, 4);

struct LigatureSet
{
  friend struct LigatureSubstFormat1;

  private:
  inline bool apply (APPLY_ARG_DEF, bool is_mark) const
  {
    unsigned int num_ligs = ligature.len;
    for (unsigned int i = 0; i < num_ligs; i++)
    {
      const Ligature &lig = this+ligature[i];
      if (lig.apply (APPLY_ARG, is_mark))
        return true;
    }

    return false;
  }

  private:
  OffsetArrayOf<Ligature>
		ligature;		/* Array LigatureSet tables
					 * ordered by preference */
};
ASSERT_SIZE (LigatureSet, 2);

struct LigatureSubstFormat1
{
  friend struct LigatureSubst;

  private:
  inline bool apply (APPLY_ARG_DEF) const
  {
    hb_codepoint_t glyph_id = IN_CURGLYPH ();

    bool first_is_mark = !!(property & HB_OT_LAYOUT_GLYPH_CLASS_MARK);

    unsigned int index = (this+coverage) (glyph_id);
    if (HB_LIKELY (index == NOT_COVERED))
      return false;

    const LigatureSet &lig_set = this+ligatureSet[index];
    return lig_set.apply (APPLY_ARG, first_is_mark);
  }

  private:
  USHORT	format;			/* Format identifier--format = 1 */
  OffsetTo<Coverage>
		coverage;		/* Offset to Coverage table--from
					 * beginning of Substitution table */
  OffsetArrayOf<LigatureSet>
		ligatureSet;		/* Array LigatureSet tables
					 * ordered by Coverage Index */
};
ASSERT_SIZE (LigatureSubstFormat1, 6);

struct LigatureSubst
{
  friend struct SubstLookupSubTable;

  private:
  inline bool apply (APPLY_ARG_DEF) const
  {
    switch (u.format) {
    case 1: return u.format1->apply (APPLY_ARG);
    default:return false;
    }
  }

  private:
  union {
  USHORT		format;		/* Format identifier */
  LigatureSubstFormat1	format1[];
  } u;
};
ASSERT_SIZE (LigatureSubst, 2);



static inline bool substitute_lookup (APPLY_ARG_DEF, unsigned int lookup_index);

struct ContextSubst : Context
{
  friend struct SubstLookupSubTable;

  private:
  inline bool apply (APPLY_ARG_DEF) const
  { return Context::apply (APPLY_ARG, substitute_lookup); }
};
ASSERT_SIZE (ContextSubst, 2);

struct ChainContextSubst : ChainContext
{
  friend struct SubstLookupSubTable;

  private:
  inline bool apply (APPLY_ARG_DEF) const
  { return ChainContext::apply (APPLY_ARG, substitute_lookup); }
};
ASSERT_SIZE (ChainContextSubst, 2);


struct ExtensionSubst : Extension
{
  friend struct SubstLookupSubTable;

  private:
  inline bool apply (APPLY_ARG_DEF) const;
};
ASSERT_SIZE (ExtensionSubst, 2);


struct ReverseChainSingleSubstFormat1
{
  friend struct ReverseChainSingleSubst;

  private:
  inline bool apply (APPLY_ARG_DEF) const
  {
    if (HB_UNLIKELY (context_length != NO_CONTEXT))
      return false; /* No chaining to this type */

    unsigned int index = (this+coverage) (IN_CURGLYPH ());
    if (HB_LIKELY (index == NOT_COVERED))
      return false;

    const OffsetArrayOf<Coverage> &lookahead = (const OffsetArrayOf<Coverage>&)
					       *((const char *) &backtrack + backtrack.get_size ());
    const ArrayOf<GlyphID> &substitute = (const ArrayOf<GlyphID>&)
					 *((const char *) &lookahead + lookahead.get_size ());

    if (match_backtrack (APPLY_ARG,
			 backtrack.len, (USHORT *) backtrack.array,
			 match_coverage, (char *) this) &&
        match_lookahead (APPLY_ARG,
			 lookahead.len, (USHORT *) lookahead.array,
			 match_coverage, (char *) this,
			 1))
    {
      IN_CURGLYPH() = substitute[index];
      buffer->in_pos--; /* Reverse! */
      return true;
    }

    return false;
  }

  private:
  USHORT	format;			/* Format identifier--format = 1 */
  OffsetTo<Coverage>
		coverage;		/* Offset to Coverage table--from
					 * beginning of table */
  OffsetArrayOf<Coverage>
		backtrack;		/* Array of coverage tables
					 * in backtracking sequence, in  glyph
					 * sequence order */
  OffsetArrayOf<Coverage>
		lookaheadX;		/* Array of coverage tables
					 * in lookahead sequence, in glyph
					 * sequence order */
  ArrayOf<GlyphID>
		substituteX;		/* Array of substitute
					 * GlyphIDs--ordered by Coverage Index */
};
ASSERT_SIZE (ReverseChainSingleSubstFormat1, 10);

struct ReverseChainSingleSubst
{
  friend struct SubstLookupSubTable;

  private:
  inline bool apply (APPLY_ARG_DEF) const
  {
    switch (u.format) {
    case 1: return u.format1->apply (APPLY_ARG);
    default:return false;
    }
  }

  private:
  union {
  USHORT				format;		/* Format identifier */
  ReverseChainSingleSubstFormat1	format1[];
  } u;
};
ASSERT_SIZE (ReverseChainSingleSubst, 2);



/*
 * SubstLookup
 */

struct SubstLookupSubTable
{
  friend struct SubstLookup;

  enum {
    Single		= 1,
    Multiple		= 2,
    Alternate		= 3,
    Ligature		= 4,
    Context		= 5,
    ChainContext	= 6,
    Extension		= 7,
    ReverseChainSingle	= 8,
  };

  bool apply (APPLY_ARG_DEF, unsigned int lookup_type) const
  {
    switch (lookup_type) {
    case Single:		return u.single->apply (APPLY_ARG);
    case Multiple:		return u.multiple->apply (APPLY_ARG);
    case Alternate:		return u.alternate->apply (APPLY_ARG);
    case Ligature:		return u.ligature->apply (APPLY_ARG);
    case Context:		return u.context->apply (APPLY_ARG);
    case ChainContext:		return u.chainContext->apply (APPLY_ARG);
    case Extension:		return u.extension->apply (APPLY_ARG);
    case ReverseChainSingle:	return u.reverseChainContextSingle->apply (APPLY_ARG);
    default:return false;
    }
  }

  private:
  union {
  USHORT			format;
  SingleSubst			single[];
  MultipleSubst			multiple[];
  AlternateSubst		alternate[];
  LigatureSubst			ligature[];
  ContextSubst			context[];
  ChainContextSubst		chainContext[];
  ExtensionSubst		extension[];
  ReverseChainSingleSubst	reverseChainContextSingle[];
  } u;
};
ASSERT_SIZE (SubstLookupSubTable, 2);


struct SubstLookup : Lookup
{
  inline const SubstLookupSubTable& get_subtable (unsigned int i) const
  { return (const SubstLookupSubTable&) Lookup::get_subtable (i); }

  /* Like get_type(), but looks through extension lookups.
   * Never returns Extension */
  inline unsigned int get_effective_type (void) const
  {
    unsigned int type = get_type ();

    if (HB_UNLIKELY (type == SubstLookupSubTable::Extension))
    {
      unsigned int count = get_subtable_count ();
      type = get_subtable(0).u.extension->get_type ();
      /* The spec says all subtables should have the same type.
       * This is specially important if one has a reverse type! */
      for (unsigned int i = 1; i < count; i++)
        if (get_subtable(i).u.extension->get_type () != type)
	  return 0;
    }

    return type;
  }

  inline bool is_reverse (void) const
  { return HB_UNLIKELY (get_effective_type () == SubstLookupSubTable::ReverseChainSingle); }

  inline bool apply_once (hb_ot_layout_t *layout,
			  hb_buffer_t    *buffer,
			  unsigned int    context_length,
			  unsigned int    nesting_level_left) const
  {
    unsigned int lookup_type = get_type ();
    unsigned int lookup_flag = get_flag ();
    unsigned int property;

    if (!_hb_ot_layout_check_glyph_property (layout, IN_CURINFO (), lookup_flag, &property))
      return false;

    for (unsigned int i = 0; i < get_subtable_count (); i++)
      if (get_subtable (i).apply (APPLY_ARG, lookup_type))
	return true;

    return false;
  }

  bool apply_string (hb_ot_layout_t *layout,
		     hb_buffer_t    *buffer,
		     hb_ot_layout_feature_mask_t mask) const
  {
    bool ret = false;

    if (HB_UNLIKELY (!buffer->in_length))
      return false;

    if (HB_LIKELY (!is_reverse ()))
    {
	/* in/out forward substitution */
	_hb_buffer_clear_output (buffer);
	buffer->in_pos = 0;
	while (buffer->in_pos < buffer->in_length)
	{
	  if ((~IN_PROPERTIES (buffer->in_pos) & mask) &&
	      apply_once (layout, buffer, NO_CONTEXT, MAX_NESTING_LEVEL))
	    ret = true;
	  else
	    _hb_buffer_next_glyph (buffer);

	}
	if (ret)
	  _hb_buffer_swap (buffer);

    }
    else
    {

	/* in-place backward substitution */
	buffer->in_pos = buffer->in_length - 1;
	do
	{
	  if ((~IN_PROPERTIES (buffer->in_pos) & mask) &&
	      apply_once (layout, buffer, NO_CONTEXT, MAX_NESTING_LEVEL))
	    ret = true;
	  else
	    buffer->in_pos--;

	}
	while ((int) buffer->in_pos >= 0);
    }

    return ret;
  }
};
ASSERT_SIZE (SubstLookup, 6);


/*
 * GSUB
 */

struct GSUB : GSUBGPOS
{
  static const hb_tag_t Tag		= HB_TAG ('G','S','U','B');

  static inline const GSUB& get_for_data (const char *data)
  { return (const GSUB&) GSUBGPOS::get_for_data (data); }

  inline const SubstLookup& get_lookup (unsigned int i) const
  { return (const SubstLookup&) GSUBGPOS::get_lookup (i); }

  inline bool substitute_lookup (hb_ot_layout_t *layout,
				 hb_buffer_t    *buffer,
			         unsigned int    lookup_index,
				 hb_ot_layout_feature_mask_t  mask) const
  { return get_lookup (lookup_index).apply_string (layout, buffer, mask); }

};
ASSERT_SIZE (GSUB, 10);


/* Out-of-class implementation for methods recursing */

inline bool ExtensionSubst::apply (APPLY_ARG_DEF) const
{
  unsigned int lookup_type = get_type ();

  if (HB_UNLIKELY (lookup_type == SubstLookupSubTable::Extension))
    return false;

  return ((SubstLookupSubTable&) get_subtable ()).apply (APPLY_ARG, lookup_type);
}

static inline bool substitute_lookup (APPLY_ARG_DEF, unsigned int lookup_index)
{
  const GSUB &gsub = *(layout->gsub);
  const SubstLookup &l = gsub.get_lookup (lookup_index);

  if (HB_UNLIKELY (nesting_level_left == 0))
    return false;
  nesting_level_left--;

  if (HB_UNLIKELY (context_length < 1))
    return false;

  return l.apply_once (layout, buffer, context_length, nesting_level_left);
}


#endif /* HB_OT_LAYOUT_GSUB_PRIVATE_H */
