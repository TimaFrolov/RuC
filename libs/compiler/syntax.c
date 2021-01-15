/*
 *	Copyright 2020 Andrey Terekhov, Victor Y. Fadeev
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

#include "syntax.h"
#include <stdlib.h>
#include "errors.h"


int getstatic(syntax *const sx, const int type)
{
	const int olddispl = sx->displ;
	sx->displ += sx->lg * size_of(sx, type);
	if (sx->lg > 0)
	{
		sx->maxdispl = (sx->displ > sx->maxdispl) ? sx->displ : sx->maxdispl;
	}
	else
	{
		sx->maxdisplg = -sx->displ;
	}
	return olddispl;
}

/**	Check if modes are equal */
int mode_is_equal(const syntax *const sx, const size_t first, const size_t second)
{
	if (sx->modetab[first] != sx->modetab[second])
	{
		return 0;
	}

	size_t length = 1;
	const int mode = sx->modetab[first];
	// Определяем, сколько полей надо сравнивать для различных типов записей
	if (mode == MSTRUCT || mode == MFUNCTION)
	{
		length = 2 + sx->modetab[first + 2];
	}

	for (size_t i = 1; i <= length; i++)
	{
		if (sx->modetab[first + i] != sx->modetab[second + i])
		{
			return 0;
		}
	}

	return 1;
}

/**	Check if representations are equal */
int repr_is_equal(const syntax *const sx, const size_t first, const size_t second)
{
	size_t i = 2;
	while (sx->reprtab[first + i] == sx->reprtab[second + i])
	{
		i++;
		if (sx->reprtab[first + i] == 0 && sx->reprtab[second + i] == 0)
		{
			return 1;
		}
	}
	return 0;
}


/*
 *	 __     __   __     ______   ______     ______     ______   ______     ______     ______
 *	/\ \   /\ "-.\ \   /\__  _\ /\  ___\   /\  == \   /\  ___\ /\  __ \   /\  ___\   /\  ___\
 *	\ \ \  \ \ \-.  \  \/_/\ \/ \ \  __\   \ \  __<   \ \  __\ \ \  __ \  \ \ \____  \ \  __\
 *	 \ \_\  \ \_\\"\_\    \ \_\  \ \_____\  \ \_\ \_\  \ \_\    \ \_\ \_\  \ \_____\  \ \_____\
 *	  \/_/   \/_/ \/_/     \/_/   \/_____/   \/_/ /_/   \/_/     \/_/\/_/   \/_____/   \/_____/
 */


int sx_init(syntax *const sx)
{
	if (sx == NULL)
	{
		return -1;
	}

	sx->pc = 4;
	sx->procd = 1;
	sx->funcnum = 2;
	sx->id = 2;
	sx->md = 1;
	sx->startmode = 1;
	sx->tc = 0;
	sx->rp = 1;

	sx->maxdisplg = 3;
	sx->main_ref = 0;
	
	sx->maxdispl = 3;
	sx->displ = -3;
	sx->curid = 2;
	sx->lg = -1;
	
	sx->prdf = -1;
	
	for (size_t i = 0; i < 256; i++)
	{
		sx->hashtab[i] = 0;
	}

	return 0;
}

int sx_check(syntax *const sx, universal_io *const io)
{
	int error_flag = 0;
	
	if (sx->main_ref == 0)
	{
		error(io, no_main_in_program);
		error_flag = -1;
	}
	
	for (int i = 0; i <= sx->prdf; i++)
	{
		if (sx->predef[i])
		{
			error(io, predef_but_notdef, sx->reprtab, sx->predef[i]);
			error_flag = -1;
		}
	}
	return error_flag;
}


int mem_increase(syntax *const sx, const size_t value)
{
	if (sx == NULL)
	{
		return -1;
	}

	sx->pc += (int)value;
	return 0;
}

int mem_add(syntax *const sx, const int value)
{
	if (sx == NULL)
	{
		return -1;
	}

	sx->pc++;
	return mem_set(sx, sx->pc - 1, value);
}

int mem_set(syntax *const sx, const size_t index, const int value)
{
	if (sx == NULL || (int)index >= sx->pc)
	{
		return -1;
	}

	sx->mem[index] = value;
	return 0;
}

int mem_get(const syntax *const sx, const size_t index)
{
	if (sx == NULL || (int)index >= sx->pc)
	{
		return INT_MAX;
	}

	return sx->mem[index];
}

size_t mem_get_size(const syntax *const sx)
{
	if (sx == NULL)
	{
		return INT_MAX;
	}
	return sx->pc;
}


int proc_set(syntax *const sx, const size_t index, const int value)
{
	if (sx == NULL || (int)index >= sx->procd)
	{
		return -1;
	}

	sx->iniprocs[index] = value;
	return 0;
}

int proc_get(const syntax *const sx, const size_t index)
{
	if (sx == NULL || (int)index >= sx->procd)
	{
		return INT_MAX;
	}

	return sx->iniprocs[index];
}


int func_add(syntax *const sx, const size_t ref)
{
	if (sx == NULL)
	{
		return -1;
	}

	sx->funcnum++;
	return func_set(sx, sx->funcnum - 1, ref);
}

int func_set(syntax *const sx, const size_t index, const size_t ref)
{
	if (sx == NULL || index >= sx->funcnum)
	{
		return -1;
	}

	sx->functions[index] = ref;
	return 0;
}

size_t func_get(const syntax *const sx, const size_t index)
{
	if (sx == NULL || index >= sx->funcnum)
	{
		return SIZE_MAX;
	}

	return sx->functions[index];
}


size_t ident_add(syntax *const sx, const size_t repr, const int type, const int mode, const int func_def)
{
	const size_t lastid = sx->id;
	sx->id += 4;
	if (repr_get_reference(sx, repr) == 0) // это может быть только MAIN
	{
		if (sx->main_ref)
		{
			return SIZE_MAX;
		}
		sx->main_ref = lastid;
	}
	// Ссылка на описание с таким же представлением в предыдущем блоке
	const size_t pred = sx->identab[lastid] = sx->reprtab[repr + 1];
	if (pred)
	{
		// pred == 0 только для main, эту ссылку портить нельзя
		// ссылка на текущее описание с этим представлением
		// (это в reprtab)
		repr_set_reference(sx, repr, lastid);
	}
	
	if (type != 1 && pred >= sx->curid) // один  и тот же идент м.б. переменной и меткой
	{
		if (func_def == 3 ? 1 : sx->identab[pred + 1] > 0 ? 1 : func_def == 1 ? 0 : 1)
		{
			return SIZE_MAX - 1;	// 1
									// только определение функции может иметь 2
									// описания, т.е. иметь предописание
		}
	}
	
	sx->identab[lastid + 1] = (int)repr; // ссылка на представление
	ident_set_mode(sx, lastid, mode);
	if (type == 1)
	{
		ident_set_mode(sx, lastid, 0);	// 0, если первым встретился goto, когда встретим метку,
										// поставим 1
		ident_set_displ(sx, lastid, 0);	// при генерации кода когда встретим метку, поставим pc
	}
	else if (type >= 1000)
	{
		// Это описание типа, тогда type - 1000 – это номер инициирующей процедуры
		ident_set_displ(sx, lastid, type);
	}
	else if (type)
	{
		if (type < 0)
		{
			ident_set_displ(sx, lastid, -(sx->displ++));
			sx->maxdispl = sx->displ;
		}
		else // identtab[sx->id+3] - номер функции, если < 0, то
			 // это функция-параметр
		{
			ident_set_displ(sx, lastid, type);
			if (func_def == 2)
			{
				sx->identab[lastid + 1] *= -1; //это предописание
				sx->predef[++sx->prdf] = repr;
			}
			else
			{
				int i;
				
				for (i = 0; i <= sx->prdf; i++)
				{
					if (sx->predef[i] == repr)
					{
						sx->predef[i] = 0;
					}
				}
			}
		}
	}
	else
	{
		ident_set_displ(sx, lastid, getstatic(sx, mode));
	}
	return lastid;
}

int ident_get_mode(const syntax *const sx, const size_t index)
{
	if (sx == NULL || index >= sx->id)
	{
		return INT_MAX;
	}
	
	return sx->identab[index + 2];
}

int ident_set_mode(syntax *const sx, const size_t index, const int mode)
{
	if (sx == NULL || index >= sx->id)
	{
		return -1;
	}
	
	sx->identab[index + 2] = mode;
	return 0;
}

int ident_get_displ(const syntax *const sx, const size_t index)
{
	if (sx == NULL || index >= sx->id)
	{
		return INT_MAX;
	}
	
	return sx->identab[index + 3];
}

int ident_set_displ(syntax *const sx, const size_t index, const int displ)
{
	if (sx == NULL || index >= sx->id)
	{
		return -1;
	}
	
	sx->identab[index + 3] = displ;
	return 0;
}


int size_of(syntax *const sx, const int mode)
{
	return mode == LFLOAT ? 2 : (mode > 0 && mode_get(sx, mode) == MSTRUCT) ? mode_get(sx, mode + 1) : 1;
}

size_t mode_add(syntax *const sx, const int *const record, const size_t size)
{
	if (sx == NULL || record == NULL)
	{
		return SIZE_MAX;
	}

	sx->modetab[sx->md] = (int) sx->startmode;
	sx->startmode = sx->md++;
	for (size_t i = 0; i < size; i++)
	{
		sx->modetab[sx->md++] = record[i];
	}

	// Checking mode duplicates
	size_t old = sx->modetab[sx->startmode];
	while (old)
	{
		if (mode_is_equal(sx, sx->startmode + 1, old + 1))
		{
			sx->md = sx->startmode;
			sx->startmode = sx->modetab[sx->startmode];
			return old + 1;
		}
		else
		{
			old = sx->modetab[old];
		}
	}

	return sx->startmode + 1;
}

int mode_get(const syntax *const sx, const size_t index)
{
	if (sx == NULL || index >= sx->md)
	{
		return INT_MAX;
	}

	return sx->modetab[index];
}


size_t repr_add(syntax *const sx, const char32_t *const spelling)
{
	const size_t old_repr = sx->rp;
	uint8_t hash = 0;
	size_t i = 0;
	sx->rp += 2;

	do
	{
		 hash += (spelling[i] & 255);
		 sx->reprtab[sx->rp++] = spelling[i];
	} while (spelling[i++] != 0);

	sx->hash = (int)hash;

	size_t cur_repr = sx->hashtab[hash];
	if (cur_repr != 0)
	{
		do
		{
			if (repr_is_equal(sx, cur_repr, old_repr))
			{
				sx->rp = old_repr;
				return cur_repr;
			}
			else
			{
				cur_repr = sx->reprtab[cur_repr];
			}
		} while (cur_repr != 0);
	}

	sx->reprtab[old_repr] = (int)sx->hashtab[hash];
	sx->hashtab[hash] = old_repr;
	// 0 - только MAIN, (< 0) - ключевые слова, 1 - обычные иденты
	sx->reprtab[old_repr + 1] = (sx->keywordsnum) ? -((++sx->keywordsnum - 2) / 4) : 1;
	return old_repr;
}

int repr_get_spelling(const syntax *const sx, const size_t index, char32_t *const spelling)
{
	if (sx == NULL || index >= sx->rp)
	{
		return -1;
	}

	size_t i = 2;
	do
	{
		spelling[i] = sx->reprtab[index + i];
		i++;
	} while (sx->reprtab[index + i] != 0);
	return 0;
}

int repr_get_reference(const syntax *const sx, const size_t index)
{
	if (sx == NULL || index >= sx->rp)
	{
		return INT_MAX;
	}

	return sx->reprtab[index + 1];
}

int repr_set_reference(syntax *const sx, const size_t index, const size_t ref)
{
	if (sx == NULL || index >= sx->rp)
	{
		return -1;
	}

	sx->reprtab[index + 1] = (int)ref;
	return 0;
}


int enter_block_scope(syntax *const sx, int *const old_displ, int *const old_lg)
{
	if (sx == NULL)
	{
		return -1;
	}
	
	sx->curid = sx->id;
	*old_displ = sx->displ;
	*old_lg = sx->lg;
	return 0;
}

int exit_block_scope(syntax *const sx, const int old_displ, const int old_lg)
{
	if (sx == NULL)
	{
		return -1;
	}
	
	for (size_t i = sx->id - 4; i >= sx->curid; i -= 4)
	{
		sx->reprtab[sx->identab[i + 1] + 1] = sx->identab[i];
	}
	sx->displ = old_displ;
	sx->lg = old_lg;
	return 0;
}

int enter_func_scope(syntax *const sx)
{
	if (sx == NULL)
	{
		return INT_MAX;
	}
	
	const int old_displ = sx->displ;
	sx->curid = sx->id;
	sx->displ = 3;
	sx->maxdispl = 3;
	sx->lg = 1;
	
	return old_displ;
}

int exit_func_scope(syntax *const sx, const size_t pred, const int scope_start)
{
	if (sx == NULL || (int)pred >= sx->tc)
	{
		return -1;
	}
	
	for (size_t i = sx->id - 4; i >= sx->curid; i -= 4)
	{
		sx->reprtab[sx->identab[i + 1] + 1] = sx->identab[i];
	}
	sx->curid = 2; // все функции описываются на одном уровне
	sx->tree[pred] = sx->maxdispl;
	sx->lg = -1;
	sx->displ = scope_start;
	
	return 0;
}
