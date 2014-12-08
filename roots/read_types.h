// pass in file handle, this reads in the dwarf stuff, returns pointer to custom
// type.
// ask for type of variable, get type
// ask for information about struct layout.

#ifndef DWARF_INCLUDES
#define DWARF_INCLUDES

#define _DEFAULT_SOURCE

#include <dwarf.h>
#include <libdwarf.h>

#include <stdio.h>
#include "read_types.h"
#include <execinfo.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define UNW_LOCAL_ONLY
#include <libunwind.h>

#define INITIAL_ROOT_SIZE 30
#define INITIAL_BACKTRACE_SIZE 50

#include "dwarf_graph.h"

static const uint8_t x86_dwarf_to_libunwind_regnum[19] = {
    UNW_X86_EAX,    UNW_X86_ECX, UNW_X86_EDX, UNW_X86_EBX, UNW_X86_ESP,
    UNW_X86_EBP,    UNW_X86_ESI, UNW_X86_EDI, UNW_X86_EIP, UNW_X86_EFLAGS,
    UNW_X86_TRAPNO, UNW_X86_ST0, UNW_X86_ST1, UNW_X86_ST2, UNW_X86_ST3,
    UNW_X86_ST4,    UNW_X86_ST5, UNW_X86_ST6, UNW_X86_ST7};

int pc_range(Dwarf_Debug dbg, Dwarf_Die *fn_die, Dwarf_Addr *lowPC,
             Dwarf_Addr *highPC);

int dwarf_backtrace(CallStack **callStack);

bool is_pointer(Dwarf_Debug dbg, Dwarf_Die *die, Dwarf_Error *err);
int type_off(Dwarf_Die *die, Dwarf_Off *ref_off, Dwarf_Error *err);
int type_of(Dwarf_Debug dbg, Dwarf_Die *die, Dwarf_Die *type_die,
            Dwarf_Error *err);

int var_location(LiveFunction *fun, Dwarf_Locdesc **location_expression,
                 int expression_count, void **location);

int dwarf_read(const char *executable, GCContext **context);

#define DEFAULT_ROOT_COUNT 25

// CallStack and context are in parameters, roots are outparameters
int get_roots(CallStack *callStack, GCContext *context, Roots **roots);

#endif
