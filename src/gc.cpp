/*
    Copyright (c) 2009 Zhang li

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.

    MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/*
    Author zhang li
    Email zlvbvbzl@gmail.com
*/

#include "state.h"

static void mark(StrKeyMap *global_vmap);
static void mark(_scope *scope);
static void mark(__pu_var &v)
{
	if (v.type() == FUN)
	{
		_scope *uv = &v.up_value();
		if (uv && !uv->base_->marked)
		{
			uv->base_->marked = true;
			mark(uv);
		}
	}
}

void mark(StrKeyMap *global_vmap)
{
	for (auto &pair : *global_vmap)
	{
		mark(pair.second);
	}
}

void mark(_scope *scope)
{
    for (auto &pair : *scope->base_->vmap_)
    {
		mark(pair.second);
    }
}

void sweep(Pu *L)
{
	auto it = L->gccontainer.begin();
	auto itend = L->gccontainer.end();
	while (it != itend)
	{
		if (!(*it)->marked)
		{
			auto *ptr = *it;
			it = L->gccontainer.erase(it);
			delete ptr;
		}
		else
		{
			(*it)->marked = false;
			++it;
		}
	}
}

void gc(Pu *L)
{
    StrKeyMap *root = L->varstack.bottom();
    mark(root);
    sweep(L);    
}

void clear_temp(Pu *L)
{
    for (auto &v : L->tempvals)
    {
        L->tempool.push_back(v);
    }
    L->tempvals.clear();
}
