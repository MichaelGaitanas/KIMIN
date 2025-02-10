import math as ma
import pykep as pk

#Perform unsigned modulo 2*pi to the argument 'angle'.
def wrap_to_2pi(angle):
    angle = ma.fmod(angle, 2.0*ma.pi)
    if angle < 0.0:
        angle += 2.0*ma.pi
    return angle

#Convert H to the Gudermannian : angle = 2*atan(tanh(H/2)).
def H2gudermannian(H):
    return 2.0*ma.atan(ma.tanh(H/2.0))

#Calculate the eccentric anomaly, given the mean anomaly M and eccentricity e (with e < 1).
#This is done by inverting Kepler's equation using a Newton-Raphson method.
def M2E(M, e):

    #If sin(M) is essentially zero, simply return M.
    if abs(ma.sin(M)) < 1e-15:
        return M

    #Initial guess for the eccentric anomaly.
    E = M + 0.85*e*ma.sin(M)/abs(ma.sin(M))
    
    iter = 0
    while True:
        E0 = E
        E = E0 - (M - E0 + e*ma.sin(E0))/(-1.0 + e*ma.cos(E0))
        
        iter += 1
        #Loop escape criterion.
        if iter > 20:
            print("Warning: In M2E(), N-R did not converge sufficiently. "
                  "Returning the last computed E = {:.15f} [rad] with |E - E_prev| = {:.15e}."
                  .format(E, abs(E - E0)))
            return wrap_to_2pi(E)
        #Convergence check.
        if abs(E - E0) <= 1e-15:
            break

    return wrap_to_2pi(E)

def M2H(M, e):

    if abs(M) < 1e-15:
        return M

    H = M/(e - 1.0)
    
    iter = 0
    while True:
        H0 = H
        H = H0 - (e*ma.sinh(H0) - H0 - M)/(e*ma.cosh(H0) - 1.0)

        iter += 1
        if iter > 20:
            print("Warning: In M2H(), N-R did not converge sufficiently. "
                  "Returning the last computed H = {:.15f} [rad] with |H - H_prev| = {:.15e}."
                  .format(H, abs(H - H0)))
            return H

        if abs(H - H0) <= 1e-15:
            break

    return H

G = 6.67430e-20; #[km^3/(kg*sec^2)]
M1 = 5.320591856403073e15 #[kg]
M2 = 4.940814359692687e14 #[kg]

a  = 3.3 #[km]
e  = 0.3 #[ ]
i  = -123 #[deg]
Om = 12 #[deg]
w  = -1 #[deg]
M  = 62 #[deg]

if e < 1.0:
    r,v = pk.par2ic([a,
                     e,
                     i*ma.pi/180,
                     Om*ma.pi/180,
                     w*ma.pi/180,
                     M2E(M*ma.pi/180,e)], G*(M1+M2))
else:
    r,v = pk.par2ic([a,
                     e,
                     i*ma.pi/180,
                     Om*ma.pi/180,
                     w*ma.pi/180,
                     H2gudermannian(M2H(M*ma.pi/180,e))], G*(M1+M2))

print('x  = %.15f'%(r[0]))
print('y  = %.15f'%(r[1]))
print('z  = %.15f'%(r[2]))

print('vx = %.15f'%(v[0]))
print('vy = %.15f'%(v[1]))
print('vz = %.15f'%(v[2]))