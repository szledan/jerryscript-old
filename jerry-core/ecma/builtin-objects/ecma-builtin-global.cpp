/* Copyright 2014-2015 Samsung Electronics Co., Ltd.
 * Copyright 2015 University of Szeged
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

#include "ecma-alloc.h"
#include "ecma-builtins.h"
#include "ecma-conversion.h"
#include "ecma-eval.h"
#include "ecma-exceptions.h"
#include "ecma-gc.h"
#include "ecma-globals.h"
#include "ecma-helpers.h"
#include "ecma-try-catch-macro.h"
#include "jrt.h"
#include "vm.h"

#define ECMA_BUILTINS_INTERNAL
#include "ecma-builtins-internal.h"

#define BUILTIN_INC_HEADER_NAME "ecma-builtin-global.inc.h"
#define BUILTIN_UNDERSCORED_ID global
#include "ecma-builtin-internal-routines-template.inc.h"

/** \addtogroup ecma ECMA
 * @{
 *
 * \addtogroup ecmabuiltins
 * @{
 *
 * \addtogroup global ECMA Global object built-in
 * @{
 */

/**
 * The Global object's 'eval' routine
 *
 * See also:
 *          ECMA-262 v5, 15.1.2.1
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_global_object_eval (ecma_value_t this_arg, /**< this argument */
                                 ecma_value_t x) /**< routine's first argument */
{
  ecma_completion_value_t ret_value = ecma_make_empty_completion_value ();

  bool is_direct_eval = vm_is_direct_eval_form_call ();
  JERRY_ASSERT (!(is_direct_eval
                  && !ecma_is_value_undefined (this_arg)));

  /* See also: ECMA-262 v5, 10.1.1 */
  bool is_called_from_strict_mode_code;
  if (is_direct_eval)
  {
    is_called_from_strict_mode_code = vm_is_strict_mode ();
  }
  else
  {
    is_called_from_strict_mode_code = false;
  }

  if (!ecma_is_value_string (x))
  {
    /* step 1 */
    ret_value = ecma_make_normal_completion_value (ecma_copy_value (x, true));
  }
  else
  {
    /* steps 2 to 8 */
    ret_value = ecma_op_eval (ecma_get_string_from_value (x),
                              is_direct_eval,
                              is_called_from_strict_mode_code);
  }

  return ret_value;
} /* ecma_builtin_global_object_eval */

/**
 * The Global object's 'parseInt' routine
 *
 * See also:
 *          ECMA-262 v5, 15.1.2.2
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_global_object_parse_int (ecma_value_t this_arg, /**< this argument */
                                      ecma_value_t string, /**< routine's first argument */
                                      ecma_value_t radix) /**< routine's second argument */
{
  ECMA_BUILTIN_CP_UNIMPLEMENTED (this_arg, string, radix);
} /* ecma_builtin_global_object_parse_int */

/**
 * The Global object's 'parseFloat' routine
 *
 * See also:
 *          ECMA-262 v5, 15.1.2.3
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_global_object_parse_float (ecma_value_t this_arg, /**< this argument */
                                        ecma_value_t string) /**< routine's first argument */
{
  ECMA_BUILTIN_CP_UNIMPLEMENTED (this_arg, string);
} /* ecma_builtin_global_object_parse_float */

/**
 * The Global object's 'isNaN' routine
 *
 * See also:
 *          ECMA-262 v5, 15.1.2.4
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_global_object_is_nan (ecma_value_t this_arg __attr_unused___, /**< this argument */
                                   ecma_value_t arg) /**< routine's first argument */
{
  ecma_completion_value_t ret_value = ecma_make_empty_completion_value ();

  ECMA_OP_TO_NUMBER_TRY_CATCH (arg_num, arg, ret_value);

  bool is_nan = ecma_number_is_nan (arg_num);

  ret_value = ecma_make_simple_completion_value (is_nan ? ECMA_SIMPLE_VALUE_TRUE
                                                        : ECMA_SIMPLE_VALUE_FALSE);

  ECMA_OP_TO_NUMBER_FINALIZE (arg_num);

  return ret_value;
} /* ecma_builtin_global_object_is_nan */

/**
 * The Global object's 'isFinite' routine
 *
 * See also:
 *          ECMA-262 v5, 15.1.2.5
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_global_object_is_finite (ecma_value_t this_arg __attr_unused___, /**< this argument */
                                      ecma_value_t arg) /**< routine's first argument */
{
  ecma_completion_value_t ret_value = ecma_make_empty_completion_value ();

  ECMA_OP_TO_NUMBER_TRY_CATCH (arg_num, arg, ret_value);

  bool is_finite = !(ecma_number_is_nan (arg_num)
                     || ecma_number_is_infinity (arg_num));

  ret_value = ecma_make_simple_completion_value (is_finite ? ECMA_SIMPLE_VALUE_TRUE
                                                           : ECMA_SIMPLE_VALUE_FALSE);

  ECMA_OP_TO_NUMBER_FINALIZE (arg_num);

  return ret_value;
} /* ecma_builtin_global_object_is_finite */

/**
 * Helper function to check whether a character is in a character bitset.
 *
 * @return true if the character is in the character bitset.
 */
static bool
ecma_builtin_global_object_character_is_in (uint32_t character, /**< character */
                                            uint8_t *bitset) /**< character set */
{
  JERRY_ASSERT (character < 128);
  return (bitset[character >> 3] & (1 << (character & 0x7))) != 0;
} /* ecma_builtin_global_object_character_is_in */

/*
 * Unescaped URI characters bitset:
 *   One bit for each character between 0 - 127.
 *   Bit is set if the character is in the unescaped URI set.
 */
static uint8_t unescaped_uri_set[16] =
{
  0x0, 0x0, 0x0, 0x0, 0xda, 0xff, 0xff, 0xaf,
  0xff, 0xff, 0xff, 0x87, 0xfe, 0xff, 0xff, 0x47
};

/*
 * Unescaped URI component characters bitset:
 *   One bit for each character between 0 - 127.
 *   Bit is set if the character is in the unescaped component URI set.
 */
static uint8_t unescaped_uri_component_set[16] =
{
  0x0, 0x0, 0x0, 0x0, 0x82, 0x67, 0xff, 0x3,
  0xfe, 0xff, 0xff, 0x87, 0xfe, 0xff, 0xff, 0x47
};

#define ECMA_BUILTIN_HEX_TO_BYTE_ERROR 0x100

/**
 * Helper function to decode a hexadecimal byte from a string.
 *
 * @return the decoded byte value
 *         It returns with ECMA_BUILTIN_HEX_TO_BYTE_ERROR if a parse error is occured.
 */
static uint32_t
ecma_builtin_global_object_hex_to_byte (ecma_char_t *source_p) /**< source string */
{
  uint32_t decoded_byte = 0;
  int i;

  /*
   * Zero terminated string, so length check is not needed.
   */
  if (*source_p != '%')
  {
    return ECMA_BUILTIN_HEX_TO_BYTE_ERROR;
  }

  for (i = 0; i < 2; i++)
  {
    source_p++;
    decoded_byte <<= 4;

    if (*source_p >= '0' && *source_p <= '9')
    {
      decoded_byte |= (uint32_t) (*source_p - '0');
    }
    else if (*source_p >= 'a' && *source_p <= 'f')
    {
      decoded_byte |= (uint32_t) (*source_p - ('a' - 10));
    }
    else if (*source_p >= 'A' && *source_p <= 'F')
    {
      decoded_byte |= (uint32_t) (*source_p - ('A' - 10));
    }
    else
    {
      return ECMA_BUILTIN_HEX_TO_BYTE_ERROR;
    }
  }

  return decoded_byte;
} /* ecma_builtin_global_object_hex_to_byte */

/**
 * Helper function to decode URI.
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_global_object_decode_uri_helper (ecma_value_t uri __attr_unused___, /**< uri argument */
                                              uint8_t *reserved_uri_bitset) /**< reserved characters bitset */
{
  ecma_completion_value_t ret_value = ecma_make_empty_completion_value ();

  ECMA_TRY_CATCH (string,
                  ecma_op_to_string (uri),
                  ret_value);

  JERRY_ASSERT (ecma_is_value_string (string));

  ecma_string_t *input_string_p = ecma_get_string_from_value (string);
  uint32_t input_length = (uint32_t) ecma_string_get_length (input_string_p);

  MEM_DEFINE_LOCAL_ARRAY (input_start_p,
                          input_length + 1,
                          ecma_char_t);

  ecma_string_to_zt_string (input_string_p,
                            input_start_p,
                            (ssize_t) (input_length + 1) * (ssize_t) sizeof (ecma_char_t));

  ecma_char_t *input_char_p = input_start_p;
  ecma_char_t *input_end_p = input_start_p + input_length;
  uint32_t output_length = 1;

  while (input_char_p < input_end_p)
  {
    if (*input_char_p != '%')
    {
      output_length++;
      input_char_p++;
      continue;
    }

    uint32_t decoded_byte = ecma_builtin_global_object_hex_to_byte (input_char_p);
    if (decoded_byte == ECMA_BUILTIN_HEX_TO_BYTE_ERROR)
    {
      ret_value = ecma_make_throw_obj_completion_value (ecma_new_standard_error (ECMA_ERROR_URI));
      break;
    }

    input_char_p += 3;

    if (decoded_byte <= 0x7f)
    {
      /*
       * We don't decode those bytes, which are part of reserved_uri_bitset
       * but not part of unescaped_uri_component_set.
       */
      if (ecma_builtin_global_object_character_is_in (decoded_byte, reserved_uri_bitset)
          && !ecma_builtin_global_object_character_is_in (decoded_byte, unescaped_uri_component_set))
      {
        output_length += 3;
      }
      else
      {
        output_length++;
      }
    }
    else if (decoded_byte < 0xc0 || decoded_byte >= 0xf8)
    {
      /*
       * Invalid UTF-8 starting bytes:
       *   10xx xxxx - UTF continuation byte
       *   1111 1xxx - maximum length is 4 bytes
       */
      ret_value = ecma_make_throw_obj_completion_value (ecma_new_standard_error (ECMA_ERROR_URI));
      break;
    }
    else
    {
      uint32_t count;
      uint32_t min;
      uint32_t character;

      if (decoded_byte < 0xe0)
      {
        count = 1;
        min = 0x80;
        character = decoded_byte & 0x1f;
      }
      else if (decoded_byte < 0xf0)
      {
        count = 2;
        min = 0x800;
        character = decoded_byte & 0x0f;
      }
      else
      {
        count = 3;
        min = 0x1000;
        character = decoded_byte & 0x07;
      }

      do
      {
        decoded_byte = ecma_builtin_global_object_hex_to_byte (input_char_p);
        if (decoded_byte == ECMA_BUILTIN_HEX_TO_BYTE_ERROR
            || (decoded_byte & 0xc0) != 0x80)
        {
          break;
        }

        character = (character << 6) + (decoded_byte & 0x3f);
        input_char_p += 3;
      }
      while (--count > 0);

      if (count != 0
          /*
           * Explanation of the character < min check: according to
           * the UTF standard, each character must be encoded
           * with the minimum amount of bytes. We need to reject
           * those characters, which does not satisfy this condition.
           */
          || character < min
          /*
           * Not allowed character ranges.
           */
          || character > 0x10ffff
          || (character >= 0xd800 && character <= 0xdfff))
      {
        ret_value = ecma_make_throw_obj_completion_value (ecma_new_standard_error (ECMA_ERROR_URI));
        break;
      }

      output_length += (character <= 0xffff) ? 1 : 2;
    }
  }

  if (ecma_is_completion_value_empty (ret_value))
  {
    MEM_DEFINE_LOCAL_ARRAY (output_start_p,
                            output_length,
                            ecma_char_t);

    input_char_p = input_start_p;
    ecma_char_t *output_char_p = output_start_p;

    while (input_char_p < input_end_p)
    {
      if (*input_char_p != '%')
      {
        *output_char_p = *input_char_p;
        output_char_p++;
        input_char_p++;
        continue;
      }

      uint32_t decoded_byte = ecma_builtin_global_object_hex_to_byte (input_char_p);
      input_char_p += 3;

      if (decoded_byte <= 0x7f)
      {
        if (ecma_builtin_global_object_character_is_in (decoded_byte, reserved_uri_bitset)
            && !ecma_builtin_global_object_character_is_in (decoded_byte, unescaped_uri_component_set))
        {
          *output_char_p = '%';
          output_char_p++;
          input_char_p -= 2;
        }
        else
        {
          *output_char_p = (ecma_char_t) decoded_byte;
          output_char_p++;
        }
      }
      else
      {
        uint32_t count;
        uint32_t character;

        JERRY_ASSERT (decoded_byte >= 0xc0 && decoded_byte < 0xf8);

        if (decoded_byte < 0xe0)
        {
          count = 1;
          character = decoded_byte & 0x1f;
        }
        else if (decoded_byte < 0xf0)
        {
          count = 2;
          character = decoded_byte & 0x0f;
        }
        else
        {
          count = 3;
          character = decoded_byte & 0x07;
        }

        do
        {
          decoded_byte = ecma_builtin_global_object_hex_to_byte (input_char_p);
          JERRY_ASSERT (decoded_byte != ECMA_BUILTIN_HEX_TO_BYTE_ERROR
                        && (decoded_byte & 0xc0) == 0x80);
          character = (character << 6) + (decoded_byte & 0x3f);
          input_char_p += 3;
        }
        while (--count > 0);

        if (character < 0x10000)
        {
          *output_char_p = (ecma_char_t) character;
          output_char_p++;
        }
        else
        {
          character -= 0x10000;
          *output_char_p = (ecma_char_t) (0xd800 | (character & 0x3ff));
          output_char_p++;
          *output_char_p = (ecma_char_t) (0xdc00 | (character >> 10));
          output_char_p++;
        }
      }
    }

    *output_char_p = '\0';
    JERRY_ASSERT (output_start_p + output_length == output_char_p + 1);

    ecma_string_t *output_string_p = ecma_new_ecma_string (output_start_p);

    ret_value = ecma_make_normal_completion_value (ecma_make_string_value (output_string_p));

    MEM_FINALIZE_LOCAL_ARRAY (output_start_p);
  }

  MEM_FINALIZE_LOCAL_ARRAY (input_start_p);

  ECMA_FINALIZE (string);
  return ret_value;
} /* ecma_builtin_global_object_decode_uri_helper */

/**
 * The Global object's 'decodeURI' routine
 *
 * See also:
 *          ECMA-262 v5, 15.1.3.1
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_global_object_decode_uri (ecma_value_t this_arg __attr_unused___, /**< this argument */
                                       ecma_value_t encoded_uri) /**< routine's first argument */
{
  return ecma_builtin_global_object_decode_uri_helper (encoded_uri, unescaped_uri_set);
} /* ecma_builtin_global_object_decode_uri */

/**
 * The Global object's 'decodeURIComponent' routine
 *
 * See also:
 *          ECMA-262 v5, 15.1.3.2
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_global_object_decode_uri_component (ecma_value_t this_arg __attr_unused___, /**< this argument */
                                                 ecma_value_t encoded_uri_component) /**< routine's
                                                                                      *   first argument */
{
  return ecma_builtin_global_object_decode_uri_helper (encoded_uri_component, unescaped_uri_component_set);
} /* ecma_builtin_global_object_decode_uri_component */

/**
 * Helper function to encode byte as hexadecimal values.
 */
static void
ecma_builtin_global_object_byte_to_hex (ecma_char_t *dest_p, /**< destination pointer */
                                        uint32_t byte) /**< value */
{
  JERRY_ASSERT (byte < 256);

  dest_p[0] = '%';
  ecma_char_t hex_digit = (ecma_char_t) (byte >> 4);
  dest_p[1] = (ecma_char_t) ((hex_digit > 9) ? (hex_digit + ('A' - 10)) : (hex_digit + '0'));
  hex_digit = (ecma_char_t) (byte & 0xf);
  dest_p[2] = (ecma_char_t) ((hex_digit > 9) ? (hex_digit + ('A' - 10)) : (hex_digit + '0'));
} /* ecma_builtin_global_object_byte_to_hex */

/**
 * Helper function to encode URI.
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_global_object_encode_uri_helper (ecma_value_t uri, /**< uri argument */
                                              uint8_t* unescaped_uri_bitset) /**< unescaped bitset */
{
  ecma_completion_value_t ret_value = ecma_make_empty_completion_value ();

  ECMA_TRY_CATCH (string,
                  ecma_op_to_string (uri),
                  ret_value);

  JERRY_ASSERT (ecma_is_value_string (string));

  ecma_string_t *input_string_p = ecma_get_string_from_value (string);
  uint32_t input_length = (uint32_t) ecma_string_get_length (input_string_p);

  MEM_DEFINE_LOCAL_ARRAY (input_start_p,
                          input_length + 1,
                          ecma_char_t);

  ecma_string_to_zt_string (input_string_p,
                            input_start_p,
                            (ssize_t) (input_length + 1) * (ssize_t) sizeof (ecma_char_t));

  /*
   * Compute destination object size.
   */
  ecma_char_t *input_char_p = input_start_p;
  uint32_t output_length = 1;
  for (uint32_t i = 0; i < input_length; i++)
  {
    /*
     * FIXME: we should improve this when unicode support will be available.
     */
    uint32_t character = *input_char_p++;

    if (character <= 0x7f)
    {
      if (ecma_builtin_global_object_character_is_in (character, unescaped_uri_bitset))
      {
        output_length++;
      }
      else
      {
        output_length += 3;
      }
    }
    else if (character <= 0x7ff)
    {
      output_length += 6;
    }
    else if (character <= 0xffff)
    {
      if (character >= 0xd800 && character <= 0xdfff)
      {
        ret_value = ecma_make_throw_obj_completion_value (ecma_new_standard_error (ECMA_ERROR_URI));
        break;
      }
      else
      {
        output_length += 9;
      }
    }
    else if (character <= 0x10ffff)
    {
      output_length += 12;
    }
    else
    {
      ret_value = ecma_make_throw_obj_completion_value (ecma_new_standard_error (ECMA_ERROR_URI));
      break;
    }
  }

  if (ecma_is_completion_value_empty (ret_value))
  {
    MEM_DEFINE_LOCAL_ARRAY (output_start_p,
                            output_length,
                            ecma_char_t);

    input_char_p = input_start_p;
    ecma_char_t *output_char_p = output_start_p;
    for (uint32_t i = 0; i < input_length; i++)
    {
      /*
       * FIXME: we should improve this when unicode support will be available.
       */
      uint32_t character = *input_char_p++;

      if (character <= 0x7f)
      {
        if (ecma_builtin_global_object_character_is_in (character, unescaped_uri_bitset))
        {
          *output_char_p++ = (ecma_char_t) character;
        }
        else
        {
          ecma_builtin_global_object_byte_to_hex (output_char_p, character);
          output_char_p += 3;
        }
      }
      else if (character <= 0x7ff)
      {
        ecma_builtin_global_object_byte_to_hex (output_char_p, 0xc0 | (character >> 6));
        output_char_p += 3;
        ecma_builtin_global_object_byte_to_hex (output_char_p, 0x80 | (character & 0x3f));
        output_char_p += 3;
      }
      else if (character <= 0xffff)
      {
        ecma_builtin_global_object_byte_to_hex (output_char_p, 0xe0 | (character >> 12));
        output_char_p += 3;
        ecma_builtin_global_object_byte_to_hex (output_char_p, 0x80 | ((character >> 6) & 0x3f));
        output_char_p += 3;
        ecma_builtin_global_object_byte_to_hex (output_char_p, 0x80 | (character & 0x3f));
        output_char_p += 3;
      }
      else
      {
        ecma_builtin_global_object_byte_to_hex (output_char_p, 0xf0 | (character >> 18));
        output_char_p += 3;
        ecma_builtin_global_object_byte_to_hex (output_char_p, 0x80 | ((character >> 12) & 0x3f));
        output_char_p += 3;
        ecma_builtin_global_object_byte_to_hex (output_char_p, 0x80 | ((character >> 6) & 0x3f));
        output_char_p += 3;
        ecma_builtin_global_object_byte_to_hex (output_char_p, 0x80 | (character & 0x3f));
        output_char_p += 3;
      }
    }

    *output_char_p = '\0';
    JERRY_ASSERT (output_start_p + output_length == output_char_p + 1);

    ecma_string_t *output_string_p = ecma_new_ecma_string (output_start_p);

    ret_value = ecma_make_normal_completion_value (ecma_make_string_value (output_string_p));

    MEM_FINALIZE_LOCAL_ARRAY (output_start_p);
  }

  MEM_FINALIZE_LOCAL_ARRAY (input_start_p);

  ECMA_FINALIZE (string);
  return ret_value;
}

/**
 * The Global object's 'encodeURI' routine
 *
 * See also:
 *          ECMA-262 v5, 15.1.3.3
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_global_object_encode_uri (ecma_value_t this_arg __attr_unused___, /**< this argument */
                                       ecma_value_t uri) /**< routine's first argument */
{
  return ecma_builtin_global_object_encode_uri_helper (uri, unescaped_uri_set);
} /* ecma_builtin_global_object_encode_uri */

/**
 * The Global object's 'encodeURIComponent' routine
 *
 * See also:
 *          ECMA-262 v5, 15.1.3.4
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_global_object_encode_uri_component (ecma_value_t this_arg __attr_unused___, /**< this argument */
                                                 ecma_value_t uri_component) /**< routine's first argument */
{
  return ecma_builtin_global_object_encode_uri_helper (uri_component, unescaped_uri_component_set);
} /* ecma_builtin_global_object_encode_uri_component */

/**
 * @}
 * @}
 * @}
 */
