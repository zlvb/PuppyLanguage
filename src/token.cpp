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
#include "error.h"
#include "PuMath.h"
#include <ctype.h>

#define CHECK_ADDLINES {if(c=='\n') ++L->line;}
#define CHECK_MINLINES {if(c=='\n') --L->line;}

extern void get_nextbytetoken(Pu *L, FILE *pbcf, Token &t);
extern void savebytecode(Pu *L, const char *fname, TokenList &tokens);
PUAPI int pu_loadbytecode(Pu *L, const char *fname);
Token get_token_from_file(Pu *L, TokenList *t);

static PuType check_token_type(const std::string &identifier)
{
    int i = 0;
    for (; i < KEYWORDS_TYPES_LENGTH; ++i)
    {
        if (identifier == KEYWORDS[i])
        {
            return (PuType)i;
        }
    }
    return VAR;
}

void append_token(Pu *L, Token &t)
{
    L->tokens.push_back(t);

    if (t.type == IF || t.type == WHILE || t.type == FUNCTION)
    {
        int startpos = L->tokens.size()-1;
        L->jumpstack.push(startpos);
    }
    else if (t.type == ELIF || t.type == ELSE)
    {
        int startpos = L->jumpstack.top();
        L->tokens[startpos].optype = (OperatorType)(L->tokens.size()-1);
        L->jumpstack.pop();
        startpos = L->tokens.size()-1;
        L->jumpstack.push(startpos);
    }
    else if (t.type == END)
    {    
        int startpos = L->jumpstack.top();
        Token &p = L->tokens[startpos];
        if (p.type != FUNCTION)
        {
            p.optype = (OperatorType)(L->tokens.size()-1);
        }
        L->jumpstack.pop();
    }
}

static int pu_sgetc(Pustrbuff &s)
{
    return s.buff[s.pos++];
}

static void _pu_ungetc(Pustrbuff &s)
{
    s.pos--;
}

static void pu_sscanf(Pustrbuff &s, const char *fmt, void *out)
{
    int n=0;
    char newfmt[128]={0};
    strncpy(newfmt,fmt, sizeof(newfmt));
    strcat(newfmt,"%n");
    sscanf(&(s.buff[s.pos]),newfmt,out,&n);
    s.pos += n;
}

static int pu_getc(Pu *L)
{
    int c;
    if (L->source.type == Pusource::ST_FILE)
    {
        c = fgetc(L->source.pf);
    }
    else
    {
        c = pu_sgetc(L->source.str);
    }
    CHECK_ADDLINES;
    return c;
}

static void pu_ungetc(Pu *L, int c)
{
    if (L->source.type == Pusource::ST_FILE)
    {
        ungetc(c, L->source.pf);
    }
    else
    {
        _pu_ungetc(L->source.str);
    }
    CHECK_MINLINES;
}

static void pu_scanf(Pusource &s, const char *fmt, void *out)
{
    if (s.type == Pusource::ST_FILE)
    {
        fscanf(s.pf,fmt,out);
    }
    else
    {
        pu_sscanf(s.str,fmt,out);
    }
}

const __pu_var *get_str_literal(Pu *L, const PuString &strVal)
{
	const __pu_var *literal_value = nullptr;
	auto it = L->const_str_vals.find(*strVal.pbuff);
	if (it != L->const_str_vals.end())
	{
		literal_value = it->second;
	}
	else
	{
		literal_value = new __pu_var(L);
		const_cast<__pu_var*>(literal_value)->SetType(STR);
		const_cast<__pu_var*>(literal_value)->strVal() = strVal;
		L->const_str_vals[*strVal.pbuff] = literal_value;
	}

	return literal_value;
}

static void get_string(Pu *L, int c, Token &newToken)
{    
    bool qtt = (c=='\'');
    c = pu_getc(L);
	PuString strValout;
    while (  (c != '\'' && qtt) || (c != '\"' && !qtt) )
    {
        if (check_source_end(c,L->source))
        {
            error(L,0, L->line);
            return;
        }
        if (c == '\\')
        {
            c = pu_getc(L);
            switch (c)
            {
            case 'n': c = '\n';break;
            case 't': c = '\t';break;
            case 'r': c = '\r';break;
            case '\'': c = '\'';break;
            case '\"': c = '\"';break;
            }
        }
        strValout += (char(c));
        c = pu_getc(L);
    }

	newToken.type = STR;
	newToken.literal_value = get_str_literal(L, strValout);
}

const __pu_var *get_int_literal(Pu *L, PU_INT intval)
{
	const __pu_var *literal_value = nullptr;
	int64_t key = intval;
	auto it = L->const_int_vals.find(key);
	if (it != L->const_int_vals.end())
	{
		literal_value = it->second;
	}
	else
	{
		literal_value = new __pu_var(L);
		const_cast<__pu_var*>(literal_value)->SetType(INTEGER);
		const_cast<__pu_var*>(literal_value)->intVal() = intval;
		L->const_int_vals[key] = literal_value;
	}

	return literal_value;
}

const __pu_var *get_num_literal(Pu *L, PU_NUMBER number)
{
	const __pu_var *literal_value = nullptr;
	uint64_t key = *(uint64_t*)&number;
	auto it = L->const_num_vals.find(key);
	if (it != L->const_num_vals.end())
	{
		literal_value = it->second;
	}
	else
	{
		literal_value = new __pu_var(L);
		const_cast<__pu_var*>(literal_value)->SetType(NUM);
		const_cast<__pu_var*>(literal_value)->numVal() = number;
		L->const_num_vals[key] = literal_value;
	}

	return literal_value;
}

static void get_num(Pu *L, int c, Token &newToken)
{
    char syntax[64] = {0};
    int nlen = 0;
    while (isdigit(c) || c == '.')
    {
        if (check_source_end(c, L->source))
        {
            error(L, 0, L->line);
            return;
        }
        syntax[nlen++] += char(c);
        c = pu_getc(L);
    }

    pu_ungetc(L, c);

    if (is_int(syntax))
    {
        newToken.type = INTEGER;
        newToken.literal_value = get_int_literal(L, atoll(syntax));
    }
    else if (is_float(syntax))
    {
        newToken.type = NUM;
        newToken.literal_value = get_num_literal(L, atof(syntax));
    }    
    else
    {
        error(L, 29);
    }
}

static bool is_space(Pu *L, int c)
{
    if (c == '#')
    {
        while (c != '\n' && !check_source_end(c,L->source))
        {
            c = pu_getc(L);
        }
        return true;
    }
    if (c == '@')
    {
        c = pu_getc(L);
        while (c != '@' && !check_source_end(c,L->source))
        {
            c = pu_getc(L);
        }
        return true;
    }

    int i = 0;
    for (; i < SPACE_TYPES_LENGTH; ++i)
    {
        if (c == SPACE_TYPES[i])
        {
            return true;
        }
    }
    return false;
}


static int check_op_type(const std::string &s)
{
    int i = 0;
    for (; i < OPERATOR_TYPES_LENGTH; ++i)
    {
        if (s == OPERATORS[i])
        {
            return i+1;
        }
    }
    return 0;
}

static bool is_op(int c)
{
    for (int i=0; OPERATOR_CHARS[i]!='\0'; ++i)
    {
        if (OPERATOR_CHARS[i] == c)
            return true;
    }
    return false;
}

static void get_op(Pu *L, int c, Token &newToken)
{
    char identifier[PU_MAXVARLEN]={0};
    int i=-1;
    newToken.type = OP;

    while (is_op(c))
    {
        if (i == 128)
        {
            error(L,28);
            return;
        }
        
        identifier[++i] = char(c);
        int optype = check_op_type(identifier);
        if (optype == 0 && c != '|' && c != '&' && c != '!')
        {
            identifier[i]='\0';
            break;
        }
        else
        {
            c = pu_getc(L);
        }
    }
    pu_ungetc(L,c);
    newToken.optype = (OperatorType)(check_op_type(identifier) - 1);
    if (newToken.type == OP)
    {
        assert(newToken.optype != -1);
    }    
}

static void get_var_key(Pu *L, int c, Token &newToken)
{
    char identifier[PU_MAXVARLEN]={0};
    int i = -1;
    while (!is_op(c) && !is_space(L,c) && !check_source_end(c,L->source))
    {
        if (i == 128)
        {
            error(L,28);
            return;
        }

        identifier[++i] = char(c);
        c = pu_getc(L);
    }
    pu_ungetc(L,c);

    newToken.type = check_token_type(identifier);
    if (newToken.type == VAR)
    {
        newToken.name = InsertStrPool(L, identifier);        
    }    
}

static void check_include(Pu *L, const char *fname)
{
    auto it = L->current_filename.begin();
    auto ite = L->current_filename.end();

    while (it != ite)
    {
        int i = (int)(*it)->length() - 1;
        for (; i >= 0; --i)
        {
			if ((*it)->operator[](i) == '.') 
			{
				break; 
			}
        }

        if (memcmp((*it)->c_str(), fname, i) == 0)
        {
            error(L, 19);
            break;
        }
        ++it;
    }
}

static void get_label(Pu *L, Token &newToken)
{
    newToken.type = LABEL;
    char label[PU_MAXVARLEN]={0};
    pu_scanf(L->source, "%s", label);
    newToken.name = InsertStrPool(L, label);
    L->labelmap.insert(std::make_pair(label, L->cur_token));
}

void parse_function_body(Pu *L, int lp, int token_from, Token &t, FILE *pbcf=0, TokenList *tl=0)
{
    for (;;)
    {
        if (t.type == IF)    
        {
            L->funstack.back()++;
        }
        else if    (t.type == WHILE)
        {
            L->funstack.back()++;
        }
        else if (t.type == END)        
        {
            L->funstack.back()--;
        }
        else if (t.type == FINISH)
        {        
            if (L->mode != PU_CODE_FROM_FILE) 
            {
                L->uncomdef.push(lp);
                return;
            }
            else
            {
                error(L,17);
                break;
            }
        }

        if (L->funstack.back() == 0)
        {        
            if (L->mode != PU_CODE_FROM_FILE)
            {
                L->uncomdef.pop();
            }
            
            L->funstack.pop_back();
            break;
        }

        if (token_from == FROM_BYTECODE)
        {
            get_nextbytetoken(L,pbcf, t);
        }
        else
        {
            t = get_token_from_file(L,tl);
            CHECKTOKENERROR;
        }
    }
    FuncPos &fps = L->funclist[lp];
    fps.end = L->tokens.size();
}

void parse_function(Pu *L, int token_from, FILE *pbcf, TokenList *tl)
{
    FuncPos fps;
    Token fname;
    int funckeywords_pos =  L->tokens.size()-1;
    if (token_from == FROM_BYTECODE)
    {
        get_nextbytetoken(L,pbcf, fname);
    }
    else
    {
        fname = get_token_from_file(L,tl);
        CHECKTOKENERROR;
    }

    Token t;
    if (fname.type != VAR)
    {
        if (fname.optype != OPT_LB)
        {
            error(L, 29);
            return;
        }

        if (token_from == FROM_BYTECODE)
        {
            get_nextbytetoken(L,pbcf, t);
        }
        else
        {
            t = get_token_from_file(L,tl);
            CHECKTOKENERROR;
        }
    }
    else
    {
        if (token_from == FROM_BYTECODE)
        {
            get_nextbytetoken(L,pbcf, t);
            get_nextbytetoken(L,pbcf, t);
        }
        else
        {
            t = get_token_from_file(L,tl);
            CHECKTOKENERROR;
            t = get_token_from_file(L,tl);
            CHECKTOKENERROR;
        }
    }
    
    while (!(t.type == OP && t.optype == OPT_RB))// )
    {
        if (t.type == OP && t.optype == OPT_COM)
        {
            //pass
        }
        else
        {
            fps.argnames.push_back(t.name);
        }
        
        if (token_from == FROM_BYTECODE)
        {
            get_nextbytetoken(L,pbcf, t);
        }
        else
        {
            t = get_token_from_file(L,tl);
            CHECKTOKENERROR;
        }
    }

    fps.start = L->tokens.size();
    if (token_from == FROM_BYTECODE)
    {
        get_nextbytetoken(L,pbcf, t);
    }
    else
    {
        t = get_token_from_file(L,tl);
        CHECKTOKENERROR;
    }
    
    L->funclist.push_back(fps);
    L->tokens[funckeywords_pos].optype = (OperatorType)(L->funclist.size()-1);
    L->funstack.push_back(1);
    parse_function_body(L,L->funclist.size()-1,token_from,t,pbcf,tl);
}


void parse_include(Pu *L, const Token &filename)
{
    check_include(L, filename.name->c_str());

    std::string newfname = (*filename.name) + ".puc";
    if (!pu_loadbytecode(L, newfname.c_str()))
    {
        Pusource old_s = L->source;
        std::string fsn(*filename.name);
        fsn += ".pu";
        L->source.pf = fopen(fsn.c_str(), "r");
        L->source.type = Pusource::ST_FILE;
        if (L->source.pf == nullptr)
        {
            error(L,20);
            return;
        }

		L->current_filename.push_back(InsertStrPool(L, fsn.c_str()));

        TokenList t;
        Token temp;
        do{
            temp = get_token_from_file(L, &t);
            CHECKTOKENERROR;
        }while(temp.type != FINISH);
        fclose(L->source.pf);
        savebytecode(L,newfname.c_str(), t);
        t.clear();

        L->source = old_s;
        L->current_filename.pop_back(); 
    }
    L->tokens.pop_back(); 
}

int findcurstart(Pu *L)
{
    int max_token = L->tokens.size();
    if (max_token <= 1)
    {
        return 0;
    }

    int ct = max_token;
    ct-=2;
    while (L->tokens[ct].type != FINISH)
    {
        if (ct == 0)
            return 0;
        --ct;

    }
    return ct+1;
}


bool check_complete(Pu *L)
{
    int cur = L->cur_token;
    L->cur_token = findcurstart(L);
    int whh=0;
    NEXT_TOKEN;
    for (;TOKEN.type != FINISH;)
    {    
        PuType tp = TOKEN.type;
        if (tp == WHILE || tp == IF || tp == FUNCTION)
        {
            ++whh;
        }
        else if (tp == END)
        {
            --whh;
        }
        NEXT_TOKEN;
    }
    L->cur_token = cur;
    if (whh<0)
    {
        error(L,26);
    }
    return whh==0;
}

Token get_token_from_file(Pu *L, TokenList *t)
{
    Token newToken;
    newToken.type = UNKNOWN;
    bool need_include = false;
    for (;;)
    {
        int c;
        do{
            c = pu_getc(L);
        }while (is_space(L,c));

        if (check_source_end(c,L->source))
        {
            newToken.type = FINISH;
        }
        else if (c == '\'' || c == '\"') //        
        {
            get_string(L, c, newToken);
        }
        else if (c == '$') 
        {
            get_label(L, newToken);
        }
        else if (is_op(c)) //        
        {
            get_op(L, c, newToken);
        }
        else if (isdigit(c) || c=='-')
        {
            get_num(L, c,newToken);
        }
        else if (!is_op(c)) 
        {
            get_var_key(L, c, newToken);
        }

        CHECKTOKENERROR newToken;

        newToken.line = L->line;
        newToken.filename = L->current_filename.back();

        append_token(L, newToken);

        if (t)
        {
            t->push_back(newToken);
        }
                        
        if (newToken.type != INCLUDE)
            break;
        else
            need_include = true;
    }
    
    if (need_include)
        parse_include(L,newToken);
    else if (newToken.type == FUNCTION)
        parse_function(L,FROM_SOURCECODE, nullptr, t);
    
    return newToken;
}

void Token::operator=( const Token &x )
{
    type = x.type;
    line = x.line;
    optype = x.optype;
    filename = x.filename;
    name = x.name;
    literal_value = x.literal_value;
}

Token::Token( const Token &x )
    : filename(x.filename)
    , literal_value(NULL)
    , exp_end(-1)
    , exp_stack(-1)
	
    , control_flow(nullptr)
{
    type = x.type;
    line = x.line;
    optype = x.optype;
    name = x.name;
    if (x.type >= NUM && x.type <= LABEL)
    {
        literal_value = x.literal_value;
    }
}

Token::~Token()
{
	if (control_flow)
	{
		delete control_flow;
		control_flow = nullptr;
	}
}

const std::string *InsertStrPool(Pu * L, const char *sname)
{
	const std::string *filename = nullptr;
	auto it = L->strpool.find(sname);
	if (it == L->strpool.end())
	{
		filename = new std::string(sname);
		L->strpool.insert(std::make_pair(*filename, filename));
	}
	else
	{
		filename = it->second;
	}

	return filename;
}
