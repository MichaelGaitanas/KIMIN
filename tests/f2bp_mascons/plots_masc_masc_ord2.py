import numpy as np
import matplotlib.pyplot as plt
import os

import matplotlib as mpl

mpl.rcParams['axes.titlesize']   = 18   # title font size
mpl.rcParams['axes.labelsize']   = 14   # x/y‐axis label font size
mpl.rcParams['xtick.labelsize']  = 14   # x‐tick numbers
mpl.rcParams['ytick.labelsize']  = 14   # y‐tick numbers

if not os.path.isdir('plots/'):
    os.mkdir('plots/')

t = np.loadtxt('io/mmo2_time.txt') #[days]
pos = np.loadtxt('io/mmo2_rel_pos.txt') #[km]
kep = np.loadtxt('io/mmo2_rel_kep.txt') #[km], [ ], [deg]
rpy1 = np.loadtxt('io/mmo2_euler_rpy1.txt') #[deg]
rpy2 = np.loadtxt('io/mmo2_euler_rpy2.txt') #[deg]
w1i = np.loadtxt('io/mmo2_ang_vel_w1i.txt') #[rad/sec]
w2i = np.loadtxt('io/mmo2_ang_vel_w2i.txt') #[rad/sec]
ener_mom_rel_err = np.loadtxt('io/mmo2_ener_mom_rel_error.txt') #|(E[i+1]-E[0])/E[0]| [ ] and |(L[i+1]-L[0])/L[0]| [ ]

################################################################################

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('a [km]')
plt.title('Relative semi-major axis')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, kep[:,0])
plt.xticks(np.arange(0, 11, 1))
plt.tight_layout()
plt.savefig('plots/mmo2_relsma.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('e [ ]')
plt.title('Relative eccentricity')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, kep[:,1])
plt.xticks(np.arange(0, 11, 1))
plt.tight_layout()
plt.savefig('plots/mmo2_relecc.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('i [deg]')
plt.title('Relative inclination')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, kep[:,2])
plt.xticks(np.arange(0, 11, 1))
plt.tight_layout()
plt.savefig('plots/mmo2_relinc.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('x [km]')
plt.title('Relative x')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, pos[:,0])
plt.xticks(np.arange(0, 11, 1))
plt.tight_layout()
plt.savefig('plots/mmo2_relx.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('y [km]')
plt.title('Relative y')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, pos[:,1])
plt.xticks(np.arange(0, 11, 1))
plt.tight_layout()
plt.savefig('plots/mmo2_rely.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('z [km]')
plt.title('Relative z')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, pos[:,2])
plt.xticks(np.arange(0, 11, 1))
plt.tight_layout()
plt.savefig('plots/mmo2_relz.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('r [km]')
plt.title('Relative distance')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, pos[:,3])
plt.xticks(np.arange(0, 11, 1))
plt.tight_layout()
plt.savefig('plots/mmo2_rel_dist.png')

################################################################################

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('roll 1 [deg]')
plt.title('Roll 1 angle')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, rpy1[:,0])
plt.xticks(np.arange(0, 11, 1))
plt.tight_layout()
plt.savefig('plots/mmo2_roll1.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('pitch 1 [deg]')
plt.title('Pitch 1 angle')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, rpy1[:,1])
plt.xticks(np.arange(0, 11, 1))
plt.tight_layout()
plt.savefig('plots/mmo2_pitch1.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('yaw 1 [deg]')
plt.title('Yaw 1 angle')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, rpy1[:,2])
plt.xticks(np.arange(0, 11, 1))
plt.tight_layout()
plt.savefig('plots/mmo2_yaw1.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('rel-yaw 1 [deg]')
plt.title('Relative yaw 1 angle')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, rpy1[:,3])
plt.xticks(np.arange(0, 11, 1))
plt.tight_layout()
plt.savefig('plots/mmo2_yaw1rel.png')

################################################################################

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('roll 2 [deg]')
plt.title('Roll 2 angle')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, rpy2[:,0])
plt.xticks(np.arange(0, 11, 1))
plt.tight_layout()
plt.savefig('plots/mmo2_roll2.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('pitch 2 [deg]')
plt.title('Pitch 2 angle')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, rpy2[:,1])
plt.xticks(np.arange(0, 11, 1))
plt.tight_layout()
plt.savefig('plots/mmo2_pitch2.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('yaw 2 [deg]')
plt.title('Yaw 2 angle')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, rpy2[:,2])
plt.xticks(np.arange(0, 11, 1))
plt.tight_layout()
plt.savefig('plots/mmo2_yaw2.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('rel-yaw 2 [deg]')
plt.title('Relative yaw 2 angle (libration)')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, rpy2[:,3])
plt.xticks(np.arange(0, 11, 1))
plt.tight_layout()
plt.savefig('plots/mmo2_yaw2rel.png')

################################################################################

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('ω1x [rad/sec]')
plt.title('x - ang. vel. 1 (inertial)')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, w1i[:,0])
plt.xticks(np.arange(0, 11, 1))
plt.tight_layout()
plt.savefig('plots/mmo2_angvel1ix.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('ω1y [rad/sec]')
plt.title('y - ang. vel. 1 (inertial)')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, w1i[:,1])
plt.xticks(np.arange(0, 11, 1))
plt.tight_layout()
plt.savefig('plots/mmo2_angvel1iy.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('ω1z [rad/sec]')
plt.title('z - ang. vel. 1 (inertial)')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, w1i[:,2])
plt.xticks(np.arange(0, 11, 1))
plt.tight_layout()
plt.savefig('plots/mmo2_angvel1iz.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('ω2x [rad/sec]')
plt.title('x - ang. vel. 2 (inertial)')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, w2i[:,0])
plt.xticks(np.arange(0, 11, 1))
plt.tight_layout()
plt.savefig('plots/mmo2_angvel2ix.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('ω2y [rad/sec]')
plt.title('y - ang. vel. 2 (inertial)')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, w2i[:,1])
plt.xticks(np.arange(0, 11, 1))
plt.tight_layout()
plt.savefig('plots/mmo2_angvel2iy.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('ω2z [rad/sec]')
plt.title('z - ang. vel. 2 (inertial)')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, w2i[:,2])
plt.xticks(np.arange(0, 11, 1))
plt.tight_layout()
plt.savefig('plots/mmo2_angvel2iz.png')

################################################################################

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('roll 2 [deg]')
plt.title('Roll 2 angle')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, rpy2[:,0])
plt.xticks(np.arange(0, 11, 1))
plt.tight_layout()
plt.savefig('plots/mmo2_roll2.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('pitch 2 [deg]')
plt.title('Pitch 2 angle')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, rpy2[:,1])
plt.xticks(np.arange(0, 11, 1))
plt.tight_layout()
plt.savefig('plots/mmo2_pitch2.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('yaw 2 [deg]')
plt.title('Yaw 2 angle')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, rpy2[:,2])
plt.xticks(np.arange(0, 11, 1))
plt.tight_layout()
plt.savefig('plots/mmo2_yaw2.png')

################################################################################

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('| (E[i+1] - E[0])/E[0] |')
plt.title('Energy conservation')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, ener_mom_rel_err[:,0])
plt.xticks(np.arange(0, 11, 1))
plt.tight_layout()
plt.savefig('plots/mmo2_dener.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('| (L[i+1] - L[0])/L[0] |')
plt.title('Momentum magnitude conservation')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, ener_mom_rel_err[:,1])
plt.xticks(np.arange(0, 11, 1))
plt.tight_layout()
plt.savefig('plots/mmo2_dmom.png')

#plt.show()
