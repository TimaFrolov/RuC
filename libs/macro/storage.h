/*
 *	Copyright 2021 Andrey Terekhov, Victor Y. Fadeev, Dmitrii Davladov
 *
 *	Licensed under the Apache License, Version 2.0 (the "License");
 *	you may not use this file except in compliance with the License.
 *	You may obtain a copy of the License at
 *
 *		http://www.apache.org/licenses/LICENSE-2.0
 *
 *	Unless required by applicable law or agreed to in writing, software
 *	distributed under the License is distributed on an "AS IS" BASIS,
 *	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *	See the License for the specific language governing permissions and
 *	limitations under the License.
 */

#pragma once

#include "hash.h"
#include "map.h"
#include "strings.h"
#include "workspace.h"


#ifdef __cplusplus
extern "C" {
#endif

/** Macro storage*/
typedef struct storage
{
	map as;				/**< Map structure */
	hash hs;			/**< Hash table */
	strings vec;		/**< Strings vector */
} storage;


/**
 *	Create macro storage
 *
 *	@return	Macro storage
 */
storage storage_create();


/**
 *	Add new macro
 *
 *	@param	stg			Macro storage
 *	@param	id			Macro name
 *	@param	value		Macro replacement
 *
 *	@return	Index of record, @c SIZE_MAX on failure
 */
size_t storage_add_utf8(storage *const stg, const char32_t *const id, const char32_t *const value);

/**
 *	Add new macro
 *
 *	@param	stg			Macro storage
 *	@param	id			Macro name
 *	@param	value		Macro replacement
 *
 *	@return	Index of record, @c SIZE_MAX on failure
 */
size_t storage_add(storage *const stg, const char32_t *const id, const char *const value);

/**
 *	Add new macro with arguments
 *
 *	@param	stg			Macro storage
 *	@param	id			Macro name
 *	@param	value		Macro replacement
 *	@param	args		Number of arguments
 *
 *	@return	Index of record, @c SIZE_MAX on failure
 */
size_t storage_add_with_args(storage *const stg, const char32_t *const id, const char32_t *const value, const size_t args);


/**
 *	Get index of record
 *
 *	@param	stg			Macro storage
 *	@param	id			Macro name
 *
 *	@return	Index of record or keyword, @c SIZE_MAX on failure
 */
size_t storage_get_index(storage *const stg, const char32_t *const id);

/**
 *	Get macro replacement by index
 *
 *	@param	stg			Macro storage
 *	@param	id			Index of record
 *
 *	@return	Macro replacement, @c NULL on failure
 */
const char *storage_get_by_index(const storage *const stg, const size_t id);

/**
 *	Get macro replacement
 *
 *	@param	stg			Macro storage
 *	@param	id			Macro name
 *
 *	@return	Macro replacement, @c NULL on failure
 */
inline const char *storage_get(storage *const stg, const char32_t *const id)
{
	return storage_get_by_index(stg, storage_get_index(stg, id));
}

/**
 *	Get arguments amount by index
 *
 *	@param	stg			Macro storage
 *	@param	id			Index of record
 *
 *	@return	Arguments amount, @c 0 on failure
 */
size_t storage_get_amount_by_index(const storage *const stg, const size_t id);

/**
 *	Get arguments amount
 *
 *	@param	stg			Macro storage
 *	@param	id			Macro name
 *
 *	@return	Arguments amount, @c 0 on failure
 */
inline size_t storage_get_amount(storage *const stg, const char32_t *const id)
{
	return storage_get_amount_by_index(stg, storage_get_index(stg, id));
}

/**
 *	Get macro argument by index
 *
 *	@param	stg			Macro storage
 *	@param	id			Index of record
 *	@param	index		Argument number
 *
 *	@return	Macro argument, @c NULL on failure
 */
const char *storage_get_arg_by_index(const storage *const stg, const size_t id, const size_t index);

/**
 *	Get macro argument
 *
 *	@param	stg			Macro storage
 *	@param	id			Macro name
 *	@param	index		Argument number
 *
 *	@return	Macro argument, @c NULL on failure
 */
inline const char *storage_get_arg(storage *const stg, const char32_t *const id, const size_t index)
{
	return storage_get_arg_by_index(stg, storage_get_index(stg, id), index);
}


/**
 *	Add macro argument by index
 *
 *	@param	stg			Macro storage
 *	@param	id			Index of record
 *	@param	index		Argument number
 *	@param	arg			Argument name
 *
 *	@return	@c 0 on success, @c -1 on failure
 */
int storage_add_arg_by_index(storage *const stg, const size_t id, const size_t index, const char32_t *const arg);

/**
 *	Add macro argument
 *
 *	@param	stg			Macro storage
 *	@param	id			Macro name
 *	@param	index		Argument number
 *	@param	arg			Argument name
 *
 *	@return	Index of record, @c SIZE_MAX on failure
 */
inline size_t storage_add_arg(storage *const stg, const char32_t *const id, const size_t index, const char32_t *const arg)
{
	return storage_add_arg_by_index(stg, storage_get_index(stg, id), index, arg);
}


/**
 *	Set new macro replacement by index
 *
 *	@param	stg			Macro storage
 *	@param	id			Index of record
 *	@param	value		Macro replacement
 *
 *	@return	Index of new record, @c SIZE_MAX on failure
 */
size_t storage_set_by_index(storage *const stg, const size_t id, const char *value);

/**
 *	Set new macro replacement by existing macro name
 *
 *	@param	stg			Macro storage
 *	@param	id			Macro name
 *	@param	value		Macro replacement
 *
 *	@return	Index of new record, @c SIZE_MAX on failure
 */
inline size_t storage_set(storage *const stg, const char32_t *const id, const char *value)
{
	return storage_set_by_index(stg, storage_get_index(stg, id), value);
}


/**
 *	Remove macro by index
 *
 *	@param	stg			Macro storage
 *	@param	id			Index of record
 *
 *	@return	@c 0 on success, @c -1 on failure
 */
int storage_remove_by_index(storage *const stg, const size_t id);

/**
 *	Remove macro
 *
 *	@param	stg			Macro storage
 *	@param	id			Macro name
 *
 *	@return	@c 0 on success, @c -1 on failure
 */
inline int storage_remove(storage *const stg, const char32_t *const id)
{
	return storage_remove_by_index(stg, storage_get_index(stg, id));
}


/**
 *	Get index of record by reading macro from io
 *
 *	@param	as			Macro storage
 *	@param	io			Universal io structure
 *	@param	last		Next character after macro
 *
 *	@return	Index of record or keyword, @c SIZE_MAX on failure
 */
size_t storage_search(storage *const stg, universal_io *const io, char32_t *const last);

/**
 *	Return the last read macro
 *
 *	@param	as			Macro storage
 *
 *	@return	Macro, @c NULL on failure
 */
const char *storage_last_read(const storage *const stg);

/**
 *	Check that macro storage is correct
 *
 *	@param	stg			Macro storage
 *
 *	@return	@c 1 on true, @c 0 on false
 */
bool storage_is_correct(const storage *const stg);


/**
 *	Free allocated memory
 *
 *	@param	stg			Macro storage
 *
 *	@return	@c 0 on success, @c -1 on failure
 */
int storage_clear(storage *const stg);

#ifdef __cplusplus
} /* extern "C" */
#endif
