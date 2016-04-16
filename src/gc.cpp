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

static void mark(StrKeyMap *vmap)
{
    for (auto &pair : *vmap)
    {
        __pu_value &v = pair.second;
        if (v.type() == FUN)
        {
            _up_value *uv = (_up_value*)v.userdata();
            if (uv && !uv->marked)
            {
                uv->marked = true;
                mark(uv->vmap);
            }
        }
    }
}

static void sweep(Pu *L)
{
    _up_value *p = L->gclink;
    _up_value *prev = p;
    while (p != 0)
    {
        if (p->marked)
        {
            p->marked = false;
            prev = p;
            p = p->next;
        }
        else
        {
            if (p == L->gclink)
            {
                L->gclink = L->gclink->next;
                prev = L->gclink;
                delete p;
                p = L->gclink;
            }
            else
            {
                _up_value *np = p->next;
                delete p;
                prev->next = np;
                p = np;
            }
        }
    }
}

void safe_decrecount(Pu *L, _up_value *n)
{
    _up_value *p = L->gclink;
    _up_value *prev = p;
    while (p != 0)
    {
        if (p != n)
        {
            prev = p;
            p = p->next;
            continue;
        }

        if ((--p->refcount) != 0)
        {
            break;
        }

        if (p == L->gclink)
        {
            L->gclink = L->gclink->next;
            prev = L->gclink;
            //delete p->vmap;
            delete p;
        }
        else
        {
            _up_value *np = p->next;
            //delete p->vmap;
            delete p;
            prev->next = np;
        }
        break;
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
    for (int i = 0; i < L->tempvals.size(); i++)
    {
        L->tempool.push_back(L->tempvals[i]);
    }
    L->tempvals.clear();
}
