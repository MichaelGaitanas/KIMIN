/* This is a single function header, meant to track my personal assignment operator ':=' inside an input file. */

#ifndef FILE_H
#define FILE_H

#include<cstdio>

bool find_assignment_operator(FILE *fp)
{
    int c = fgetc(fp);
    while (c != EOF)
    {
        if (c == ':')
        {
            c = fgetc(fp);
            if (c == '=')
                return true; //Found assignment!
        }
        else
            c = fgetc(fp); //Proceed to the next character.
    }
    return false;
}

#endif