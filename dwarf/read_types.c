#include "read_types.h"

void freeArray(Array array) {
  if (array->contents) {
    free(array->contents);
  }
  free(array);
}

void arrayAppend(Array array, void *item) {
  if (array->count >= array->capacity) {
    array->capacity *= 2;
    array->contents =
        realloc(array->contents, array->capacity * sizeof(void *));
  }
  array->contents[array->count] = item;
  array->count++;
}

Array newHeapArray(int capacity) {
  Array out = calloc(1, sizeof(struct HeapArray));

  out->count = 0;
  out->capacity = capacity;
  out->contents = calloc(out->capacity, sizeof(void *));

  return out;
}

bool is_pointer(Dwarf_Debug dbg, Dwarf_Die *die, Dwarf_Error *err) {
  Dwarf_Half type_tag;

  Dwarf_Die type_die;

  if (type_of(dbg, die, &type_die, err) != DW_DLV_OK) {
    fprintf(stderr, "Error getting type offset\n");
    exit(-11);
  }

  if (dwarf_tag(type_die, &type_tag, err) != DW_DLV_OK) {
    perror("Error in dwarf_tag\n");
    exit(-1);
  }

  return type_tag == DW_TAG_pointer_type;
}

int type_off(Dwarf_Die *die, Dwarf_Off *ref_off, Dwarf_Error *err) {
  Dwarf_Attribute type;
  int status;

  if ((status = dwarf_attr(*die, DW_AT_type, &type, err)) != DW_DLV_OK) {
    if (status == DW_DLV_NO_ENTRY) {
      fprintf(stderr, "No type information associated with die\n");
    } else {
      fprintf(stderr, "Error %d in getting type attribute: %s\n", status,
              dwarf_errmsg(*err));
    }
    return status;
  }

  if ((status = dwarf_global_formref(type, ref_off, err)) != DW_DLV_OK) {
    fprintf(stderr, "Error %d in getting type offset: %s\n", status,
            dwarf_errmsg(*err));
    return status;
  }

  return DW_DLV_OK;
}

int type_of(Dwarf_Debug dbg, Dwarf_Die *die, Dwarf_Die *type_die,
            Dwarf_Error *err) {
  Dwarf_Attribute type;
  Dwarf_Off ref_off = 0;

  if (type_off(die, &ref_off, err) != DW_DLV_OK) {
    fprintf(stderr, "Error in getting type offset: %s\n", dwarf_errmsg(*err));
    return -1;
  }

  int status;

  if ((status = dwarf_offdie(dbg, ref_off, type_die, err)) != DW_DLV_OK) {
    fprintf(stderr, "Error %d in getting die at offset: %s\n", status,
            dwarf_errmsg(*err));
    return status;
  }

  return DW_DLV_OK;
}

int var_location(LiveFunction *fun, Dwarf_Locdesc **llbufarray,
                 int expression_count, void **location) {

  for (int i = 0; i < expression_count; ++i) {
    Dwarf_Locdesc *llbuf = llbufarray[i];

    if (llbuf->ld_cents <= 0) {
      fprintf(stderr, "Expression has no nodes.\n");
      exit(-1);
    }

    Dwarf_Small op = llbuf->ld_s[0].lr_atom;

    Dwarf_Addr pc = (Dwarf_Addr)fun->pc;

    if (op == DW_OP_fbreg) {

      /* printf("fbreg: \n"); */

      if (llbuf->ld_lopc != 0 && llbuf->ld_lopc > pc) {
        continue;
      }

      if (llbuf->ld_hipc != 0 && llbuf->ld_hipc < pc) {
        continue;
      }

      Dwarf_Unsigned offset = llbuf->ld_s[0].lr_number;

      *location = (void *)fun->sp + offset;

      return DW_DLV_OK;

    } else {

      Dwarf_Signed offset;

      unw_regnum_t reg;
      unw_word_t reg_value = 0;

      switch (op) {
      case DW_OP_breg0:
      case DW_OP_breg1:
      case DW_OP_breg2:
      case DW_OP_breg3:
      case DW_OP_breg4:
      case DW_OP_breg5:
      case DW_OP_breg6:
      case DW_OP_breg7:
      case DW_OP_breg8:
      case DW_OP_breg9:
      case DW_OP_breg10:
      case DW_OP_breg11:
      case DW_OP_breg12:
      case DW_OP_breg13:
      case DW_OP_breg14:
      case DW_OP_breg15:
      case DW_OP_breg16:
      case DW_OP_breg17:
      case DW_OP_breg18: {
        reg = x86_dwarf_to_libunwind_regnum[op - DW_OP_breg0];

        offset = llbuf->ld_s[i].lr_number;

        char *buff = calloc(100, sizeof(char));
        unw_word_t offp;
        unw_get_proc_name(&(fun->cursor), buff, 100, &offp);
        /* printf("name: %s\n", buff); */

        if (unw_get_reg(&(fun->cursor), reg, &reg_value) != 0) {
          fprintf(stderr, "Error occurred reading register\n");
          exit(-1);
        }

        *location = (void *)reg_value + offset;

        /* printf("breg %d register: %s register value: %p offset: %ld,
         * location: %p\n", */
        /*        reg, */
        /*        unw_regname(reg), */
        /*        (void *)reg_value, */
        /*        offset, */
        /*        *location); */

        return DW_DLV_OK;

        break;
      }

      default:
        printf("other: %x\n", op);
      }
    }
  }

  return 0;
}

void freeCallstack(CallStack *callStack) {
  free(callStack->stack);
  free(callStack);
}

int dwarf_backtrace(CallStack **returnStack) {
  *returnStack = calloc(1, sizeof(CallStack));

  CallStack *callStack = *returnStack;
  callStack->stack = calloc(INITIAL_LIVE_FUNCTION_SIZE, sizeof(LiveFunction));
  callStack->count = 0;
  callStack->capacity = INITIAL_LIVE_FUNCTION_SIZE;

  unw_cursor_t cursor;

  unw_context_t uc;
  unw_word_t ip, sp;

  unw_getcontext(&uc);
  unw_init_local(&cursor, &uc);
  while (unw_step(&cursor) > 0) {
    unw_get_reg(&cursor, UNW_REG_IP, &ip);
    unw_get_reg(&cursor, UNW_REG_SP, &sp);

    if (callStack->count >= callStack->capacity) {
      callStack->capacity *= 2;
      callStack->stack = realloc(callStack->stack,
                                 callStack->capacity * sizeof(LiveFunction *));
    }
    callStack->stack[callStack->count].cursor = cursor;
    callStack->stack[callStack->count].pc = (void *)ip;
    callStack->stack[callStack->count].sp = (void *)sp;
    printf("%p => %p\n", (void *)ip, (void *)sp);
    callStack->count++;
  }

  return callStack->count;
}

// Replace with something that uses data structures
/*
int type_roots(TypedPointers* out){
  if(dwarfHandle == NULL){
    perror("Uninitialized dwarf handle\n");
  } else {
    func_dies_in_stack(dwarfHandle, callStack, frames);

    for(int i=0; i < callStack->count; i++){
      printf("pc: %p sp: %p\n", (void *)callStack->stack[i].pc, (void
*)callStack->stack[i].sp);
    }

    if(frames > 0){
      for(int i=0; i < callStack->count; i++){
      }
    }

  return 0;
};

void type_fun(Dwarf_Debug dbg, LiveFunction* fun, TypedPointers* roots,
Dwarf_Error* err){
  Dwarf_Die child_die;

  // Expect the CU DIE to have children
  if(dwarf_child(fun->fn_die, &child_die, err) == DW_DLV_ERROR){
    perror("Error getting child of function DIE\n");
    return;
  }

  Dwarf_Half tag;
  Dwarf_Addr lowPC, highPC;

  while(1){
    if (dwarf_tag(child_die, &tag, err) != DW_DLV_OK){
      perror("Error in dwarf_tag\n");
    }
    if (tag == DW_TAG_lexical_block){

      pc_range(dbg, child_die, &lowPC, &highPC);
      if(fun->pc > lowPC && fun->pc < highPC){
        // typeScope(pc, dbg, child_die, pointer_store_size, pointers);
      }

    } else if(tag == DW_TAG_formal_parameter){
      //
    } else if(tag == DW_TAG_variable){

      Dwarf_Die type_die;
      if(type_of(dbg, child_die, &type_die, err) != DW_DLV_OK){
        perror("Error in typing variable\n");
      }
      // TODO: Handle typedefs
      // TODO: Handle const indirection

      Dwarf_Half type_tag;

      if (dwarf_tag(type_die, &type_tag, err) != DW_DLV_OK){
        perror("Error in dwarf_tag\n");
      }

      if(type_tag == DW_TAG_pointer_type){

        void* pointer_location = NULL;

        printf("printing var start\n");
        if(var_location(dbg, fun, child_die, &pointer_location, err) !=
DW_DLV_OK){
          perror("Error deriving the location of a variable\n");
        }

        if(roots->filled == roots->capacity - 1){
          roots->contents = realloc(roots->contents, (roots->capacity) * 2 *
sizeof(RootPointer *));
          roots->capacity = (roots->capacity) * 2;
       }

        if(roots->filled >= roots->capacity){
          roots->contents = realloc(roots->contents, roots->capacity * 2);
          roots->capacity = roots->capacity * 2;
        }

        (roots->contents)[roots->filled].type_die = type_die;
        (roots->contents)[roots->filled].location = pointer_location;

        roots->filled++;
      }
    }


    int rc = dwarf_siblingof(dbg, child_die, &child_die, err);

    if(rc == DW_DLV_ERROR){
      perror("Error getting sibling of DIE\n");
    } else if(rc == DW_DLV_NO_ENTRY){
      return;
    }
  }
};


*/
