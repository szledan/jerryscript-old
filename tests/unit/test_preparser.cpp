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

#include <string.h>

#include "jrt.h"
#include "mem-allocator.h"
#include "opcodes.h"
#include "common.h"
#include "parser.h"
#include "serializer.h"

/**
 * Unit test's main function.
 */
int
main (int __attr_unused___ argc,
      char __attr_unused___ **argv)
{
  char program[] = "a=1;var a;";
  bool is_ok;

  mem_init ();
  serializer_init ();
  parser_set_show_opcodes (true);
  parser_init ();
  parser_parse_program (program, strlen (program));
  parser_free ();

  opcode_t opcodes[] =
  {
    getop_meta (OPCODE_META_TYPE_SCOPE_CODE_FLAGS, // [ ]
                OPCODE_SCOPE_CODE_FLAGS_NOT_REF_ARGUMENTS_IDENTIFIER
                | OPCODE_SCOPE_CODE_FLAGS_NOT_REF_EVAL_IDENTIFIER,
                INVALID_VALUE),
    getop_reg_var_decl (128, 129),  // var tmp128 .. tmp129;
    getop_var_decl (0),             // var a;
    getop_assignment (129, 1, 1),   // tmp129 = 1: SMALLINT;
    getop_assignment (0, 6, 129),   // a = tmp129 : TYPEOF (tmp129);
    getop_exitval (0)               // exit 0;
  };

  if (!opcodes_equal ((const opcode_t *) serializer_get_bytecode (), opcodes, 5))
  {
    is_ok = false;
  }
  else
  {
    is_ok = true;
  }

  serializer_free ();
  mem_finalize (false);

  return (is_ok ? 0 : 1);
} /* main */
