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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../puppy.h"

void pu_console(Pu *L);

int main(int argc, char **argv)
{
    srand((unsigned int)time(nullptr));
    Pu *L = pu_open();
    if (argc < 2)
    {   
        pu_console(L);
        pu_close(L);
        L = nullptr;
    }
    else
    {
        if (argc > 1 && strcmp(argv[1], "-c") == 0)
        {
            pu_makebytecode(L, argv[2]);
            pu_close(L);
            return 0;
        }

        if (pu_load(L, argv[1]) == PU_SUCCESS)
        {
            pu_run(L);
        }
        else
        {
            printf("%s\n", "error.");
        }
        pu_close(L);
        L = nullptr;

#ifdef _WIN32
        system("pause");
#endif
    }
}
