#pragma once

/*
DOCUMENTATION:

main API: 

ch_obj::Model ch_obj::load_model(char *path);

Model data structure:

vb: vertex (position) buffer
nb: normal buffer
ib: index buffer
int vb_count: number of floats in vb
int nb_count: number of floats in nb
int ib_count: number of indices* in ib (one index = vi + ti + ni)
bool is_invalid: flag set to true if model fails to load
char *e_msg: reason why model failed to load

vb layout:

x0|y0|z0|x1|y1|z1|....

nb layout:

x0|y0|z0|x1|y1|z1|....

ib layout:

 |vi0|ti0|ni0|vi1|ti1|ni1|...
|                              |                              |
--------------------------------------------------------------
                 .             |                             |
.          vertex 0                      vertex 1

where vi = position index, ti = texture index, ni = normal index

ib always represents vertices of list of triangles.

Each vertex has a position index, a uv index, a normal index.
uv index and normal index can be -1, meaning that particular attribute
does not exist for that vertex.

 Any of vb, nb, and ib could be null, denoting that the buffer is empty.
ib indexes directly into vb and nb.


*/

// plug your own assert
#ifndef CH_ASSERT
#define CH_ASSERT(Value) assert(Value)
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

namespace ch_obj
{
    struct Model
    {
        float *vb;
        int vb_count;
        float *nb;
        int nb_count;
        int *ib;
        int ib_count;
        
        bool is_invalid;
        char e_msg[256];
    };
    
    template <typename T> struct Stretchy_Array
    {
        T *data;
        int count;
        int cap;
        
        void push(T item);
        T &operator[](int i);
        T operator[](int i) const;
        void free();
    };
    
    enum Token_Type
    {
        token_type_invalid = 0,
        token_type_id,
        token_type_integer,
        token_type_real,
        token_type_punc, // one letter punctuations
        token_type_count,
    };
    
    struct Token
    {
        Token_Type type;
        int line_num;
        int col_num;
        
        char *text;
        int len;
        
        union
        {
            char as_punc;
            int as_integer;
            float as_real;
        };
        
        bool is_id(char *name);
        bool is_int();
        bool is_real();
        bool is_punc(char punc);
        
        float force_real();
    };
    
    struct Parser
    {
        Stretchy_Array<Token> tokens;
        int next_ti;
        char *e_msg;
        bool has_error;
        Token bad_token;
        
        bool has_next();
        Token next();
        Token peek();
        Token expect_int(char *msg);
        Token expect_real(char *msg);
        Token expect_punc(char punc, char *msg);
        Token expect_num(char *msg); // int or real
        
        void error(Token bad_token, char *msg);
    };
    
    struct Face_V
    {
        int p;
        int t;
        int n;
    };
    
    //
    //
    // stretchy array
    
    template <typename T> void Stretchy_Array<T>::push(T item)
    {
        if (data)
        {
            if (count >= cap)
            {
                cap = 2 * cap + 1;
                data = (T *)realloc(data, cap * sizeof(T));
            }
            data[count++] = item;
        }
        else
        {
            cap = 2;
            data = (T *)calloc(cap, sizeof(T));
            data[count++] = item;
        }
    }
    
    template <typename T> void Stretchy_Array<T>::free()
    {
        ::free(data);
    }
    
    template <typename T> T &Stretchy_Array<T>::operator[](int i)
    {
        return data[i];
    }
    
    template <typename T> T Stretchy_Array<T>::operator[](int i) const
    {
        return data[i];
    }
    
    //
    //
    // token
    
    Token invalid_token()
    {
        Token t = {};
        t.type = token_type_invalid;
        
        return t;
    }
    
    bool Token::is_id(char *name)
    {
        return (type == token_type_id && strncmp(name, text, len) == 0);
    }
    
    bool Token::is_int()
    {
        return type == token_type_integer;
    }
    
    bool Token::is_real()
    {
        return type == token_type_real;
    }
    
    bool Token::is_punc(char c)
    {
        return type == token_type_punc && as_punc == c;
    }
    
    float Token::force_real()
    {
        CH_ASSERT(type == token_type_integer || type == token_type_real);
        if (type == token_type_integer)
        {
            return float(as_integer);
        }
        else if (type == token_type_real)
        {
            return float(as_real);
        }
        return 0.0f;
    }
    
    //
    //
    // parser
    
    Parser init_parser(Stretchy_Array<Token> tokens)
    {
        Parser p = {};
        
        p.tokens = tokens;
        p.next_ti = 0;
        
        return p;
    }
    
    bool Parser::has_next()
    {
        return next_ti < tokens.count;
    }
    
    Token Parser::peek()
    {
        if (next_ti >= 0 && next_ti < tokens.count)
        {
            return tokens[next_ti];
        }
        else
        {
            return invalid_token();
        }
    }
    
    Token Parser::next()
    {
        if (next_ti >= 0 && next_ti < tokens.count)
        {
            return tokens[next_ti++];
        }
        else
        {
            return invalid_token();
        }
    }
    
    Token Parser::expect_int(char *msg)
    {
        Token t = next();
        if (!t.is_int())
        {
            error(t, msg);
        }
        return t;
    }
    
    Token Parser::expect_real(char *msg)
    {
        Token t = next();
        if (!t.is_real())
        {
            error(t, msg);
        }
        return t;
    }
    
    Token Parser::expect_punc(char punc, char *msg)
    {
        Token t = next();
        if (!t.is_punc(punc))
        {
            error(t, msg);
        }
        return t;
    }
    
    Token Parser::expect_num(char *msg)
    {
        Token t = next();
        if (!t.is_int() && !t.is_real())
        {
            error(t, msg);
        }
        return t;
    }
    
    void Parser::error(Token bad_t, char *msg)
    {
        if (!has_error)
        {
            e_msg = msg;
            bad_token = bad_t;
            has_error = true;
        }
    }
    
    //
    //
    // face vert
    
    Face_V init_fv(int p, int n, int t)
    {
        Face_V f = {};
        f.p = p;
        f.t = t;
        f.n = n;
        return f;
    }
    
    //
    //
    // file io
    
    char *read_text_file(char *path)
    {
        char *text = 0;
        
        FILE *f = fopen(path, "rb");
        if (f)
        {
            fseek(f, 0, SEEK_END);
            size_t byte_count = ftell(f);
            rewind(f);
            
            text = (char *)calloc(byte_count + 1, 1); // (+1) for null terminator
            fread(text, 1, byte_count, f);
            fclose(f);
        }
        
        return text;
    }
    
    //
    //
    // lexer
    
    inline char lower(char c)
    {
        if (c >= 'A' && c <= 'Z')
        {
            return c - ('A' - 'a');
        }
        return c;
    }
    
    inline bool is_alpha(char c)
    {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
    }
    
    inline bool is_num(char c)
    {
        return c >= '0' && c <= '9';
    }
    
    inline bool is_alphanum(char c)
    {
        return is_alpha(c) || is_num(c);
    }
    
    char *eat_til_newline(char *c)
    {
        while (*c != '\n')
        {
            ++c;
        }
        return c + 1;
    }
    
    char *scan_num(char *c, Token *t, int line_num)
    {
        int sign = 1;
        if (*c == '-')
        {
            sign = -1;
            ++c;
        }
        
        int whole = 0;
        while (is_num(*c))
        {
            whole = 10 * whole + (*c - '0');
            ++c;
        }
        
        if (*c == '.') // real
        {
            ++c;
            
            float decimal = 0.0f;
            float weight = 0.1f;
            while (is_num(*c))
            {
                decimal += weight * float(*c - '0');
                weight *= 0.1f;
                ++c;
            }
            
            *t = {};
            t->type = token_type_real;
            t->line_num = line_num;
            t->as_real = float(sign) * (float(whole) + decimal);
        }
        else // int
        {
            *t = {};
            t->type = token_type_integer;
            t->line_num = line_num;
            t->as_integer = sign * whole;
        }
        
        return c;
    }
    
    Stretchy_Array<Token> lex(char *text)
    {
        Stretchy_Array<Token> tokens = {};
        
        int line_num = 1;
        char *c = text;
        while (*c)
        {
            if (*c == '#') // comment
            {
                c = eat_til_newline(c);
                ++line_num;
            }
            else if (is_alpha(*c) || *c == '_') // id
            {
                Token t = {};
                t.type = token_type_id;
                t.line_num = line_num;
                t.text = c;
                t.len = 0;
                
                while (is_alphanum(*c) || *c == '.' || *c == '_' || *c == '-')
                {
                    ++c;
                    ++t.len;
                }
                
                tokens.push(t);
            }
            else if (*c == '-' || *c == '.' || is_num(*c)) // int/reals
            {
                Token t = {};
                c = scan_num(c, &t, line_num);
                if (lower(*c) != 'e')
                {
                    tokens.push(t);
                }
                else // scientific case
                {
                    //TODO(chen): handle integer base or float exp
                    ++c;
                    float base = t.as_real;
                    c = scan_num(c, &t, line_num);
                    float exp = (float)t.as_integer;
                    
                    t = {};
                    t.type = token_type_real;
                    t.line_num = line_num;
                    t.as_real = base * powf(10.0f, exp);
                    tokens.push(t);
                }
            }
            else if (*c == '/') // punctuations
            {
                Token t = {};
                t.type = token_type_punc;
                t.line_num = line_num;
                t.as_punc = *c;
                tokens.push(t);
                
                ++c;
            }
            else if (*c == '\n')
            {
                ++line_num;
                ++c;
            }
            else // garbage
            {
                ++c;
            }
        }
        
        return tokens;
    }
    
    //
    //
    // parser
    
    Face_V parse_v(Parser *p)
    {
        Face_V fv = {};
        
        int pi = p->expect_int("expected vertex index (an integer)").as_integer;
        
        if (!p->peek().is_punc('/'))
        {
            fv.p = pi;
            return fv;
        }
        
        p->expect_punc('/', "expected slash (vertex index segregator)");
        
        if (p->peek().is_punc('/'))
        {
            p->next(); // punc '/'
            
            int ni = p->expect_int("expected normal index (an integer)").as_integer;
            fv.p = pi;
            fv.n = ni;
            return fv;
        }
        
        int ti = p->expect_int("expected uv index (an integer)").as_integer;
        
        if (!p->peek().is_punc('/'))
        {
            fv.p = pi;
            fv.t = ti;
            return fv;
        }
        
        p->next(); // punc '/'
        
        int ni = p->expect_int("expected normal index (an integer)").as_integer;
        fv.p = pi;
        fv.t = ti;
        fv.n = ni;
        return fv;
    }
    
    //NOTE(chen): OBJ's wacky indices ... sigh
    //
    //            index base: 1
    //            negative index: relative to the last vertex seen so far,
    //                            meaning, -1 corresponds to last vertex
    int fix_index(int index, int count)
    {
        if (count == 0)
        {
            return -1;
        }
        
        if (index > 0)
        {
            return index - 1;
        }
        else
        {
            return count + index;
        }
    }
    
    //
    //
    // loader
    
    Model load_model(char *path)
    {
        Model res = {};
        
        char *text = read_text_file(path);
        if (text)
        {
            Stretchy_Array<Token> tokens = lex(text);
            Parser p = init_parser(tokens);
            
            Stretchy_Array<float> pb = {};
            Stretchy_Array<float> nb = {};
            Stretchy_Array<Face_V> ib = {};
            
            while (p.has_next() && !p.has_error)
            {
                Token t = p.next();
                if (t.is_id("v"))
                {
                    char *msg = "parsing vertex pos: expected a number";
                    float x = p.expect_num(msg).force_real();
                    float y = p.expect_num(msg).force_real();
                    float z = p.expect_num(msg).force_real();
                    
                    pb.push(x);
                    pb.push(y);
                    pb.push(z);
                }
                else if (t.is_id("vn"))
                {
                    char *msg = "parsing vertex normal: expected a number";
                    float x = p.expect_num(msg).force_real();
                    float y = p.expect_num(msg).force_real();
                    float z = p.expect_num(msg).force_real();
                    
                    nb.push(x);
                    nb.push(y);
                    nb.push(z);
                }
                else if (t.is_id("f"))
                {
                    Face_V fvs[16] = {};
                    
                    int count = 0;
                    while (p.peek().is_int())
                    {
                        if (count >= 16)
                        {
                            p.error(p.peek(), "n-gon exceeds 16 verts, which is ch_obj's limit");
                            break;
                        }
                        
                        int v_count = pb.count / 3;
                        int n_count = nb.count / 3;
                        
                        Face_V fv = parse_v(&p);
                        fv.p = fix_index(fv.p, v_count);
                        //TODO(chen): fix tex coord index
                        //fv.t = fix_index(fv.t, tb.count);
                        fv.t = -1;
                        fv.n = fix_index(fv.n, n_count);
                        fvs[count++] = fv;
                    }
                    
                    if (count >= 3)
                    {
                        int tri_count = count - 2;
                        for (int ti = 0; ti < tri_count; ++ti)
                        {
                            ib.push(fvs[0]);
                            ib.push(fvs[ti+1]);
                            ib.push(fvs[ti+2]);
                        }
                    }
                    else
                    {
                        p.error(t, "face statment has less than 3 vertices");
                    }
                }
                else if (t.is_id("mtllib"))
                {
                    Token mtl_file = p.next();
                    
                    //TODO(chen): skip for now
                }
                else if (t.is_id("vt"))
                {
                    Token u = p.next();
                    Token v = p.next();
                    if (p.peek().is_real())
                    {
                        Token w = p.next();
                    }
                    
                    //TODO(chen): skip for now
                }
                else if (t.is_id("o"))
                {
                    Token object = p.next();
                    
                    //TODO(chen): skip for now
                }
                else if (t.is_id("g"))
                {
                    Token group = p.next();
                    
                    //TODO(chen): skip for now
                }
                else if (t.is_id("s"))
                {
                    Token smoothing = p.next();
                    
                    //NOTE(chen): ignored
                }
                else if (t.is_id("usemtl"))
                {
                    Token mtl_name = p.next();
                    
                    //TODO(chen): skip for now
                }
                else
                {
                    p.error(t, "unknown statement prefix");
                }
            }
            
            if (p.has_error)
            {
                snprintf(res.e_msg, sizeof(res.e_msg), 
                         "PARSER ERROR at line %d: %s", 
                         p.bad_token.line_num, p.e_msg);
                res.is_invalid = true;
                
                free(text);
                return res;
            }
            
            // export 
            res.vb = (float *)malloc(sizeof(float) * pb.count);
            res.vb_count = pb.count;
            res.nb = (float *)malloc(sizeof(float) * nb.count);
            res.nb_count = nb.count;
            res.ib = (int *)malloc(sizeof(int) * 3 * ib.count);
            res.ib_count = ib.count;
            
            for (int i = 0; i < pb.count; ++i)
            {
                res.vb[i] = pb[i];
            }
            
            for (int i = 0; i < nb.count; ++i)
            {
                res.nb[i] = nb[i];
            }
            
            int ib_c = 0;
            for (int i = 0; i < ib.count; ++i)
            {
                res.ib[ib_c++] = ib[i].p;
                res.ib[ib_c++] = ib[i].t;
                res.ib[ib_c++] = ib[i].n;
            }
            
            pb.free();
            nb.free();
            ib.free();
            
            free(text);
        }
        else
        {
            snprintf(res.e_msg, sizeof(res.e_msg), "specified path \"%s\" found", path);
            res.is_invalid = true;
        }
        
        return res;
    }
}