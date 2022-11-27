#ifndef FAST_ATOF
#define FAST_ATOF


#include <stdlib.h>
#include <math.h>


const float fast_atof_table[] =        {
                                                                                0.f,
                                                                                0.1f,
                                                                                0.01f,
                                                                                0.001f,
                                                                                0.0001f,
                                                                                0.00001f,
                                                                                0.000001f,
                                                                                0.0000001f,
                                                                                0.00000001f,
                                                                                0.000000001f,
                                                                                0.0000000001f,
                                                                                0.00000000001f,
                                                                                0.000000000001f,
                                                                                0.0000000000001f,
                                                                                0.00000000000001f,
                                                                                0.000000000000001f
                                                                        };

//! Provides a fast function for converting a string into a float,
//! about 6 times faster than atof in win32.
inline char* fast_atof_move(char* c, float& out)
{
        bool inv = false;
        char *t;
        float f;

        if (*c=='-')
        {
                c++;
                inv = true;
        }

        f = (float)strtol(c, &t, 10);

        c = t;

        if (*c == '.')
        {
                c++;

                float pl = (float)strtol(c, &t, 10);
                pl *= fast_atof_table[t-c];

                f += pl;

                c = t;

                if (*c == 'e')
                {
                        ++c;
                        float exp = (float)strtol(c, &t, 10);
                        f *= (float)pow(10.0f, exp);
                        c = t;
                }
        }

        if (inv)
                f *= -1.0f;
        
        out = f;
        return c;
}

//! Provides a fast function for converting a string into a float,
//! about 6 times faster than atof in win32.
inline const char* fast_atof_move_const(const char* c, float& out)
{
        bool inv = false;
        char *t;
        float f;

        if (*c=='-')
        {
                c++;
                inv = true;
        }

        f = (float)strtol(c, &t, 10);

        c = t;

        if (*c == '.')
        {
                c++;

                float pl = (float)strtol(c, &t, 10);
                pl *= fast_atof_table[t-c];

                f += pl;

                c = t;

                if (*c == 'e') 
                { 
                        ++c; 
                        float exp = (float)strtol(c, &t, 10); 
                        f *= (float)pow(10.0f, exp); 
                        c = t; 
                }
        }

        if (inv)
                f *= -1.0f;
        
        out = f;
        return c;
}


inline float fast_atof(const char* c)
{
        float ret;
        fast_atof_move_const(c, ret);
        return ret;
}



#endif