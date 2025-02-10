import math as ma
import pykep as pk

#Perform unsigned modulo 2*pi to the argument 'angle'.
def wrap_to_2pi(angle):
    angle = ma.fmod(angle, 2.0*ma.pi)
    if angle < 0.0:
        angle += 2.0*ma.pi
    return angle

def E2M(E, e):
    return wrap_to_2pi(E - e*ma.sin(E))

#Calculate the mean anomaly, given the hyperbolic anomaly and the eccentricity.
def H2M(H, e):
    return e*ma.sinh(H) - H #Here we do not wrap in 2*pi, because the hyperbolic mean anomaly is not a periodic angle (unlike in the elliptical case).

#Invert the Gudermannian for e > 1.
def Gudermannian2H(gd):
    z = ma.tan(gd/2)
    return ma.log((1 + z)/(1 - z))

G = 6.67430e-20; #[km^3/(kg*sec^2)]
M1 = 5.320591856403073e15 #[kg]
M2 = 4.940814359692687e14 #[kg]

x  = 3 #[km]
y  = 0.32 #km]
z  = -1 #[km]
vx = 0.001 #[km/sec]
vy = 0.011373781832982 #[km/sec]
vz = -0.01 #[km/sec]

kep = pk.ic2par([x,y,z],[vx,vy,vz], G*(M1+M2))

if kep[1] < 1:
    print('a  = %.15f'%(kep[0]))
    print('e  = %.15f'%(kep[1]))
    print('i  = %.15f'%(kep[2]*180/ma.pi))
    print('Om = %.15f'%(kep[3]*180/ma.pi))
    print('w  = %.15f'%(kep[4]*180/ma.pi))
    print('M  = %.15f'%(E2M(kep[5], kep[1])*180/ma.pi))
else:
    M = H2M(Gudermannian2H(kep[5]), kep[1])*180/ma.pi
    print('a  = %.15f'%(kep[0]))
    print('e  = %.15f'%(kep[1]))
    print('i  = %.15f'%(kep[2]*180/ma.pi))
    print('Om = %.15f'%(kep[3]*180/ma.pi))
    print('w  = %.15f'%(kep[4]*180/ma.pi))
    print('M  = %.15f'%(M))