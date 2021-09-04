/* Provides tests for `hash_table.c`
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

// For strdup
#define _XOPEN_SOURCE 500

#include "test_hash_table.h"

#include <check.h>
#include <stdlib.h>
#include <string.h>

#include "hash_table.h"

// Helpers
HashTable *ht;
HTIterator *hti;

void common_setup() {
  ht = NULL;
  hti = NULL;
}
void common_teardown() {
  HTIterator_free(hti);
  HashTable_free(ht, &free);
}

// Bogus input handling test cases
void bogus_setup() {
  common_setup();
}
void bogus_teardown() {
  common_teardown();
}

START_TEST(free_null) {
  // Segfaults on failure
  HashTable_free(NULL, &free);
} END_TEST

START_TEST(free_with_null) {
  // Segfaults on failure.
  HashTable_free(ht, NULL);

  // Make sure ht isn't double free'd in the cleanup
  ht = NULL;
} END_TEST

START_TEST(num_elements_null) {
  ck_assert(HashTable_num_elements(NULL) == -1);
} END_TEST

START_TEST(insert_into_null) {
  ck_assert(!HashTable_insert(NULL, (unsigned char *)"abc", 0, (HTValue)0,
        NULL));
} END_TEST

START_TEST(insert_null_key) {
  ht = HashTable_allocate();

  ck_assert(!HashTable_insert(ht, NULL, 0, (HTValue)0, NULL));
  ck_assert(HashTable_num_elements(ht) == 0);
} END_TEST

START_TEST(find_from_null) {
  ck_assert(HashTable_find(NULL, (unsigned char *)"abc", 0) == NULL);
} END_TEST

START_TEST(find_null_key) {
  ht = HashTable_allocate();

  ck_assert(HashTable_find(ht, NULL, 0) == NULL);
} END_TEST

START_TEST(find_non_existent_entry) {
  ht = HashTable_allocate();
  ck_assert(!HashTable_insert(ht, (unsigned char *)"abc", 0, strdup("def"),
        NULL));

  ck_assert(HashTable_find(ht, (unsigned char *)"ghi", 0) == NULL);
} END_TEST

START_TEST(remove_from_null) {
  ck_assert(!HashTable_remove(NULL, (unsigned char *)"abc", 0, NULL));
} END_TEST

START_TEST(remove_null_key) {
  ht = HashTable_allocate();

  ck_assert(!HashTable_remove(ht, NULL, 0, NULL));
} END_TEST

START_TEST(remove_non_existent_entry) {
  ht = HashTable_allocate();

  ck_assert(!HashTable_remove(ht, (unsigned char *)"abc", 0, NULL));
} END_TEST

START_TEST(iter_allocate_null) {
  ck_assert(HTIterator_allocate(NULL) == NULL);
} END_TEST

START_TEST(iter_free_null) {
  // fails on segfault.
  HTIterator_free(NULL);
} END_TEST

START_TEST(iter_is_valid_null) {
  ck_assert(!HTIterator_is_valid(NULL));
} END_TEST

START_TEST(iter_next_null) {
  ck_assert(!HTIterator_next(NULL));
} END_TEST

START_TEST(iter_next_invalid) {
  ht = HashTable_allocate();
  hti = HTIterator_allocate(ht);
  ck_assert(!HTIterator_is_valid(hti));

  ck_assert(!HTIterator_next(hti));
} END_TEST

START_TEST(iter_get_null) {
  const unsigned char *key;
  size_t key_len;
  HTValue value;

  ck_assert(!HTIterator_get(NULL, &key, &key_len, &value));
} END_TEST

START_TEST(iter_get_invalid) {
  ht = HashTable_allocate();
  hti = HTIterator_allocate(ht);
  ck_assert(!HTIterator_is_valid(hti));

  const unsigned char *key;
  size_t key_len;
  HTValue value;

  ck_assert(!HTIterator_get(hti, &key, &key_len, &value));
} END_TEST

START_TEST(iter_remove_null) {
  unsigned char *key;
  size_t key_len;
  HTValue value;

  ck_assert(!HTIterator_remove(NULL, &key, &key_len, &value));
} END_TEST

START_TEST(iter_remove_invalid) {
  ht = HashTable_allocate();
  hti = HTIterator_allocate(ht);
  ck_assert(!HTIterator_is_valid(hti));

  unsigned char *key;
  size_t key_len;
  HTValue value;

  ck_assert(!HTIterator_remove(hti, &key, &key_len, &value));
} END_TEST

// Entry handling test cases
// English are keys, French are values
char *one;
char *two;
char *three;
char *un;
char *deux;
char *trois;

void entry_setup() {
  common_setup();
  one = strdup("one");
  two = strdup("two");
  three = strdup("three");
  un = strdup("un");
  deux = strdup("deux");
  trois = strdup("trois");

  ht = HashTable_allocate();

  ck_assert(!HashTable_insert(ht, (unsigned char *)one, 0, un, NULL)); 
  ck_assert(!HashTable_insert(ht, (unsigned char *)two, 0, deux, NULL)); 
  ck_assert(!HashTable_insert(ht, (unsigned char *)three, 0, trois, NULL)); 
}
void entry_teardown() {
  common_teardown();
  free(one);
  free(two);
  free(three);
}

START_TEST(num_elements_empty) {
  HashTable *empty = HashTable_allocate();

  ck_assert(HashTable_num_elements(empty) == 0);

  HashTable_free(empty, NULL);
} END_TEST

START_TEST(num_elements) {
  ck_assert(HashTable_num_elements(ht) == 3);
} END_TEST

START_TEST(insert) {
  HTValue old_value = (HTValue)0xDEADBEEF;

  unsigned char *key = (unsigned char *)"key";
  const char *value = "value";
  ck_assert(!HashTable_insert(ht, key, 0, strdup(value), &old_value));

  HTValue *found_value = HashTable_find(ht, key, 0);
  ck_assert(found_value != NULL);

  ck_assert(strcmp((char *)*found_value, value) == 0);
  ck_assert(old_value == (HTValue)0xDEADBEEF);
} END_TEST

START_TEST(insert_overwrite) {
  // Check to make sure there's an entry for "one" in the table
  ck_assert(HashTable_find(ht, (unsigned char *)one, 0) != NULL);

  HTValue old_value;
  HTValue new_value = strdup("eins");
  ck_assert(HashTable_insert(ht, (unsigned char *)one, 0, new_value,
        &old_value));

  ck_assert(strcmp((char *)old_value, (char *)un) == 0);
  ck_assert(
      strcmp(
        *HashTable_find(ht, (unsigned char *)one, 0),
        (char *)new_value
        ) == 0
      );

  // Since this won't get freed by HashTable_free anymore
  free(un);
} END_TEST

START_TEST(find) {
  ck_assert(strcmp(*HashTable_find(ht, (unsigned char *)one, 0), un) == 0);
  ck_assert(strcmp(*HashTable_find(ht, (unsigned char *)two, 0), deux) == 0);
  ck_assert(strcmp(*HashTable_find(ht, (unsigned char *)three, 0), trois) == 0);
} END_TEST

START_TEST(remove) {
  HTValue old_value;
  ck_assert(HashTable_remove(ht, (unsigned char *)one, 0, &old_value));
  ck_assert(strcmp(old_value, un) == 0);
} END_TEST

START_TEST(key_length) {
  HTValue *one_by_explicit_len_ptr =
    HashTable_find(ht, (unsigned char *)one, strlen(one));
  ck_assert(one_by_explicit_len_ptr != NULL);
  ck_assert(strcmp(*one_by_explicit_len_ptr, un) == 0);
} END_TEST

// Iterator test cases
const uint8_t max_key = UINT8_MAX;
HTIterator *hti;
static void iter_setup() {
  ht = HashTable_allocate();
  for (uint8_t key = 0; key < max_key; key++) {
    intptr_t val = (intptr_t) ~key;
    ck_assert(
        !HashTable_insert(ht, &key, sizeof(uint8_t), (HTValue)val, NULL));
  }
  hti = HTIterator_allocate(ht);
}
static void iter_teardown() {
  HTIterator_free(hti);
  HashTable_free(ht, NULL);
}

START_TEST(iterator_coverage) {
  uint8_t times_seen[max_key];
  memset(times_seen, 0, sizeof(times_seen));

  while (HTIterator_is_valid(hti)) {
    const uint8_t *key_ptr;
    size_t key_size;
    HTValue value;
    
    ck_assert(HTIterator_get(hti, (const unsigned char **)
          &key_ptr, &key_size, &value));
    ck_assert((intptr_t)value == (intptr_t) ~(*key_ptr));
    times_seen[*key_ptr]++;
    
    HTIterator_next(hti);
  }

  for (uint8_t key = 0; key < max_key; key++) {
    ck_assert(times_seen[key] == 1);
  }
} END_TEST

START_TEST(iterator_remove) {
  uint8_t times_seen[max_key];
  memset(times_seen, 0, sizeof(times_seen));

  while (HTIterator_is_valid(hti)) {
    const uint8_t *key_ptr;
    size_t key_size;
    HTValue value;
    
    ck_assert(HTIterator_remove(hti, (unsigned char **) &key_ptr, &key_size,
          &value));
    ck_assert((intptr_t)value == (intptr_t) ~(*key_ptr));
    times_seen[*key_ptr]++;
  }

  for (uint8_t key = 0; key < max_key; key++) {
    ck_assert(times_seen[key] == 1);
    ck_assert(HashTable_find(ht, &key, sizeof(uint8_t)) == NULL);
  }
} END_TEST

Suite *hash_table_tests() {
  Suite *s = suite_create("HashTable");

  TCase *tc_bogus = tcase_create("bogus input");
  tcase_add_checked_fixture(tc_bogus, &bogus_setup, &bogus_teardown);
  tcase_add_test(tc_bogus, free_null);
  tcase_add_test(tc_bogus, free_with_null);
  tcase_add_test(tc_bogus, num_elements_null);
  tcase_add_test(tc_bogus, insert_into_null);
  tcase_add_test(tc_bogus, insert_null_key);
  tcase_add_test(tc_bogus, find_from_null);
  tcase_add_test(tc_bogus, find_null_key);
  tcase_add_test(tc_bogus, find_non_existent_entry);
  tcase_add_test(tc_bogus, remove_from_null);
  tcase_add_test(tc_bogus, remove_null_key);
  tcase_add_test(tc_bogus, remove_non_existent_entry);
  tcase_add_test(tc_bogus, iter_allocate_null);
  tcase_add_test(tc_bogus, iter_free_null);
  tcase_add_test(tc_bogus, iter_is_valid_null);
  tcase_add_test(tc_bogus, iter_next_null);
  tcase_add_test(tc_bogus, iter_next_invalid);
  tcase_add_test(tc_bogus, iter_get_null);
  tcase_add_test(tc_bogus, iter_get_invalid);
  tcase_add_test(tc_bogus, iter_remove_null);
  tcase_add_test(tc_bogus, iter_remove_invalid);
  suite_add_tcase(s, tc_bogus);

  TCase *tc_entry = tcase_create("entry handling");
  tcase_add_checked_fixture(tc_entry, &entry_setup, &entry_teardown);
  tcase_add_test(tc_entry, num_elements_empty);
  tcase_add_test(tc_entry, num_elements);
  tcase_add_test(tc_entry, insert);
  tcase_add_test(tc_entry, insert_overwrite);
  tcase_add_test(tc_entry, find);
  tcase_add_test(tc_entry, remove);
  tcase_add_test(tc_entry, key_length);
  suite_add_tcase(s, tc_entry);

  TCase *tc_iter = tcase_create("iterator");
  tcase_add_checked_fixture(tc_iter, &iter_setup, &iter_teardown);
  tcase_add_test(tc_iter, iterator_coverage); 
  tcase_add_test(tc_iter, iterator_remove); 
  suite_add_tcase(s, tc_iter);
  return s;
}

