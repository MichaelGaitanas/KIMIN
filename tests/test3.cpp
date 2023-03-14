
#include<cstdio>
#include<cmath>

class properties
{
public:
    double M1,M2;
    void assign_values()
}

class propagator
{
public:
    double x,y,z;

    void build_params(const properties &props)
    {
        printf("Started physics\n");
        for (double z = 0.0; z < 20000.0; z += 0.001)
        {
            double y = exp(sin(sqrt(z*fabs(z)+ cos(z))));
        }
        printf("Finished physics\n");
        return;
    }
};

int main()
{
	
	return 0;
}