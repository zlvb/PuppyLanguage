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
#include <sys/stat.h>
#include <time.h>

extern void append_token(Pu *L, Token &t);
extern void parse_include(Pu *L, const Token &filename);
extern void parse_function(Pu *L, int token_from, FILE *pbcf, TokenList *tl);
extern Token get_token_from_file(Pu *L, TokenList *t);
extern bool check_complete(Pu *L);
extern void parse_function_body(Pu *L, 
								int ldef, 
								int token_from, 
								Token &t, 
								FILE *pbcf=0, 
								TokenList *tl=0);

static void save_token(FILE *pbcf, TokenList &tokens)
{
	TokenList::iterator it = tokens.begin();
	TokenList::iterator ite = tokens.end();
	while (it != ite)
	{
		char ctp = (char)it->type;
		fwrite(&ctp, sizeof(ctp),1,pbcf);
		switch (it->type)
		{
		case NUM:
			fwrite(&it->value.numVal(), sizeof(it->value.numVal()),1,pbcf);
			break;
		case OP:{
			char cop = (char)it->optype;
			fwrite(&cop, sizeof(cop), 1, pbcf);
				}break;
		case VAR:{
			char sl = (char)it->value.strVal().length();
			fwrite(&sl, sizeof(sl), 1, pbcf);
			fwrite(it->value.strVal().c_str(), sl,1,pbcf);
				 }break;
		case STR:{
			size_t sl = it->value.strVal().length();
			fwrite(&sl, sizeof(size_t), 1, pbcf);
			fwrite(it->value.strVal().c_str(), it->value.strVal().length(),1,pbcf);
				 }break;
        default:
            break;
		}
		++it;
	}
}

void savebytecode(Pu *, const char *fname, TokenList &tokensave)
{
	FILE *pbcf = fopen(fname, "wb");
	time_t ti; 
	ti=time(NULL);
	fwrite(&ti, sizeof(ti), 1, pbcf);
	save_token(pbcf, tokensave);
	fclose(pbcf);
}

void get_nextbytetoken(Pu *L, FILE *pbcf, Token &t)
{
	char type = (char)UNKNOWN;
	bool needinclude = false;
	for (;;)
	{
		if (fread(&type, sizeof(char), 1, pbcf))
		{
			t.type = (PuType)type;
			t.line = 0;

			switch (type)
			{
			case NUM:
                t.value.SetType(NUM);
				fread(&(t.value.numVal()), sizeof(t.value.numVal()), 1, pbcf);				
				break;

			case OP:{
				char cop = 0;
				fread(&cop, sizeof(cop), 1, pbcf);
				t.optype = (OperatorType)cop;
					}break;

			case VAR:{
				char l = 0;
				fread(&l,sizeof(l),1,pbcf);
				char c[PU_MAXVARLEN] = {0};
				memset(c,0,l+1);
				fread(c, 1, l, pbcf);
                t.value.SetType(STR);
				t.value.strVal() = c;				
					 }break;

			case STR:{
				size_t l = 0;
				fread(&l,sizeof(l),1,pbcf);
				char *c = (char*)g_pumalloc(l+1);				
				fread(c, 1, l, pbcf);
                c[l] = 0;
                t.value.SetType(STR);
				t.value.strVal() = c;	
                g_pufree(c);
					 }break;
			}
			append_token(L,t);
		}

		if (type != INCLUDE)
		{
			break;
		}
		else
		{
			needinclude = true;
		}
	}
	
	if (needinclude)
		parse_include(L,t);
	else if (type == FUNCTION)
		parse_function(L,FROM_BYTECODE, pbcf, 0);
}

static void get_token_from_bytecode(Pu *L, FILE *pbcf)
{
	for (;;)
	{
		Token t;
		t.type = UNKNOWN;
		get_nextbytetoken(L,pbcf, t);
		if (t.type == FINISH)
			break;
	}
}

PUAPI void pu_getbytecodename(char *bytecodename, 
							  const char *csource_code_name)
{
	size_t i = strlen(csource_code_name)-1;
	for ( ; i != 0; --i)
	{
		if (csource_code_name[i] == '.')
			break;
	}
	
	memcpy(bytecodename,csource_code_name,i);
	memcpy(bytecodename+i,".puc",5);
}


PUAPI int pu_loadbytecode(Pu *L, const char *fname)
{
	FILE *pbcf = fopen(fname, "rb");
	if (!pbcf)
		return 0;

	time_t ti; 
	fread(&ti, sizeof(ti), 1, pbcf);

	struct stat buf;
	char sfn[1024] = {0};
	strncpy(sfn, fname, strlen(fname)-1);
	if (stat( sfn ,&buf) != 0)
	{
		fclose(pbcf);
		return 0;
	}
	else if (buf.st_mtime > ti)
	{
		fclose(pbcf);
		return 0;
	}

	get_token_from_bytecode(L,pbcf);

	fclose(pbcf);
	return 1;
}

PUAPI PURESULT pu_load(Pu *L, const char *sname)
{
	size_t i = strlen(sname)-1;
	for ( ; i != 0; --i)
	{
		if (sname[i] == '.')
			break;
	}

	if (strcmp(&(sname[i]), ".puc") == 0)
	{
		return (PURESULT)pu_loadbytecode(L, sname);
	}
	
	L->source.pf = fopen(sname, "r");
	L->source.type = Pusource::ST_FILE;
	if (L->source.pf == NULL)
	{
		return PU_FAILED;
	}

	L->current_filename.push_back(sname);

	TokenList t;
	Token temp;
	do
	{
		temp = get_token_from_file(L, &t);
		CHECKTOKENERROR PU_FAILED;
	}while(temp.type != FINISH);
	fclose(L->source.pf);
	L->source.pf = NULL;

	if (!check_complete(L))
	{
		return PU_FAILED;
	}
	
	char bname[1024]={0};
	pu_getbytecodename(bname,sname);
	savebytecode(L, bname, t);
	t.clear();
	L->cur_token = 0;
	L->current_filename.pop_back();
	L->jumpstack.release();
	return PU_SUCCESS;
}


PUAPI PURESULT pu_loadbuff(Pu *L, const char *str)
{
	if (str == NULL)
	{
		return PU_FAILED;
	}

	char buffname[64]={0};

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4311)
#endif
	PU_SNPRINTF(buffname, sizeof(buffname), "buffer_%lu", (size_t)str);
#ifdef _MSC_VER
#pragma warning(pop)
#endif
	L->current_filename.push_back(buffname);
	L->source.type = Pusource::ST_BUFFER;
	L->source.str.buff = str;
	L->source.str.pos = 0;
	
    Token temp = get_token_from_file(L,NULL);
     while(L->tokens[L->tokens.size()-1].type != FINISH) {        
	    if (L->funstack.size() > 0 && L->funstack.back() >= 1)// 如果有函数未结束，继续获取函数内容
		    parse_function_body(L,L->uncomdef.top(), 
		    FROM_SOURCECODE, temp, 
		    NULL,NULL);
	    else
		    temp = get_token_from_file(L,NULL);
	    CHECKTOKENERROR PU_FAILED;
    }
	L->source.str.buff = NULL;
	L->source.str.pos = 0;
	L->cur_token = 0;
	L->current_filename.pop_back();
	return PU_SUCCESS;
}


PUAPI void pu_makebytecode(Pu *L, const char *sname)
{
	pu_load(L, sname);
	L->cur_token = 0;
	char codename[1024]={0};
	pu_getbytecodename(codename,sname);
	TokenList t;
	do
	{	
		NEXT_TOKEN;
		t.push_back(TOKEN);
	}while(TOKEN.type != FINISH);
	savebytecode(L,codename,t);
}
