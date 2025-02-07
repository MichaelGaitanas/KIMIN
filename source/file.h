#ifndef FILE_H
#define FILE_H

#include<cstdio>

//This function is used to track the assignment operator ':=' inside a file, which is used to read inputs.
bool find_assignment_operator(FILE *fp)
{
    int c = fgetc(fp);
    while (c != EOF)
    {
        if (c == ':')
        {
            c = fgetc(fp);
            if (c == '=')
                return true;
        }
        else
            c = fgetc(fp); //Update c when it is not ':'.
    }
    return false;
}

#endif