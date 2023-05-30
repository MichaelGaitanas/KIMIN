
int main()
{
    Properties props;
    Integrator integ(props);
    Solution sol(integ);

    std::cout << sol.p << "  " << sol.i << "  " << sol.s << "\n";

    return 0;
}