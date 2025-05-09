import numpy as np
import matplotlib.pyplot as plt
import os

if not os.path.isdir('plots/'):
    os.mkdir('plots/')

t = np.loadtxt('io/time.txt') #[days]
pos = np.loadtxt('io/rel_pos.txt') #[km]
rpy1 = np.loadtxt('io/euler_rpy1.txt') #[deg]
rpy2 = np.loadtxt('io/euler_rpy2.txt') #[deg]
ener_mom = np.loadtxt('io/ener_mom_rel_error.txt') #[ ]

################################################################################

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('rel x [km]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, pos[:,0])
plt.savefig('plots/relx.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('rel y [km]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, pos[:,1])
plt.savefig('plots/rely.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('rel z [km]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, pos[:,2])
plt.savefig('plots/relz.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('rel dist [km]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, pos[:,3])
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
plt.ylabel('energy error [ ]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, ener_mom[:,0])
plt.savefig('plots/ener.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('momentum magnitude error [ ]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, ener_mom[:,1])
plt.savefig('plots/mom.png')

################################################################################

plt.show()
