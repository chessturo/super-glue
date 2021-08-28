/* Provides a doubly linked list data structure.
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

#ifndef SUPER_GLUE_LIB_INCLUDE_LINKED_LIST_H_
#define SUPER_GLUE_LIB_INCLUDE_LINKED_LIST_H_

#include <stdbool.h>

typedef struct _ll LinkedList;
typedef struct _lli LLIterator;
typedef void* LLPayload;
typedef void(*LLPayloadFreeFn)(LLPayload payload);

// Allocates a new LinkedList. Caller assumes responsibility for later passing
// the returned pointer to `LinkedList_free`.
//
// Returns NULL if out of memory, a pointer to a newly allocated LinkedList
// otherwise.
LinkedList *LinkedList_allocate(); 

// Frees a given LinkedList structure. The payload of each node in the linked
// list is passed to `payload_free`.
//
// list         - The list to free. If NULL this function is safe to call, and
//                results in a NO OP.
// payload_free - The payload of each list is passed to this function in order
//                to help the client make sure the contents of the list are
//                freed. If the payload is statically allocated, pass a NO OP or
//                NULL.
void LinkedList_free(LinkedList *list, LLPayloadFreeFn payload_free);

// Returns the number of elements in a LinkedList structure. Returns -1 if
// `list` is NULL.
int LinkedList_num_elements(LinkedList *list);

// Prepends (i.e., inserts at the head of the list) an element to `list`.
//
// list    - The list to add an element to.
// payload - The element to add.
//
// Returns true if the payload is successfully added, false otherwise (e.g.,
// if `list` is NULL or there's not enough memory to allocate internal
// bookkeeping data).
bool LinkedList_prepend(LinkedList *list, LLPayload payload);

// Appends (i.e., inserts at the end of the list) and element to `list`.
//
// list    - The list to add an element to.
// payload - The element to add.
//
// Returns true if the payload is successfully added, false otherwise (e.g.,
// if `list` is NULL or there's not enough memory to allocate internal
// bookkeeping data).
bool LinkedList_append(LinkedList *list, LLPayload payload);

// Removes an element from the front of a LinkedList. The caller assumes
// responibility for dealing with `*payload_out` depending upon the underlying
// data.
//
// list        - The list to remove an element from.
// payload_out - An output parameter. The removed element is returned through
//               this parameter. If this parameter is NULL, this function
//               returns false without modifying the list.
//
// Returns true on success, false otherwise (e.g., `list` is NULL or empty).
// If `false` is returned then the list is not modified.
bool LinkedList_pop_head(LinkedList *list, LLPayload *payload_out);

// Removes an element from the end of a LinkedList. The caller assumes
// responibility for dealing with `*payload_out` depending upon the underlying
// data.
//
// list        - The list to remove an element from.
// payload_out - An output parameter. The removed element is returned through
//               this parameter. If this parameter is NULL, this function
//               returns false without modifying the list.
//
// Returns true on success, false otherwise (e.g., `list` is NULL or empty).
// If `false` is returned then the list is not modified.
bool LinkedList_pop_tail(LinkedList *list, LLPayload *payload_out);

// Allocates a new LLIterator struct for the given list. Don't attempt to use
// this iterator if the underlying list is modified using LinkedList_* methods;
// the only way to safely modify the list is using LLIterator_* methods or
// free the iterator and create a new one after the modification.
//
// Returns the new LLIterator struct on success, NULL on failure (e.g., if
// `list` is NULL or there's not enough memory).
LLIterator *LLIterator_allocate(LinkedList *list);

// Frees a LLIterator struct without modifying the underlying list. NO OP if the
// provided iterator is NULL.
void LLIterator_free(LLIterator *lli);

// Checks if the given LLIterator is valid. An iterator is said to be valid when
// it is "pointing" at an element in the list. An example of an invalid iterator
// would be one where it was pointing at the end of the list and then advanced
// to the next element, or an iterator for an empty list.
//
// lli - The iterator to query
//
// Returns true if the iterator is valid, false if it is invalid or NULL.
bool LLIterator_is_valid(LLIterator *lli);

// Returns a pointer to the payload of the linked list node that `lli` is
// "pointing" to. Do not free this pointer.
//
// lli - The iterator to query
//
// Returns a pointer to the payload of the node that `lli` is pointing to.
// Returns NULL on failure (e.g., lli is invalid).
LLPayload *LLIterator_get(LLIterator *lli);

// Removes the node that `lli` is pointing to from the underlying list and
// returns the node's payload through an output parameter. The iterator can be
// left in three possible states after the return of this call:
// - `lli` was pointing at the only element in the list; in this case `lli`
//   becomes an invalid iterator.
// - `lli` was pointing at the end of the list with >= 2 elements; in this case
//   `lli` is now pointing at the original elements predecessor. (e.g., in the
//   list A <-> B <-> C, if `lli` was pointing at C and had remove called, it
//   would now be pointing at B).
// - `lli` was pointing at any element in a list of length >= 2 that's not the
//   tail of the list. In this case, `lli` is pointing at the element that was
//   originally after the element removed (e.g., in the list A <-> B <-> C,
//   where lli was originally pointing at B, after this function is called it
//   would be pointing at C).
//
// lli         - The iterator to remove an element at.
// payload_out - An output parameter, `*payload_out` is set to be the payload
//               of the removed element. If this parameter is NULL then false
//               is returned by this function and the underlying list is not
//               modified.
//
// Returns true on success, false otherwise (e.g., an invalid iterator). If
// false is returned then the underlying list has not been modified.
bool LLIterator_remove(LLIterator *lli, LLPayload *payload_out);

// Advances `lli` to the next node in the list.
//
// lli - The iterator to advance.
//
// Returns true if `lli` is a valid iterator after this operation, false
// otherwise (including if `lli` was invalid/NULL when the method was called).
bool LLIterator_next(LLIterator *lli);

// Moves `lli` to the previous node in the list.
//
// lli - The iterator to move.
//
// Returns true if `lli` is a valid iterator after this operation, false
// otherwise (including if `lli` was invalid/NULL when the method was called).
bool LLIterator_prev(LLIterator *lli);

// Rewinds `lli` to the first node in the list.
//
// lli - The iterator to rewind.
//
// Returns true if `lli` is a valid iterator after this operation, false
// otherwise (e.g., the underlying list is empty, `lli` is NULL).
bool LLIterator_rewind(LLIterator *lli);

// Fast forwards `lli` to the last node in the list.
//
// lli - The iterator to fast forward.
//
// Returns true if `lli` is a valid iterator after this operation, false
// otherwise (e.g., the underlying list is empty, `lli` is NULL).
bool LLIterator_fast_forward(LLIterator *lli);

#endif  // SUPER_GLUE_LIB_INCLUDE_LINKED_LIST_H_

