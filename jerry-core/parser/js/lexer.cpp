/* Copyright 2015 Samsung Electronics Co., Ltd.
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

#include "mem-allocator.h"
#include "jrt-libc-includes.h"
#include "lexer.h"
#include "parser.h"
#include "stack.h"
#include "opcodes.h"
#include "syntax-errors.h"
#include "parser.h"
#include "ecma-helpers.h"

static token saved_token, prev_token, sent_token, empty_token;

static bool allow_dump_lines = false, strict_mode;
static size_t buffer_size = 0;

/* Represents the contents of a script.  */
static const char *buffer_start = nullptr;
static const char *buffer = nullptr;
static const char *token_start;
static ecma_char_t *strings_cache = nullptr;
static size_t strings_cache_size = 0;
static size_t strings_cache_used_size = 0;

#define LA(I)       (get_char (I))

enum
{
  literals_global_size
};
STATIC_STACK (literals, literal)
literal *literals_data = nullptr;
size_t literals_count = 0;

static bool
is_empty (token tok)
{
  return tok.type == TOK_EMPTY;
}

static locus
current_locus (void)
{
  if (token_start == NULL)
  {
    return (locus) (buffer - buffer_start);
  }
  else
  {
    return (locus) (token_start - buffer_start);
  }
}

static char
get_char (size_t i)
{
  if ((buffer + i) >= (buffer_start + buffer_size))
  {
    return '\0';
  }
  return *(buffer + i);
}

static void
dump_current_line (void)
{
  const char *i;

  if (!allow_dump_lines)
  {
    return;
  }

  printf ("// ");

  for (i = buffer; *i != '\n' && *i != 0; i++)
  {
    putchar (*i);
  }
  putchar ('\n');
}

static token
create_token (token_type type, literal_index_t uid)
{
  token ret;

  ret.type = type;
  ret.loc = current_locus () - (type == TOK_STRING ? 1 : 0);
  ret.uid = uid;

  return ret;
}

/**
 * Compare specified string to literal
 *
 * @return true - if the literal contains exactly the specified string,
 *         false - otherwise.
 */
static bool
string_equals_to_literal (const ecma_char_t *str_p, /**< characters buffer */
                          ecma_length_t length, /**< string's length */
                          literal lit) /**< literal */
{
  if (lit.type == LIT_STR)
  {
    if (lit.data.lp.length == length
        && strncmp ((const char *) lit.data.lp.str, (const char*) str_p, length) == 0)
    {
      return true;
    }
  }
  else if (lit.type == LIT_MAGIC_STR)
  {
    const char *magic_str_p = (const char *) ecma_get_magic_string_zt (lit.data.magic_str_id);

    if (strlen (magic_str_p) == length
        && strncmp (magic_str_p, (const char*) str_p, length) == 0)
    {
      return true;
    }
  }

  return false;
} /* string_equals_to_literal */

static literal
adjust_string_ptrs (literal lit, size_t diff)
{
  if (lit.type != LIT_STR)
  {
    return lit;
  }

  literal ret;

  ret.type = LIT_STR;
  ret.data.lp.length = lit.data.lp.length;
  ret.data.lp.hash = lit.data.lp.hash;
  ret.data.lp.str = lit.data.lp.str + diff;

  return ret;
}

static literal
add_string_to_string_cache (const ecma_char_t* str, ecma_length_t length)
{
  if (strings_cache_used_size + length * sizeof (ecma_char_t) >= strings_cache_size)
  {
    strings_cache_size = mem_heap_recommend_allocation_size (strings_cache_used_size
                                                             + ((size_t) length + 1) * sizeof (ecma_char_t));
    ecma_char_t *temp = (ecma_char_t *) mem_heap_alloc_block (strings_cache_size,
                                                              MEM_HEAP_ALLOC_SHORT_TERM);
    if (strings_cache)
    {
      memcpy (temp, strings_cache, strings_cache_used_size);
      STACK_ITERATE_VARG_SET (literals, adjust_string_ptrs, 0, (size_t) (temp - strings_cache));
      mem_heap_free_block ((uint8_t *) strings_cache);
    }
    strings_cache = temp;
  }
  strncpy ((char *) (strings_cache + strings_cache_used_size), (const char*) str, length);
  (strings_cache + strings_cache_used_size)[length] = '\0';
  const literal res = create_literal_from_zt (strings_cache + strings_cache_used_size, length);
  strings_cache_used_size = (size_t) (((size_t) length + 1) * sizeof (ecma_char_t) + strings_cache_used_size);
  return res;
}

/**
 * Convert string to token of specified type
 *
 * @return token descriptor
 */
static token
convert_string_to_token (token_type tt, /**< token type */
                         const ecma_char_t *str_p, /**< characters buffer */
                         ecma_length_t length) /**< string's length */
{
  JERRY_ASSERT (str_p != NULL);

  for (literal_index_t i = 0; i < STACK_SIZE (literals); i++)
  {
    const literal lit = STACK_ELEMENT (literals, i);
    if ((lit.type == LIT_STR || lit.type == LIT_MAGIC_STR)
        && string_equals_to_literal (str_p, length, lit))
    {
      return create_token (tt, i);
    }
  }

  literal lit = create_literal_from_str (str_p, length);
  JERRY_ASSERT (lit.type == LIT_STR || lit.type == LIT_MAGIC_STR);
  if (lit.type == LIT_STR)
  {
    lit = add_string_to_string_cache (str_p, length);
  }

  STACK_PUSH (literals, lit);

  return create_token (tt, (literal_index_t) (STACK_SIZE (literals) - 1));
}

/**
 * Try to decore specified string as keyword
 *
 * @return if specified string represents a keyword, return corresponding keyword token,
 *         else if it is 'null' - return TOK_NULL token,
 *         else if it is 'true' or 'false' - return TOK_BOOL with corresponding boolean value,
 *         else - return empty_token.
 */
static token
decode_keyword (const ecma_char_t *str_p, /**< characters buffer */
                size_t length) /**< string's length */
{
  typedef struct
  {
    const char *keyword_p;
    keyword keyword_id;
  } kw_descr_t;

  const kw_descr_t keywords[] =
  {
#define KW_DESCR(literal, keyword_id) { literal, keyword_id }
    KW_DESCR ("break", KW_BREAK),
    KW_DESCR ("case", KW_CASE),
    KW_DESCR ("catch", KW_CATCH),
    KW_DESCR ("class", KW_CLASS),
    KW_DESCR ("const", KW_CONST),
    KW_DESCR ("continue", KW_CONTINUE),
    KW_DESCR ("debugger", KW_DEBUGGER),
    KW_DESCR ("default", KW_DEFAULT),
    KW_DESCR ("delete", KW_DELETE),
    KW_DESCR ("do", KW_DO),
    KW_DESCR ("else", KW_ELSE),
    KW_DESCR ("enum", KW_ENUM),
    KW_DESCR ("export", KW_EXPORT),
    KW_DESCR ("extends", KW_EXTENDS),
    KW_DESCR ("finally", KW_FINALLY),
    KW_DESCR ("for", KW_FOR),
    KW_DESCR ("function", KW_FUNCTION),
    KW_DESCR ("if", KW_IF),
    KW_DESCR ("in", KW_IN),
    KW_DESCR ("instanceof", KW_INSTANCEOF),
    KW_DESCR ("interface", KW_INTERFACE),
    KW_DESCR ("import", KW_IMPORT),
    KW_DESCR ("implements", KW_IMPLEMENTS),
    KW_DESCR ("let", KW_LET),
    KW_DESCR ("new", KW_NEW),
    KW_DESCR ("package", KW_PACKAGE),
    KW_DESCR ("private", KW_PRIVATE),
    KW_DESCR ("protected", KW_PROTECTED),
    KW_DESCR ("public", KW_PUBLIC),
    KW_DESCR ("return", KW_RETURN),
    KW_DESCR ("static", KW_STATIC),
    KW_DESCR ("super", KW_SUPER),
    KW_DESCR ("switch", KW_SWITCH),
    KW_DESCR ("this", KW_THIS),
    KW_DESCR ("throw", KW_THROW),
    KW_DESCR ("try", KW_TRY),
    KW_DESCR ("typeof", KW_TYPEOF),
    KW_DESCR ("var", KW_VAR),
    KW_DESCR ("void", KW_VOID),
    KW_DESCR ("while", KW_WHILE),
    KW_DESCR ("with", KW_WITH),
    KW_DESCR ("yield", KW_YIELD)
#undef KW_DESCR
  };

  keyword kw = KW_NONE;

  for (uint32_t i = 0; i < sizeof (keywords) / sizeof (kw_descr_t); i++)
  {
    if (strlen (keywords[i].keyword_p) == length
        && !strncmp (keywords[i].keyword_p, (const char *) str_p, length))
    {
      kw = keywords[i].keyword_id;
      break;
    }
  }

  if (!strict_mode)
  {
    switch (kw)
    {
      case KW_INTERFACE:
      case KW_IMPLEMENTS:
      case KW_LET:
      case KW_PACKAGE:
      case KW_PRIVATE:
      case KW_PROTECTED:
      case KW_PUBLIC:
      case KW_STATIC:
      case KW_YIELD:
      {
        return convert_string_to_token (TOK_NAME, str_p, (ecma_length_t) length);
      }

      default:
      {
        break;
      }
    }
  }

  if (kw != KW_NONE)
  {
    return create_token (TOK_KEYWORD, kw);
  }
  else
  {
    const ecma_char_t *false_p = ecma_get_magic_string_zt (ECMA_MAGIC_STRING_FALSE);
    const ecma_char_t *true_p = ecma_get_magic_string_zt (ECMA_MAGIC_STRING_TRUE);
    const ecma_char_t *null_p = ecma_get_magic_string_zt (ECMA_MAGIC_STRING_NULL);

    if (strlen ((const char*) false_p) == length
        && !strncmp ((const char*) str_p, (const char*) false_p, length))
    {
      return create_token (TOK_BOOL, false);
    }
    else if (strlen ((const char*) true_p) == length
             && !strncmp ((const char*) str_p, (const char*) true_p, length))
    {
      return create_token (TOK_BOOL, true);
    }
    else if (strlen ((const char*) null_p) == length
             && !strncmp ((const char*) str_p, (const char*) null_p, length))
    {
      return create_token (TOK_NULL, 0);
    }
    else
    {
      return empty_token;
    }
  }
} /* decode_keyword */

static token
convert_seen_num_to_token (ecma_number_t num)
{
  for (literal_index_t i = 0; i < STACK_SIZE (literals); i++)
  {
    const literal lit = STACK_ELEMENT (literals, i);
    if (lit.type != LIT_NUMBER)
    {
      continue;
    }
    if (lit.data.num == num)
    {
      return create_token (TOK_NUMBER, i);
    }
  }

  STACK_PUSH (literals, create_literal_from_num (num));

  return create_token (TOK_NUMBER, (literal_index_t) (STACK_SIZE (literals) - 1));
}

const literal *
lexer_get_literals (void)
{
  if (STACK_SIZE (literals) > 0 && STACK_SIZE (literals) > literals_count)
  {
    if (literals_data)
    {
      mem_heap_free_block (literals_data);
    }
    literals_count = STACK_SIZE (literals);
    STACK_CONVERT_TO_RAW_DATA (literals, literals_data);
  }
  return literals_data;
}

literal_index_t
lexer_get_literals_count (void)
{
  return (literal_index_t) STACK_SIZE (literals);
}

literal_index_t
lexer_lookup_literal_uid (literal lit)
{
  for (literal_index_t i = 0; i < STACK_SIZE (literals); i++)
  {
    if (literal_equal_type (STACK_ELEMENT (literals, i), lit))
    {
      return i;
    }
  }
  return INVALID_VALUE;
}

literal
lexer_get_literal_by_id (literal_index_t id)
{
  JERRY_ASSERT (id != INVALID_LITERAL);
  JERRY_ASSERT (id < STACK_SIZE (literals));
  return STACK_ELEMENT (literals, id);
}

const ecma_char_t *
lexer_get_strings_cache (void)
{
  return strings_cache;
}

void
lexer_add_literal_if_not_present (literal lit)
{
  for (literal_index_t i = 0; i < STACK_SIZE (literals); i++)
  {
    if (literal_equal_type (STACK_ELEMENT (literals, i), lit))
    {
      return;
    }
  }

  if (lit.type == LIT_STR)
  {
    lit = add_string_to_string_cache (lit.data.lp.str, lit.data.lp.length);
  }

  STACK_PUSH (literals, lit);
}

static void
new_token (void)
{
  JERRY_ASSERT (buffer);
  token_start = buffer;
}

static void
consume_char (void)
{
  JERRY_ASSERT (buffer);
  buffer++;
}

#define RETURN_PUNC_EX(TOK, NUM) \
  do \
  { \
    token tok = create_token (TOK, 0); \
    buffer += NUM; \
    return tok; \
  } \
  while (0)

#define RETURN_PUNC(TOK) RETURN_PUNC_EX(TOK, 1)

#define IF_LA_N_IS(CHAR, THEN_TOK, ELSE_TOK, NUM) \
  do \
  { \
    if (LA (NUM) == CHAR) \
    { \
      RETURN_PUNC_EX (THEN_TOK, NUM + 1); \
    } \
    else \
    { \
      RETURN_PUNC_EX (ELSE_TOK, NUM); \
    } \
  } \
  while (0)

#define IF_LA_IS(CHAR, THEN_TOK, ELSE_TOK) \
  IF_LA_N_IS (CHAR, THEN_TOK, ELSE_TOK, 1)

#define IF_LA_IS_OR(CHAR1, THEN1_TOK, CHAR2, THEN2_TOK, ELSE_TOK) \
  do \
  { \
    if (LA (1) == CHAR1) \
    { \
      RETURN_PUNC_EX (THEN1_TOK, 2); \
    } \
    else if (LA (1) == CHAR2) \
    { \
      RETURN_PUNC_EX (THEN2_TOK, 2); \
    } \
    else \
    { \
      RETURN_PUNC (ELSE_TOK); \
    } \
  } \
  while (0)

static uint32_t
hex_to_int (char hex)
{
  switch (hex)
  {
    case '0': return 0x0;
    case '1': return 0x1;
    case '2': return 0x2;
    case '3': return 0x3;
    case '4': return 0x4;
    case '5': return 0x5;
    case '6': return 0x6;
    case '7': return 0x7;
    case '8': return 0x8;
    case '9': return 0x9;
    case 'a':
    case 'A': return 0xA;
    case 'b':
    case 'B': return 0xB;
    case 'c':
    case 'C': return 0xC;
    case 'd':
    case 'D': return 0xD;
    case 'e':
    case 'E': return 0xE;
    case 'f':
    case 'F': return 0xF;
    default: JERRY_UNREACHABLE ();
  }
}

/**
 * Try to decode specified character as SingleEscapeCharacter (ECMA-262, v5, 7.8.4)
 *
 * If specified character is a SingleEscapeCharacter, convert it according to ECMA-262 v5, Table 4.
 * Otherwise, output it as is.
 *
 * @return true - if specified character is a SingleEscapeCharacter,
 *         false - otherwise.
 */
static bool
convert_single_escape_character (ecma_char_t c, /**< character to decode */
                                 ecma_char_t *out_converted_char_p) /**< out: decoded character */
{
  ecma_char_t converted_char;
  bool is_single_escape_character = true;

  switch (c)
  {
    case 'b':
    {
      converted_char = (ecma_char_t) '\b';
      break;
    }

    case 't':
    {
      converted_char = (ecma_char_t) '\t';
      break;
    }

    case 'n':
    {
      converted_char = (ecma_char_t) '\n';
      break;
    }

    case 'v':
    {
      converted_char = (ecma_char_t) '\v';
      break;
    }

    case 'f':
    {
      converted_char = (ecma_char_t) '\f';
      break;
    }

    case 'r':
    {
      converted_char = (ecma_char_t) '\r';
      break;
    }

    case '"':
    case '\'':
    case '\\':
    {
      converted_char = (ecma_char_t) c;
      break;
    }

    default:
    {
      converted_char = (ecma_char_t) c;
      is_single_escape_character = false;
      break;
    }
  }

  if (out_converted_char_p != NULL)
  {
    *out_converted_char_p = converted_char;
  }

  return is_single_escape_character;
} /* convert_single_escape_character */

/**
 * Convert specified string to token of specified type, transforming escape sequences
 *
 * @return token descriptor
 */
static token
convert_string_to_token_transform_escape_seq (token_type tok_type, /**< type of token to produce */
                                              const char *source_str_p, /**< string to convert,
                                                                         *   located in source buffer */
                                              size_t source_str_size) /**< size of the string */
{
  token ret;

  if (source_str_size == 0)
  {
    return convert_string_to_token (tok_type,
                                    ecma_get_magic_string_zt (ECMA_MAGIC_STRING__EMPTY),
                                    0);
  }
  else
  {
    JERRY_ASSERT (source_str_p != NULL);
  }

  MEM_DEFINE_LOCAL_ARRAY (str_buf_p,
                          source_str_size,
                          ecma_char_t);

  const char *source_str_iter_p = source_str_p;
  ecma_char_t *str_buf_iter_p = str_buf_p;

  bool is_correct_sequence = true;
  bool every_char_islower = true;
  bool every_char_allowed_in_identifier = true;

  while (source_str_iter_p < source_str_p + source_str_size)
  {
    ecma_char_t converted_char;

    if (*source_str_iter_p != '\\')
    {
      converted_char = (ecma_char_t) *source_str_iter_p++;

      JERRY_ASSERT (str_buf_iter_p <= str_buf_p + source_str_size);
      JERRY_ASSERT (source_str_iter_p <= source_str_p + source_str_size);
    }
    else
    {
      source_str_iter_p++;

      const ecma_char_t escape_character = (ecma_char_t) *source_str_iter_p++;
      JERRY_ASSERT (source_str_iter_p <= source_str_p + source_str_size);

      if (isdigit (escape_character))
      {
        if (escape_character == '0')
        {
          JERRY_UNIMPLEMENTED ("<NUL> character is not currently supported.\n");
        }
        else
        {
          /* Implementation-defined (ECMA-262 v5, B.1.2): octal escape sequences are not implemented */
          is_correct_sequence = false;
          break;
        }
      }
      else if (escape_character == 'u'
               || escape_character == 'x')
      {
        const uint32_t hex_chars_num = (escape_character == 'u' ? 4u : 2u);

        if (source_str_iter_p + hex_chars_num > source_str_p + source_str_size)
        {
          is_correct_sequence = false;
          break;
        }

        bool chars_are_hex = true;
        uint16_t char_code = 0;

        for (uint32_t i = 0; i < hex_chars_num; i++)
        {
          const char nc = *source_str_iter_p++;

          if (!isxdigit (nc))
          {
            chars_are_hex = false;
            break;
          }
          else
          {
            /*
             * Check that highest 4 bits are zero, so the value would not overflow.
             */
            JERRY_ASSERT ((char_code & 0xF000u) == 0);

            char_code = (uint16_t) (char_code << 4u);
            char_code = (uint16_t) (char_code + hex_to_int (nc));
          }
        }

        JERRY_ASSERT (str_buf_iter_p <= str_buf_p + source_str_size);
        JERRY_ASSERT (source_str_iter_p <= source_str_p + source_str_size);

        if (!chars_are_hex)
        {
          is_correct_sequence = false;
          break;
        }

        /*
         * In CONFIG_ECMA_CHAR_ASCII mode size of ecma_char_t is 1 byte, so the conversion
         * would ignore highest part of 2-byte value, and in CONFIG_ECMA_CHAR_UTF16 mode this
         * would be just an assignment of 2-byte value.
         */
        converted_char = (ecma_char_t) char_code;
      }
      else if (ecma_char_is_line_terminator (escape_character))
      {
        if (source_str_iter_p + 1 <= source_str_p + source_str_size)
        {
          char nc = *source_str_iter_p;

          if (escape_character == '\x0D'
              && nc == '\x0A')
          {
            source_str_iter_p++;
          }
        }

        continue;
      }
      else
      {
        convert_single_escape_character ((ecma_char_t) escape_character, &converted_char);
      }
    }

    *str_buf_iter_p++ = converted_char;
    JERRY_ASSERT (str_buf_iter_p <= str_buf_p + source_str_size);

    if (!islower (converted_char))
    {
      every_char_islower = false;

      if (!isalpha (converted_char)
          && !isdigit (converted_char)
          && converted_char != '$'
          && converted_char != '_')
      {
        every_char_allowed_in_identifier = false;
      }
    }
  }

  if (is_correct_sequence)
  {
    ecma_length_t length = (ecma_length_t) (str_buf_iter_p - str_buf_p);
    ret = empty_token;

    if (tok_type == TOK_NAME)
    {
      if (every_char_islower)
      {
        ret = decode_keyword (str_buf_p, length);
      }
      else if (!every_char_allowed_in_identifier)
      {
        PARSE_ERROR ("Malformed identifier name", source_str_p - buffer_start);
      }
    }

    if (is_empty (ret))
    {
      ret = convert_string_to_token (tok_type, str_buf_p, length);
    }
  }
  else
  {
    PARSE_ERROR ("Malformed escape sequence", source_str_p - buffer_start);
  }

  MEM_FINALIZE_LOCAL_ARRAY (str_buf_p);

  return ret;
} /* convert_string_to_token_transform_escape_seq */

/**
 * Parse identifier (ECMA-262 v5, 7.6) or keyword (7.6.1.1)
 */
static token
parse_name (void)
{
  ecma_char_t c = (ecma_char_t) LA (0);

  token known_token = empty_token;

  JERRY_ASSERT (isalpha (c) || c == '$' || c == '_');

  new_token ();

  while (true)
  {
    c = (ecma_char_t) LA (0);

    if (!isalpha (c)
        && !isdigit (c)
        && c != '$'
        && c != '_'
        && c != '\\')
    {
      break;
    }
    else
    {
      consume_char ();

      if (c == '\\')
      {
        bool is_correct_sequence = (LA (0) == 'u');
        if (is_correct_sequence)
        {
          consume_char ();
        }

        for (uint32_t i = 0;
             is_correct_sequence && i < 4;
             i++)
        {
          if (!isxdigit (LA (0)))
          {
            is_correct_sequence = false;
            break;
          }

          consume_char ();
        }

        if (!is_correct_sequence)
        {
          PARSE_ERROR ("Malformed escape sequence", token_start - buffer_start);
        }
      }
    }
  }

  known_token = convert_string_to_token_transform_escape_seq (TOK_NAME,
                                                              token_start,
                                                              (size_t) (buffer - token_start));

  token_start = NULL;

  return known_token;
} /* parse_name */

/* In this function we cannot use strtol function
   since there is no octal literals in ECMAscript.  */
static token
parse_number (void)
{
  char c = LA (0);
  bool is_hex = false;
  bool is_fp = false;
  bool is_exp = false;
  bool is_overflow = false;
  ecma_number_t fp_res = .0;
  size_t tok_length = 0, i;
  uint32_t res = 0;
  token known_token;

  JERRY_ASSERT (isdigit (c) || c == '.');

  if (c == '0')
  {
    if (LA (1) == 'x' || LA (1) == 'X')
    {
      is_hex = true;
    }
  }

  if (c == '.')
  {
    JERRY_ASSERT (!isalpha (LA (1)));
    is_fp = true;
  }

  if (is_hex)
  {
    // Eat up '0x'
    consume_char ();
    consume_char ();
    new_token ();
    while (true)
    {
      c = LA (0);
      if (!isxdigit (c))
      {
        break;
      }
      consume_char ();
    }

    if (isalpha (c) || c == '_' || c == '$')
    {
      PARSE_ERROR ("Integer literal shall not contain non-digit characters", buffer - buffer_start);
    }

    tok_length = (size_t) (buffer - token_start);

    for (i = 0; i < tok_length; i++)
    {
      if (!is_overflow)
      {
        res = (res << 4) + hex_to_int (token_start[i]);
      }
      else
      {
        fp_res = fp_res * 16 + (ecma_number_t) hex_to_int (token_start[i]);
      }

      if (res > 255)
      {
        fp_res = (ecma_number_t) res;
        is_overflow = true;
        res = 0;
      }
    }

    if (is_overflow)
    {
      known_token = convert_seen_num_to_token (fp_res);
      token_start = NULL;
      return known_token;
    }
    else
    {
      known_token = create_token (TOK_SMALL_INT, (uint8_t) res);
      token_start = NULL;
      return known_token;
    }
  }

  JERRY_ASSERT (!is_hex && !is_exp);

  new_token ();

  // Eat up '.'
  if (is_fp)
  {
    consume_char ();
  }

  while (true)
  {
    c = LA (0);
    if (is_fp && c == '.')
    {
      FIXME (/* This is wrong: 1..toString ().  */)
      PARSE_ERROR ("Integer literal shall not contain more than one dot character", buffer - buffer_start);
    }
    if (is_exp && (c == 'e' || c == 'E'))
    {
      PARSE_ERROR ("Integer literal shall not contain more than exponential marker ('e' or 'E')",
                   buffer - buffer_start);
    }

    if (c == '.')
    {
      if (isalpha (LA (1)) || LA (1) == '_' || LA (1) == '$')
      {
        PARSE_ERROR ("Integer literal shall not contain non-digit character after got character",
                     buffer - buffer_start);
      }
      is_fp = true;
      consume_char ();
      continue;
    }

    if (c == 'e' || c == 'E')
    {
      if (LA (1) == '-' || LA (1) == '+')
      {
        consume_char ();
      }
      if (!isdigit (LA (1)))
      {
        PARSE_ERROR ("Integer literal shall not contain non-digit character after exponential marker ('e' or 'E')",
                     buffer - buffer_start);
      }
      is_exp = true;
      consume_char ();
      continue;
    }

    if (isalpha (c) || c == '_' || c == '$')
    {
      PARSE_ERROR ("Integer literal shall not contain non-digit characters", buffer - buffer_start);
    }

    if (!isdigit (c))
    {
      break;
    }

    consume_char ();
  }

  tok_length = (size_t) (buffer - token_start);;
  if (is_fp || is_exp)
  {
    ecma_char_t *temp = (ecma_char_t*) mem_heap_alloc_block ((size_t) (tok_length + 1),
                                                             MEM_HEAP_ALLOC_SHORT_TERM);
    strncpy ((char *) temp, token_start, (size_t) (tok_length));
    temp[tok_length] = '\0';
    ecma_number_t res = ecma_zt_string_to_number (temp);
    JERRY_ASSERT (!ecma_number_is_nan (res));
    mem_heap_free_block (temp);
    known_token = convert_seen_num_to_token (res);
    token_start = NULL;
    return known_token;
  }

  if (*token_start == '0' && tok_length != 1)
  {
    if (strict_mode)
    {
      PARSE_ERROR ("Octal tnteger literals are not allowed in strict mode", token_start - buffer_start);
    }
    for (i = 0; i < tok_length; i++)
    {
      if (!is_overflow)
      {
        res = res * 8 + hex_to_int (token_start[i]);
      }
      else
      {
        fp_res = fp_res * 8 + (ecma_number_t) hex_to_int (token_start[i]);
      }
      if (res > 255)
      {
        fp_res = (ecma_number_t) res;
        is_overflow = true;
        res = 0;
      }
    }
  }
  else
  {
    for (i = 0; i < tok_length; i++)
    {
      if (!is_overflow)
      {
        res = res * 10 + hex_to_int (token_start[i]);
      }
      else
      {
        fp_res = fp_res * 10 + (ecma_number_t) hex_to_int (token_start[i]);
      }
      if (res > 255)
      {
        fp_res = (ecma_number_t) res;
        is_overflow = true;
        res = 0;
      }
    }
  }

  if (is_overflow)
  {
    known_token = convert_seen_num_to_token (fp_res);
    token_start = NULL;
    return known_token;
  }
  else
  {
    known_token = create_token (TOK_SMALL_INT, (uint8_t) res);
    token_start = NULL;
    return known_token;
  }
}

/**
 * Parse string literal (ECMA-262 v5, 7.8.4)
 */
static token
parse_string (void)
{
  ecma_char_t c = (ecma_char_t) LA (0);
  JERRY_ASSERT (c == '\'' || c == '"');

  consume_char ();
  new_token ();

  const bool is_double_quoted = (c == '"');
  const char end_char = (is_double_quoted ? '"' : '\'');

  do
  {
    c = (ecma_char_t) LA (0);
    consume_char ();

    if (c == '\0')
    {
      PARSE_ERROR ("Unclosed string", token_start - buffer_start);
    }
    else if (ecma_char_is_line_terminator (c))
    {
      PARSE_ERROR ("String literal shall not contain newline character", token_start - buffer_start);
    }
    else if (c == '\\')
    {
      ecma_char_t nc = (ecma_char_t) LA (0);

      if (convert_single_escape_character (nc, NULL))
      {
        consume_char ();
      }
      else if (ecma_char_is_line_terminator (nc))
      {
        consume_char ();

        if (ecma_char_is_carriage_return (nc))
        {
          nc = (ecma_char_t) LA (0);

          if (ecma_char_is_new_line (nc))
          {
            consume_char ();
          }
        }
      }
    }
  }
  while (c != end_char);

  token ret = convert_string_to_token_transform_escape_seq (TOK_STRING,
                                                            token_start,
                                                            (size_t) (buffer - token_start) - 1u);

  token_start = NULL;

  return ret;
} /* parse_string */

static void
grobble_whitespaces (void)
{
  char c = LA (0);

  while ((isspace (c) && c != '\n'))
  {
    consume_char ();
    c = LA (0);
  }
}

static void
lexer_set_source (const char * source)
{
  buffer_start = source;
  buffer = buffer_start;
}

static bool
replace_comment_by_newline (void)
{
  char c = LA (0);
  bool multiline;
  bool was_newlines = false;

  JERRY_ASSERT (LA (0) == '/');
  JERRY_ASSERT (LA (1) == '/' || LA (1) == '*');

  multiline = (LA (1) == '*');

  consume_char ();
  consume_char ();

  while (true)
  {
    c = LA (0);
    if (!multiline && (c == '\n' || c == '\0'))
    {
      return false;
    }
    if (multiline && c == '*' && LA (1) == '/')
    {
      consume_char ();
      consume_char ();

      if (was_newlines)
      {
        return true;
      }
      else
      {
        return false;
      }
    }
    if (multiline && c == '\n')
    {
      was_newlines = true;
    }
    if (multiline && c == '\0')
    {
      PARSE_ERROR ("Unclosed multiline comment", buffer - buffer_start);
    }
    consume_char ();
  }
}

static token
lexer_next_token_private (void)
{
  char c = LA (0);

  JERRY_ASSERT (token_start == NULL);

  if (isalpha (c) || c == '$' || c == '_')
  {
    return parse_name ();
  }

  if (isdigit (c) || (c == '.' && isdigit (LA (1))))
  {
    return parse_number ();
  }

  if (c == '\n')
  {
    consume_char ();
    return create_token (TOK_NEWLINE, 0);
  }

  if (c == '\0')
  {
    return create_token (TOK_EOF, 0);
  }

  if (c == '\'' || c == '"')
  {
    return parse_string ();
  }

  if (isspace (c))
  {
    grobble_whitespaces ();
    return lexer_next_token_private ();
  }

  if (c == '/' && LA (1) == '*')
  {
    if (replace_comment_by_newline ())
    {
      token ret;

      ret.type = TOK_NEWLINE;
      ret.uid = 0;

      return ret;
    }
    else
    {
      return lexer_next_token_private ();
    }
  }

  if (c == '/' && LA (1) == '/')
  {
    replace_comment_by_newline ();
    return lexer_next_token_private ();
  }

  switch (c)
  {
    case '{': RETURN_PUNC (TOK_OPEN_BRACE); break;
    case '}': RETURN_PUNC (TOK_CLOSE_BRACE); break;
    case '(': RETURN_PUNC (TOK_OPEN_PAREN); break;
    case ')': RETURN_PUNC (TOK_CLOSE_PAREN); break;
    case '[': RETURN_PUNC (TOK_OPEN_SQUARE); break;
    case ']': RETURN_PUNC (TOK_CLOSE_SQUARE); break;
    case '.': RETURN_PUNC (TOK_DOT); break;
    case ';': RETURN_PUNC (TOK_SEMICOLON); break;
    case ',': RETURN_PUNC (TOK_COMMA); break;
    case '~': RETURN_PUNC (TOK_COMPL); break;
    case ':': RETURN_PUNC (TOK_COLON); break;
    case '?': RETURN_PUNC (TOK_QUERY); break;

    case '*': IF_LA_IS ('=', TOK_MULT_EQ, TOK_MULT); break;
    case '/': IF_LA_IS ('=', TOK_DIV_EQ, TOK_DIV); break;
    case '^': IF_LA_IS ('=', TOK_XOR_EQ, TOK_XOR); break;
    case '%': IF_LA_IS ('=', TOK_MOD_EQ, TOK_MOD); break;

    case '+': IF_LA_IS_OR ('+', TOK_DOUBLE_PLUS, '=', TOK_PLUS_EQ, TOK_PLUS); break;
    case '-': IF_LA_IS_OR ('-', TOK_DOUBLE_MINUS, '=', TOK_MINUS_EQ, TOK_MINUS); break;
    case '&': IF_LA_IS_OR ('&', TOK_DOUBLE_AND, '=', TOK_AND_EQ, TOK_AND); break;
    case '|': IF_LA_IS_OR ('|', TOK_DOUBLE_OR, '=', TOK_OR_EQ, TOK_OR); break;

    case '<':
    {
      switch (LA (1))
      {
        case '<': IF_LA_N_IS ('=', TOK_LSHIFT_EQ, TOK_LSHIFT, 2); break;
        case '=': RETURN_PUNC_EX (TOK_LESS_EQ, 2); break;
        default: RETURN_PUNC (TOK_LESS);
      }
      break;
    }
    case '>':
    {
      switch (LA (1))
      {
        case '>':
        {
          switch (LA (2))
          {
            case '>': IF_LA_N_IS ('=', TOK_RSHIFT_EX_EQ, TOK_RSHIFT_EX, 3); break;
            case '=': RETURN_PUNC_EX (TOK_RSHIFT_EQ, 3); break;
            default: RETURN_PUNC_EX (TOK_RSHIFT, 2);
          }
          break;
        }
        case '=': RETURN_PUNC_EX (TOK_GREATER_EQ, 2); break;
        default: RETURN_PUNC (TOK_GREATER);
      }
      break;
    }
    case '=':
    {
      if (LA (1) == '=')
      {
        IF_LA_N_IS ('=', TOK_TRIPLE_EQ, TOK_DOUBLE_EQ, 2);
      }
      else
      {
        RETURN_PUNC (TOK_EQ);
      }
      break;
    }
    case '!':
    {
      if (LA (1) == '=')
      {
        IF_LA_N_IS ('=', TOK_NOT_DOUBLE_EQ, TOK_NOT_EQ, 2);
      }
      else
      {
        RETURN_PUNC (TOK_NOT);
      }
      break;
    }
    default: PARSE_SORRY ("Unknown character", buffer - buffer_start);
  }
  PARSE_SORRY ("Unknown character", buffer - buffer_start);
}

token
lexer_next_token (void)
{
  if (buffer == buffer_start)
  {
    dump_current_line ();
  }

  if (!is_empty (saved_token))
  {
    sent_token = saved_token;
    saved_token = empty_token;
    goto end;
  }

  prev_token = sent_token;
  sent_token = lexer_next_token_private ();

  if (sent_token.type == TOK_NEWLINE)
  {
    dump_current_line ();
  }

end:
  return sent_token;
}

void
lexer_save_token (token tok)
{
  JERRY_ASSERT (is_empty (saved_token));
  saved_token = tok;
}

token
lexer_prev_token (void)
{
  return prev_token;
}

void
lexer_seek (size_t locus)
{
  JERRY_ASSERT (locus < buffer_size);
  JERRY_ASSERT (token_start == NULL);

  buffer = buffer_start + locus;
  saved_token = empty_token;
}

void
lexer_locus_to_line_and_column (size_t locus, size_t *line, size_t *column)
{
  JERRY_ASSERT (locus <= buffer_size);
  const char *buf;
  size_t l = 0, c = 0;
  for (buf = buffer_start; (size_t) (buf - buffer_start) < locus; buf++)
  {
    if (*buf == '\n')
    {
      c = 0;
      l++;
      continue;
    }
    c++;
  }

  if (line)
  {
    *line = l;
  }
  if (column)
  {
    *column = c;
  }
}

void
lexer_dump_line (size_t line)
{
  size_t l = 0;
  for (const char *buf = buffer_start; *buf != '\0'; buf++)
  {
    if (l == line)
    {
      for (; *buf != '\n' && *buf != '\0'; buf++)
      {
        putchar (*buf);
      }
      return;
    }
    if (*buf == '\n')
    {
      l++;
    }
  }
}

const char *
lexer_keyword_to_string (keyword kw)
{
  switch (kw)
  {
    case KW_BREAK: return "break";
    case KW_CASE: return "case";
    case KW_CATCH: return "catch";
    case KW_CLASS: return "class";

    case KW_CONST: return "const";
    case KW_CONTINUE: return "continue";
    case KW_DEBUGGER: return "debugger";
    case KW_DEFAULT: return "default";
    case KW_DELETE: return "delete";

    case KW_DO: return "do";
    case KW_ELSE: return "else";
    case KW_ENUM: return "enum";
    case KW_EXPORT: return "export";
    case KW_EXTENDS: return "extends";

    case KW_FINALLY: return "finally";
    case KW_FOR: return "for";
    case KW_FUNCTION: return "function";
    case KW_IF: return "if";
    case KW_IN: return "in";

    case KW_INSTANCEOF: return "instanceof";
    case KW_INTERFACE: return "interface";
    case KW_IMPORT: return "import";
    case KW_IMPLEMENTS: return "implements";
    case KW_LET: return "let";

    case KW_NEW: return "new";
    case KW_PACKAGE: return "package";
    case KW_PRIVATE: return "private";
    case KW_PROTECTED: return "protected";
    case KW_PUBLIC: return "public";

    case KW_RETURN: return "return";
    case KW_STATIC: return "static";
    case KW_SUPER: return "super";
    case KW_SWITCH: return "switch";
    case KW_THIS: return "this";

    case KW_THROW: return "throw";
    case KW_TRY: return "try";
    case KW_TYPEOF: return "typeof";
    case KW_VAR: return "var";
    case KW_VOID: return "void";

    case KW_WHILE: return "while";
    case KW_WITH: return "with";
    case KW_YIELD: return "yield";
    default: JERRY_UNREACHABLE ();
  }
}

const char *
lexer_token_type_to_string (token_type tt)
{
  switch (tt)
  {
    case TOK_EOF: return "End of file";
    case TOK_NAME: return "Identifier";
    case TOK_KEYWORD: return "Keyword";
    case TOK_SMALL_INT: /* FALLTHRU */
    case TOK_NUMBER: return "Number";

    case TOK_NULL: return "null";
    case TOK_BOOL: return "bool";
    case TOK_NEWLINE: return "newline";
    case TOK_STRING: return "string";
    case TOK_OPEN_BRACE: return "{";

    case TOK_CLOSE_BRACE: return "}";
    case TOK_OPEN_PAREN: return "(";
    case TOK_CLOSE_PAREN: return ")";
    case TOK_OPEN_SQUARE: return "[";
    case TOK_CLOSE_SQUARE: return "]";

    case TOK_DOT: return ".";
    case TOK_SEMICOLON: return ";";
    case TOK_COMMA: return ",";
    case TOK_LESS: return "<";
    case TOK_GREATER: return ">";

    case TOK_LESS_EQ: return "<=";
    case TOK_GREATER_EQ: return "<=";
    case TOK_DOUBLE_EQ: return "==";
    case TOK_NOT_EQ: return "!=";
    case TOK_TRIPLE_EQ: return "===";

    case TOK_NOT_DOUBLE_EQ: return "!==";
    case TOK_PLUS: return "+";
    case TOK_MINUS: return "-";
    case TOK_MULT: return "*";
    case TOK_MOD: return "%%";

    case TOK_DOUBLE_PLUS: return "++";
    case TOK_DOUBLE_MINUS: return "--";
    case TOK_LSHIFT: return "<<";
    case TOK_RSHIFT: return ">>";
    case TOK_RSHIFT_EX: return ">>>";

    case TOK_AND: return "&";
    case TOK_OR: return "|";
    case TOK_XOR: return "^";
    case TOK_NOT: return "!";
    case TOK_COMPL: return "~";

    case TOK_DOUBLE_AND: return "&&";
    case TOK_DOUBLE_OR: return "||";
    case TOK_QUERY: return "?";
    case TOK_COLON: return ":";
    case TOK_EQ: return "=";

    case TOK_PLUS_EQ: return "+=";
    case TOK_MINUS_EQ: return "-=";
    case TOK_MULT_EQ: return "*=";
    case TOK_MOD_EQ: return "%%=";
    case TOK_LSHIFT_EQ: return "<<=";

    case TOK_RSHIFT_EQ: return ">>=";
    case TOK_RSHIFT_EX_EQ: return ">>>=";
    case TOK_AND_EQ: return "&=";
    case TOK_OR_EQ: return "|=";
    case TOK_XOR_EQ: return "^=";

    case TOK_DIV: return "/";
    case TOK_DIV_EQ: return "/=";
    default: JERRY_UNREACHABLE ();
  }
}

void
lexer_set_strict_mode (bool is_strict)
{
  strict_mode = is_strict;
}

void
lexer_init_source (const char *source, size_t source_size)
{
  empty_token.type = TOK_EMPTY;
  empty_token.uid = 0;
  empty_token.loc = 0;

  saved_token = prev_token = sent_token = empty_token;

  buffer_size = source_size;
  lexer_set_source (source);
  lexer_set_strict_mode (false);
}

void
lexer_init (bool show_opcodes)
{
#ifndef JERRY_NDEBUG
  allow_dump_lines = show_opcodes;
#else /* JERRY_NDEBUG */
  (void) show_opcodes;
  allow_dump_lines = false;
#endif /* JERRY_NDEBUG */

  if (literals_count)
  {
    STACK_INIT_FROM_RAW (literals, literals_data, literals_count * sizeof (literal));
  }
  else
  {
    STACK_INIT (literals);
  }
}

void
lexer_free (void)
{
  STACK_FREE (literals);
}
