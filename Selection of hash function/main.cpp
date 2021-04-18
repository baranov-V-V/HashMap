#include <algorithm>
#include <time.h>
#include <sys/stat.h>
#include "../StackTrace.h"
struct Words {
  unsigned char* key = NULL;
  unsigned char* val = NULL;
};
#include "../Vector.h"

const size_t kDictWordsNum = 19993;
const char* kDictFileName = "../dict";
size_t kHashMapSize = 10007;

const size_t kPrimeMod = ((size_t)1 << 62) - 1;
const size_t kRotationShift = 1;

struct Dict {
  FILE* file = NULL;
  unsigned char* file_buf = NULL;
  const char* name = kDictFileName;
  size_t words_num = kDictWordsNum;
  size_t sz = 0;
};

struct Map {
  Vector** arr = NULL;
  size_t sz = 0;
};

void DictConstruct(Dict* dict) {
  $;
  assert(dict != NULL);
  dict->file = fopen(dict->name, "r");
  assert(dict->file != NULL && "Can not open dictionary");

  struct stat file_info;
  stat(dict->name, &file_info);
  dict->sz = file_info.st_size;

  dict->file_buf = (unsigned char*)calloc(dict->sz, sizeof(unsigned char));
  fread(dict->file_buf, sizeof(unsigned char), dict->sz, dict->file);
  fclose(dict->file);
  $$;
}

void DictDestroy(Dict* dict) {
  $;
  assert(dict != NULL);
  free(dict->file_buf);
  $$;
}

void MapConstruct(Map* map, size_t sz, Dict* dict, size_t (*hash_func)(unsigned char*)) {
  $;
  assert(map != NULL);
  assert(sz > 0);
  assert(dict != NULL);
  assert(hash_func != NULL);

  map->sz = sz;
  map->arr = (Vector**)calloc(map->sz, sizeof(Vector*));

  unsigned char* eng_word = NULL;
  unsigned char* rus_word = NULL;
  Vector** cur_vec_ptr = NULL;
  unsigned char* ptr = dict->file_buf;
  for (size_t i = 0; i < dict->words_num; ++i) {
    eng_word = ptr;
    ptr = (unsigned char*)strchr((char*)ptr, '-');
    *ptr++ = '\0';

    rus_word = ptr;
    ptr = (unsigned char*)strchr((char*)ptr, '\n');
    *ptr++ = '\0';
    cur_vec_ptr = map->arr + (hash_func(eng_word) % map->sz);
    if (*cur_vec_ptr == NULL) {
      *cur_vec_ptr = NewVector();
    }
    VectorPushback(*cur_vec_ptr, {eng_word, rus_word});
  }
  $$;
}

void MapDestroy(Map* map) {
  $;
  for (size_t i = 0; i < map->sz; ++i) {
    if (map->arr[i] != NULL) {
      VectorDestroy(map->arr[i]);
    }
  }
  free(map->arr);
  $$;
}

namespace HashFuncs {  // --------------------------------------------------
  size_t Polynomial(unsigned char* key) {
    $;
    assert(key != NULL);
    size_t hash_val = 0;
    for (size_t i = 0; key[i] != '\0'; ++i) {
      hash_val = ((hash_val << 5) - hash_val + key[i]) % kHashMapSize;
    }
    RETURN hash_val;
  }

  inline size_t RolNumber(size_t hash_val) {
    return (hash_val << kRotationShift) | (hash_val >> (64 - kRotationShift));
  }
  size_t Rol(unsigned char* key) {
    $;
    size_t hash_val = 0;
    for (size_t i = 0; key[i] != '\0'; ++i) {
      hash_val = RolNumber(hash_val) ^ key[i];
    }
    RETURN hash_val;
  }

  inline size_t RorNumber(size_t hash_val) {
    return (hash_val >> kRotationShift) | (hash_val << (64 - kRotationShift));
  }
  size_t Ror(unsigned char* key) {
    $;
    size_t hash_val = 0;
    for (size_t i = 0; key[i] != '\0'; ++i) {
      hash_val = RorNumber(hash_val) ^ key[i];
    }
    RETURN hash_val;
  }

  size_t LettersSum(unsigned char* key) {
    $;
    assert(key != NULL);
    size_t res = 0;
    for (size_t i = 0; key[i] != '\0'; ++i) {
      res += (size_t)key[i];
    }
    RETURN res;
  }

  size_t FirstLetter(unsigned char* key) {
    assert(key != NULL);
    return (size_t)key[0];
  }

  size_t Length(unsigned char* key) {
    assert(key != NULL);
    return strlen((char*)key);
  }
}  // namespace HashFuncs---------------------------------------------------

void SortMap(Map* map) {
  std::sort(map->arr, map->arr + map->sz, [](Vector* vec1, Vector* vec2) {
    if (vec1 == NULL) {
      return false;
    } else if (vec2 == NULL) {
      return true;
    } else {
      return vec1->sz > vec2->sz;
    }
  });
}

void PrintResult(Map* map, size_t x_scale, size_t y_scale, const char* label) {
  $;
  assert(map != NULL);
  
  static char py_file_name[30] = {};
  sprintf(py_file_name, "Py files/%s.py", label);
  FILE* file = fopen(py_file_name, "w");

  fprintf(file, "import matplotlib.pylab as plt\n");
  fprintf(file, "list = [\n");
  size_t collisions = 0;
  SortMap(map);

  for (size_t i = 0; i < map->sz; ++i) {
    if (map->arr[i] != NULL) {
      collisions = map->arr[i]->sz;
      for (size_t j = 0; j < collisions; ++j) {
        fprintf(file, "%lu, ", i);
      }
      fprintf(file, "\n");
    } else {
      break;
    }
  }
  fprintf(file, "]\nbins = [i / 2 - 0.25 for i in range(%lu)]\n", x_scale * 2);
  fprintf(file, "plt.xlabel('bucket idx')\nplt.ylabel('number of collisions')\n");
  fprintf(file, "plt.xlim(xmin=0, xmax = %lu)\n", x_scale);
  fprintf(file, "plt.ylim(ymin=0, ymax = %lu)\n", y_scale);
  fprintf(file, "plt.title('%s')\n", label);
  fprintf(file, "plt.hist(list, bins=bins)\nplt.show()");
  fclose(file);
  $$;
}

int main() {
  $;
  signal(SIGSEGV, PrintStackInfoAndExit);
  signal(SIGABRT, PrintStackInfoAndExit);

  size_t funcs_num = 6;
  size_t (*hash_funcs[funcs_num])(unsigned char*) = { HashFuncs::Polynomial,
                                                      HashFuncs::Rol,
                                                      HashFuncs::Ror,
                                                      HashFuncs::LettersSum,
                                                      HashFuncs::FirstLetter,
                                                      HashFuncs::Length };
  size_t x_scales[funcs_num] = { 8700, 8700, 8700, 8700, 25, 25 };
  size_t y_scales[funcs_num] = { 90, 90, 90, 90, 2943, 2943 };
  const char* labels[funcs_num] = { "Polynomial", "Rol", "Ror", "LettersSum",
                                    "FirstLetter", "Length" };
  //                  Polynomial -> Rol -> Ror -> LettersSum -> FirstLetter -> Length
  // max_buckets_num: 8700         6800   1725    1200          25             20  
  // max_collisions:  9            25     60      90            2108           2943

  for (size_t i = 0; i < funcs_num; ++i) {
    Dict dict = {};
    Map map = {};
    DictConstruct(&dict);
    MapConstruct(&map, kHashMapSize, &dict, hash_funcs[i]);
    PrintResult(&map, x_scales[i], y_scales[i], labels[i]);
    DictDestroy(&dict);
    MapDestroy(&map);
  }
  $$;
}
