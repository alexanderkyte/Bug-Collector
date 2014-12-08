#include "read_types.h"

static int z;
int y = 400;

typedef struct {
  int x;
  void *next;
} Node;

int fact(int n) {
  if (n == 1) {
    return 1;
  } else {
    return n * fact(n - 1);
  }
}

int main(int argc, char **argv) {
  z = 100000;
  int x = 10;
  Node *v = calloc(1, sizeof(Node));
  v->x = 100;
  v->next = NULL;

  if (argc <= 0) {
    perror("Argv does not contain executable name. Something went very wrong");
    free(v);
    return -1;
  }

  double testDouble[30];
  char *testStr = "hello world";

  void *test = (void *)999;

  printf("test location: %p\n", &test);
  printf("testStr location: %p\n", &testStr);
  printf("node v location: %p\n", &v);

  GCContext *context;
  dwarf_read(argv[0], &context);
  printf("init done\n");
  CallStack *callStack;
  dwarf_backtrace(&callStack);
  Roots *roots;
  get_roots(callStack, context, &roots);
  printf("got roots\n");

  for (int i = 0; i < roots->roots->count; i++) {
    Root *root = roots->roots->contents[i];
    printf("index: %d\n", root->typeIndex);
    Type *type = context->types->contents[root->typeIndex];
    // I know it'll always be a pointer
    int typeIndex = type->info.pointerInfo->targetType.index;
    printf("typeindex: %d\n", typeIndex);
    if (typeIndex < context->types->count) {
      Type *target = context->types->contents[typeIndex];
      printf("location: %p, type: %d, type category: %s\n", root->location,
             type->key.index, target->dieName);
    }
  }

  printf("done\n");

  // printf("| name | contents | children | lowpc | highpc |\n");
  // for (int i = 0; i < context->functions->count; i++) {
  //   Function *fun = context->functions->contents[i];
  //   printf(
  //     "%s %p %p */ "%d %d\n",
  //     fun->dieName,
  //     fun->topScope->contents,
  //     fun->topScope->children,
  //     fun->topScope->lowPC,
  //     fun->topScope->highPC
  //   );
  // }

  freeContext(context);
  freeCallstack(callStack);
  freeRoots(roots);

  free(v);

  return y + z + x + fact(5) + 1;
}
