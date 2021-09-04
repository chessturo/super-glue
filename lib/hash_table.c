/* Provides a hash table data structure.
   Copyright 2021 Mitchell Levy

This file is a part of super-glue

super-glue is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

super-glue is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with super-glue.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "hash_table.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "linked_list.h"

// Change to `1` if there are errors with hash collisions. This will have a
// performance penalty because instead of just checking the hashes, if two
// hashes match the full key will be checked as well.
#ifndef COLLISION_RESIST
#define COLLISION_RESIST 1
#endif

typedef uint64_t Hash64;

// Typedef'd to HashTable in hash_table.h
struct _HT {
  LinkedList **buckets;
  int num_buckets;
  int num_elems;
};
// Typedef'd to HTIterator in hash_table.h
struct _HTIt {
  LLIterator *bucket_iter;
  HashTable *table;
  int bucket_idx;
};

typedef struct {
  Hash64 hash;
  unsigned char *key;
  size_t key_len;
  HTValue value;
} HTEntry;

// Computes the hash of data using the Fowler-Noll-Vo 1a Hash with a hash length
// of 64 bits.
//
// data     - the data to hash.
// data_len - the length of data in unsigned chars.
//
// Returns a Hash64 of the given data.
static Hash64 FNV1a_64bit(const unsigned char *data, size_t data_len);
// Frees a HTEntry structure, used in HashTable_free to free each entry in the
// HashTable. Sensitive to the value of `value_free`.
static void HTEntry_free(void *e);
// Advances the given LLIterator until it reaches an element that matches the
// given hash (or optionally, if COLLISION_RESIST is enabled, the given key as
// well).
//
// Returns true if a matching element was found, false otherwise. False on NULL.
#if COLLISION_RESIST
static bool advance_to_target(LLIterator *iter, Hash64 hash, unsigned char *key,
    size_t key_len);
#else
static bool advance_to_target(LLIterator *iter, Hash64 hash);
#endif
// Gets true key length (helper if user passes key_len = 0 => strlen(key))
static inline size_t get_true_key_len(unsigned char *key, size_t key_len) {
  size_t true_key_len;
  if (key_len == 0) {
    true_key_len = 0;
    for (unsigned char *c = key; *c != 0; c++) true_key_len++;
  } else {
    true_key_len = key_len;
  }
  return true_key_len;
}

#define DEFAULT_BUCKETS 8
HashTable *HashTable_allocate() {
  HashTable *ht = malloc(sizeof(HashTable));
  if (ht == NULL) return NULL;

  ht->num_elems = 0;
  ht->num_buckets = DEFAULT_BUCKETS;
  ht->buckets = malloc(sizeof(LinkedList *) * DEFAULT_BUCKETS);
  if (ht->buckets == NULL) {
    free(ht);
    return NULL;
  }

  for (int i = 0; i < ht->num_buckets; i++) {
    ht->buckets[i] = LinkedList_allocate();
  }

  return ht;
}

static _Thread_local HTValue_free value_free;
void HashTable_free(HashTable *ht, HTValue_free ll_value_free) {
  if (ht == NULL) return;
  
  value_free = ll_value_free;
  for (int i = 0; i < ht->num_buckets; i++) {
    LinkedList_free(ht->buckets[i], HTEntry_free);
  }

  free(ht->buckets);
  free(ht);
}

static void HTEntry_free(void *e) {
  HTEntry *entry = e;
  if (value_free != NULL) {
    value_free(entry->value);
  }
  free(entry->key);
  free(entry);
}

int HashTable_num_elements(HashTable *ht) {
  if (ht == NULL) return -1;
  return ht->num_elems;
}

#define FNV_PRIME 0x00000100000001B3
#define FNV_INIT 0xcbf29ce484222325
// A public domain reference implementation of the FNV hash function can be
// found on Landon Curt Noll's webpage:
// <http://www.isthe.com/chongo/src/fnv/hash_64.c>
static Hash64 FNV1a_64bit(const unsigned char *data, size_t data_len) {
  Hash64 hash = FNV_INIT;
  const unsigned char *current = data;
  while (current < data + data_len) {
    hash ^= *current;
    hash *= FNV_PRIME;
    current++;
  }

  return hash;
}

#if COLLISION_RESIST
static bool advance_to_target(LLIterator *iter, Hash64 hash, unsigned char *key,
    size_t key_len) {
#else
static bool advance_to_target(LLIterator *iter, Hash64 hash) {
#endif

  if (iter == NULL) return false;

#if COLLISION_RESIST
  if (key == NULL) return false;
#endif
  
  while (LLIterator_is_valid(iter)) {
    HTEntry *current = *LLIterator_get(iter);
    if (current->hash == hash) { 
#if COLLISION_RESIST
      if (current->key_len == key_len &&
          memcmp(current->key, key, key_len) == 0)
        return true;
#else
      return true;
#endif
    }
    LLIterator_next(iter);
  }

  return false;
}

#define bucket_idx_by_hash(ht, hash) ((hash) % (ht)->num_buckets)

// FIXME way to deal with malloc failure
bool HashTable_insert(HashTable *ht, unsigned char *key, size_t key_len,
    HTValue new_value, HTValue *old_value) {
  if (ht == NULL || key == NULL) return false;

  // If the user is using the "length zero -> C string" short hand, find the
  // actual length of the string.
  size_t true_key_len = get_true_key_len(key, key_len);

  // Set up HTEntry members
  Hash64 hash = FNV1a_64bit(key, true_key_len);
  unsigned char *key_cpy = malloc(true_key_len);
  memcpy(key_cpy, key, true_key_len);

  HTEntry *new_entry = malloc(sizeof(HTEntry));
  new_entry->hash = hash;
  new_entry->key = key_cpy;
  new_entry->key_len = true_key_len;
  new_entry->value = new_value;
  
  bool found = false;
  LinkedList *bucket = ht->buckets[bucket_idx_by_hash(ht, hash)];
  LLIterator *bucket_iter = LLIterator_allocate(bucket);
#if COLLISION_RESIST
  if (advance_to_target(bucket_iter, hash, key, true_key_len)) {
#else
  if (advance_to_target(bucket_iter, hash)) {
#endif
    found = true;
    HTEntry *old_entry;
    LLIterator_remove(bucket_iter, (LLPayload *)&old_entry);
    if (old_value != NULL) *old_value = old_entry->value;
    free(old_entry->key);
    free(old_entry);
  }

  LinkedList_prepend(bucket, new_entry);
  if (!found) ht->num_elems++;

  return found;
}

// FIXME way to deal with LLIterator_allocate failure
HTValue *HashTable_find(HashTable *ht, unsigned char *key, size_t key_len) {
  if (ht == NULL || key == NULL) return NULL;

  size_t true_key_len = get_true_key_len(key, key_len);
  Hash64 hash = FNV1a_64bit(key, true_key_len);
  
  LLIterator *bucket_iter =
    LLIterator_allocate(ht->buckets[bucket_idx_by_hash(ht, hash)]);
#if COLLISION_RESIST
  bool found = advance_to_target(bucket_iter, hash, key, true_key_len);
#else
  bool found = advance_to_target(bucket_iter, hash);
#endif 
  if (!found) return NULL;
  HTEntry *entry = *LLIterator_get(bucket_iter);
  return &entry->value;
}

// FIXME way to deal with LLIterator_allocate failure
bool HashTable_remove(HashTable *ht, unsigned char *key, size_t key_len,
    HTValue *old_value) {
  if (ht == NULL || key == NULL) return false;

  size_t true_key_len = get_true_key_len(key, key_len);
  Hash64 hash = FNV1a_64bit(key, true_key_len);
  
  LLIterator *bucket_iter =
    LLIterator_allocate(ht->buckets[bucket_idx_by_hash(ht, hash)]);
#if COLLISION_RESIST
  bool found = advance_to_target(bucket_iter, hash, key, true_key_len);
#else
  bool found = advance_to_target(bucket_iter, hash);
#endif 
  if (!found) return false;

  HTEntry *old_entry;
  LLIterator_remove(bucket_iter, (LLPayload *)&old_entry);
  *old_value = old_entry->value;
  free(old_entry);

  ht->num_elems--;
  return true;
}

HTIterator *HTIterator_allocate(HashTable *ht) {
  if (ht == NULL) return NULL;
  HTIterator *iter = malloc(sizeof(HTIterator));
  if (ht->num_elems == 0) {
    // If the hash table is empty, just return an invalid iterator.
    iter->bucket_iter = NULL;
    return iter;
  }
  iter->table = ht;

  // Start at -1 so the first run of the do-while sets it to zero. For tables
  // with few elements, not all buckets will have elements, so we need to skip
  // over the empty buckets.
  iter->bucket_idx = -1;
  do {
    iter->bucket_idx++;
    iter->bucket_iter = LLIterator_allocate(ht->buckets[iter->bucket_idx]);
  } while (!LLIterator_is_valid(iter->bucket_iter));
  return iter;
}

void HTIterator_free(HTIterator *hti) {
  if (hti == NULL) return;
  LLIterator_free(hti->bucket_iter);
  free(hti);
}

bool HTIterator_is_valid(HTIterator *hti) {
  if (hti == NULL) return false;
  if (hti->table->num_elems == 0) return false;
  return hti->bucket_iter != NULL;
}

bool HTIterator_next(HTIterator *hti) {
  if (hti == NULL) return false;
  if (!HTIterator_is_valid(hti)) return false;
  if (!LLIterator_next(hti->bucket_iter)) {
    LLIterator_free(hti->bucket_iter);
    do {
      // If there aren't any more buckets to iterate, set bucket_iter to NULL to
      // signal the iterator is invalid and return.
      if (hti->bucket_idx == hti->table->num_buckets - 1) {
        hti->bucket_iter = NULL;
        return false;
      }
      hti->bucket_idx++; 
      hti->bucket_iter =
        LLIterator_allocate(hti->table->buckets[hti->bucket_idx]);
    } while(!LLIterator_is_valid(hti->bucket_iter));
  }
  return true;
}

bool HTIterator_get(HTIterator *hti, const unsigned char **key_out,
    size_t *key_len_out, HTValue *value_out) {
  if (hti == NULL || !HTIterator_is_valid(hti)) return false;
  HTEntry *entry = *LLIterator_get(hti->bucket_iter);
  if (key_out != NULL) *key_out = entry->key;
  if (key_len_out != NULL) *key_len_out = entry->key_len;
  if (value_out != NULL) *value_out = entry->value;
  return true;
}

bool HTIterator_remove(HTIterator *hti, unsigned char **key_out,
    size_t *key_len_out, HTValue *value_out) {
  if (hti == NULL || !HTIterator_is_valid(hti)) return false;

  unsigned char *key;
  size_t key_len;
  if (!HTIterator_get(hti, (const unsigned char **)&key, &key_len, NULL)) {
    return false;
  }

  HTIterator_next(hti);

  if (key_out != NULL) *key_out = key;
  if (key_len_out != NULL) *key_len_out = key_len;
  HTValue ignored;
  HashTable_remove(hti->table, key, key_len,
      value_out != NULL ? value_out : &ignored);
  return true;
}

