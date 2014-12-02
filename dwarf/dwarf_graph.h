#ifndef DWARF_GRAPH
#define DWARF_GRAPH

#ifdef DEBUG
#include <assert.h>
#endif

#include <stdbool.h>

typedef struct HeapArray {
  void **contents;
  int capacity;
  int count;
} *Array;

void arrayAppend(Array array, void *item);
Array newHeapArray(int capacity);

#define SENTINEL_INDEX -1

typedef union {
  Dwarf_Off offset;
  int index;
} TypeKey;

#define DEFAULT_SCOPE_CONTENTS_SIZE 5
#define DEFAULT_SCOPE_CHILDREN_SIZE 2

typedef struct Scope {
  Array contents; // Contains variables
  Array children;
  void *lowPC;
  void *highPC;
} Scope;

#define INITIAL_LIVE_FUNCTION_SIZE 20

typedef struct Function {
  Scope *topScope;
#ifdef DEBUG
  char *dieName;
#endif
} Function;

typedef struct {
  Dwarf_Locdesc **location;
  int expression_count;
  TypeKey type;
} RootInfo;

int dwarf_read_root(Dwarf_Debug dbg, Dwarf_Die *child_die, RootInfo **info,
                    Dwarf_Error *err);

typedef enum {
  POINTER_TYPE,
  STRUCTURE_TYPE,
  UNION_TYPE,
  ARRAY_TYPE,
  BASE_TYPE
} TypeCategory;

// handle typedefs in variable traversal
/* TYPEDEF_TYPE, */
// Both non-pointer types?
/* DW_TAG_base_type */
/* DW_TAG_const_type */

// Remove pointers like typedefs?

typedef struct {
  int layersOfIndirection;
  TypeKey targetType;
  bool voidStar;
} PointerInfo;

#define DEFAULT_STRUCT_MEMBER_LIST_SIZE 10

typedef struct StructMember {
  int offset;
  TypeKey type;
} StructMember;

typedef struct {
  // These two are parallel arrays
  Array members;
} StructInfo;

#define DEFAULT_UNION_TYPE_LIST_SIZE 5
typedef struct {
  Array alternatives; // The key of pointer types contained
} UnionInfo;

typedef struct {
  TypeKey elementTypes;
  int count;
} ArrayInfo;

typedef struct {
  TypeCategory category;
  TypeKey key;
#ifdef DEBUG
  char *dieName;
#endif
  union info {
    PointerInfo *pointerInfo;
    StructInfo *structInfo;
    UnionInfo *unionInfo;
    ArrayInfo *pointerArrayInfo;
  } info;
} Type;

int dwarf_read_function(Dwarf_Debug dbg, Dwarf_Die *fn_die, Function **fun,
                        Dwarf_Error *err);

int dwarf_read_scope(Dwarf_Debug dbg,
                     Dwarf_Die *top_die,
                     Scope *parent_scope,
                     Scope **top_scope,
                     Dwarf_Error *err);

int dwarf_read_struct(Dwarf_Debug dbg, Dwarf_Die *type_die,
                      StructInfo **structInfo, Dwarf_Error *err);
int dwarf_read_union(Dwarf_Debug dbg, Dwarf_Die *type_die,
                     UnionInfo **unionInfo, Dwarf_Error *err);
int dwarf_read_pointer(Dwarf_Debug dbg, Dwarf_Die *type_die,
                       PointerInfo **pointerInfo, Dwarf_Error *err);
int dwarf_read_array(Dwarf_Debug dbg, Dwarf_Die *type_die,
                     ArrayInfo **arrayInfo, Dwarf_Error *err);
int dwarf_read_member_offset(Dwarf_Debug dbg, Dwarf_Die *member_die,
                             int *offset, Dwarf_Error *err);

#define INITIAL_TYPE_LIST_SIZE 20
#define INITIAL_FUNCTION_LIST_SIZE 20

typedef struct {
  Array types;
  Array functions;
} GCContext;

int dwarf_type_die(Dwarf_Debug dbg, GCContext *context, Dwarf_Die child_die,
                   Dwarf_Error *err);

void finalizeContext(GCContext *context);

typedef struct {
  unw_cursor_t cursor;
  Function *function;
  void *pc;
  void *sp;
} LiveFunction;

typedef struct {
  int count;
  int capacity;
  LiveFunction *stack;
} CallStack;



typedef struct {
  void *location;
  int typeIndex;
} Root;

typedef struct { Array roots; } Roots;

void freeCleanup(GCContext *context);
void freeRoots(Roots* roots);
void freeCallstack(CallStack *callStack);
void freeContext(GCContext *context);
void freeArray(Array array);

#endif
