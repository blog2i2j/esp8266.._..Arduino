/*
  StackThunk.h - Allow use second stack for BearSSL calls

  BearSSL uses a significant amount of stack space, much larger than
  the default Arduino core stack. These routines handle swapping
  between a secondary, user-allocated stack on the heap and the real
  stack.

  Copyright (c) 2017 Earle F. Philhower, III. All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
  Modified 8 May 2015 by Hristo Gochkov (proper post and file upload handling)
*/

#ifndef _STACKTHUNK_H
#define _STACKTHUNK_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void stack_thunk_yield(void);

extern void stack_thunk_add_ref();
extern void stack_thunk_del_ref();
extern void stack_thunk_repaint();

extern uint32_t stack_thunk_get_refcnt();
extern uint32_t stack_thunk_get_stack_top();
extern uint32_t stack_thunk_get_stack_bot();
extern uint32_t stack_thunk_get_cont_sp();
extern uint32_t stack_thunk_get_max_usage();
extern void stack_thunk_dump_stack();
extern void stack_thunk_fatal_smashing();

// Globals required for thunking operation
extern uint32_t *stack_thunk_ptr;
extern uint32_t *stack_thunk_top;
extern uint32_t *stack_thunk_save;
extern uint32_t stack_thunk_refcnt;

// Thunking macro
#define make_stack_thunk(fcnToThunk) \
__asm__ ("\n\
.text\n\
.literal_position\n\
.literal .LC_STACK_VALUE"#fcnToThunk", 0xdeadbeef\n\
\n\
.text\n\
.global thunk_"#fcnToThunk"\n\
.type thunk_"#fcnToThunk", @function\n\
.align 4\n\
thunk_"#fcnToThunk":\n\
  addi a1, a1, -16      /* Allocate space for saved registers on stack */\n\
  s32i a0, a1, 12       /* Store A0, trounced by calls */\n\
  s32i a15, a1, 8       /* Store A15 (our temporary one) */\n\
  movi a15, stack_thunk_save  /* Store A1(SP) in temp space */\n\
  s32i a1, a15, 0\n\
  movi a15, stack_thunk_top   /* Load A1(SP) with thunk stack */\n\
  l32i.n a1, a15, 0\n\
  call0 "#fcnToThunk"      /* Do the call */\n\
  /* Check the stack canary wasn't overwritten */\n\
  movi a15, stack_thunk_ptr\n\
  l32i.n a15, a15, 0    /* A15 now has the pointer to stack end*/ \n\
  l32i.n a15, a15, 0    /* A15 now has contents of last stack entry */\n\
  l32r a0, .LC_STACK_VALUE"#fcnToThunk" /* A0 now has the check value */\n\
  beq a0, a15, .L1"#fcnToThunk"\n\
  call0 stack_thunk_fatal_smashing\n\
.L1"#fcnToThunk":\n\
  movi a15, stack_thunk_save  /* Restore A1(SP) */\n\
  l32i.n a1, a15, 0/* Restore A1(SP) */\n\
  movi a0, 0 /* Purge temporary storage */\n\
  s32i.n a0, a15, 0\n\
  l32i.n a15, a1, 8/* Restore A15 */\n\
  l32i.n a0, a1, 12/* Restore A0 */\n\
  addi a1, a1, 16       /* Free up stack and return to caller */\n\
  ret\n\
.size thunk_"#fcnToThunk", . - thunk_"#fcnToThunk"\n");

#ifdef __cplusplus
}
#endif

#endif
