/* Copyright 2014-2015 Samsung Electronics Co., Ltd.
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

#ifndef DESERIALIZER_H
#define DESERIALIZER_H

#include "jrt.h"
#include "ecma-globals.h"
#include "opcodes.h"
#include "scopes-tree.h"
#include "literal.h"

void deserializer_init (void);
void deserializer_set_strings_buffer (const ecma_char_t *);
literal deserialize_literal_by_id (literal_index_t);
literal_index_t deserialize_lit_id_by_uid (uint8_t, opcode_counter_t);
const void *deserialize_bytecode (void);
opcode_t deserialize_opcode (opcode_counter_t);
op_meta deserialize_op_meta (opcode_counter_t);
uint8_t deserialize_min_temp (void);
void deserializer_free (void);

#endif //DESERIALIZER_H