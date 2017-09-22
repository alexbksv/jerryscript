/* Copyright JS Foundation and other contributors, http://js.foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef JERRYX_MODULE_H
#define JERRYX_MODULE_H

#include "jerryscript.h"

/**
 * Declare the signature for the module initialization function.
 */
typedef jerry_value_t (*jerryx_native_module_on_resolve_t) (void);

/**
 * Declare the structure used to define a module. One should only make use of this structure via the
 * JERRYX_NATIVE_MODULE macro declared below.
 */
typedef struct jerryx_native_module_t
{
  const jerry_char_t *name_p; /**< name of the module */
  const jerryx_native_module_on_resolve_t on_resolve; /**< function that returns a new instance of the module */
  struct jerryx_native_module_t *next_p; /**< pointer to next module in the list */
} jerryx_native_module_t;

/**
 * Declare the constructor and destructor attributes. These evaluate to nothing if this extension is built without
 * library constructor/destructor support.
 */
#ifdef ENABLE_INIT_FINI
#define JERRYX_MODULE_CONSTRUCTOR_ATTRIBUTE __attribute__((constructor))
#define JERRYX_MODULE_DESTRUCTOR_ATTRIBUTE __attribute__((destructor))
#define JERRYX_MODULE_REGISTRATION_QUALIFIER static
#else /* !ENABLE_INIT_FINI */
#define JERRYX_MODULE_CONSTRUCTOR_ATTRIBUTE
#define JERRYX_MODULE_DESTRUCTOR_ATTRIBUTE
#define JERRYX_MODULE_REGISTRATION_QUALIFIER
#endif /* ENABLE_INIT_FINI */

/**
 * Having two levels of macros allows strings to be used unquoted.
 */
#define JERRYX_NATIVE_MODULE(module_name, on_resolve_cb)  \
  JERRYX_NATIVE_MODULE_IMPLEM(module_name, on_resolve_cb)

#define JERRYX_NATIVE_MODULE_IMPLEM(module_name, on_resolve_cb)        \
  static jerryx_native_module_t _ ## module_name ## _definition =      \
  {                                                                    \
    .name_p = (jerry_char_t *) #module_name,                           \
    .on_resolve = (on_resolve_cb),                                     \
    .next_p = NULL                                                     \
  };                                                                   \
                                                                       \
  JERRYX_MODULE_REGISTRATION_QUALIFIER void                            \
  module_name ## _register (void) JERRYX_MODULE_CONSTRUCTOR_ATTRIBUTE; \
  JERRYX_MODULE_REGISTRATION_QUALIFIER void                            \
  module_name ## _register (void)                                      \
  {                                                                    \
    jerryx_native_module_register(&_##module_name##_definition);       \
  }                                                                    \
                                                                       \
  JERRYX_MODULE_REGISTRATION_QUALIFIER void                            \
  module_name ## _unregister (void)                                    \
  JERRYX_MODULE_DESTRUCTOR_ATTRIBUTE;                                  \
  JERRYX_MODULE_REGISTRATION_QUALIFIER void                            \
  module_name ## _unregister (void)                                    \
  {                                                                    \
    jerryx_native_module_unregister(&_##module_name##_definition);     \
  }

/**
 * Register a native module. This makes it available for loading via jerryx_module_resolve, when
 * jerryx_module_native_resolver is passed in as a possible resolver.
 */
void jerryx_native_module_register (jerryx_native_module_t *module_p);

/**
 * Unregister a native module. This removes the module from the list of available native modules, meaning that
 * subsequent calls to jerryx_module_resolve with jerryx_module_native_resolver will not be able to find it.
 */
void jerryx_native_module_unregister (jerryx_native_module_t *module_p);

/**
 * Declare the JerryScript module resolver so that it may be added to an array of jerryx_module_resolver_t items and
 * thus passed to jerryx_module_resolve.
 */
bool jerryx_module_native_resolver (const jerry_char_t *name, jerry_value_t *result);

/**
 * Declare the function pointer type for module resolvers.
 */
typedef bool (*jerryx_module_resolver_t) (const jerry_char_t *name, jerry_value_t *result);

/**
 * Load a copy of a module into the current context using the provided module resolvers, or return one that was already
 * loaded if it is found.
 */
jerry_value_t jerryx_module_resolve (const jerry_char_t *name, const jerryx_module_resolver_t *resolvers, size_t count);

#endif /* !JERRYX_MODULE_H */