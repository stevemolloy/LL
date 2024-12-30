#ifndef _SDM_LIB_H
#define _SDM_LIB_H

/* This library provides the following:
 *
 * char *sdm_shift_args(int *argc, char ***argv);      Peel arguments off the **argv array typically provided to main, decrementing argc appropriately.
 * char *sdm_read_entire_file(const char *file_path);  Read the contents of a file into a character array. This character array is malloc'ed and so should be freed by the user.
 * SDM_FREE_AND_NULL(ptr)                              Free the memory pointed to by ptr, and then set ptr to NULL.
 * #define SDM_FREE SDM_FREE_AND_NULL
 * #define SDM_MALLOC malloc
 * 
 * # DYNAMIC ARRAYS
 * ================
 * SDM_ENSURE_ARRAY_CAP(da, cap)       Ensure that the dynamic array, da, has a capacity equal-to cap. Realloc is used if needed.
 * SDM_ENSURE_ARRAY_MIN_CAP(da, cap)   Ensure that the dynamic array, da, has a capacity equal-to or greater-than cap. Realloc is used if needed.
 * DEFAULT_CAPACITY 128                Default capacity in bytes to be used when the capacity has not been set by the user.
 * SDM_ARRAY_PUSH(da, item)            Push the value of item to the dynamic array, da, reallocing if needed.
 * SDM_ARRAY_SWAP(da, ind1, ind2)      Swap the elements at the marked indices (if length==capacity this will extend the array)
 * SDM_ARRAY_FREE(da)                  Free the memory in the dynamic array, da, and zero things
 * SDM_ARRAY_RESET(da)                 Reset the length of the dynamic array, da, to zero, effectively emptying it. No memory is freed by this.
 * 
 * # STRING VIEWS
 * ==============
 * sdm_string_view sdm_cstr_as_sv(char *cstr);                                 Return a string view containing the provided c-string
 * sdm_string_view sdm_sized_str_as_sv(char *cstr, size_t length);             Return a string view containing the c-string with the provided size.
 * sdm_string_view sdm_sv_pop_by_delim(sdm_string_view *SV, const char delim); Pop off the start of the provided string view up to the given delimiter.
 * void sdm_sv_trim(sdm_string_view *SV);                                      Trim any whitespace from the start of the string view
 * SDM_SV_F "%.*s"                                                             A printf helper.
 * SDM_SV_Vals(S) (int)(S).length, (S).data                                    A printf helper.
 * 
 * # MEMORY ARENA
 * ==============
 * #define SDM_ARENA_DEFAULT_CAP 256 * 1024*1024              Default capacity of the memory arena when not supplied by the user
 * void sdm_arena_init(sdm_arena_t *arena, size_t capacity);  Initialise a memory arena with a certain capacity and malloc the required space.
 * void *sdm_arena_alloc(sdm_arena_t *arena, size_t size);    Allocate a region of size bytes in the given arena, and return a pointer to the start of this region.
 * void sdm_arena_free(sdm_arena_t *arena);                   Deallocate all memory in the arena, and zero everything
 */

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define SDM_FREE_AND_NULL(ptr) \
do {                           \
  if ((ptr)) free((ptr));      \
  (ptr) = NULL;                \
} while (0)

#define SDM_FREE SDM_FREE_AND_NULL
#ifndef SDM_MALLOC
void *active_alloc(size_t size);
void *active_realloc(void *ptr, size_t size);
#define SDM_MALLOC active_alloc
#define SDM_REALLOC active_realloc
#endif

#define hash jenkins_one_at_a_time_hash

#define SDM_ENSURE_ARRAY_CAP(da, cap) do {                     \
    (da).capacity = cap;                                       \
    (da).data = SDM_REALLOC((da).data,                             \
        (da).capacity * sizeof((da).data[0]));                 \
    if ((da).data == NULL) {                                   \
      fprintf(stderr, "ERR: Couldn't alloc memory.\n");        \
      exit(1);                                                 \
    }                                                          \
  } while (0)

#define SDM_ENSURE_ARRAY_MIN_CAP(da, cap) do {                 \
    if ((da).capacity < cap) {                                 \
      (da).capacity = cap;                                     \
      (da).data = SDM_REALLOC((da).data,                           \
          (da).capacity * sizeof((da).data[0]));               \
      if ((da).data == NULL) {                                 \
        fprintf(stderr, "ERR: Couldn't alloc memory. \n");     \
        exit(1);                                               \
      }                                                        \
    }                                                          \
  } while (0)

#define SDM_ARRAY_POP(da) (da).data[--((da).length)]

#define DEFAULT_CAPACITY 128

#define SDM_ARRAY_PUSH(da, item) do {                             \
    if (((da).capacity == 0) || ((da).data == NULL)) {            \
      (da).capacity = DEFAULT_CAPACITY;                           \
      (da).data = SDM_MALLOC((da).capacity * sizeof((da).data[0]));   \
      memset((da).data, 0, (da).capacity * sizeof((da).data[0])); \
      if ((da).data == NULL) {                                    \
        fprintf(stderr, "ERR: Couldn't alloc memory.\n");         \
        exit(1);                                                  \
      }                                                           \
    }                                                             \
    while ((da).length >= (da).capacity) {                        \
      (da).capacity *= 2;                                         \
      (da).data = SDM_REALLOC((da).data,                              \
           (da).capacity * sizeof((da).data[0]));                 \
      if ((da).data == NULL) {                                    \
        fprintf(stderr, "ERR: Couldn't alloc memory.\n");         \
        exit(1);                                                  \
      }                                                           \
    }                                                             \
    (da).data[(da).length++] = item;                              \
  } while (0);

#define SDM_ARRAY_SWAP(da, ind1, ind2)                                   \
do {                                                                     \
  assert((int)ind1 < (int)da.length && "First index is out of bounds");  \
  assert((int)ind2 < (int)da.length && "Second index is out of bounds"); \
  SDM_ARRAY_PUSH(da, da.data[ind1]);                                     \
  da.data[ind1] = da.data[ind2];                                         \
  da.data[ind2] = da.data[da.length - 1];                                \
  da.length--;                                                           \
} while (0)

#define SDM_ARRAY_FREE(da) do {                                \
    SDM_FREE((da).data);                                           \
    (da).length = 0;                                           \
    (da).capacity = 0;                                         \
  } while (0);

#define SDM_ARRAY_RESET(da) do { (da).length = 0; } while (0)

typedef struct {
  size_t length;
  char *data;
} sdm_string_view;

#define SDM_SV_F "%.*s"
#define SDM_SV_Vals(S) (int)(S).length, (S).data

char *sdm_shift_args(int *argc, char ***argv);

char *sdm_read_entire_file(const char *file_path);

sdm_string_view sdm_cstr_as_sv(char *cstr);
char *sdm_sv_to_cstr(sdm_string_view sv);
sdm_string_view sdm_sized_str_as_sv(char *cstr, size_t length);
sdm_string_view sdm_pop_by_length(sdm_string_view *SV, size_t len);
sdm_string_view sdm_sv_pop_by_delim(sdm_string_view *SV, const char delim);
sdm_string_view sdm_sv_pop_by_whitespace(sdm_string_view *SV);
int sdm_sv_pop_integer(sdm_string_view *SV);
char sdm_sv_pop_one_char(sdm_string_view *SV);
void sdm_sv_trim(sdm_string_view *SV);
int sdm_svncmp(sdm_string_view SV, const char *cmp);
bool sdm_sv_compare(sdm_string_view SV1, sdm_string_view SV2);

typedef struct {
  char key[32];
  double value;
  bool occupied;
} Dbl_KVPair;

typedef struct {
  Dbl_KVPair *data;
  size_t length;
  size_t capacity;
} DblArray;

#define DEFAULT_HM_CAP 256

#define SET_HM_CAPACITY(hm, cap)                                              \
  do {                                                                        \
    (hm)->capacity = (cap);                                                   \
    if ((hm)->data == NULL) {                                                 \
      (hm)->data = SDM_MALLOC(hm->capacity * sizeof(hm->data[0]));                \
    }                                                                         \
    else {                                                                    \
      (hm)->data = SDM_REALLOC((hm)->data, (hm)->capacity * sizeof(hm->data[0])); \
    }                                                                         \
    if ((hm)->data == NULL) {                                                 \
      fprintf(stderr, "ERR: Can't alloc.\n");                                 \
      exit(1);                                                                \
    }                                                                         \
    memset((hm)->data, 0, (hm)->capacity * sizeof(hm->data[0]));              \
  } while (0)

#define FREE_HASHMAP(hm) \
  do {                   \
    (hm).capacity = 0;   \
    SDM_FREE((hm).data);     \
  } while (0);

#define GET_HASHMAP_INDEX(hm, value_of_key, index_addr)                      \
  do {                                                                       \
    *(index_addr) = -1;                                                      \
    uint32_t location = get_hashmap_location((value_of_key), (hm).capacity); \
    for (size_t offset=0; offset<(hm).capacity; offset++) {                  \
      int new_location = (location + offset) % (hm).capacity;                \
      if ((hm).data[new_location].occupied &&                                \
        (strcmp((hm).data[new_location].key, (value_of_key))==0)) {          \
        *(index_addr) = new_location;                                        \
        break;                                                               \
      }                                                                      \
    }                                                                        \
  } while (0)

#define HM_VAL_AT(hm, index) \
  (hm).data[index].value

#define PUSH_TO_HASHMAP(hm1, value_of_key, value_of_value)                                   \
  do {                                                                                       \
    uint32_t location;                                                                       \
    if ((hm1)->capacity == 0) {                                                              \
      SET_HM_CAPACITY((hm1), DEFAULT_HM_CAP);                                                \
    }                                                                                        \
    if ((hm1)->length >= (hm1)->capacity) {                                                  \
      fprintf(stderr, "Hashmap has insufficient capacity.\n");                               \
      exit(1);                                                                               \
    }                                                                                        \
    location = get_hashmap_location((value_of_key), (hm1)->capacity);                        \
                                                                                             \
    if (!(hm1)->data[location].occupied ||                                                   \
      ((hm1)->data[location].occupied && strcmp((value_of_key), (hm1)->data[location].key)==0)) {  \
      strcpy((hm1)->data[location].key, (value_of_key));                                     \
      (hm1)->data[location].value = (value_of_value);                                        \
      (hm1)->data[location].occupied = true;                                                 \
      (hm1)->length++;                                                                       \
    } else {                                                                                 \
      for (size_t i=1; i<(hm1)->capacity; i++) {                                             \
        size_t new_location = (location + i) % (hm1)->capacity;                              \
        if (!(hm1)->data[new_location].occupied ||                                           \
          ((hm1)->data[new_location].occupied && strcmp((value_of_key), (hm1)->data[new_location].key)==0)) { \
          strcpy((hm1)->data[new_location].key, (value_of_key));                             \
          (hm1)->data[new_location].value = (value_of_value);                                \
          (hm1)->data[new_location].occupied = true;                                         \
          (hm1)->length++;                                                                   \
          break;                                                                             \
        }                                                                                    \
      }                                                                                      \
    }                                                                                        \
  } while (0)

void push_to_dblarray(DblArray *hm, char *key, double value);
uint32_t get_hashmap_location(const char* key, size_t capacity);
uint32_t jenkins_one_at_a_time_hash(const uint8_t* key, size_t length);

#define SDM_ARENA_DEFAULT_CAP 256 * 1024*1024

typedef struct sdm_arena_t sdm_arena_t;

struct sdm_arena_t {
  size_t length;
  size_t capacity;
  void *start;
  sdm_arena_t *next;
};

void sdm_arena_init(sdm_arena_t *arena, size_t capacity);
void *sdm_arena_alloc(sdm_arena_t *arena, size_t size);
void *sdm_arena_realloc(sdm_arena_t *arena, void *ptr, size_t size);
void sdm_arena_free(sdm_arena_t *arena);

#endif /* ifndef _SDM_LIB_H */

