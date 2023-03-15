#include<iostream>

class Properties
{
public:
   int var1;
   int var2;
   Properties() : var1(5),
                  var2(6)
   { printf("Instantiated 'Properties' --> props\n"); }
};

class Propagator : public Properties
{
public:
   int var3;
   Propagator(Properties &props)
   {
        if (props.var1 > 0)
            var3 = 1;
        else
            var3 = -1;
   }
};

int main()
{
   Properties props;
   
   return 0;
}