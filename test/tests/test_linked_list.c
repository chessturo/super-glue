/* Provides tests for `linked_list.c`
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

#include "test_linked_list.h"

#include <check.h>
#include <stdbool.h>

#include "linked_list.h"

// Helper variables
static LinkedList *ll;
static LLPayload one;
static LLPayload two;
static LLPayload three;

// Helper functions
// Compares two linked lists for equality. Two linked lists are said to be equal
// if the payload values for all nodes are equal and in the same order.
static bool LinkedList_eq(LinkedList *l1, LinkedList *l2) {
  if (LinkedList_num_elements(l1) != LinkedList_num_elements(l2)) {
    return false;
  }
  LLIterator *li1 = LLIterator_allocate(l1);
  LLIterator *li2 = LLIterator_allocate(l2);
  ck_assert(li1 != NULL);
  ck_assert(li2 != NULL);

  while (LLIterator_is_valid(li1)) {
    if (*LLIterator_get(li1) != *LLIterator_get(li2)) {
      LLIterator_free(li1);
      LLIterator_free(li2);
      return false;
    }
    LLIterator_next(li1);
    LLIterator_next(li2);
  }

  LLIterator_free(li1);
  LLIterator_free(li2);
  return true;
}

static void common_setup() {
  ll = NULL;
  one = (LLPayload)1;
  two = (LLPayload)2;
  three = (LLPayload)3;
}
static void common_teardown() { }

// Core test cases
static void core_setup() {
  common_setup();
}
static void core_teardown() {
  common_teardown();
}

START_TEST(allocate) {
  ll = LinkedList_allocate();
  ck_assert_msg(ll != NULL, "Allocating a new LinkedList shouldn't return "
      "NULL");
  LinkedList_free(ll, NULL);
} END_TEST

// Bougs input test cases
static void bogus_input_setup() {
  common_setup();
  ll = LinkedList_allocate();
  ck_assert(ll != NULL);
}
static void bogus_input_teardown() {
  common_teardown();
  LinkedList_free(ll, NULL);
}

START_TEST(free_null) {
  // fails on segfault
  LinkedList_free(NULL, NULL);
} END_TEST

START_TEST(num_elements_null) {
  int res = LinkedList_num_elements(NULL);
  ck_assert_msg(res == -1, "Calling num_elements on NULL should result in -1");
} END_TEST

START_TEST(prepend_null) {
  ck_assert_msg(!LinkedList_prepend(NULL, one), "Prepending to NULL should "
      "return false");
} END_TEST

START_TEST(append_null) {
  ck_assert_msg(!LinkedList_append(NULL, one), "Appending to NULL should "
      "return false");
} END_TEST

START_TEST(pop_head_null_list) {
  LLPayload out_ref = (LLPayload)0xDEADBEEF;
  LLPayload out = out_ref;
  ck_assert_msg(!LinkedList_pop_head(NULL, &out), "Popping from the head of "
      "NULL should return false");
  ck_assert_msg(out == out_ref, "Popping from the head of NULL should not "
      "modify the output parameter");
} END_TEST

START_TEST(pop_head_empty_list) {
  LLPayload out_ref = (LLPayload)0xDEADBEEF;
  LLPayload out = out_ref;
  LinkedList *empty = ll;

  ck_assert_msg(!LinkedList_pop_head(empty, &out), "Popping from the head of "
      "an empty list should return false");
  ck_assert_msg(out == out_ref, "Popping from the head of an empty list should "
      "not modify the output parameter");
} END_TEST

START_TEST(pop_tail_null_list) {
  LLPayload out_ref = (LLPayload)0xDEADBEEF;
  LLPayload out = out_ref;
  ck_assert_msg(!LinkedList_pop_tail(NULL, &out), "Popping from the tail of "
      "NULL should return false");
  ck_assert_msg(out == out_ref, "Popping from the tail of NULL should not "
      "modify the output parameter");
} END_TEST

START_TEST(pop_tail_empty_list) {
  LLPayload out_ref = (LLPayload)0xDEADBEEF;
  LLPayload out = out_ref;
  LinkedList *empty = ll;

  ck_assert_msg(!LinkedList_pop_tail(empty, &out), "Popping from the tail of "
      "an empty list should return false");
  ck_assert_msg(out == out_ref, "Popping from the tail of an empty list should "
      "not modify the output parameter");
} END_TEST

START_TEST(iterator_allocate_null) {
  ck_assert_msg(LLIterator_allocate(NULL) == NULL, "Allocating an iterator for "
      "a NULL list should return NULL");
} END_TEST

START_TEST(iterator_free_null) {
  // fails on segfault
  LLIterator_free(NULL);
} END_TEST

START_TEST(iterator_get_null) {
  ck_assert_msg(LLIterator_get(NULL) == NULL, "Getting an element from a NULL "
      "iterator should return NULL");
} END_TEST

START_TEST(iterator_get_invalid) {
  LinkedList *empty = ll;
  LLIterator *invalid = LLIterator_allocate(empty);

  ck_assert_msg(!LLIterator_is_valid(invalid), "An iterator for an empty list "
      "should be invalid");
  ck_assert_msg(LLIterator_get(invalid) == NULL, "Getting an element from an "
      "invalid interator should return NULL");
} END_TEST

START_TEST(iterator_remove_from_null) {
  LLPayload out_ref = (LLPayload)0xDEADBEEF;
  LLPayload out = out_ref;

  ck_assert_msg(!LLIterator_remove(NULL, &out), "Removing an element from a "
      "NULL iterator should return false");
  ck_assert_msg(out == out_ref, "Removing an element from a NULL iterator "
      "should not modify the output parameter");
} END_TEST

START_TEST(iterator_remove_from_invalid) {
  LLPayload out_ref = (LLPayload)0xDEADBEEF;
  LLPayload out = out_ref;
  LinkedList *empty = ll;
  LLIterator *invalid = LLIterator_allocate(empty);

  ck_assert_msg(!LLIterator_is_valid(invalid), "An iterator for an empty list "
      "should be invalid");
  ck_assert_msg(!LLIterator_remove(invalid, &out), "Removing an element from "
      "an invalid iterator should return false");
  ck_assert_msg(out == out_ref, "Removing an element from an invalid iterator "
      "should not modify the output parameter");
} END_TEST

START_TEST(iterator_next_null) {
  ck_assert_msg(!LLIterator_next(NULL), "Advancing a NULL iterator to the next "
      "element should return false");
} END_TEST

START_TEST(iterator_prev_null) {
  ck_assert_msg(!LLIterator_prev(NULL), "Moving a NULL iterator to the "
      "previous element should return false");
} END_TEST

START_TEST(iterator_rewind_null) {
  ck_assert_msg(!LLIterator_prev(NULL), "Rewinding a NULL iterator should "
      "return false");
} END_TEST

START_TEST(iterator_fast_forward_null) {
  ck_assert_msg(!LLIterator_prev(NULL), "Fast forwarding a NULL iterator "
      "should return false");
} END_TEST

// List manipulation test cases
static void list_manipulation_setup() {
  common_setup();
  ll = LinkedList_allocate();
  ck_assert(ll != NULL);
}
static void list_manipulation_teardown() {
  common_teardown();
  LinkedList_free(ll, NULL);
}

START_TEST(num_elements) {
  ck_assert(LinkedList_num_elements(ll) == 0);

  LinkedList_append(ll, one);
  ck_assert(LinkedList_num_elements(ll) == 1);

  LinkedList_append(ll, two);
  ck_assert(LinkedList_num_elements(ll) == 2);
  
  LLPayload out;
  LinkedList_pop_head(ll, &out);
  ck_assert(LinkedList_num_elements(ll) == 1);

  LinkedList_pop_head(ll, &out);
  ck_assert(LinkedList_num_elements(ll) == 0);
} END_TEST;

START_TEST(prepend_empty) {
  LinkedList_prepend(ll, one);
  ck_assert(LinkedList_num_elements(ll) == 1);
  LLPayload out;
  ck_assert(LinkedList_pop_head(ll, &out));  
  // Use ptr assertion since LLPayload has width of void*
  ck_assert(one == out);
} END_TEST

START_TEST(prepend) {
  LinkedList_append(ll, two);
  LinkedList_append(ll, three);

  LinkedList *ll_cmp = LinkedList_allocate();
  ck_assert(ll_cmp != NULL);
  LinkedList_append(ll_cmp, one);
  LinkedList_append(ll_cmp, two);
  LinkedList_append(ll_cmp, three);

  LinkedList_prepend(ll, one);

  ck_assert(LinkedList_num_elements(ll) == 3);
  ck_assert(LinkedList_eq(ll, ll_cmp));
} END_TEST

START_TEST(append_empty) {
  LinkedList_append(ll, one);

  ck_assert(LinkedList_num_elements(ll) == 1);
  LLPayload out;
  ck_assert(LinkedList_pop_head(ll, &out));
  ck_assert(one == out);
} END_TEST

START_TEST(append) {
  LinkedList_prepend(ll, two);
  LinkedList_prepend(ll, one);

  LinkedList *ll_cmp = LinkedList_allocate();
  ck_assert(ll_cmp != NULL);
  LinkedList_prepend(ll_cmp, three);
  LinkedList_prepend(ll_cmp, two);
  LinkedList_prepend(ll_cmp, one);

  LinkedList_append(ll, three);

  ck_assert(LinkedList_num_elements(ll) == 3);
  ck_assert(LinkedList_eq(ll, ll_cmp));
} END_TEST

START_TEST(pop_head_len_one) {
  LinkedList_append(ll, one);

  LLPayload out;
  ck_assert(LinkedList_pop_head(ll, &out));

  ck_assert(LinkedList_num_elements(ll) == 0);
  ck_assert(out == one);
} END_TEST

START_TEST(pop_head) {
  LinkedList_append(ll, one);
  LinkedList_append(ll, two);
  LinkedList_append(ll, three);

  LinkedList *ll_cmp = LinkedList_allocate();
  LinkedList_append(ll_cmp, two);
  LinkedList_append(ll_cmp, three);

  LLPayload out;
  ck_assert(LinkedList_pop_head(ll, &out));
  ck_assert(LinkedList_num_elements(ll) == 2);
  ck_assert(LinkedList_eq(ll, ll_cmp));
} END_TEST

START_TEST(pop_tail_len_one) {
  LinkedList_append(ll, one);

  LLPayload out;
  ck_assert(LinkedList_pop_tail(ll, &out));

  ck_assert(LinkedList_num_elements(ll) == 0);
  ck_assert(out == one);
} END_TEST

START_TEST(pop_tail) {
  LinkedList_append(ll, one);
  LinkedList_append(ll, two);
  LinkedList_append(ll, three);

  LinkedList *ll_cmp = LinkedList_allocate();
  LinkedList_append(ll_cmp, one);
  LinkedList_append(ll_cmp, two);

  LLPayload out;
  ck_assert(LinkedList_pop_tail(ll, &out));
  ck_assert(LinkedList_num_elements(ll) == 2);
  ck_assert(LinkedList_eq(ll, ll_cmp));
} END_TEST

// Iterator test cases
LLIterator *lli;
void iterator_setup() {
  common_setup();
  ll = LinkedList_allocate();
  ck_assert(ll != NULL);

  LinkedList_append(ll, one);
  LinkedList_append(ll, two);
  LinkedList_append(ll, three);

  lli = LLIterator_allocate(ll);
  ck_assert(lli != NULL);
}
void iterator_teardown() {
  LLIterator_free(lli);
  LinkedList_free(ll, NULL);
}

START_TEST(iter_valid) {
  LinkedList *empty = LinkedList_allocate();
  LLIterator *invalid = LLIterator_allocate(empty);

  ck_assert(LLIterator_is_valid(lli));
  ck_assert(!LLIterator_is_valid(invalid));

  LLIterator_fast_forward(lli);
  ck_assert(!LLIterator_next(lli));
  ck_assert(!LLIterator_is_valid(lli));

  LLIterator_rewind(lli);
  ck_assert(LLIterator_is_valid(lli));

  LLIterator_free(invalid);
  LinkedList_free(empty, NULL);
} END_TEST

START_TEST(iter_get) {
  ck_assert(*LLIterator_get(lli) == one);

  LLIterator_next(lli);
  ck_assert(*LLIterator_get(lli) == two);

  LLIterator_next(lli);
  ck_assert(*LLIterator_get(lli) == three);

  LLIterator_rewind(lli);
  ck_assert(*LLIterator_get(lli) == one);
} END_TEST

START_TEST(iter_remove) {
  LLIterator_next(lli); // Should now be pointing at two
  ck_assert(*LLIterator_get(lli) == two);

  // Test middle case
  // 1 <-> 2 <-> 3
  //      /|\ letters to silence -Wcomment
  //       |
  // to
  // 1 <-> 3
  //      /|\ letters to silence -Wcomment
  //       |
  LLPayload out;
  ck_assert(LLIterator_remove(lli, &out));
  ck_assert(out == two);
  ck_assert(*LLIterator_get(lli) == three);

  // Test end case
  // 1 <-> 3
  //      /|\ letters to silence -Wcomment
  //       |
  // to 
  //  1
  // /|\ letters to silence -Wcomment
  //  |
  ck_assert(LLIterator_remove(lli, &out));
  ck_assert(out == three);
  ck_assert(*LLIterator_get(lli) == one);

  // Test case with only one element
  ck_assert(LLIterator_remove(lli, &out));
  ck_assert(out == one);
  ck_assert(!LLIterator_is_valid(lli));
} END_TEST

START_TEST(iter_next) {
  ck_assert(LLIterator_next(lli)); // Now points at two
  ck_assert(LLIterator_next(lli)); // Now points at three
  ck_assert(!LLIterator_next(lli)); // Now invalid
} END_TEST

START_TEST(iter_prev) {
  LLIterator_fast_forward(lli);
  ck_assert(LLIterator_prev(lli)); // Now points at two
  ck_assert(LLIterator_prev(lli)); // Now points at one
  ck_assert(!LLIterator_prev(lli)); // Now invalid
} END_TEST

START_TEST(iter_rewind) {
  LLIterator_next(lli); // get lli off the first element
  ck_assert(LLIterator_rewind(lli));
  ck_assert(*LLIterator_get(lli) == one);
} END_TEST

START_TEST(iter_fast_forward) {
  ck_assert(LLIterator_fast_forward(lli));
  ck_assert(*LLIterator_get(lli) == three);
} END_TEST

Suite *linked_list_tests() {
  Suite *s = suite_create("LinkedList");
  TCase *tc_core = tcase_create("core");
  tcase_add_checked_fixture(tc_core, &core_setup, &core_teardown);
  tcase_add_test(tc_core, allocate);
  suite_add_tcase(s, tc_core);

  TCase *tc_bogus = tcase_create("bogus input");
  tcase_add_checked_fixture(tc_bogus, &bogus_input_setup,
      &bogus_input_teardown);
  tcase_add_test(tc_bogus, free_null);
  tcase_add_test(tc_bogus, num_elements_null);
  tcase_add_test(tc_bogus, prepend_null);
  tcase_add_test(tc_bogus, append_null);
  tcase_add_test(tc_bogus, pop_head_null_list);
  tcase_add_test(tc_bogus, pop_head_empty_list);
  tcase_add_test(tc_bogus, pop_tail_null_list);
  tcase_add_test(tc_bogus, pop_tail_empty_list);
  tcase_add_test(tc_bogus, iterator_allocate_null);
  tcase_add_test(tc_bogus, iterator_free_null);
  tcase_add_test(tc_bogus, iterator_get_null);
  tcase_add_test(tc_bogus, iterator_get_invalid);
  tcase_add_test(tc_bogus, iterator_remove_from_null);
  tcase_add_test(tc_bogus, iterator_remove_from_invalid);
  tcase_add_test(tc_bogus, iterator_next_null);
  tcase_add_test(tc_bogus, iterator_prev_null);
  tcase_add_test(tc_bogus, iterator_rewind_null);
  tcase_add_test(tc_bogus, iterator_fast_forward_null);
  suite_add_tcase(s, tc_bogus);

  TCase *tc_list = tcase_create("list manipulation");
  tcase_add_checked_fixture(tc_list, &list_manipulation_setup,
      &list_manipulation_teardown);
  tcase_add_test(tc_list, num_elements);
  tcase_add_test(tc_list, prepend_empty);
  tcase_add_test(tc_list, prepend);
  tcase_add_test(tc_list, append_empty);
  tcase_add_test(tc_list, append);
  tcase_add_test(tc_list, pop_head_len_one);
  tcase_add_test(tc_list, pop_head);
  tcase_add_test(tc_list, pop_tail_len_one);
  tcase_add_test(tc_list, pop_tail);
  suite_add_tcase(s, tc_list);

  TCase *tc_iter = tcase_create("iterator");
  tcase_add_checked_fixture(tc_iter, &iterator_setup, &iterator_teardown);
  tcase_add_test(tc_iter, iter_valid);
  tcase_add_test(tc_iter, iter_get);
  tcase_add_test(tc_iter, iter_remove);
  tcase_add_test(tc_iter, iter_next);
  tcase_add_test(tc_iter, iter_prev);
  tcase_add_test(tc_iter, iter_rewind);
  tcase_add_test(tc_iter, iter_fast_forward);
  suite_add_tcase(s, tc_iter);

  return s;
}

