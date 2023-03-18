#include<cstdio>

class Properties
{
public:
    bool ell_checkbox, ord2_checkbox;
    int x;
    Properties() : ell_checkbox(true),
                   ord2_checkbox(false),
                   x(3)
    { }


};

class Propagator : public Properties
{
public:
    float y,z;
    Propagator(const Properties &properties) : Properties(properties)
    {
        y = 5.0f;
        printf("%d\n",x);
    }
};

int main()
{
    Properties props;
    printf("%d  %d  %d\n",props.ell_checkbox, props.ord2_checkbox, props.x);

    Propagator propag(props);
    //printf("%d  %d  %f\n",propag.ell_checkbox, propag.ord2_checkbox, propag.y);

    return 0;
}