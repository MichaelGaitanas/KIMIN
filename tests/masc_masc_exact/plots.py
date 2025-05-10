import numpy as np
import matplotlib.pyplot as plt
import os

if not os.path.isdir('plots/'):
    os.mkdir('plots/')

t = np.loadtxt('io/time_parallel.txt')/86400.0 #[days]
rv = np.loadtxt('io/pos_vel_parallel.txt') #[km]
rpy1 = np.loadtxt('io/roll_pitch_yaw_1_parallel.txt')*180/np.pi #[deg]
w1b = np.loadtxt('io/w1_body_parallel.txt') #[rad/sec]
w1i = np.loadtxt('io/w1_inertial_parallel.txt') #[rad/sec]
rpy2 = np.loadtxt('io/roll_pitch_yaw_2_parallel.txt')*180/np.pi #[deg]
w2b = np.loadtxt('io/w2_body_parallel.txt') #[rad/sec]
w2i = np.loadtxt('io/w2_inertial_parallel.txt') #[rad/sec]
ener_mom = np.loadtxt('io/energy_momentum_parallel.txt') #E [kg*km^2/sec^2] and L [kg*km^2/sec]

E = ener_mom[:,0]
dE = np.abs((E - E[0])/E[0])
L = np.sqrt(ener_mom[:,1]**2 + ener_mom[:,2]**2 + ener_mom[:,3]**2)
dL = np.abs((L - L[0])/L[0])

################################################################################

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('rel x [km]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, rv[:,0])
plt.savefig('plots/relx.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('rel y [km]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, rv[:,1])
plt.savefig('plots/rely.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('rel z [km]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, rv[:,2])
plt.savefig('plots/relz.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('rel dist [km]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, np.sqrt(rv[:,0]**2 + rv[:,1]**2 + rv[:,2]**2))
plt.savefig('plots/rel_dist.png')

################################################################################

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('roll1 [deg]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, rpy1[:,0])
plt.savefig('plots/roll1.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('pitch1 [deg]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, rpy1[:,1])
plt.savefig('plots/pitch1.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('yaw1 [deg]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, rpy1[:,2])
plt.savefig('plots/yaw1.png')

################################################################################

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('w1ix [rad/sec]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, w1i[:,0])
plt.savefig('plots/w1ix.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('w1iy [rad/sec]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, w1i[:,1])
plt.savefig('plots/w1iy.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('w1iz [rad/sec]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, w1i[:,2])
plt.savefig('plots/w1iz.png')

################################################################################

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('w1bx [rad/sec]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, w1b[:,0])
plt.savefig('plots/w1bx.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('w1by [rad/sec]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, w1b[:,1])
plt.savefig('plots/w1by.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('w1bz [rad/sec]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, w1b[:,2])
plt.savefig('plots/w1bz.png')

################################################################################

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('roll2 [deg]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, rpy2[:,0])
plt.savefig('plots/roll2.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('pitch2 [deg]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, rpy2[:,1])
plt.savefig('plots/pitch2.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('yaw2 [deg]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, rpy2[:,2])
plt.savefig('plots/yaw2.png')

################################################################################

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('w2ix [rad/sec]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, w2i[:,0])
plt.savefig('plots/w2ix.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('w2iy [rad/sec]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, w2i[:,1])
plt.savefig('plots/w2iy.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('w2iz [rad/sec]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, w2i[:,2])
plt.savefig('plots/w2iz.png')

################################################################################

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('w2bx [rad/sec]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, w2b[:,0])
plt.savefig('plots/w2bx.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('w2by [rad/sec]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, w2b[:,1])
plt.savefig('plots/w2by.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('w2bz [rad/sec]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, w2b[:,2])
plt.savefig('plots/w2bz.png')

################################################################################

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('ener [kg*km^2/sec^2]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, E)
plt.savefig('plots/ener.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('mom [kg*km^2/sec]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, L)
plt.savefig('plots/mom.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('dE/E0')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, dE)
plt.savefig('plots/dener.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('dL/L0')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, dL)
plt.savefig('plots/dmom.png')

################################################################################

#plt.show()
