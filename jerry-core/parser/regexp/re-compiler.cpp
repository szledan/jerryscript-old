/* Copyright 2014-2015 Samsung Electronics Co., Ltd.
 * Copyright 2015 University of Szeged.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ecma-exceptions.h"
#include "ecma-helpers.h"
#include "ecma-try-catch-macro.h"
#include "jrt-libc-includes.h"
#include "mem-heap.h"
#include "re-compiler.h"
#include "stdio.h"

#define REGEXP_BYTECODE_BLOCK_SIZE 256UL
#define BYTECODE_LEN(bc_ctx_p) (static_cast<uint32_t> (bc_ctx_p->current_p - bc_ctx_p->block_start_p))

void
regexp_dump_bytecode (re_bytecode_ctx_t *bc_ctx);

static re_bytecode_t*
realloc_regexp_bytecode_block (re_bytecode_ctx_t *bc_ctx_p)
{
  JERRY_ASSERT (bc_ctx_p->block_end_p - bc_ctx_p->block_start_p >= 0);
  size_t old_size = static_cast<size_t> (bc_ctx_p->block_end_p - bc_ctx_p->block_start_p);
  JERRY_ASSERT (!bc_ctx_p->current_p && !bc_ctx_p->block_end_p && !bc_ctx_p->block_start_p);

  size_t new_block_size = old_size + REGEXP_BYTECODE_BLOCK_SIZE;
  JERRY_ASSERT (bc_ctx_p->current_p - bc_ctx_p->block_start_p >= 0);
  size_t current_ptr_offset = static_cast<size_t> (bc_ctx_p->current_p - bc_ctx_p->block_start_p);

  re_bytecode_t *new_block_start_p = (re_bytecode_t *) mem_heap_alloc_block (new_block_size,
                                                                                     MEM_HEAP_ALLOC_SHORT_TERM);
  if (bc_ctx_p->current_p)
  {
    memcpy (new_block_start_p, bc_ctx_p->block_start_p, static_cast<size_t> (current_ptr_offset));
    mem_heap_free_block (bc_ctx_p->block_start_p);
  }
  bc_ctx_p->block_start_p = new_block_start_p;
  bc_ctx_p->block_end_p = new_block_start_p + new_block_size;
  bc_ctx_p->current_p = new_block_start_p + current_ptr_offset;

  return bc_ctx_p->current_p;
} /* realloc_regexp_bytecode_block */

static void
bytecode_list_append (re_bytecode_ctx_t *bc_ctx_p, re_bytecode_t bytecode)
{
  re_bytecode_t *current_p = bc_ctx_p->current_p;
  if (current_p  + sizeof (re_bytecode_t) > bc_ctx_p->block_end_p)
  {
    current_p = realloc_regexp_bytecode_block (bc_ctx_p);
  }

  *current_p = bytecode;
  bc_ctx_p->current_p += sizeof (re_bytecode_t);
} /* bytecode_list_append */

static void
bytecode_list_insert (re_bytecode_ctx_t *bc_ctx_p, re_bytecode_t bytecode, size_t offset)
{
  re_bytecode_t *current_p = bc_ctx_p->current_p;
  if (current_p  + sizeof (re_bytecode_t) > bc_ctx_p->block_end_p)
  {
    realloc_regexp_bytecode_block (bc_ctx_p);
  }

  re_bytecode_t *src_p = bc_ctx_p->block_start_p + offset;
  if ((BYTECODE_LEN (bc_ctx_p) - offset) > 0)
  {
    re_bytecode_t *dest_p = src_p + sizeof (re_bytecode_t);
    re_bytecode_t *tmp_block_start_p = (re_bytecode_t *) mem_heap_alloc_block ((BYTECODE_LEN (bc_ctx_p) - offset),
                                                                                       MEM_HEAP_ALLOC_SHORT_TERM);
    memcpy (tmp_block_start_p, src_p, (size_t) (BYTECODE_LEN (bc_ctx_p) - offset));
    memcpy (dest_p, tmp_block_start_p, (size_t) (BYTECODE_LEN (bc_ctx_p) - offset));
    mem_heap_free_block (tmp_block_start_p);
  }
  *src_p = bytecode;
  bc_ctx_p->current_p += sizeof (re_bytecode_t);
} /* bytecode_list_insert  */

static void
append_opcode (re_bytecode_ctx_t *bc_ctx_p,
               re_opcode_t opcode)
{
  bytecode_list_append (bc_ctx_p, (re_bytecode_t) opcode);
}

static void
append_u32 (re_bytecode_ctx_t *bc_ctx_p,
            uint32_t value)
{
  bytecode_list_append (bc_ctx_p, (re_bytecode_t) value);
}

static void
append_jump_offset (re_bytecode_ctx_t *bc_ctx_p,
                    uint32_t value)
{
  value += static_cast<uint32_t> (sizeof (re_bytecode_t));
  bytecode_list_append (bc_ctx_p, (re_bytecode_t) value);
}

static void
insert_opcode (re_bytecode_ctx_t *bc_ctx_p,
               uint32_t offset,
               re_opcode_t opcode)
{
  bytecode_list_insert (bc_ctx_p, (re_bytecode_t) opcode, offset);
}

static void
insert_u32 (re_bytecode_ctx_t *bc_ctx_p,
            uint32_t offset,
            uint32_t value)
{
  bytecode_list_insert (bc_ctx_p, (re_bytecode_t) value, offset);
}

re_opcode_t
get_opcode (re_bytecode_t **bc_p)
{
  re_bytecode_t bytecode = **bc_p;
  (*bc_p)++;
  return (re_opcode_t) bytecode;
}

uint32_t
get_value (re_bytecode_t **bc_p)
{
  /* FIXME: Read 32bit! */
  re_bytecode_t bytecode = **bc_p;
  (*bc_p)++;
  return (uint32_t) bytecode;
}

/**
 * Insert simple atom iterator
 */
static void
insert_simple_iterator (re_compiler_ctx_t *re_ctx_p, /**< RegExp compiler context */
                        uint32_t new_atom_start_offset) /**< atom start offset */
{
  uint32_t atom_code_length;
  uint32_t offset;
  uint32_t qmin, qmax;

  qmin = re_ctx_p->current_token.qmin;
  qmax = re_ctx_p->current_token.qmax;
  JERRY_ASSERT (qmin <= qmax);

  /* FIXME: optimize bytecode lenght. Store 0 rather than INF */

  append_opcode (re_ctx_p->bytecode_ctx_p, RE_OP_MATCH);   /* complete 'sub atom' */
  uint32_t bytecode_length = BYTECODE_LEN (re_ctx_p->bytecode_ctx_p);
  atom_code_length = (uint32_t) (bytecode_length - new_atom_start_offset);

  offset = new_atom_start_offset;
  insert_u32 (re_ctx_p->bytecode_ctx_p, offset, atom_code_length);
  insert_u32 (re_ctx_p->bytecode_ctx_p, offset, qmax);
  insert_u32 (re_ctx_p->bytecode_ctx_p, offset, qmin);
  if (re_ctx_p->current_token.greedy)
  {
    insert_opcode (re_ctx_p->bytecode_ctx_p, offset, RE_OP_GREEDY_ITERATOR);
  }
  else
  {
    insert_opcode (re_ctx_p->bytecode_ctx_p, offset, RE_OP_NON_GREEDY_ITERATOR);
  }
} /* insert_simple_iterator */

/**
 * Get the type of a group start
 */
static re_opcode_t
get_start_opcode_type (re_compiler_ctx_t *re_ctx_p, /**< RegExp compiler context */
                       bool is_capturable) /**< is capturabel group */
{
  if (is_capturable)
  {
    if (re_ctx_p->current_token.qmin == 0)
    {
      if (re_ctx_p->current_token.greedy)
      {
        return RE_OP_CAPTURE_GREEDY_ZERO_GROUP_START;
      }
      else
      {
        return RE_OP_CAPTURE_NON_GREEDY_ZERO_GROUP_START;
      }
    }
    else
    {
      return RE_OP_CAPTURE_GROUP_START;
    }
  }
  else
  {
    if (re_ctx_p->current_token.qmin == 0)
    {
      if (re_ctx_p->current_token.greedy)
      {
        return RE_OP_NON_CAPTURE_GREEDY_ZERO_GROUP_START;
      }
      else
      {
        return RE_OP_NON_CAPTURE_NON_GREEDY_ZERO_GROUP_START;
      }
    }
    else
    {
      return RE_OP_NON_CAPTURE_GROUP_START;
    }
  }

  JERRY_UNREACHABLE ();
  return 0;
} /* get_start_opcode_type */

/**
 * Get the type of a group end
 */
static re_opcode_t
get_end_opcode_type (re_compiler_ctx_t *re_ctx_p, /**< RegExp compiler context */
                     bool is_capturable) /**< is capturabel group */
{
  if (is_capturable)
  {
    if (re_ctx_p->current_token.greedy)
    {
      return RE_OP_CAPTURE_GREEDY_GROUP_END;
    }
    return RE_OP_CAPTURE_NON_GREEDY_GROUP_END;
  }
  else
  {
    if (re_ctx_p->current_token.greedy)
    {
      return RE_OP_NON_CAPTURE_GREEDY_GROUP_END;
    }
    return RE_OP_NON_CAPTURE_NON_GREEDY_GROUP_END;
  }

  JERRY_UNREACHABLE ();
  return 0;
} /* get_end_opcode_type */

static void
insert_into_group (re_compiler_ctx_t *re_ctx_p, /**< RegExp compiler context */
                   uint32_t group_start_offset, /**< offset of group start */
                   uint32_t idx, /**< index of group */
                   bool is_capturable) /**< is capturabel group */
{
  uint32_t qmin, qmax;
  re_opcode_t start_opcode = get_start_opcode_type (re_ctx_p, is_capturable);
  re_opcode_t end_opcode = get_end_opcode_type (re_ctx_p, is_capturable);
  uint32_t start_head_offset_len;

  qmin = re_ctx_p->current_token.qmin;
  qmax = re_ctx_p->current_token.qmax;
  JERRY_ASSERT (qmin <= qmax);

  start_head_offset_len = (uint32_t) BYTECODE_LEN (re_ctx_p->bytecode_ctx_p);
  insert_u32 (re_ctx_p->bytecode_ctx_p, group_start_offset, idx);
  insert_u32 (re_ctx_p->bytecode_ctx_p, group_start_offset, start_opcode);
  start_head_offset_len = (uint32_t) BYTECODE_LEN (re_ctx_p->bytecode_ctx_p) - start_head_offset_len;
  append_opcode (re_ctx_p->bytecode_ctx_p, end_opcode);
  append_u32 (re_ctx_p->bytecode_ctx_p, idx);
  append_u32 (re_ctx_p->bytecode_ctx_p, qmin);
  append_u32 (re_ctx_p->bytecode_ctx_p, qmax);

  group_start_offset += start_head_offset_len;
  append_jump_offset (re_ctx_p->bytecode_ctx_p,
                      (uint32_t) BYTECODE_LEN (re_ctx_p->bytecode_ctx_p) - group_start_offset);

  if (start_opcode != RE_OP_CAPTURE_GROUP_START && start_opcode != RE_OP_NON_CAPTURE_GROUP_START)
  {
    insert_u32 (re_ctx_p->bytecode_ctx_p,
                group_start_offset,
                (uint32_t) BYTECODE_LEN (re_ctx_p->bytecode_ctx_p) - group_start_offset);
  }
} /* insert_into_group */

/**
 * Parse alternatives
 */
static ecma_completion_value_t
parse_alternative (re_compiler_ctx_t *re_ctx_p, /**< RegExp compiler context */
                   bool expect_eof) /**< expect end of file */
{
  uint32_t idx;
  re_bytecode_ctx_t *bc_ctx_p = re_ctx_p->bytecode_ctx_p;

  uint32_t alterantive_offset = BYTECODE_LEN (re_ctx_p->bytecode_ctx_p);

  if (re_ctx_p->recursion_depth >= RE_COMPILE_RECURSION_LIMIT)
  {
    return ecma_make_throw_obj_completion_value (ecma_new_standard_error (ECMA_ERROR_RANGE));
  }
  re_ctx_p->recursion_depth++;

  while (true)
  {
    re_ctx_p->current_token = re_parse_next_token (re_ctx_p->parser_ctx_p);
    uint32_t new_atom_start_offset = BYTECODE_LEN (re_ctx_p->bytecode_ctx_p);

    switch (re_ctx_p->current_token.type)
    {
      case RE_TOK_START_CAPTURE_GROUP:
      {
        idx = re_ctx_p->num_of_captures++;
        JERRY_DDLOG ("Compile a capture group start (idx: %d)\n", idx);

        parse_alternative (re_ctx_p, false);
        insert_into_group (re_ctx_p, new_atom_start_offset, idx, true);
        break;
      }
      case RE_TOK_START_NON_CAPTURE_GROUP:
      {
        idx = re_ctx_p->num_of_non_captures++;
        JERRY_DDLOG ("Compile a non-capture group start (idx: %d)\n", idx);

        parse_alternative (re_ctx_p, false);
        insert_into_group (re_ctx_p, new_atom_start_offset, idx, false);
        break;
      }
      case RE_TOK_CHAR:
      {
        JERRY_DDLOG ("Compile character token: %c, qmin: %d, qmax: %d\n",
                     re_ctx_p->current_token.value, re_ctx_p->current_token.qmin, re_ctx_p->current_token.qmax);

        append_opcode (bc_ctx_p, RE_OP_CHAR);
        append_u32 (bc_ctx_p, re_ctx_p->current_token.value);

        if ((re_ctx_p->current_token.qmin != 1) || (re_ctx_p->current_token.qmax != 1))
        {
          insert_simple_iterator (re_ctx_p, new_atom_start_offset);
        }
        break;
      }
      case RE_TOK_PERIOD:
      {
        JERRY_DDLOG ("Compile a period\n");
        append_opcode (bc_ctx_p, RE_OP_PERIOD);

        if ((re_ctx_p->current_token.qmin != 1) || (re_ctx_p->current_token.qmax != 1))
        {
          insert_simple_iterator (re_ctx_p, new_atom_start_offset);
        }
        break;
      }
      case RE_TOK_ALTERNATIVE:
      {
        JERRY_DDLOG ("Compile an alternative\n");
        insert_u32 (bc_ctx_p, alterantive_offset, BYTECODE_LEN (bc_ctx_p) - alterantive_offset);
        append_opcode (bc_ctx_p, RE_OP_ALTERNATIVE);
        alterantive_offset = BYTECODE_LEN (re_ctx_p->bytecode_ctx_p);
        break;
      }
      case RE_TOK_END_GROUP:
      {
        JERRY_DDLOG ("Compile a group end\n");

        if (expect_eof)
        {
          JERRY_ERROR_MSG ("Unexpected end of paren.\n");
          return ecma_make_throw_obj_completion_value (ecma_new_standard_error (ECMA_ERROR_SYNTAX));
        }

        insert_u32 (bc_ctx_p, alterantive_offset, BYTECODE_LEN (bc_ctx_p) - alterantive_offset);
        re_ctx_p->recursion_depth--;
        return ecma_make_empty_completion_value ();
      }
      case RE_TOK_EOF:
      {
        if (!expect_eof)
        {
          JERRY_ERROR_MSG ("Unexpected end of pattern.\n");
          return ecma_make_throw_obj_completion_value (ecma_new_standard_error (ECMA_ERROR_SYNTAX));
        }

        insert_u32 (bc_ctx_p, alterantive_offset, BYTECODE_LEN (bc_ctx_p) - alterantive_offset);
        re_ctx_p->recursion_depth--;
        return ecma_make_empty_completion_value ();
      }
      default:
      {
        JERRY_ERROR_MSG ("Unexpected RegExp token.\n");
        return ecma_make_throw_obj_completion_value (ecma_new_standard_error (ECMA_ERROR_SYNTAX));
      }
    }
  }
  JERRY_UNREACHABLE ();
} /* parse_alternative */

/**
 * Parse RegExp flags (global, ignoreCase, multiline)
 */
static ecma_completion_value_t
parse_regexp_flags (re_compiler_ctx_t *re_ctx_p, /**< RegExp compiler context */
                    ecma_string_t *flags) /**< flags */
{
  ecma_completion_value_t ret_value = ecma_make_empty_completion_value ();

  int32_t chars = ecma_string_get_length (flags);
  MEM_DEFINE_LOCAL_ARRAY (flags_start_p, chars + 1, ecma_char_t);
  ssize_t zt_str_size = (ssize_t) sizeof (ecma_char_t) * (chars + 1);
  ecma_string_to_zt_string (flags, flags_start_p, zt_str_size);

  ecma_char_t *flags_p = flags_start_p;
  while (flags_p
         && *flags_p != '\0'
         && ecma_is_completion_value_empty (ret_value))
  {
    ecma_char_t ch = *flags_p;
    switch (ch)
    {
      case 'g':
      {
        if (re_ctx_p->flags & RE_FLAG_GLOBAL)
        {
          JERRY_ERROR_MSG ("Invalid RegExp flags.\n");
          ret_value = ecma_make_throw_obj_completion_value (ecma_new_standard_error (ECMA_ERROR_SYNTAX));
        }
        re_ctx_p->flags |= RE_FLAG_GLOBAL;
        break;
      }
      case 'i':
      {
        if (re_ctx_p->flags & RE_FLAG_IGNORE_CASE)
        {
          JERRY_ERROR_MSG ("Invalid RegExp flags.\n");
          ret_value = ecma_make_throw_obj_completion_value (ecma_new_standard_error (ECMA_ERROR_SYNTAX));
        }
        re_ctx_p->flags |= RE_FLAG_IGNORE_CASE;
        break;
      }
      case 'm':
      {
        if (re_ctx_p->flags & RE_FLAG_MULTILINE)
        {
          JERRY_ERROR_MSG ("Invalid RegExp flags.\n");
          ret_value = ecma_make_throw_obj_completion_value (ecma_new_standard_error (ECMA_ERROR_SYNTAX));
        }
        re_ctx_p->flags |= RE_FLAG_MULTILINE;
        break;
      }
      default:
      {
        JERRY_ERROR_MSG ("Invalid RegExp flags.\n");
        ret_value = ecma_make_throw_obj_completion_value (ecma_new_standard_error (ECMA_ERROR_SYNTAX));
        break;
      }
    }
    flags_p++;
  }

  MEM_FINALIZE_LOCAL_ARRAY (flags_start_p);

  return ret_value;
} /* parse_regexp_flags  */

/**
 * Compilation of RegExp bytecode
 */
ecma_completion_value_t
regexp_compile_bytecode (ecma_property_t *bytecode, /**< bytecode */
                         ecma_string_t *pattern, /**< pattern */
                         ecma_string_t *flags) /**< flags */
{
  ecma_completion_value_t ret_value = ecma_make_empty_completion_value ();
  re_compiler_ctx_t re_ctx;
  re_ctx.flags = 0;
  re_ctx.num_of_non_captures = 0;
  re_ctx.recursion_depth = 0;

  if (flags != NULL)
  {
    ECMA_TRY_CATCH (empty, parse_regexp_flags (&re_ctx, flags), ret_value);
    ECMA_FINALIZE (empty);

    if (!ecma_is_completion_value_empty (ret_value))
    {
      return ret_value;
    }
  }

  re_bytecode_ctx_t bc_ctx;
  bc_ctx.block_start_p = NULL;
  bc_ctx.block_end_p = NULL;
  bc_ctx.current_p = NULL;

  re_ctx.bytecode_ctx_p = &bc_ctx;

  int32_t chars = ecma_string_get_length (pattern);
  MEM_DEFINE_LOCAL_ARRAY (pattern_start_p, chars + 1, ecma_char_t);
  ssize_t zt_str_size = (ssize_t) sizeof (ecma_char_t) * (chars + 1);
  ecma_string_to_zt_string (pattern, pattern_start_p, zt_str_size);

  re_parser_ctx_t parser_ctx;
  parser_ctx.pattern_start_p = pattern_start_p;
  parser_ctx.current_char_p = pattern_start_p;
  parser_ctx.number_of_groups = -1;
  re_ctx.parser_ctx_p = &parser_ctx;

  /* 1. Parse RegExp pattern */
  re_ctx.num_of_captures = 1;
  append_opcode (&bc_ctx, RE_OP_SAVE_AT_START);

  ECMA_TRY_CATCH (empty, parse_alternative (&re_ctx, true), ret_value);
  ECMA_FINALIZE (empty);

  append_opcode (&bc_ctx, RE_OP_SAVE_AND_MATCH);
  append_opcode (&bc_ctx, RE_OP_EOF);

  /* 2. Insert extra informations for bytecode header */
  insert_u32 (&bc_ctx, 0, (uint32_t) re_ctx.num_of_non_captures);
  insert_u32 (&bc_ctx, 0, (uint32_t) re_ctx.num_of_captures * 2);
  insert_u32 (&bc_ctx, 0, (uint32_t) re_ctx.flags);

  MEM_FINALIZE_LOCAL_ARRAY (pattern_start_p);

  ECMA_SET_POINTER (bytecode->u.internal_property.value, bc_ctx.block_start_p);

#ifdef JERRY_ENABLE_LOG
  regexp_dump_bytecode (&bc_ctx);
#endif

  return ret_value;
} /* regexp_compile_bytecode */

#ifdef JERRY_ENABLE_LOG
/**
 * RegExp bytecode dumper
 */
void
regexp_dump_bytecode (re_bytecode_ctx_t *bc_ctx_p)
{
  re_bytecode_t *bytecode_p = bc_ctx_p->block_start_p;
  JERRY_DLOG ("%d ", get_value (&bytecode_p));
  JERRY_DLOG ("%d ", get_value (&bytecode_p));
  JERRY_DLOG ("%d | ", get_value (&bytecode_p));

  re_opcode_t op;
  while ((op = get_opcode (&bytecode_p)))
  {
    switch (op)
    {
      case RE_OP_MATCH:
      {
        JERRY_DLOG ("MATCH, ");
        break;
      }
      case RE_OP_CHAR:
      {
        JERRY_DLOG ("CHAR ");
        JERRY_DLOG ("%c, ", (char) get_value (&bytecode_p));
        break;
      }
      case RE_OP_CAPTURE_NON_GREEDY_ZERO_GROUP_START:
      {
        JERRY_DLOG ("N");
        /* FALLTHRU */
      }
      case RE_OP_CAPTURE_GREEDY_ZERO_GROUP_START:
      {
        JERRY_DLOG ("GZ_START ");
        JERRY_DLOG ("%d ", get_value (&bytecode_p));
        JERRY_DLOG ("%d ", get_value (&bytecode_p));
        JERRY_DLOG ("%d, ", get_value (&bytecode_p));
        break;
      }
      case RE_OP_CAPTURE_GROUP_START:
      {
        JERRY_DLOG ("START ");
        JERRY_DLOG ("%d ", get_value (&bytecode_p));
        JERRY_DLOG ("%d, ", get_value (&bytecode_p));
        break;
      }
      case RE_OP_CAPTURE_NON_GREEDY_GROUP_END:
      {
        JERRY_DLOG ("N");
        /* FALLTHRU */
      }
      case RE_OP_CAPTURE_GREEDY_GROUP_END:
      {
        JERRY_DLOG ("G_END ");
        JERRY_DLOG ("%d ", get_value (&bytecode_p));
        JERRY_DLOG ("%d ", get_value (&bytecode_p));
        JERRY_DLOG ("%d ", get_value (&bytecode_p));
        JERRY_DLOG ("%d, ", get_value (&bytecode_p));
        break;
      }
      case RE_OP_NON_CAPTURE_NON_GREEDY_ZERO_GROUP_START:
      {
        JERRY_DLOG ("N");
        /* FALLTHRU */
      }
      case RE_OP_NON_CAPTURE_GREEDY_ZERO_GROUP_START:
      {
        JERRY_DLOG ("GZ__START ");
        JERRY_DLOG ("%d ", get_value (&bytecode_p));
        JERRY_DLOG ("%d ", get_value (&bytecode_p));
        JERRY_DLOG ("%d, ", get_value (&bytecode_p));
        break;
      }
      case RE_OP_NON_CAPTURE_GROUP_START:
      {
        JERRY_DLOG ("NC_START ");
        JERRY_DLOG ("%d ", get_value (&bytecode_p));
        JERRY_DLOG ("%d, ", get_value (&bytecode_p));
        break;
      }
      case RE_OP_NON_CAPTURE_NON_GREEDY_GROUP_END:
      {
        JERRY_DLOG ("N");
        /* FALLTHRU */
      }
      case RE_OP_NON_CAPTURE_GREEDY_GROUP_END:
      {
        JERRY_DLOG ("G_NC_END ");
        JERRY_DLOG ("%d ", get_value (&bytecode_p));
        JERRY_DLOG ("%d ", get_value (&bytecode_p));
        JERRY_DLOG ("%d ", get_value (&bytecode_p));
        JERRY_DLOG ("%d, ", get_value (&bytecode_p));
        break;
      }
      case RE_OP_SAVE_AT_START:
      {
        JERRY_DLOG ("RE_START ");
        JERRY_DLOG ("%d, ", get_value (&bytecode_p));
        break;
      }
      case RE_OP_SAVE_AND_MATCH:
      {
        JERRY_DLOG ("RE_END, ");
        break;
      }
      case RE_OP_GREEDY_ITERATOR:
      {
        JERRY_DLOG ("RE_OP_GREEDY_ITERATOR ");
        JERRY_DLOG ("%d ", get_value (&bytecode_p));
        JERRY_DLOG ("%d ", get_value (&bytecode_p));
        JERRY_DLOG ("%d, ", get_value (&bytecode_p));
        break;
      }
      case RE_OP_NON_GREEDY_ITERATOR:
      {
        JERRY_DLOG ("RE_OP_NON_GREEDY_ITERATOR ");
        JERRY_DLOG ("%d, ", get_value (&bytecode_p));
        JERRY_DLOG ("%d, ", get_value (&bytecode_p));
        JERRY_DLOG ("%d, ", get_value (&bytecode_p));
        break;
      }
      case RE_OP_PERIOD:
      {
        JERRY_DLOG ("RE_OP_PERIOD ");
        break;
      }
      case RE_OP_ALTERNATIVE:
      {
        JERRY_DLOG ("RE_OP_ALTERNATIVE ");
        JERRY_DLOG ("%d, ", get_value (&bytecode_p));
        break;
      }
      default:
      {
        JERRY_DLOG ("UNKNOWN(%d), ", (uint32_t) op);
        break;
      }
    }
  }
  JERRY_DLOG ("EOF\n");
} /* regexp_dump_bytecode */
#endif
