import numpy as np
import matplotlib.pyplot as plt
import os

if not os.path.isdir('plots/'):
    os.mkdir('plots/')

t = np.loadtxt('io/mmo3_time.txt') #[days]
pos = np.loadtxt('io/mmo3_rel_pos.txt') #[km]
rpy1 = np.loadtxt('io/mmo3_euler_rpy1.txt') #[deg]
rpy2 = np.loadtxt('io/mmo3_euler_rpy2.txt') #[deg]
ener_mom_rel_err = np.loadtxt('io/mmo3_ener_mom_rel_error.txt') #dE/E0 [ ] and dL/L0 [ ]

################################################################################

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('rel z [km]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, pos[:,2])
plt.savefig('plots/mmo3_relz.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('rel dist [km]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, pos[:,3])
plt.savefig('plots/mmo3_rel_dist.png')

################################################################################

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('roll1 [deg]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, rpy1[:,0])
plt.savefig('plots/mmo3_roll1.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('pitch1 [deg]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, rpy1[:,1])
plt.savefig('plots/mmo3_pitch1.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('yaw1 [deg]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, rpy1[:,2])
plt.savefig('plots/mmo3_yaw1.png')

################################################################################

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('roll2 [deg]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, rpy2[:,0])
plt.savefig('plots/mmo3_roll2.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('pitch2 [deg]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, rpy2[:,1])
plt.savefig('plots/mmo3_pitch2.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('yaw2 [deg]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, rpy2[:,2])
plt.savefig('plots/mmo3_yaw2.png')

################################################################################

plt.figure()
plt.xlabel('t [sec]')
plt.ylabel('dE/E0')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, ener_mom_rel_err[:,0])
plt.savefig('plots/mmo3_dener.png')

plt.figure()
plt.xlabel('t [sec]')
plt.ylabel('dL/L0')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, ener_mom_rel_err[:,1])
plt.savefig('plots/mmo3_dmom.png')

################################################################################

#plt.show()
