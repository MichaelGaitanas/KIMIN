#include<cstdio>

class fart
{
public:
   int var1;
   int var2;
   int var3;
   fart() : var3(5)
   {
      var1 = 4;
   }
};

int main()
{
    fart prrrr;
    printf("%d  %d  %d\n",prrrr.var1,prrrr.var2,prrrr.var3);
    return 0;
}