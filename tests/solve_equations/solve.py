import numpy as np

def main():
    #Find all the roots of [a*x + b = 0].
    a, b = 0.1, 13.1
    if a == 0:
        raise ValueError("Coefficient 'a' cannot be zero in a linear equation.")
    x_lin = -b/a
    print(f"\n[{x_lin.real:.16f}, {x_lin.imag if isinstance(x_lin, complex) else 0:.16f}i]\n")
    
    #Find all the roots of [a*x^2 + b*x + c = 0].
    a, b, c = -123.0, 22.0, 763
    x_quad = np.roots([a, b, c])
    print(f"size = {len(x_quad)}")
    for root in x_quad:
        print(f"[{root.real:.16f}, {root.imag:.16f}i]")
    print()
    
    #Find all the roots of [a*x^3 + b*x^2 + c*x + d = 0].
    a, b, c, d = 123.0, 0.0, -0.00003, 4
    x_cub = np.roots([a, b, c, d])
    print(f"size = {len(x_cub)}")
    for root in x_cub:
        print(f"[{root.real:.16f}, {root.imag:.16f}i]")
    print()
    
    #Find all the roots of [a*x^4 + b*x^3 + c*x^2 + d*x + e = 0].
    a, b, c, d, e = 3210.0, 0.021, 321.0, -3111.0, -5.1234
    x_quart = np.roots([a, b, c, d, e])
    print(f"size = {len(x_quart)}")
    for root in x_quart:
        print(f"[{root.real:.16f}, {root.imag:.16f}i]")
    print()

if __name__ == "__main__":
    main()