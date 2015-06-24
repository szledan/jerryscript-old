/* Copyright 2015 Samsung Electronics Co., Ltd.
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

#include "ecma-alloc.h"
#include "ecma-builtin-helpers.h"
#include "ecma-globals.h"
#include "ecma-helpers.h"
#include "ecma-objects.h"

#ifndef CONFIG_ECMA_COMPACT_PROFILE_DISABLE_DATE_BUILTIN

#define ECMA_BUILTINS_INTERNAL
#include "ecma-builtins-internal.h"

#define BUILTIN_INC_HEADER_NAME "ecma-builtin-date-prototype.inc.h"
#define BUILTIN_UNDERSCORED_ID date_prototype
#include "ecma-builtin-internal-routines-template.inc.h"

/** \addtogroup ecma ECMA
 * @{
 *
 * \addtogroup ecmabuiltins
 * @{
 *
 * \addtogroup dateprototype ECMA Date.prototype object built-in
 * @{
 */

/**
 * The Date.prototype object's 'toString' routine
 *
 * See also:
 *          ECMA-262 v5, 15.9.5.2
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_date_prototype_to_string (ecma_value_t this_arg) /**< this argument */
{
  ECMA_BUILTIN_CP_UNIMPLEMENTED (this_arg);
} /* ecma_builtin_date_prototype_to_string */

/**
 * The Date.prototype object's 'toDateString' routine
 *
 * See also:
 *          ECMA-262 v5, 15.9.5.3
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_date_prototype_to_date_string (ecma_value_t this_arg) /**< this argument */
{
  ECMA_BUILTIN_CP_UNIMPLEMENTED (this_arg);
} /* ecma_builtin_date_prototype_to_date_string */

/**
 * The Date.prototype object's 'toTimeString' routine
 *
 * See also:
 *          ECMA-262 v5, 15.9.5.4
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_date_prototype_to_time_string (ecma_value_t this_arg) /**< this argument */
{
  ECMA_BUILTIN_CP_UNIMPLEMENTED (this_arg);
} /* ecma_builtin_date_prototype_to_time_string */

/**
 * The Date.prototype object's 'toLocaleString' routine
 *
 * See also:
 *          ECMA-262 v5, 15.9.5.5
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_date_prototype_to_locale_string (ecma_value_t this_arg) /**< this argument */
{
  ECMA_BUILTIN_CP_UNIMPLEMENTED (this_arg);
} /* ecma_builtin_date_prototype_to_locale_string */

/**
 * The Date.prototype object's 'toLocaleDateString' routine
 *
 * See also:
 *          ECMA-262 v5, 15.9.5.6
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_date_prototype_to_locale_date_string (ecma_value_t this_arg) /**< this argument */
{
  ECMA_BUILTIN_CP_UNIMPLEMENTED (this_arg);
} /* ecma_builtin_date_prototype_to_locale_date_string */

/**
 * The Date.prototype object's 'toLocaleTimeString' routine
 *
 * See also:
 *          ECMA-262 v5, 15.9.5.7
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_date_prototype_to_locale_time_string (ecma_value_t this_arg) /**< this argument */
{
  ECMA_BUILTIN_CP_UNIMPLEMENTED (this_arg);
} /* ecma_builtin_date_prototype_to_locale_time_string */

/**
 * The Date.prototype object's 'valueOf' routine
 *
 * See also:
 *          ECMA-262 v5, 15.9.5.8
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_date_prototype_value_of (ecma_value_t this_arg) /**< this argument */
{
  return ecma_builtin_date_prototype_get_time (this_arg);
} /* ecma_builtin_date_prototype_value_of */

/**
 * The Date.prototype object's 'getTime' routine
 *
 * See also:
 *          ECMA-262 v5, 15.9.5.9
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_date_prototype_get_time (ecma_value_t this_arg) /**< this argument */
{
  if (!(ecma_is_value_object (this_arg)
        && (ecma_object_get_class_name (obj_p) == ECMA_OBJECT_TYPE_DATE)))
  {
    return ecma_make_throw_obj_completion_value (ecma_new_standard_error (ECMA_ERROR_TYPE));
  }

  ecma_property_t *prim_value_prop_p = ecma_get_internal_property (obj_p,
                                                                   ECMA_INTERNAL_PROPERTY_PRIMITIVE_DATE_VALUE);

  ecma_number_t *prim_value_num_p = ECMA_GET_NON_NULL_POINTER (ecma_number_t,
                                                               prim_value_prop_p->u.internal_property.value);

  ecma_number_t *ret_num_p = ecma_alloc_number ();
  *ret_num_p = *prim_value_num_p;

  return ecma_make_normal_completion_value (ecma_make_number_value (ret_num_p));
} /* ecma_builtin_date_prototype_get_time */

/**
 * The Date.prototype object's 'getFullYear' routine
 *
 * See also:
 *          ECMA-262 v5, 15.9.5.10
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_date_prototype_get_full_year (ecma_value_t this_arg) /**< this argument */
{
  ecma_completion_value_t ret_value = ecma_make_empty_completion_value ();

  /* 1. */
  ECMA_TRY_CATCH (value, ecma_builtin_date_prototype_get_time (this_arg), ret_value);
  ecma_number_t *this_num = ecma_get_number_from_value (value);

  /* 2. */
  if (ecma_number_is_nan (*this_num))
  {
    ecma_string_t *nan_str_p = ecma_get_magic_string (ECMA_MAGIC_STRING_NAN);
    ret_value = ecma_make_normal_completion_value (ecma_make_string_value (nan_str_p));
  }
  else
  {
    /* 3. */
    int full_year = ecma_date_year_from_time (ecma_date_local_time (*this_num));

    ecma_number_t *ret_num_p = ecma_alloc_number ();
    *ret_num_p = ecma_int32_to_number (full_year);
    ret_value = ecma_make_normal_completion_value (ecma_make_number_value (ret_num_p));
  }
  ECMA_FINALIZE (value);

  return ret_value;
} /* ecma_builtin_date_prototype_get_full_year */

/**
 * The Date.prototype object's 'getUTCFullYear' routine
 *
 * See also:
 *          ECMA-262 v5, 15.9.5.11
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_date_prototype_get_utc_full_year (ecma_value_t this_arg) /**< this argument */
{
  ecma_completion_value_t ret_value = ecma_make_empty_completion_value ();

  /* 1. */
  ECMA_TRY_CATCH (value, ecma_builtin_date_prototype_get_time (this_arg), ret_value);
  ecma_number_t *this_num = ecma_get_number_from_value (value);

  /* 2. */
  if (ecma_number_is_nan (*this_num))
  {
    ecma_string_t *nan_str_p = ecma_get_magic_string (ECMA_MAGIC_STRING_NAN);
    ret_value = ecma_make_normal_completion_value (ecma_make_string_value (nan_str_p));
  }
  else
  {
    /* 3. */
    int utc_full_year = ecma_date_year_from_time (*this_num);

    ecma_number_t *ret_num_p = ecma_alloc_number ();
    *ret_num_p = ecma_int32_to_number (utc_full_year);
    ret_value = ecma_make_normal_completion_value (ecma_make_number_value (ret_num_p));
  }
  ECMA_FINALIZE (value);

  return ret_value;
} /* ecma_builtin_date_prototype_get_utc_full_year */

/**
 * The Date.prototype object's 'getMonth' routine
 *
 * See also:
 *          ECMA-262 v5, 15.9.5.12
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_date_prototype_get_month (ecma_value_t this_arg) /**< this argument */
{
  ecma_completion_value_t ret_value = ecma_make_empty_completion_value ();

  /* 1. */
  ECMA_TRY_CATCH (value, ecma_builtin_date_prototype_get_time (this_arg), ret_value);
  ecma_number_t *this_num = ecma_get_number_from_value (value);

  /* 2. */
  if (ecma_number_is_nan (*this_num))
  {
    ecma_string_t *nan_str_p = ecma_get_magic_string (ECMA_MAGIC_STRING_NAN);
    ret_value = ecma_make_normal_completion_value (ecma_make_string_value (nan_str_p));
  }
  else
  {
    /* 3. */
    int month = ecma_date_month_from_time (ecma_date_local_time (*this_num));

    ecma_number_t *ret_num_p = ecma_alloc_number ();
    *ret_num_p = ecma_int32_to_number (month);
    ret_value = ecma_make_normal_completion_value (ecma_make_number_value (ret_num_p));
  }
  ECMA_FINALIZE (value);

  return ret_value;
} /* ecma_builtin_date_prototype_get_month */

/**
 * The Date.prototype object's 'getUTCMonth' routine
 *
 * See also:
 *          ECMA-262 v5, 15.9.5.13
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_date_prototype_get_utc_month (ecma_value_t this_arg) /**< this argument */
{
  ecma_completion_value_t ret_value = ecma_make_empty_completion_value ();

  /* 1. */
  ECMA_TRY_CATCH (value, ecma_builtin_date_prototype_get_time (this_arg), ret_value);
  ecma_number_t *this_num = ecma_get_number_from_value (value);

  /* 2. */
  if (ecma_number_is_nan (*this_num))
  {
    ecma_string_t *nan_str_p = ecma_get_magic_string (ECMA_MAGIC_STRING_NAN);
    ret_value = ecma_make_normal_completion_value (ecma_make_string_value (nan_str_p));
  }
  else
  {
    /* 3. */
    int utc_month = ecma_date_month_from_time (*this_num);

    ecma_number_t *ret_num_p = ecma_alloc_number ();
    *ret_num_p = ecma_int32_to_number (utc_month);
    ret_value = ecma_make_normal_completion_value (ecma_make_number_value (ret_num_p));
  }
  ECMA_FINALIZE (value);

  return ret_value;
} /* ecma_builtin_date_prototype_get_utc_month */

/**
 * The Date.prototype object's 'getDate' routine
 *
 * See also:
 *          ECMA-262 v5, 15.9.5.14
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_date_prototype_get_date (ecma_value_t this_arg) /**< this argument */
{
  ecma_completion_value_t ret_value = ecma_make_empty_completion_value ();

  /* 1. */
  ECMA_TRY_CATCH (value, ecma_builtin_date_prototype_get_time (this_arg), ret_value);
  ecma_number_t *this_num = ecma_get_number_from_value (value);

  /* 2. */
  if (ecma_number_is_nan (*this_num))
  {
    ecma_string_t *nan_str_p = ecma_get_magic_string (ECMA_MAGIC_STRING_NAN);
    ret_value = ecma_make_normal_completion_value (ecma_make_string_value (nan_str_p));
  }
  else
  {
    /* 3. */
    int date = ecma_date_date_from_time (ecma_date_local_time (*this_num));

    ecma_number_t *ret_num_p = ecma_alloc_number ();
    *ret_num_p = ecma_int32_to_number (date);
    ret_value = ecma_make_normal_completion_value (ecma_make_number_value (ret_num_p));
  }
  ECMA_FINALIZE (value);

  return ret_value;
} /* ecma_builtin_date_prototype_get_date */

/**
 * The Date.prototype object's 'getUTCDate' routine
 *
 * See also:
 *          ECMA-262 v5, 15.9.5.15
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_date_prototype_get_utc_date (ecma_value_t this_arg) /**< this argument */
{
  ecma_completion_value_t ret_value = ecma_make_empty_completion_value ();

  /* 1. */
  ECMA_TRY_CATCH (value, ecma_builtin_date_prototype_get_time (this_arg), ret_value);
  ecma_number_t *this_num = ecma_get_number_from_value (value);

  /* 2. */
  if (ecma_number_is_nan (*this_num))
  {
    ecma_string_t *nan_str_p = ecma_get_magic_string (ECMA_MAGIC_STRING_NAN);
    ret_value = ecma_make_normal_completion_value (ecma_make_string_value (nan_str_p));
  }
  else
  {
    /* 3. */
    int utc_date = ecma_date_date_from_time (*this_num);

    ecma_number_t *ret_num_p = ecma_alloc_number ();
    *ret_num_p = ecma_int32_to_number (utc_date);
    ret_value = ecma_make_normal_completion_value (ecma_make_number_value (ret_num_p));
  }
  ECMA_FINALIZE (value);

  return ret_value;
} /* ecma_builtin_date_prototype_get_utc_date */

/**
 * The Date.prototype object's 'getDay' routine
 *
 * See also:
 *          ECMA-262 v5, 15.9.5.16
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_date_prototype_get_day (ecma_value_t this_arg) /**< this argument */
{
  ecma_completion_value_t ret_value = ecma_make_empty_completion_value ();

  /* 1. */
  ECMA_TRY_CATCH (value, ecma_builtin_date_prototype_get_time (this_arg), ret_value);
  ecma_number_t *this_num = ecma_get_number_from_value (value);

  /* 2. */
  if (ecma_number_is_nan (*this_num))
  {
    ecma_string_t *nan_str_p = ecma_get_magic_string (ECMA_MAGIC_STRING_NAN);
    ret_value = ecma_make_normal_completion_value (ecma_make_string_value (nan_str_p));
  }
  else
  {
    /* 3. */
    int day = ecma_date_week_day (ecma_date_local_time (*this_num));

    ecma_number_t *ret_num_p = ecma_alloc_number ();
    *ret_num_p = ecma_int32_to_number (day);
    ret_value = ecma_make_normal_completion_value (ecma_make_number_value (ret_num_p));
  }
  ECMA_FINALIZE (value);

  return ret_value;
} /* ecma_builtin_date_prototype_get_day */

/**
 * The Date.prototype object's 'getUTCDay' routine
 *
 * See also:
 *          ECMA-262 v5, 15.9.5.17
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_date_prototype_get_utc_day (ecma_value_t this_arg) /**< this argument */
{
  ecma_completion_value_t ret_value = ecma_make_empty_completion_value ();

  /* 1. */
  ECMA_TRY_CATCH (value, ecma_builtin_date_prototype_get_time (this_arg), ret_value);
  ecma_number_t *this_num = ecma_get_number_from_value (value);

  /* 2. */
  if (ecma_number_is_nan (*this_num))
  {
    ecma_string_t *nan_str_p = ecma_get_magic_string (ECMA_MAGIC_STRING_NAN);
    ret_value = ecma_make_normal_completion_value (ecma_make_string_value (nan_str_p));
  }
  else
  {
    /* 3. */
    int utc_day = ecma_date_week_day (*this_num);

    ecma_number_t *ret_num_p = ecma_alloc_number ();
    *ret_num_p = ecma_int32_to_number (utc_day);
    ret_value = ecma_make_normal_completion_value (ecma_make_number_value (ret_num_p));
  }
  ECMA_FINALIZE (value);

  return ret_value;
} /* ecma_builtin_date_prototype_get_utc_day */

/**
 * The Date.prototype object's 'getHours' routine
 *
 * See also:
 *          ECMA-262 v5, 15.9.5.18
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_date_prototype_get_hours (ecma_value_t this_arg) /**< this argument */
{
  ecma_completion_value_t ret_value = ecma_make_empty_completion_value ();

  /* 1. */
  ECMA_TRY_CATCH (value, ecma_builtin_date_prototype_get_time (this_arg), ret_value);
  ecma_number_t *this_num = ecma_get_number_from_value (value);

  /* 2. */
  if (ecma_number_is_nan (*this_num))
  {
    ecma_string_t *nan_str_p = ecma_get_magic_string (ECMA_MAGIC_STRING_NAN);
    ret_value = ecma_make_normal_completion_value (ecma_make_string_value (nan_str_p));
  }
  else
  {
    /* 3. */
    ecma_number_t hour = ecma_date_hour_from_time (ecma_date_local_time (*this_num));

    ecma_number_t *ret_num_p = ecma_alloc_number ();
    *ret_num_p = hour;
    ret_value = ecma_make_normal_completion_value (ecma_make_number_value (ret_num_p));
  }
  ECMA_FINALIZE (value);

  return ret_value;
} /* ecma_builtin_date_prototype_get_hours */

/**
 * The Date.prototype object's 'getUTCHours' routine
 *
 * See also:
 *          ECMA-262 v5, 15.9.5.19
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_date_prototype_get_utc_hours (ecma_value_t this_arg) /**< this argument */
{
  ecma_completion_value_t ret_value = ecma_make_empty_completion_value ();

  /* 1. */
  ECMA_TRY_CATCH (value, ecma_builtin_date_prototype_get_time (this_arg), ret_value);
  ecma_number_t *this_num = ecma_get_number_from_value (value);

  /* 2. */
  if (ecma_number_is_nan (*this_num))
  {
    ecma_string_t *nan_str_p = ecma_get_magic_string (ECMA_MAGIC_STRING_NAN);
    ret_value = ecma_make_normal_completion_value (ecma_make_string_value (nan_str_p));
  }
  else
  {
    /* 3. */
    ecma_number_t utc_hour = ecma_date_hour_from_time (*this_num);

    ecma_number_t *ret_num_p = ecma_alloc_number ();
    *ret_num_p = utc_hour;
    ret_value = ecma_make_normal_completion_value (ecma_make_number_value (ret_num_p));
  }
  ECMA_FINALIZE (value);

  return ret_value;
} /* ecma_builtin_date_prototype_get_utc_hours */

/**
 * The Date.prototype object's 'getMinutes' routine
 *
 * See also:
 *          ECMA-262 v5, 15.9.5.20
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_date_prototype_get_minutes (ecma_value_t this_arg) /**< this argument */
{
  ecma_completion_value_t ret_value = ecma_make_empty_completion_value ();

  /* 1. */
  ECMA_TRY_CATCH (value, ecma_builtin_date_prototype_get_time (this_arg), ret_value);
  ecma_number_t *this_num = ecma_get_number_from_value (value);

  /* 2. */
  if (ecma_number_is_nan (*this_num))
  {
    ecma_string_t *nan_str_p = ecma_get_magic_string (ECMA_MAGIC_STRING_NAN);
    ret_value = ecma_make_normal_completion_value (ecma_make_string_value (nan_str_p));
  }
  else
  {
    /* 3. */
    ecma_number_t minutes = ecma_date_min_from_time (ecma_date_local_time (*this_num));

    ecma_number_t *ret_num_p = ecma_alloc_number ();
    *ret_num_p = minutes;
    ret_value = ecma_make_normal_completion_value (ecma_make_number_value (ret_num_p));
  }
  ECMA_FINALIZE (value);

  return ret_value;
} /* ecma_builtin_date_prototype_get_minutes */

/**
 * The Date.prototype object's 'getUTCMinutes' routine
 *
 * See also:
 *          ECMA-262 v5, 15.9.5.21
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_date_prototype_get_utc_minutes (ecma_value_t this_arg) /**< this argument */
{
  ecma_completion_value_t ret_value = ecma_make_empty_completion_value ();

  /* 1. */
  ECMA_TRY_CATCH (value, ecma_builtin_date_prototype_get_time (this_arg), ret_value);
  ecma_number_t *this_num = ecma_get_number_from_value (value);

  /* 2. */
  if (ecma_number_is_nan (*this_num))
  {
    ecma_string_t *nan_str_p = ecma_get_magic_string (ECMA_MAGIC_STRING_NAN);
    ret_value = ecma_make_normal_completion_value (ecma_make_string_value (nan_str_p));
  }
  else
  {
    /* 3. */
    ecma_number_t utc_minutes = ecma_date_min_from_time (*this_num);

    ecma_number_t *ret_num_p = ecma_alloc_number ();
    *ret_num_p = utc_minutes;
    ret_value = ecma_make_normal_completion_value (ecma_make_number_value (ret_num_p));
  }
  ECMA_FINALIZE (value);

  return ret_value;
} /* ecma_builtin_date_prototype_get_utc_minutes */

/**
 * The Date.prototype object's 'getSeconds' routine
 *
 * See also:
 *          ECMA-262 v5, 15.9.5.22
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_date_prototype_get_seconds (ecma_value_t this_arg) /**< this argument */
{
  ECMA_BUILTIN_CP_UNIMPLEMENTED (this_arg);
} /* ecma_builtin_date_prototype_get_seconds */

/**
 * The Date.prototype object's 'getUTCSeconds' routine
 *
 * See also:
 *          ECMA-262 v5, 15.9.5.23
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_date_prototype_get_utc_seconds (ecma_value_t this_arg) /**< this argument */
{
  ECMA_BUILTIN_CP_UNIMPLEMENTED (this_arg);
} /* ecma_builtin_date_prototype_get_utc_seconds */

/**
 * The Date.prototype object's 'getMilliseconds' routine
 *
 * See also:
 *          ECMA-262 v5, 15.9.5.24
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_date_prototype_get_milliseconds (ecma_value_t this_arg) /**< this argument */
{
  ECMA_BUILTIN_CP_UNIMPLEMENTED (this_arg);
} /* ecma_builtin_date_prototype_get_milliseconds */

/**
 * The Date.prototype object's 'getUTCMilliseconds' routine
 *
 * See also:
 *          ECMA-262 v5, 15.9.5.25
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_date_prototype_get_utc_milliseconds (ecma_value_t this_arg) /**< this argument */
{
  ECMA_BUILTIN_CP_UNIMPLEMENTED (this_arg);
} /* ecma_builtin_date_prototype_get_utc_milliseconds */

/**
 * The Date.prototype object's 'getTimezoneOffset' routine
 *
 * See also:
 *          ECMA-262 v5, 15.9.5.26
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_date_prototype_get_timezone_offset (ecma_value_t this_arg) /**< this argument */
{
  ECMA_BUILTIN_CP_UNIMPLEMENTED (this_arg);
} /* ecma_builtin_date_prototype_get_timezone_offset */

/**
 * The Date.prototype object's 'setTime' routine
 *
 * See also:
 *          ECMA-262 v5, 15.9.5.27
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_date_prototype_set_time (ecma_value_t this_arg, /**< this argument */
                                      ecma_value_t arg) /**< time */
{
  ECMA_BUILTIN_CP_UNIMPLEMENTED (this_arg, arg);
} /* ecma_builtin_date_prototype_set_time */

/**
 * The Date.prototype object's 'setMilliseconds' routine
 *
 * See also:
 *          ECMA-262 v5, 15.9.5.28
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_date_prototype_set_milliseconds (ecma_value_t this_arg, /**< this argument */
                                              ecma_value_t arg) /**< millisecond */
{
  ECMA_BUILTIN_CP_UNIMPLEMENTED (this_arg, arg);
} /* ecma_builtin_date_prototype_set_milliseconds */

/**
 * The Date.prototype object's 'setUTCMilliseconds' routine
 *
 * See also:
 *          ECMA-262 v5, 15.9.5.29
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_date_prototype_set_utc_milliseconds (ecma_value_t this_arg, /**< this argument */
                                                  ecma_value_t arg) /**< millisecond */
{
  ECMA_BUILTIN_CP_UNIMPLEMENTED (this_arg, arg);
} /* ecma_builtin_date_prototype_set_utc_milliseconds */

/**
 * The Date.prototype object's 'setSeconds' routine
 *
 * See also:
 *          ECMA-262 v5, 15.9.5.30
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_date_prototype_set_seconds (ecma_value_t this_arg, /**< this argument */
                                         ecma_value_t arg1, /**< second */
                                         ecma_value_t arg2) /**< millisecond */
{
  ECMA_BUILTIN_CP_UNIMPLEMENTED (this_arg, arg1, arg2);
} /* ecma_builtin_date_prototype_set_seconds */

/**
 * The Date.prototype object's 'setUTCSeconds' routine
 *
 * See also:
 *          ECMA-262 v5, 15.9.5.31
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_date_prototype_set_utc_seconds (ecma_value_t this_arg, /**< this argument */
                                             ecma_value_t arg1, /**< second */
                                             ecma_value_t arg2) /**< millisecond */
{
  ECMA_BUILTIN_CP_UNIMPLEMENTED (this_arg, arg1, arg2);
} /* ecma_builtin_date_prototype_set_utc_seconds */

/**
 * The Date.prototype object's 'setMinutes' routine
 *
 * See also:
 *          ECMA-262 v5, 15.9.5.32
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_date_prototype_set_minutes (ecma_value_t this_arg, /**< this argument */
                                         const ecma_value_t args[], /**< arguments list */
                                         ecma_length_t args_number) /**< number of arguments */
{
  ECMA_BUILTIN_CP_UNIMPLEMENTED (this_arg, args, args_number);
} /* ecma_builtin_date_prototype_set_minutes */

/**
 * The Date.prototype object's 'setUTCMinutes' routine
 *
 * See also:
 *          ECMA-262 v5, 15.9.5.33
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_date_prototype_set_utc_minutes (ecma_value_t this_arg, /**< this argument */
                                             const ecma_value_t args[], /**< arguments list */
                                             ecma_length_t args_number) /**< number of arguments */
{
  ECMA_BUILTIN_CP_UNIMPLEMENTED (this_arg, args, args_number);
} /* ecma_builtin_date_prototype_set_utc_minutes */

/**
 * The Date.prototype object's 'setHours' routine
 *
 * See also:
 *          ECMA-262 v5, 15.9.5.34
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_date_prototype_set_hours (ecma_value_t this_arg, /**< this argument */
                                       const ecma_value_t args[], /**< arguments list */
                                       ecma_length_t args_number) /**< number of arguments */
{
  ECMA_BUILTIN_CP_UNIMPLEMENTED (this_arg, args, args_number);
} /* ecma_builtin_date_prototype_set_hours */

/**
 * The Date.prototype object's 'setUTCHours' routine
 *
 * See also:
 *          ECMA-262 v5, 15.9.5.35
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_date_prototype_set_utc_hours (ecma_value_t this_arg, /**< this argument */
                                           const ecma_value_t args[], /**< arguments list */
                                           ecma_length_t args_number) /**< number of arguments */
{
  ECMA_BUILTIN_CP_UNIMPLEMENTED (this_arg, args, args_number);
} /* ecma_builtin_date_prototype_set_utc_hours */

/**
 * The Date.prototype object's 'setDate' routine
 *
 * See also:
 *          ECMA-262 v5, 15.9.5.36
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_date_prototype_set_date (ecma_value_t this_arg, /**< this argument */
                                      ecma_value_t arg) /**< date */
{
  ECMA_BUILTIN_CP_UNIMPLEMENTED (this_arg, arg);
} /* ecma_builtin_date_prototype_set_date */

/**
 * The Date.prototype object's 'setUTCDate' routine
 *
 * See also:
 *          ECMA-262 v5, 15.9.5.37
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_date_prototype_set_utc_date (ecma_value_t this_arg, /**< this argument */
                                          ecma_value_t arg) /**< date */
{
  ECMA_BUILTIN_CP_UNIMPLEMENTED (this_arg, arg);
} /* ecma_builtin_date_prototype_set_utc_date */

/**
 * The Date.prototype object's 'setMonth' routine
 *
 * See also:
 *          ECMA-262 v5, 15.9.5.38
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_date_prototype_set_month (ecma_value_t this_arg, /**< this argument */
                                       ecma_value_t arg1, /**< month */
                                       ecma_value_t arg2) /**< day */
{
  ECMA_BUILTIN_CP_UNIMPLEMENTED (this_arg, arg1, arg2);
} /* ecma_builtin_date_prototype_set_month */

/**
 * The Date.prototype object's 'setUTCMonth' routine
 *
 * See also:
 *          ECMA-262 v5, 15.9.5.39
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_date_prototype_set_utc_month (ecma_value_t this_arg, /**< this argument */
                                           ecma_value_t arg1, /**< month */
                                           ecma_value_t arg2) /**< day */
{
  ECMA_BUILTIN_CP_UNIMPLEMENTED (this_arg, arg1, arg2);
} /* ecma_builtin_date_prototype_set_utc_month */

/**
 * The Date.prototype object's 'setFullYear' routine
 *
 * See also:
 *          ECMA-262 v5, 15.9.5.40
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_date_prototype_set_full_year (ecma_value_t this_arg, /**< this argument */
                                           const ecma_value_t args[], /**< arguments list */
                                           ecma_length_t args_number) /**< number of arguments */
{
  ECMA_BUILTIN_CP_UNIMPLEMENTED (this_arg, args, args_number);
} /* ecma_builtin_date_prototype_set_full_year */

/**
 * The Date.prototype object's 'setUTCFullYear' routine
 *
 * See also:
 *          ECMA-262 v5, 15.9.5.41
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_date_prototype_set_utc_full_year (ecma_value_t this_arg, /**< this argument */
                                               const ecma_value_t args[], /**< arguments list */
                                               ecma_length_t args_number) /**< number of arguments */
{
  ECMA_BUILTIN_CP_UNIMPLEMENTED (this_arg, args, args_number);
} /* ecma_builtin_date_prototype_set_utc_full_year */

/**
 * The Date.prototype object's 'toUTCString' routine
 *
 * See also:
 *          ECMA-262 v5, 15.9.5.42
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_date_prototype_to_utc_string (ecma_value_t this_arg) /**< this argument */
{
  ECMA_BUILTIN_CP_UNIMPLEMENTED (this_arg);
} /* ecma_builtin_date_prototype_to_utc_string */

/**
 * The Date.prototype object's 'toISOString' routine
 *
 * See also:
 *          ECMA-262 v5, 15.9.5.43
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_date_prototype_to_iso_string (ecma_value_t this_arg) /**< this argument */
{
  ECMA_BUILTIN_CP_UNIMPLEMENTED (this_arg);
} /* ecma_builtin_date_prototype_to_iso_string */

/**
 * The Date.prototype object's 'toJSON' routine
 *
 * See also:
 *          ECMA-262 v5, 15.9.5.44
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_date_prototype_to_json (ecma_value_t this_arg, /**< this argument */
                                     ecma_value_t arg) /**< key */
{
  ECMA_BUILTIN_CP_UNIMPLEMENTED (this_arg, arg);
} /* ecma_builtin_date_prototype_to_json */

/**
 * @}
 * @}
 * @}
 */

#endif /* !CONFIG_ECMA_COMPACT_PROFILE_DISABLE_DATE_BUILTIN */

