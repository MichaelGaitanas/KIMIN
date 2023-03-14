
#include<cstdio>
#include<cmath>

class Properties
{
public:
    double M1,M2;
};

class Propagator
{
public:
    double x,y,z;
    Propagator(const Properties &properties)
    {
        if (properties.M1 <= 0.0)
            x = 17.0;
    }
};

int main()
{
    Properties properties;
    properties.M1 =  -3.0;
    properties.M2 = -34.0;

    Propagator propagator(properties);

    printf("x = %lf\n",propagator.x);

	
	return 0;
}