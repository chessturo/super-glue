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

#include "linked_list.h"

#include <stdlib.h>

typedef struct lln {
  struct lln *next, *prev;
  LLPayload payload;
} LLNode;

// Typedef'd to LinkedList in "linked_list.h"
struct _ll {
  LLNode *head, *tail;
  int num_elems;
};

// Typedef'd to LLIterator in "linked_list.h"
struct _lli {
  LinkedList *list;
  LLNode *current;
};

LinkedList *LinkedList_allocate() {
  LinkedList *ll = malloc(sizeof(LinkedList));
  if (ll == NULL) return NULL;

  ll->head = NULL;
  ll->tail = NULL;
  ll->num_elems = 0;
  return ll;
}

void LinkedList_free(LinkedList *list, LLPayloadFreeFn payload_free) {
  if (list == NULL) return;
  LLNode *curr = list->head;

  // Doesn't use an LLIterator since there's a possibility LLIterator_allocate()
  // could fail, and there's no meaningful way to deal with that.
  while (curr != NULL) {
    if (payload_free != NULL) {
      payload_free(curr->payload);
    }
    LLNode *next = curr->next;
    free(curr);
    curr = next;
  }

  free(list);
}

int LinkedList_num_elements(LinkedList *list) {
  if (list == NULL) return -1;
  return list->num_elems;
}

bool LinkedList_prepend(LinkedList *list, LLPayload payload) {
  if (list == NULL) return false;

  LLNode *new = malloc(sizeof(LLNode));
  if (new == NULL) return false;
  new->prev = NULL;
  new->next = list->head;
  new->payload = payload;

  if (list->num_elems == 0) {
    list->tail = new;
  } else {
    list->head->prev = new;
  }
  list->head = new;
  list->num_elems++;
  return true;
}

bool LinkedList_append(LinkedList *list, LLPayload payload) {
  if (list == NULL) return false;

  LLNode *new = malloc(sizeof(LLNode));
  if (new == NULL) return false;
  new->next = NULL;
  new->prev = list->tail;
  new->payload = payload;

  if (list->num_elems == 0) {
    list->head = new;
  } else {
    list->tail->next = new;
  }
  list->tail = new;
  list->num_elems++;
  return true;
}

bool LinkedList_pop_head(LinkedList *list, LLPayload *payload_out) {
  if (list == NULL || payload_out == NULL) return false;
  if (list->num_elems == 0) return false;

  LLNode *to_pop = list->head;
  if (list->num_elems != 1) {
    list->head = list->head->next;
    list->head->prev = NULL;
  } else {
    list->head = NULL;
    list->tail = NULL;
  }

  *payload_out = to_pop->payload;
  free(to_pop);
  list->num_elems--;
  return true;
}

bool LinkedList_pop_tail(LinkedList *list, LLPayload *payload_out) {
  if (list == NULL || payload_out == NULL) return false;
  if (list->num_elems == 0) return false;

  LLNode *to_pop = list->tail;
  if (list->num_elems != 1) {
    list->tail = list->tail->prev;
    list->tail->next = NULL;
  } else {
    list->tail = NULL;
    list->head = NULL;
  }

  *payload_out = to_pop->payload;
  free(to_pop);
  list->num_elems--;
  return true;
}

LLIterator *LLIterator_allocate(LinkedList *list) {
  if (list == NULL) return NULL;

  LLIterator *lli = malloc(sizeof(LLIterator));
  lli->list = list;
  lli->current = list->head;
  return lli;
}

void LLIterator_free(LLIterator *lli) {
  free(lli);
}

bool LLIterator_is_valid(LLIterator *lli) {
  if (lli == NULL) return false;
  return lli->current != NULL;
}

LLPayload *LLIterator_get(LLIterator *lli) {
  if (lli == NULL || lli->current == NULL) return NULL;
  return &(lli->current->payload);
}

bool LLIterator_remove(LLIterator *lli, LLPayload *payload_out) {
  if (lli == NULL || payload_out == NULL || lli->current == NULL) return false;

  // Edge cases where lli is pointing at the head/tail of the list
  if (lli->current == lli->list->head) {
    LinkedList_pop_head(lli->list, payload_out);
    lli->current = lli->list->head;
    return true;
  } else if (lli->current == lli->list->tail) {
    LinkedList_pop_tail(lli->list, payload_out);
    lli->current = lli->list->tail;
    return true;
  }

  LLNode *to_remove = lli->current;
  // Stitch together the list around to_remove
  lli->current->prev->next = lli->current->next;
  lli->current->next->prev = lli->current->prev;
  lli->list->num_elems--;

  // Update lli to no longer be pointing at the removed node
  lli->current = to_remove->next;

  *payload_out = to_remove->payload;
  free(to_remove);
  return true;
}

bool LLIterator_next(LLIterator *lli) {
  if (lli == NULL || lli->current == NULL) return false;
  lli->current = lli->current->next;
  return lli->current != NULL;
}

bool LLIterator_prev(LLIterator *lli) {
  if (lli == NULL || lli->current == NULL) return false;
  lli->current = lli->current->prev;
  return lli->current != NULL;
}

bool LLIterator_rewind(LLIterator *lli) {
  if (lli == NULL || lli->list->num_elems == 0) return false;
  lli->current = lli->list->head;
  return true;
}

bool LLIterator_fast_forward(LLIterator *lli) {
  if (lli == NULL || lli->list->num_elems == 0) return false;
  lli->current = lli->list->tail;
  return true;
}


