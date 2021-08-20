/* Definition of functions to aid in processing command-line arguments.
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

#define _XOPEN_SOURCE 700

#include "process_args.h"

#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "state.h"
#include "util.h"

// Used to aid in processing of parameters that take extra info (e.g.,
// `--port=8080`).
typedef union {
  int32_t numeric;
  char *string;
} OptInfoData;
typedef enum {
  NONE = 0,
  INT,
  STRING,
} OptInfoType;
typedef struct {
  OptInfoType type;
  OptInfoData data;
} OptInfo;

// Keeps track of which args are available and their properties
typedef enum {
  OPT_INTERACTIVE = 0,
  OPT_VERSION,
  OPT_PORT,
} OptId;
// This integer must have at least as many bits as there are possible options.
typedef uint8_t OptsApplied_t;
typedef enum {
  NOT_UNIQ = 0,
  UNIQ,
} OptUniq;
typedef struct {
  OptId id;
  char short_name;
  const char *long_name;
  OptInfoType info_type;
  OptUniq unique;
} Option;

typedef enum {
  OPT_OK = 0,
  OPT_MEM,
  OPT_INVALID_USE,
  OPT_UNKNOWN,
  OPT_AMBIGUOUS,
} ProcessOptionResult;
typedef enum {
  INFO_OK = 0,
  INFO_MEM,
  INFO_RANGE,
  INFO_NOT_INT,
} ParseInfoResult;

static Option valid_options[] = {
    {OPT_INTERACTIVE, 'i', "interactive", NONE, UNIQ},
    {OPT_VERSION, 'v', "version", NONE, UNIQ},
    {OPT_PORT, 'p', "port", INT, UNIQ},
};

// Processes a single option (where an option is of the form "-oinfo" [note that
// this form is only valid if -o takes extra info, otherwise it is parsed as "-o
// -i -n -f -o"] "-o info" [again, only valid if -o takes extra info, otherwise
// "info" is treated as the end of options/as a file], "-o=info",
// "--option=info", or "--option info"). The long form of the option supports
// abbreviation of the option name when unambiguous. Caller assumes ownership of
// the `error` string, which must be passed to `free`.
//
// option_strs_len - The length of the `option strs` array.
// option_strs     - All of the options that were given on the command line.
//                   While this function processes these options one at a time,
//                   passing them all in at once simplifies dealing with options
//                   like "-o info" where splitting on spaces would case an
//                   issue.
// idx             - The index into `options` to begin processing from.
// error           - An output parameter. If there is an error in processing the
//                   given `option_str`, this parameter will contain a string
//                   describing the error suitable for presentation to the
//                   end-user. This string must be free'd be the caller. Set to
//                   NULL on success or if there's an error allocating memory
//                   for the string.
// next_idx        - An output parameter that represents the next item in
//                   `options` that need to be processed (e.g., "-o" would have
//                   this as `idx + 1` whereas "-o info" would have it as
//                   `idx + 2`). Set to -1 on failure.
// options_count   - An output parameter containing the length of `options`. Set
//                   to -1 on failure.
// options         - An output parameter that is set to an `Option **`, pointing
//                   at an array of `Option *` with length `options_count`. This
//                   array should be passed to `free` after the successful
//                   completion of this function. Set to NULL on failure.
// info            - An output parameter containing a pointer to an allocated
//                   `OptInfo` struct. If no extra info was provided, this
//                   parameter is set to NULL. Note that this is never an array
//                   because it's an error to specify multiple options *and*
//                   extra info. If any info is provided, `options_count` must
//                   be 1. This pointer must be passed to `free_opt_info` by the
//                   caller.
//
// Since there's a lot that can go wrong while processing these options (as
// they're user supplied), the return type reflects a number of different kinds
// of failure. `OPT_OK` signals successful completion, but it should be noted
// that `process_option` will process options independently of each other, even
// within a single call (e.g., "-vvvvv" will return `OPT_OK` even though
// `OPT_VERSION` has `unique` of `UNIQ`). The various failure results will
// occur under the following circumstances:
// - OPT_MEM: Unable to allocate the `options` array.
// - OPT_INVALID_USE: User error; the user gave info when disallowed (e.g.,
//   "--version=3") or ommitted/gave the wrong type when necessary.
// - OPT_UNKNOWN: User error; the user gave an unknown option.
// - OPT_AMBIGUOUS: User error; the user shortened a long-form option in an
//   ambiguous way.
static ProcessOptionResult process_option(int option_strs_len,
    char *option_strs[], int idx, char **error, int *next_idx,
    int *options_count, Option ***options, OptInfo **info);

// Performs a fuzzy lookup of `option_name` in `valid_options`. This is designed
// to facilitate the behavior that GNU long options provide, where long options
// can be shortened provided that they're distinct (e.g., "--forma=verbose" is
// equaivalent to "--format=verbose" assuming no other options start with the
// string "forma").
//
// option_str  - The string provided by the user.
// options_len - An output parameter, set to the length of the `options` array.
//               Set to `-1` on error.
// options     - An output parameter, contains all of the `Option *`s in
//               `valid_options` whose `long_name` beings with `option_str`. The
//               caller assumes ownership of this array, and must free it using
//               `free`. Set to `NULL` on error.
//
// Returns true if `option_str` has any matches (i.e., `*options_len` > 0) and
// false on error or if no matches are found.
static bool fuzzy_get_opt_by_long_name(const char *option_str, int *options_len,
    Option ***options);

// Returns a pointer to a statically allocated `Option` struct that has a
// `short_name` that matches the one given to this call. Returns NULL if no
// matches are found.
static Option *get_opt_by_short_name(char short_name);

// Returns a pointer to the option with a given id. Returns `NULL` if none found
// (but this shouldn't happen except for in the case of programmer error).
static Option *get_opt_by_id(OptId id);

// Parses a string into a dynamically allocated `OptInfo` struct of type `type`.
//
// info_str - The string passed by the user as extra info (e.g., "123" in
//            "-o123", "abc" in "--option=abc", etc).
// type     - The type that `info_str` should be parsed as. It's possible to
//            parse integers as strings, but not the other way around.
// info     - An output parameter set to a `malloc` allocated pointer to an 
//            `OptInfo` struct. Set to `NULL` on error.
//
// Returns a `ParseInfoResult` based on the status of the call.
// - `INFO_OK`      - the call was successful.
// - `INFO_RANGE`   - when attempting to parse `info_str` as an integer, the
//                    value given was out of range of an `int32_t`.
// - `INFO_NOT_INT` - attemted to parse `info_str` as an integer, but the string
//                    contained non-numeric characters.
static ParseInfoResult parse_info_str(const char *info_str, OptInfoType type,
    OptInfo **info);

// Given a string like "abcinfo" where 'c' is the short name of an option that
// takes extra info, this function will return `3`, representing options 'a',
// 'b', and 'c'.
//
// On success, returns the number of options as described above with *error
// set to NULL.
//
// One error, returns a negative value and sets *error to a string indicating
// the error. This new string is dynamically allocated; it is the caller's
// responsibility to pass it to `free`.
static int count_combined_options(const char *opt_str, char **error);

// Frees an `OptInfo` struct allocated by `process_option`
//
// ptr - A pointer to the struct to free.
static void free_opt_info(OptInfo *ptr);

ArgsResult process_args(int argc, char *argv[], State **state,
    ConfigFiles **files, char **error) {
  // Initialize `*files`  and `*state` to NULL. This way if the call returns
  // early, the caller can still pass them to their free functions.
  *state = NULL;
  *files = NULL;

  // A bit field that keeps track of which options have been applied. The bit
  // that corresponds to each option is described by the `OptId` enum. The
  // purpose of keeping track of this data is preventing options that have
  // `unique` set to `UNIQ` from being set multiple times.
  OptsApplied_t applied_options = 0;

  if (argc == 1) {
    *error = strdup("No paramters given");
    return ARGS_NONE;
  }

  // Initialize a global state and assign it to the output parameter.
  if (!alloc_state(state)) {
    *error = strdup(strerror(ENOMEM));
    return ARGS_MEM;
  }

  int current_arg = 1;
  // Loop over all args between argv[1] and the first arg that doesn't start
  // with a '-'. Breaks on '--'.
  while(current_arg < argc && *(argv[current_arg]) == '-') {
    // If we're just given '-', we're actually done with the argument and
    // need to treat that as a file (since the user wants us to read from
    // STDIN).
    if (*(argv[current_arg] + 1) == '\0') {
      break;
    }

    // If we're just given '--', then we need to break out of the loop *while
    // incrementing `current_arg`, since the user wants us to be finished
    // parsing command line options, but treating '--' as a file would be
    // nonsense.
    if (strcmp(argv[current_arg], "--") == 0) {
      current_arg++;
      break;
    }

    // This is our standard case; we just try and process argv as an option
    int options_count;
    Option **options = NULL;
    OptInfo *info = NULL;
    ProcessOptionResult res =
        process_option(argc, argv, current_arg, error, &current_arg,
            &options_count, &options, &info);

    if (res != OPT_OK) {
      // None of the cases set `*error` because it should've been set in the
      // call to `process_option`.
      free(options);
      free_opt_info(info);
      switch (res) {
        // Silence -Wswitch, if this branch has taken there's been some serious
        // memory corruption.
        case OPT_OK:
          abort();
        case OPT_MEM:
          return ARGS_MEM;
        case OPT_UNKNOWN:
          return ARGS_UNKNOWN;
        case OPT_INVALID_USE:
          return ARGS_INVALID_USE;
        case OPT_AMBIGUOUS:
          return ARGS_AMBIGUOUS;
      }
    }

    free(*error);

    for (int i = 0; i < options_count; i++) {
      // Use `applied_options` to keep track of which options we've already
      // seen.
      Option *current_option = options[i];
      OptsApplied_t shifted_val = 1 << current_option->id;
      if (current_option->unique && applied_options & shifted_val) {
        alloc_sprintf(error, "Option --%s can only be applied once.",
            current_option->long_name);
        free(options);
        return ARGS_CONFLICT;
      }
      applied_options |= shifted_val;

      // Actually deal with each option, updating the `*state` variable
      // accordingly.
      switch (current_option->id) {
        case OPT_VERSION:
          (*state)->version_info_requested = true;
          break;
        case OPT_INTERACTIVE:
          (*state)->interactive = true;
          break;
        case OPT_PORT:
          if (info->data.numeric < 0 || info->data.numeric > UINT16_MAX) {
            alloc_sprintf(error, "--%s can only take values between 0 and %d",
                current_option->long_name, UINT16_MAX);
            free_opt_info(info);
            free(options);
            return ARGS_INVALID_USE;
          }
          (*state)->port = htons(info->data.numeric);
          break;
      }
    }

    // Handle the special case where `--version` cannot be applied with other
    // options.
    OptsApplied_t shifted_version = 1 << OPT_VERSION;
    if (applied_options & shifted_version &&
        applied_options != shifted_version) {
      alloc_sprintf(error, "--%s cannot be combined with other options.",
          get_opt_by_id(OPT_VERSION)->long_name);
      free(options);
      return ARGS_CONFLICT;
    }

    free_opt_info(info);
    free(options);
  }

  if (current_arg == argc) {
    // All tokens provieded on the command line were options, no files.
    return ARGS_NO_FILES;
  } else if ((*state)->version_info_requested) {
    // Checks if --version was supplied with files. Since this makes no sense,
    // report an error.
    return ARGS_INVALID_USE;
  }

  if (!alloc_config_files(argc - current_arg, &(argv[current_arg]), files,
        error)) {
    return ARGS_FILE_ERR;
  }

  return ARGS_OK;
}

#define SET_OUTPUT_PARAMS_ERROR \
  do { \
    *next_idx = -1; \
    *options_count = -1; \
    *options = NULL; \
    *info = NULL; \
  } while (0);
static ProcessOptionResult process_option(int option_strs_len,
    char *option_strs[], int idx, char **error, int *next_idx,
    int *options_count, Option ***options, OptInfo **info) {
  const char *current_option = option_strs[idx];
  const char *info_str;
  Option *last_option;
  *error = NULL;

  // Check if this is a long-form option or a short-form option
  if (*(current_option + 1) == '-') {
    // This is a long-form option, so we don't have to worry about having
    // multiple options crammed into one.

    // `option_name` is a copy of the identifying part of the option (e.g.,
    // the "option" in "--option=info"). `option_name` must be freed, as the
    // contents are allocated with strdup/strndup.
    char *option_name = NULL;

    char *equals_location = strchr(current_option + 2, '=');
    if (equals_location == NULL) {
      option_name = strdup(current_option + 2);
    } else {
      option_name =
          strndup(current_option + 2, equals_location - (current_option + 2));
    }

    // Check for errors from both `strdup` and `strndup`.
    if (option_name == NULL) {
      *error = strdup(strerror(ENOMEM));
      SET_OUTPUT_PARAMS_ERROR;
      return OPT_MEM;
    }

    // Lower-case the option name
    for (char *s = option_name; *s; s++) *s = tolower(*s);

    // Attempt to find the Option identified by `option_name`, doing any
    // necessary error-handling
    Option **matches;
    int matches_len;
    bool opt_found =
        fuzzy_get_opt_by_long_name(option_name, &matches_len, &matches);
    if (!opt_found) {
      SET_OUTPUT_PARAMS_ERROR;
      alloc_sprintf(error, "Unknown option: --%s", option_name);
      free(option_name);
      free(matches);
      return OPT_UNKNOWN;
    } else if (matches_len > 1) {
      SET_OUTPUT_PARAMS_ERROR;
      const char msg_start[] = "Ambiguous option: --%s; possibilities: ";
      const char fmt_base[] = "--%s";
      const char fmt_additional[] = ", --%s";

      // They need the `-1` to account for the '\0' that's removed when the
      // strings are concatenated, and a `-2` to account for the "%s"
      size_t error_buf_size = sizeof(msg_start) - 1 + strlen(option_name) - 2;
      // Include the '\0' here because we need to store that when we
      // `snprintf` just this possibility later on
      size_t max_possibility_size =
          sizeof(fmt_base) - 2 + strlen(matches[0]->long_name);
      // But don't include it here because the overall string won't contain a
      // '\0' for each option.
      error_buf_size += max_possibility_size - 1;
      for (int i = 1; i < matches_len; i++) {
        size_t current_possibility_size =
            sizeof(fmt_additional) - 2 + strlen(matches[i]->long_name);
        error_buf_size += current_possibility_size - 1;
        if (current_possibility_size > max_possibility_size) {
          max_possibility_size = current_possibility_size;
        }
      }

      // Add one extra byte for the '\0', since none of our strings earlier
      // included it.
      error_buf_size += 1;

      // Allocate a buffer big enough to hold the whole string.
      *error = malloc(error_buf_size);
      if (*error == NULL) {
        free(option_name);
        free(matches);
        return OPT_AMBIGUOUS;
      }
      // Set the first char to be '\0' so that it's treated as starting as a
      // zero length string.
      **error = '\0';

      // For each of the matches, generate the proper string using `fmt_base` or
      // `fmt_additional` and `snprintf`, then append it onto our buffer with
      // `strncat`.
      char *tmp_buf = malloc(max_possibility_size);
      if (tmp_buf == NULL) {
        free(option_name);
        free(matches);
        return OPT_AMBIGUOUS;
      }
      snprintf(tmp_buf, max_possibility_size, fmt_base, matches[0]->long_name);
      strncat(*error, tmp_buf, error_buf_size);
      for (int i = 1; i < matches_len; i++) {
        snprintf(tmp_buf, max_possibility_size, fmt_additional,
            matches[i]->long_name);
        strncat(*error, tmp_buf, error_buf_size);
      }

      free(tmp_buf);
      free(option_name);
      free(matches);
      return OPT_AMBIGUOUS;
    }
    free(option_name);

    // We unambiguously know which option the user intended, so we can set the
    // output parameters accordingly. `next_idx` can't be set yet because we
    // might have to consume the next element of `option_strs` as option info.
    *options_count = 1;
    *options = malloc(sizeof(Option *) * *options_count);
    (*options)[0] = matches[0];
    last_option = (*options)[0];
    free(matches);

    if (*options == NULL) {
      SET_OUTPUT_PARAMS_ERROR;
      *error = strdup(strerror(ENOMEM));
      return OPT_MEM;
    }

    // If there's an equals sign, we know for sure that the user is intending to
    // pass extra info
    if (equals_location != NULL) {
      // If this argument doesn't take extra info, then that's an error
      if (last_option->info_type == NONE) {
        alloc_sprintf(error, "Option --%s does not take an option-argument.",
            last_option->long_name);
        free(*options);
        SET_OUTPUT_PARAMS_ERROR;
        return OPT_INVALID_USE;
      }

      *next_idx = idx + 1;
      info_str = equals_location + 1;
    } else if (last_option->info_type != NONE) {
      // If this option does take extra info, then parse the next option as
      // an info, regardless of its actual contents.
      if (idx + 1 == option_strs_len) {
        alloc_sprintf(error, "No option-argument provided to --%s, which "
            "requires one.", last_option->long_name);
        free(*options);
        SET_OUTPUT_PARAMS_ERROR;
        return OPT_INVALID_USE;
      }

      *next_idx = idx + 2;
      info_str = option_strs[idx + 1];
    } else {
      // No equals sign, not expecting info, so just consume `idx` and move on
      // by returning, no need to try and parse `info_str`
      *next_idx = idx + 1;
      *info = NULL;
      return OPT_OK;
    }
  } else {
    // Set up all of our convenience variables, as well as some of our output
    // parameters
    *options_count = count_combined_options(current_option + 1, error);
    if (*options_count < 0) {
      SET_OUTPUT_PARAMS_ERROR;
      return OPT_UNKNOWN;
    }
    char *opt_str = malloc((*options_count + 1) * sizeof(char));
    strncpy(opt_str, current_option + 1, *options_count);
    opt_str[*options_count] = '\0';

    *options = malloc(sizeof(Option *) * (*options_count));

    for (int i = 0; i < *options_count; i++) {
      (*options)[i] = get_opt_by_short_name(opt_str[i]);
    }
    free(opt_str);

    // Check if the last option is one that's supposed to take extra info. If
    // not, then we're done! Otherwise, parse the extra info into an output
    // parameter. Check for an `=` to check for user error (e.g., "-o=abc" when
    // option "o" doesn't take extra info)
    last_option = (*options)[*options_count - 1];
    if (last_option->info_type == NONE) {
      if (strchr(current_option, '=') != NULL) {
        alloc_sprintf(error, "Option -%c does not require an option-argument",
            last_option->short_name);
        free(*options);
        SET_OUTPUT_PARAMS_ERROR;
        return OPT_INVALID_USE;
      }
      *info = NULL;
      *next_idx = idx + 1;
      return OPT_OK;
    } else {
      // Check if the last character of `current_option` is a terminator. This
      // acts as a check for if we're dealing with "-o info" or "-oinfo".
      if (current_option[*options_count + 1] == '\0') {
        // We need to consume the next value in `option_strs` after `idx` as our
        // info for this option.
        if (idx + 1 == option_strs_len) {
          // There isn't anything else in `option_strs`, which indcates the user
          // did not provide info to an option that needed it.
          alloc_sprintf(error, "Option -%c requires an option-argument",
              (*options)[*options_count - 1]->short_name);
          free(*options);
          SET_OUTPUT_PARAMS_ERROR;
          return OPT_INVALID_USE;
        }
        // We're consuming the next entry in `option_strs`, so we want to skip
        // over the `80` in `-p 80` when we move on to the next option
        *next_idx = idx + 2;
        info_str = option_strs[idx + 1];
      } else {
        // `info_str` should just be a pointer to the first non-option character
        // in `current_option`.
        info_str = current_option + *options_count + 1;
        if (*info_str == '=') info_str++;  // Skip an equals sign if its given.
        // Our info string is in `option_strs[idx]` so we want the next option
        // processed to be `option_strs[idx + 1]`
        *next_idx = idx + 1;
      }
    }
  }

  ParseInfoResult res = parse_info_str(info_str, last_option->info_type,
      info);
  if (res != INFO_OK) {
    free(*options);
    free(*info);
    SET_OUTPUT_PARAMS_ERROR;
    if (res == INFO_MEM) {
      *error = strdup(strerror(ENOMEM));
      return OPT_MEM;
    }
    if (res == INFO_RANGE) {
      alloc_sprintf(error, "Option-argument \"%s\" given to --%s is out of "
          "range.", info_str, last_option->long_name);
      return OPT_INVALID_USE;
    }
    if (res == INFO_NOT_INT) {
      alloc_sprintf(error, "Option-argument \"%s\" given to --%s cannot be "
          "parsed as an integer.", info_str, last_option->long_name);
      return OPT_INVALID_USE;
    }
  }
  return OPT_OK;
}

#define NUM_VALID_OPTIONS (int)(sizeof(valid_options) / sizeof(*valid_options))
static bool fuzzy_get_opt_by_long_name(const char *option_name,
    int *options_len, Option ***options) {
  // `*options` is large enough to hold pointers to all entries in the
  // `valid_options` array.
  *options = malloc(NUM_VALID_OPTIONS * sizeof(Option *));
  if (*options == NULL) {
    *options_len = -1;
    return false;
  }

  *options_len = 0;
  for (int i = 0; i < NUM_VALID_OPTIONS; i++) {
    const char *curr_opt_name = valid_options[i].long_name;
    // Check if `curr_opt_name` starts with `option_name`.
    if (strstr(curr_opt_name, option_name) == curr_opt_name) {
      // Check for an exact match
      if (strlen(curr_opt_name) == strlen(option_name)) {
        (*options)[0] = &(valid_options[i]);
        *options_len = 1;
        return true;
      } else {
        (*options)[*options_len] = &(valid_options[i]);
        (*options_len)++;
      }
    }
  }

  return *options_len > 0;
}

static Option *get_opt_by_short_name(char short_name) {
  for (int i = 0; i < NUM_VALID_OPTIONS; i++) {
    if (valid_options[i].short_name == short_name) {
      return &(valid_options[i]);
    }
  }
  return NULL;
}

static Option *get_opt_by_id(OptId id) {
  for (int i = 0; i < NUM_VALID_OPTIONS; i++) {
    if (valid_options[i].id == id) {
      return &(valid_options[i]);
    }
  }
  return NULL;
}

static ParseInfoResult parse_info_str(const char *info_str, OptInfoType type,
    OptInfo **info) {
  *info = malloc(sizeof(OptInfo));
  if (*info == NULL) {
    return INFO_MEM;
  }

  (*info)->type = type;

  switch (type) {
    // Silence -Wswitch, if this `case` is ever taken, that indicates a design
    // error.
    case NONE:
      abort();
    case INT:
      // Since `OptInfoData` stores an `int32_t` for compliance with POSIX
      // guidelines, the only way to read the string properly is with
      // `sscanf`
      (*info)->data.numeric = 0;
      int sscanf_result = sscanf(info_str, "%"SCNd32, &((*info)->data.numeric));
      if (sscanf_result == EOF || sscanf_result == 0) {
        if (sscanf_result == EOF && errno == ERANGE) {
          return INFO_RANGE;
        } else {
          return INFO_NOT_INT;
        }
      }
      // This acts as a check for garbage characters at the end of the
      // info string that get ignored by `sscanf`.
      if ((size_t)snprintf(NULL, 0, "%"PRId32, (*info)->data.numeric) !=
          strlen(info_str)) {
        return INFO_NOT_INT;
      }
      return INFO_OK;
    case STRING:
      (*info)->data.string = strdup(info_str);
      if ((*info)->data.string == NULL) {
        return INFO_MEM;
      }
      return INFO_OK;
  }
  // This should be unreachable (assuming a valid value for `type`), but it
  // causes GCC to complain. The only way we can reach this is programmer error
  // or memory corruption.
  abort();
}

static int count_combined_options(const char *opt_str, char **error) {
  int index = 0;
  while (opt_str[index] != '\0' && opt_str[index] != '=') {
    Option *current_opt = get_opt_by_short_name(opt_str[index]);
    if (current_opt == NULL) {
      alloc_sprintf(error, "Unknown short option -%c", opt_str[index]);
      return -1;
    }
    if (current_opt->info_type != NONE) return index + 1;
    index++;
  }
  return index;
}

static void free_opt_info(OptInfo *ptr) {
  if (ptr == NULL) return;
  if (ptr->type == STRING) {
    free(ptr->data.string);
  }
  free(ptr);
}

