/* Declaration of structs that aid in managing global state and functions
   to manage these structs.
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

#ifndef SUPER_GLUE_INCLUDE_STATE_H_ 
#define SUPER_GLUE_INCLUDE_STATE_H_ 

#include <stdbool.h>
#include <stdio.h>

// Specifies the global state of the current execution of super-glue.
typedef struct {
  bool interactive;
  bool version_info_requested;
} State;

// Holds references to the configuration files that are currently in use.
typedef struct {
  int file_count;
  FILE **files;  // Array of `FILE *`, length `file_count`.
} ConfigFiles;

// Attempts to malloc a `State`. Caller has responsibility of calling
// `free_state` on the returned `State`.
//
// state - An output parameter through which a newly allocated `State` struct
//         is returned. Contents are unspecified on failure.
//
// Returns true if successfully able to allocate the new struct, false
// otherwise.
bool alloc_state(State **state);

// Frees a pointer to an allocated `State` struct. This struct should've been
// initialized using `alloc_state`. Do not attempt to free any of the members
// of `state` before passing it to this function.
//
// state - A pointer to a `State` struct created using `alloc_state`. This
//         pointer should not be used after this function returns.
void free_state(State *state);

// Attempts to malloc a `ConfigFiles` struct. The caller has the responsibility 
// of eventually calling `free_config_files` on the returned `ConfigFiles`. This
// method opens files supplied and stores the open `FILE *`s in the returned
// struct.
//
// file_count - The length of the file_names array, stored directly in the
//              returned struct.
// file_names - The file names to attempt to open and store in the returned
//              struct.
// files      - An output parameter holding a `ConfigFiles` struct that's been
//              fully populated. Unspecified contents on failure.
// error      - On success, set to NULL. On error, filled with a malloc'd string
//              containing details of the error that occurred, suitable for
//              presentation to the user. If memory cannot be allocated for the
//              string, it's set to NULL and `false` is returned.
//
// This method returns true on success, false otherwise.
bool alloc_config_files(int file_count, char *file_names[], ConfigFiles **files,
    char **error);

// Frees a pointer to an allocated `ConfigFiles` struct. Do not attempt to free
// any members of the struct before passing it to this function.
//
// files - A pointer to a `ConfigFiles` struct allocated using
//         `alloc_config_files`. Do not attempt to use this pointer after this
//         function has returned.
void free_config_files(ConfigFiles *files);
#endif  // SUPER_GLUE_INCLUDE_STATE_H_ 
