#include<cstdio>

class Properties
{
public:
   int var1;
   int var2;
   Properties() : var1(5),
                  var2(6)
   { }

   void change_public_vars()
   {
      var1 = 15;
      var2 = 16;
   }
};

class Propagator : public Properties
{
public:
   int var3;

   Propagator(const Properties &properties)
   {
      var1 = properties.var1;
      var2 = properties.var2;
   }

   void print_var1_and_var2()
   {
      printf("%d  %d\n",var1,var2);
      return;
   }
};

int main()
{
   Properties properties;
   properties.change_public_vars();
   printf("%d  %d\n",properties.var1, properties.var2);

   Propagator propagator(properties);
   propagator.print_var1_and_var2();

   return 0;
}