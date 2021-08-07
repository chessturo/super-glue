/* Contains definitions of functions that help manage global state structs.
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

#include "util.h"
#define _XOPEN_SOURCE 500

#include "state.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool alloc_state(State **state) {
  *state = malloc(sizeof(State));
  if (*state == NULL) {
    return false;
  }

  // Initialize to be a default `State`
  (*state)->interactive = false;
  (*state)->version_info_requested = false;

  return true;
}

void free_state(State *state) {
  if (state == NULL) return;
  free(state);
}

bool alloc_config_files(int file_count, char *file_names[], ConfigFiles **files,
    char **error) {
  *error = NULL;
  *files = malloc(sizeof(ConfigFiles));
  if (*files == NULL) {
    *error = strdup(strerror(ENOMEM));
    return false; 
  }

  // Set up the struct members.
  (*files)->file_count = file_count;
  (*files)->files = (FILE **)malloc(file_count * sizeof(FILE *));
  if ((*files)->files == NULL) {
    *error = strdup(strerror(ENOMEM));
    free(*files);
    // Safety to prevent a double-free if the user passes *files into
    // `FreeConfigFiles`, despite the fact they shouldn't.
    *files = NULL;
    return false;
  }
  
  // Populate the array of files.
  for (int i = 0; i < file_count; i++) {
    FILE *current_file = fopen(file_names[i], "r");
    if (current_file == NULL) {
      alloc_sprintf(error, "Error opening file \"%s\" - %s",
          file_names[i], strerror(errno));
      (*files)->file_count = i;  // Current length of `(*files)->files`.
      // Closes any files we may have opened eariler.
      free_config_files(*files);
      // Safety to prevent a double-free if the user passes *files into
      // `FreeConfigFiles`, despite the fact they shouldn't.
      *files = NULL; 
      return false;
    }

    (*files)->files[i] = current_file;
  }

  return true;
}

void free_config_files(ConfigFiles *files) {
  if (files == NULL) return;

  // `fclose` all open `FILE *`s
  for (int i = 0; i < files->file_count; i++) {
    // Ignore errors from `fclose`, there's no real way to handle them,
    // and we still need to close the rest of the files.
    fclose(files->files[i]);
  }

  // Free the array of `FILE *`s
  free(files->files);

  // Free the overall struct
  free(files);
}
