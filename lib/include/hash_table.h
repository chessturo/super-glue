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

#ifndef SUPER_GLUE_LIB_INCLUDE_HASH_TABLE_H_
#define SUPER_GLUE_LIB_INCLUDE_HASH_TABLE_H_

#include <stdbool.h>
#include <stddef.h>

typedef struct _HT HashTable;
typedef struct _HTIt HTIterator;

typedef void *HTValue;

typedef void(*HTValue_free)(HTValue);

// Allocates a new HashTable structure. Caller assumesresponsibility of
// eventually passing the returned pointer to HashTable_free.
//
// Returns a pointer to a newly allocated HashTable structure, or NULL on
// failure (such as being out of memory).
HashTable *HashTable_allocate();

// Frees a HashTable structure, and optionally all elements in it.
//
// ht         - The HashTable to free.
// value_free - Each HT_Value in the table is passed to this function to be
//              freed. If NULL is passed, the overall table itself is freed but
//              none of the HT_Values in it are.
void HashTable_free(HashTable *ht, HTValue_free value_free);

// Returns the number of elements in a HashTable.
//
// ht - The HashTable to query.
//
// Returns the number of elements in a HashTable. If ht is NULL, returns -1.
int HashTable_num_elements(HashTable *ht);

// Inserts an entry into the hash table with the given key/value. If an entry
// already exists with the given key, it is overwritten.
//
// ht        - The HashTable to insert the value into. If this value is NULL the
//             function returns false without doing anything.
// key       - The lookup key for this value. This can be arbitrary data, and
//             it does not need to be hashed prior to passing it to this
//             function. This function makes a copy of key, and therefore does
//             not take ownership of key's memory if its dyamically allocated.
//             If key is NULL, this function returns false without doing
//             anything.
// key_len   - The length of key, in unsigned chars. If zero is passed, key is
//             assumed to be terminated with a '\0', useful if your keys are
//             NUL terminated strings. When passing a zero as a length, the
//             '\0' is NOT considered to be part of the key.
// new_value - The value that key will map to in the HashTable.
// old_value - If key already exists in the hash table, this output parameter
//             is populated with the previous value. *old_value is not modified
//             if key did not previously exist in the HashTable. Ignored if
//             NULL.
//
// Returns true if there was previously an entry for key in ht. In other words,
// returns true when *old_value has been populated.
bool HashTable_insert(HashTable *ht, unsigned char *key, size_t key_len,
    HTValue new_value, HTValue *old_value);

// Searches a HashTable for a given key. If an entry with the key exists, a
// pointer to its value is returned. Note, this is a pointer into the HashTable,
// so do not attempt to free it, and remember that any modifications to the
// underlying data will be reflected when the key is looked up again.
//
// ht      - The HashTable to query. If NULL, this function returns NULL.
// key     - A pointer to the key to lookup. If NULL, this function returns
//           NULL.
// key_len - The length of the key in unsigned chars. If zero, key is assumed
//           to end with a '\0', useful for NUL terminated strings as keys.
//
// Returns a pointer to the value associated with the given key, or NULL if
// the key isn't present in ht.
HTValue *HashTable_find(HashTable *ht, unsigned char *key, size_t key_len);

// Removes an entry from a HashTable with a given key.
//
// ht        - The HashTable to remove from.
// key       - The key of the entry to remove.
// key_len   - The length of the key in unsigned chars.
// old_value - An output parameter; if an entry with a matching key is found in
//             the table, the removed value is passed through this parameter.
//             This allows for the value to be appropriately freed if needed.
// 
// Returns true if a value was found and removed, false if an element with a
// matching key wasn't found. If this function returns false, *old_value isn't
// modified.
bool HashTable_remove(HashTable *ht, unsigned char *key, size_t key_len,
    HTValue *old_value);

// Allocates a new HTIterator. This iterator will iterate over a HashTable in
// an arbitrary order (not necessarily the order of element insertion). If an
// element is added or removed during the lifetime of this iterator, its further
// use is undefined. The caller takes responsibility of eventually passing the
// pointer returned by this function to HTIterator_free.
//
// ht - The HashTable to iterate over.
//
// Returns a pointer to a newly allocated HTIterator on success, NULL on
// failure (e.g., out of memory, ht is NULL).
HTIterator *HTIterator_allocate(HashTable *ht);

// Frees a HTIterator.
//
// hti - The iterator to free.
void HTIterator_free(HTIterator *hti);

// Checks if a HTIterator is valid (i.e., if a call to HTIterator_get would 
// succeed).
//
// hti - The HTIterator to query
//
// Returns true if the iterator is valid, false otherwise.
bool HTIterator_is_valid(HTIterator *hti);

// Advances a HTIterator to the next element.
//
// hti - The iterator to advance.
//
// Returns true if the HTIterator is valid after the operation, false otherwise
// (including if hti was invalid/NULL when the method was called).
bool HTIterator_next(HTIterator *hti);

// Gets the entry pointed to by the given HTIterator
//
// hti         - The iterator to query.
// key_out     - An output parameter set to the key. **key_out should not be
//               modified. Ignored if NULL.
// key_len_out - An output parameter set to the length of the key. Ignored if
//               NULL.
// value_out   - An output parameter set to the entry's value. Ignored if NULL.
//
// Returns true if an element was successfully retrieved, false otherwise (e.g.,
// if hti is NULL or invalid).
bool HTIterator_get(HTIterator *hti, const unsigned char **key_out,
    size_t *key_len_out, HTValue *value_out);

// Removes the entry pointed to by the given HTIterator. The removed value is
// passed back to the caller through the output parameters; the caller assumes
// responsibility for the associated memory.
//
// hti         - The iterator to query.
// key_out     - An output parameter set to the key. Ignored if NULL.
// key_len_out - An output parameter set to the length of the key. Ignored if
//               NULL.
// value_out   - An output parameter set to the entry's value. Ignored if NULL.
//
// Returns true if an element was removed, false otherwise (hti is
// NULL/invalid). On success, hti is advanced to the next entry, and may now
// be invalid if the removed element was the last entry in the table.
bool HTIterator_remove(HTIterator *hti, unsigned char **key_out,
    size_t *key_len_out, HTValue *value_out);

#endif  // SUPER_GLUE_LIB_INCLUDE_HASH_TABLE_H_

