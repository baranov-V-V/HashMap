#include "StackTrace.h"

struct Vector {
  Words* arr = NULL;
  size_t sz = 0;
  size_t capacity = 0;
  size_t mincapacity = 2;
  size_t increase_capacity = 2;
};

Vector* NewVector() {
  $;
  Vector vector;
  vector.arr = (Words*)calloc(vector.mincapacity, sizeof(Words));
  assert(vector.arr != NULL);
  vector.capacity = vector.mincapacity;

  Vector* new_vec = (Vector*)calloc(1, sizeof(Vector));
  assert(new_vec != NULL);
  memcpy(new_vec, &vector, sizeof(vector));

  RETURN new_vec;
}

void VectorRealloc(Vector* vec) {
  $;
  assert(vec != NULL);
  vec->capacity *= vec->increase_capacity;
  vec->arr = (Words*)realloc(vec->arr, vec->capacity * sizeof(Words));
  assert(vec->arr != NULL);
  $$;
}

void VectorPushback(Vector* vec, Words val) {
  $;
  assert(vec != NULL);
  if (vec->sz >= vec->capacity) {
    VectorRealloc(vec);
  }
  vec->arr[vec->sz++] = val;
  $$;
}

void VectorDestroy(Vector* vec) {
  $;
  assert(vec != NULL);
  free(vec->arr);
  free(vec);
  $$;
}
