#include<cstdio>

class Solution
{
public:
    double x,y,z;
};

class Graphics
{
public:
    Solution solution;
    int a,b;
    Graphics() : a(5), b(6) { }
    void yield_solution(const Solution &solution)
    {
        this->solution = solution;
        return;
    }
};

int main()
{
    Solution solution;
    solution.x = 100.0;
    solution.y = 200.0;
    solution.z = 300.0;

    Graphics graphics;
    graphics.yield_solution(solution);

    printf("%lf %lf %lf\n",solution.x, solution.y, solution.z);
    printf("%lf %lf %lf\n",graphics.solution.x, graphics.solution.y, graphics.solution.z);

    return 0;
}