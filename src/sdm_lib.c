#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "sdm_lib.h"

char *sdm_read_entire_file(const char *file_path) {
  // Reads an entire file into a char array, and returns a ptr to this. The ptr should be freed by the caller
  FILE *f = fopen(file_path, "r");
  if (f==NULL) {
    fprintf(stderr, "Could not read %s: %s\n", file_path, strerror(errno));
    exit(1);
  }

  fseek(f, 0L, SEEK_END);
  int sz = ftell(f);
  fseek(f, 0L, SEEK_SET);

  char *contents = SDM_MALLOC((sz + 1) * sizeof(char));
  if (contents==NULL) {
    fprintf(stderr, "Could not allocate memory. Buy more RAM I guess?\n");
    exit(1);
  }
  fread(contents, 1, sz, f);

  fclose(f);
  
  return contents;
}

sdm_string_view sdm_cstr_as_sv(char *cstr) {
  return (sdm_string_view){
    .data = cstr,
    .length = strlen(cstr)
  };
}

char *sdm_sv_to_cstr(sdm_string_view sv) {
  char *ret = SDM_MALLOC(sv.length + 1);
  memset(ret, 0, sv.length + 1);
  memcpy(ret, sv.data, sv.length);
  return ret;
}

sdm_string_view sdm_sized_str_as_sv(char *cstr, size_t length) {
  return (sdm_string_view) {
    .data = cstr,
    .length = length
  };
}

sdm_string_view sdm_pop_by_length(sdm_string_view *SV, size_t len) {
  sdm_string_view retval = (sdm_string_view) {
    .data = SV->data,
    .length = len,
  };
  SV->data += len;
  SV->length -= len;
  return retval;
}

sdm_string_view sdm_sv_pop_by_whitespace(sdm_string_view *SV) {
  sdm_string_view ret = {0};
  ret.data = SV->data;

  while (!isspace(*SV->data) && (SV->length>0)) {
    SV->data++;
    SV->length--;
    ret.length++;
  }
  SV->data++;
  SV->length--;

  return ret;
}

sdm_string_view sdm_sv_pop_by_delim(sdm_string_view *SV, const char delim) {
  sdm_string_view ret = {0};
  ret.data = SV->data;

  while ((*SV->data != delim) && (SV->length>0)) {
    SV->data++;
    SV->length--;
    ret.length++;
  }
  SV->data++;
  SV->length--;

  return ret;
}

int sdm_sv_pop_integer(sdm_string_view *SV) {
  char *new_pos;
  int retval = strtol(SV->data, &new_pos, 0);
  size_t diff = new_pos - SV->data;
  SV->data = new_pos;
  SV->length -= diff;
  return retval;
}

char sdm_sv_pop_one_char(sdm_string_view *SV) {
  char retval = SV->data[0];
  SV->data++;
  SV->length--;
  return retval;
}

void sdm_sv_trim(sdm_string_view *SV) {
  while (isspace(*SV->data) && SV->length>0) {
    SV->data++;
    SV->length--;
  }
}

int sdm_svncmp(sdm_string_view SV, const char *cmp) {
  size_t n = strlen(cmp) < SV.length ? strlen(cmp) : SV.length;
  return strncmp(SV.data, cmp, n);
}

char *sdm_shift_args(int *argc, char ***argv) {
  if (*argc <= 0) return NULL;
  (*argc)--;
  char **ret = *argv;
  (*argv)++;
  return *ret;
}

uint32_t get_hashmap_location(const char* key, size_t capacity) {
  uint32_t key_hash = hash((uint8_t*)key, strlen(key));
  return key_hash % capacity;
}

void resize_dblarray(DblArray *hm) {
  DblArray resized_array = {0};
  SET_HM_CAPACITY((&resized_array), hm->capacity * 2);
  for (size_t i=0; i<hm->capacity; i++) {
    if (!hm->data[i].occupied) continue;
    push_to_dblarray(&resized_array, hm->data[i].key, hm->data[i].value);
  }
  hm->capacity = resized_array.capacity;
  SDM_FREE(hm->data);
  hm->data = resized_array.data;
}

void push_to_dblarray(DblArray *hm, char *key, double value) {
  if (hm->length >= hm->capacity) resize_dblarray(hm);
  PUSH_TO_HASHMAP(hm, key, value);
}

// https://en.wikipedia.org/wiki/Jenkins_hash_function
uint32_t jenkins_one_at_a_time_hash(const uint8_t* key, size_t length) {
  size_t i = 0;
  uint32_t hash = 0;
  while (i != length) {
    hash += key[i++];
    hash += hash << 10;
    hash ^= hash >> 6;
  }
  hash += hash << 3;
  hash ^= hash >> 11;
  hash += hash << 15;
  return hash;
}

void sdm_arena_init(sdm_arena_t *arena, size_t capacity) {
  arena->start = malloc(capacity);
  if (arena->start == NULL) {
    fprintf(stderr, "Memory problem. Aborting.\n");
    exit(1);
  }
  memset(arena->start, 0, capacity);
  arena->next = malloc(sizeof(*arena->next));
  if (arena->next == NULL) {
    fprintf(stderr, "Memory problem. Aborting.\n");
    exit(1);
  }
  memset(arena->next, 0, sizeof(*arena->next));
  arena->capacity = capacity;
  arena->length = 0;
}

void *sdm_arena_alloc(sdm_arena_t *arena, size_t size) {
  if (arena->start == NULL) {
    size_t capacity = (arena->capacity > 0) ? arena->capacity : SDM_ARENA_DEFAULT_CAP;
    while (capacity < size) {
      capacity *= 2;
    }
    sdm_arena_init(arena, capacity);
  }

  if (arena->capacity - arena->length < size) {
    if (arena->next->capacity == 0) arena->next->capacity = arena->capacity;
    return sdm_arena_alloc(arena->next, size);
  }

  void *return_val = (char*)arena->start + arena->length;

  if (size > 0)
    arena->length += size;
  else
    arena->length += 1;

  return return_val;
}

void *sdm_arena_realloc(sdm_arena_t *arena, void *ptr, size_t size) {
  void *retval = sdm_arena_alloc(arena, size);
  if (ptr) memcpy(retval, ptr, size);
  return retval;
}

void sdm_arena_free(sdm_arena_t *arena) {
  if (arena->next) {
    sdm_arena_free(arena->next);
  }

  SDM_FREE_AND_NULL(arena->start);

  arena->length = 0;
  arena->capacity = 0;

  SDM_FREE_AND_NULL(arena->next);
}

bool sdm_sv_compare(sdm_string_view SV1, sdm_string_view SV2) {
  if (SV1.length != SV2.length) return false;
  for (size_t i=0; i<SV1.length; i++) {
    if (SV1.data[i] != SV2.data[i]) return false;
  }
  return true;
}


