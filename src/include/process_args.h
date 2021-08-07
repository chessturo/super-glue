/* Declaration of functions/constants to aid in command-line argument processing
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

#ifndef SUPER_GLUE_INCLUDE_PROCESS_ARGS_H_
#define SUPER_GLUE_INCLUDE_PROCESS_ARGS_H_

#include <stdio.h>

#include "state.h"

typedef enum {
  ARGS_OK = 0,       // Signals success
  ARGS_NO_FILES,     // Signals the user passed no files. Note that this isn't
                     // necessarily an error, for example if a `--version` flag
                     // is given.
  ARGS_UNKNOWN,      // Signals that the user attempted to use an unknown
                     // option.
  ARGS_FILE_ERR,     // Signals there was an error opening files the user gave,
                     // with the specific error being passed through an output 
                     // parameter.
  ARGS_CONFLICT,     // Signals that the user provided two incompatible options.
                     // The specifics of the conflict are passed through an
                     // output parameter.
  ARGS_MEM,          // Signals that the process is out of memory.
  ARGS_NONE,         // Signals the user passed no arguments.
  ARGS_INVALID_USE,  // Signals the user gave extra info to this argument when
                     // unneccessary, or ommitted it when needed (e.g., running
                     // with a parameter `--interactive=50`).
  ARGS_AMBIGUOUS,    // Signals the user abbreviated a long-form option in an
                     // ambiguous way.
} ArgsResult;

// Parses command-line input, including passed-in options and files. The caller
// assumes owndership of the `error` string, which must be passed to `free`.
//
// argc  - The length of argv, analogous to `main`
// argv  - The vector of arguments passed into this call of super-glue.
//         `argv[0]` should be the name of the super-glue binary.
// state - An output parameter that describes the state `super-glue` 
//         should run in, after all the arguments have been parsed. Caller
//         assumes responsibility for passing this struct to `free_state` 
//         in the future, regardless of the return value of this call.
// files - An output parameter that is given a pointer to a newly allocated
//         `ConfigFiles` struct. Caller assumes responsibility of freeing this
//         structure using `free_config_files` regardless of the return value
//         of this call.
// error - An output parameter that includes details about any errors that
//         occurred while processing the arguments. Suitable for printing
//         to the user, along with a usage message. Value is NULL if the call
//         completes successfully or there was an error allocating the string.
//         It is the caller's responsibility to pass the `char *` returned
//         through this parameter to `free`.
// 
// Returns an `ArgsResult` depending upon the status of the call.
ArgsResult process_args(int argc, char *argv[], State **state,
    ConfigFiles **files, char **error);

#endif  // SUPER_GLUE_INCLUDE_PROCESS_ARGS_H_
