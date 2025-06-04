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

mme_t = np.loadtxt('io/mme_time.txt') #[days]
mme_pos = np.loadtxt('io/mme_rel_pos.txt') #[km]
mme_kep = np.loadtxt('io/mme_rel_kep.txt')
mme_rpy1 = np.loadtxt('io/mme_euler_rpy1.txt') #[deg]
mme_rpy2 = np.loadtxt('io/mme_euler_rpy2.txt') #[deg]
mme_w1i = np.loadtxt('io/mme_ang_vel_w1i.txt') #[rad/sec]
mme_w2i = np.loadtxt('io/mme_ang_vel_w2i.txt') #[rad/sec]
mme_ener_mom_rel_err = np.loadtxt('io/mme_ener_mom_rel_error.txt') #|(E[i+1]-E[0])/E[0]| [ ] and |(L[i+1]-L[0])/L[0]| [ ]

mmo2_t = np.loadtxt('io/mmo2_time.txt') #[days]
mmo2_pos = np.loadtxt('io/mmo2_rel_pos.txt') #[km]
mmo2_kep = np.loadtxt('io/mmo2_rel_kep.txt')
mmo2_rpy1 = np.loadtxt('io/mmo2_euler_rpy1.txt') #[deg]
mmo2_rpy2 = np.loadtxt('io/mmo2_euler_rpy2.txt') #[deg]
mmo2_w1i = np.loadtxt('io/mmo2_ang_vel_w1i.txt') #[rad/sec]
mmo2_w2i = np.loadtxt('io/mmo2_ang_vel_w2i.txt') #[rad/sec]
mmo2_ener_mom_rel_err = np.loadtxt('io/mmo2_ener_mom_rel_error.txt') #|(E[i+1]-E[0])/E[0]| [ ] and |(L[i+1]-L[0])/L[0]| [ ]

mmo3_t = np.loadtxt('io/mmo3_time.txt') #[days]
mmo3_pos = np.loadtxt('io/mmo3_rel_pos.txt') #[km]
mmo3_kep = np.loadtxt('io/mmo3_rel_kep.txt')
mmo3_rpy1 = np.loadtxt('io/mmo3_euler_rpy1.txt') #[deg]
mmo3_rpy2 = np.loadtxt('io/mmo3_euler_rpy2.txt') #[deg]
mmo3_w1i = np.loadtxt('io/mmo3_ang_vel_w1i.txt') #[rad/sec]
mmo3_w2i = np.loadtxt('io/mmo3_ang_vel_w2i.txt') #[rad/sec]
mmo3_ener_mom_rel_err = np.loadtxt('io/mmo3_ener_mom_rel_error.txt') #|(E[i+1]-E[0])/E[0]| [ ] and |(L[i+1]-L[0])/L[0]| [ ]

mmo4_t = np.loadtxt('io/mmo4_time.txt') #[days]
mmo4_pos = np.loadtxt('io/mmo4_rel_pos.txt') #[km]
mmo4_kep = np.loadtxt('io/mmo4_rel_kep.txt')
mmo4_rpy1 = np.loadtxt('io/mmo4_euler_rpy1.txt') #[deg]
mmo4_rpy2 = np.loadtxt('io/mmo4_euler_rpy2.txt') #[deg]
mmo4_w1i = np.loadtxt('io/mmo4_ang_vel_w1i.txt') #[rad/sec]
mmo4_w2i = np.loadtxt('io/mmo4_ang_vel_w2i.txt') #[rad/sec]
mmo4_ener_mom_rel_err = np.loadtxt('io/mmo4_ener_mom_rel_error.txt') #|(E[i+1]-E[0])/E[0]| [ ] and |(L[i+1]-L[0])/L[0]| [ ]

################################################################################

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('a [km]')
plt.title('Relative semi-major axis')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(mmo2_t, mmo2_kep[:,0], label = 'Order 2')
plt.plot(mmo2_t, mmo3_kep[:,0], label = 'Order 3')
plt.plot(mmo2_t, mmo4_kep[:,0], label = 'Order 4')
plt.plot(mmo2_t, mme_kep[:,0], '--', label = 'Mascons')
plt.xticks(np.arange(0, 21, 5))
plt.legend(loc = 'best')
plt.tight_layout()
plt.savefig('plots/all_relsma.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('e [ ]')
plt.title('Relative eccentricity')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(mmo2_t, mmo2_kep[:,1], label = 'Order 2')
plt.plot(mmo2_t, mmo3_kep[:,1], label = 'Order 3')
plt.plot(mmo2_t, mmo4_kep[:,1], label = 'Order 4')
plt.plot(mmo2_t, mme_kep[:,1], '--', label = 'Mascons')
plt.xticks(np.arange(0, 21, 5))
plt.legend(loc = 'best')
plt.tight_layout()
plt.savefig('plots/all_relecc.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('i [deg]')
plt.title('Relative inclination')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(mmo2_t, mmo2_kep[:,2], label = 'Order 2')
plt.plot(mmo2_t, mmo3_kep[:,2], label = 'Order 3')
plt.plot(mmo2_t, mmo4_kep[:,2], label = 'Order 4')
plt.plot(mmo2_t, mme_kep[:,2], '--', label = 'Mascons')
plt.xticks(np.arange(0, 21, 5))
plt.legend(loc = 'best')
plt.tight_layout()
plt.savefig('plots/all_relinc.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('x [km]')
plt.title('Relative x')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(mmo2_t, mmo2_pos[:,0], label = 'Order 2')
plt.plot(mmo3_t, mmo3_pos[:,0], label = 'Order 3')
plt.plot(mmo4_t, mmo4_pos[:,0], label = 'Order 4')
plt.plot(mme_t, mme_pos[:,0], '--', label = 'Mascons')
plt.xticks(np.arange(0, 21, 5))
plt.legend(loc = 'best')
plt.tight_layout()
plt.savefig('plots/all_relx.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('y [km]')
plt.title('Relative y')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(mmo2_t, mmo2_pos[:,1], label = 'Order 2')
plt.plot(mmo3_t, mmo3_pos[:,1], label = 'Order 3')
plt.plot(mmo4_t, mmo4_pos[:,1], label = 'Order 4')
plt.plot(mme_t, mme_pos[:,1], '--', label = 'Mascons')
plt.xticks(np.arange(0, 21, 5))
plt.legend(loc = 'best')
plt.tight_layout()
plt.savefig('plots/all_rely.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('z [km]')
plt.title('Relative z')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(mmo2_t, mmo2_pos[:,2], label = 'Order 2', zorder=10)
plt.plot(mmo3_t, mmo3_pos[:,2], label = 'Order 3')
plt.plot(mmo4_t, mmo4_pos[:,2], label = 'Order 4')
plt.plot(mme_t, mme_pos[:,2], '--', label = 'Mascons')
plt.xticks(np.arange(0, 21, 5))
plt.legend(loc = 'best')
plt.tight_layout()
plt.savefig('plots/all_relz.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('r [km]')
plt.title('Relative distance')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(mmo2_t, mmo2_pos[:,3], label = 'Order 2')
plt.plot(mmo3_t, mmo3_pos[:,3], label = 'Order 3')
plt.plot(mmo4_t, mmo4_pos[:,3], label = 'Order 4')
plt.plot(mme_t, mme_pos[:,3], '--', label = 'Mascons')
plt.xticks(np.arange(0, 21, 5))
plt.legend(loc = 'best')
plt.tight_layout()
plt.savefig('plots/all_rel_dist.png')

################################################################################

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('roll 1 [deg]')
plt.title('Roll 1 angle')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(mmo2_t, mmo2_rpy1[:,0], label = 'Order 2', zorder=10)
plt.plot(mmo3_t, mmo3_rpy1[:,0], label = 'Order 3')
plt.plot(mmo4_t, mmo4_rpy1[:,0], label = 'Order 4')
plt.plot(mme_t, mme_rpy1[:,0], '--', label = 'Mascons')
plt.xticks(np.arange(0, 21, 5))
plt.legend(loc = 'best')
plt.tight_layout()
plt.savefig('plots/all_roll1.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('pitch 1 [deg]')
plt.title('Pitch 1 angle')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(mmo2_t, mmo2_rpy1[:,1], label = 'Order 2', zorder=10)
plt.plot(mmo3_t, mmo3_rpy1[:,1], label = 'Order 3')
plt.plot(mmo4_t, mmo4_rpy1[:,1], label = 'Order 4')
plt.plot(mme_t, mme_rpy1[:,1], '--', label = 'Mascons')
plt.xticks(np.arange(0, 21, 5))
plt.legend(loc = 'best')
plt.tight_layout()
plt.savefig('plots/all_pitch1.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('yaw 1 [deg]')
plt.title('Yaw 1 angle')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(mmo2_t, mmo2_rpy1[:,2], label = 'Order 2')
plt.plot(mmo3_t, mmo3_rpy1[:,2], label = 'Order 3')
plt.plot(mmo4_t, mmo4_rpy1[:,2], label = 'Order 4')
plt.plot(mme_t, mme_rpy1[:,2], '--', label = 'Mascons')
plt.xticks(np.arange(0, 21, 5))
plt.legend(loc = 'best')
plt.tight_layout()
plt.savefig('plots/all_yaw1.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('rel-yaw 1 [deg]')
plt.title('Relative yaw 1 angle')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(mmo2_t, mmo2_rpy1[:,3], label = 'Order 2')
plt.plot(mmo3_t, mmo3_rpy1[:,3], label = 'Order 3')
plt.plot(mmo4_t, mmo4_rpy1[:,3], label = 'Order 4')
plt.plot(mme_t, mme_rpy1[:,3], '--', label = 'Mascons')
plt.xticks(np.arange(0, 21, 5))
plt.legend(loc = 'best')
plt.tight_layout()
plt.savefig('plots/all_yaw1rel.png')

################################################################################

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('roll 2 [deg]')
plt.title('Roll 2 angle')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(mmo2_t, mmo2_rpy2[:,0], label = 'Order 2')
plt.plot(mmo3_t, mmo3_rpy2[:,0], label = 'Order 3')
plt.plot(mmo4_t, mmo4_rpy2[:,0], label = 'Order 4')
plt.plot(mme_t, mme_rpy2[:,0], '--', label = 'Mascons')
plt.xticks(np.arange(0, 21, 5))
plt.legend(loc = 'best')
plt.tight_layout()
plt.savefig('plots/all_roll2.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('pitch 2 [deg]')
plt.title('Pitch 2 angle')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(mmo2_t, mmo2_rpy2[:,1], label = 'Order 2')
plt.plot(mmo3_t, mmo3_rpy2[:,1], label = 'Order 3')
plt.plot(mmo4_t, mmo4_rpy2[:,1], label = 'Order 4')
plt.plot(mme_t, mme_rpy2[:,1], '--', label = 'Mascons')
plt.xticks(np.arange(0, 21, 5))
plt.legend(loc = 'best')
plt.tight_layout()
plt.savefig('plots/all_pitch2.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('yaw 2 [deg]')
plt.title('Yaw 2 angle')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(mmo2_t, mmo2_rpy2[:,2], label = 'Order 2')
plt.plot(mmo3_t, mmo3_rpy2[:,2], label = 'Order 3')
plt.plot(mmo4_t, mmo4_rpy2[:,2], label = 'Order 4')
plt.plot(mme_t, mme_rpy2[:,2], '--', label = 'Mascons')
plt.xticks(np.arange(0, 21, 5))
plt.legend(loc = 'best')
plt.tight_layout()
plt.savefig('plots/all_yaw2.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('rel-yaw 2 [deg]')
plt.title('Relative yaw 2 angle')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(mmo2_t, mmo2_rpy2[:,3], label = 'Order 2')
plt.plot(mmo3_t, mmo3_rpy2[:,3], label = 'Order 3')
plt.plot(mmo4_t, mmo4_rpy2[:,3], label = 'Order 4')
plt.plot(mme_t, mme_rpy2[:,3], '--', label = 'Mascons')
plt.xticks(np.arange(0, 21, 5))
plt.legend(loc = 'best')
plt.tight_layout()
plt.savefig('plots/all_yaw2rel.png')

################################################################################

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('ω1x [rad/sec]')
plt.title('x - ang. vel. 1 (inertial)')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(mmo2_t, mmo2_w1i[:,0], label = 'Order 2')
plt.plot(mmo3_t, mmo3_w1i[:,0], label = 'Order 3')
plt.plot(mmo4_t, mmo4_w1i[:,0], label = 'Order 4')
plt.plot(mme_t, mme_w1i[:,0], '--', label = 'Mascons')
plt.xticks(np.arange(0, 21, 5))
plt.tight_layout()
plt.savefig('plots/all_angvel1ix.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('ω1y [rad/sec]')
plt.title('y - ang. vel. 1 (inertial)')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(mmo2_t, mmo2_w1i[:,1], label = 'Order 2')
plt.plot(mmo3_t, mmo3_w1i[:,1], label = 'Order 3')
plt.plot(mmo4_t, mmo4_w1i[:,1], label = 'Order 4')
plt.plot(mme_t, mme_w1i[:,1], '--', label = 'Mascons')
plt.xticks(np.arange(0, 21, 5))
plt.tight_layout()
plt.savefig('plots/all_angvel1iy.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('ω1z [rad/sec]')
plt.title('z - ang. vel. 1 (inertial)')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(mmo2_t, mmo2_w1i[:,2], label = 'Order 2')
plt.plot(mmo3_t, mmo3_w1i[:,2], label = 'Order 3')
plt.plot(mmo4_t, mmo4_w1i[:,2], label = 'Order 4')
plt.plot(mme_t, mme_w1i[:,2], '--', label = 'Mascons')
plt.xticks(np.arange(0, 21, 5))
plt.tight_layout()
plt.savefig('plots/all_angvel1iz.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('ω2x [rad/sec]')
plt.title('x - ang. vel. 2 (inertial)')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(mmo2_t, mmo2_w2i[:,0], label = 'Order 2')
plt.plot(mmo3_t, mmo3_w2i[:,0], label = 'Order 3')
plt.plot(mmo4_t, mmo4_w2i[:,0], label = 'Order 4')
plt.plot(mme_t, mme_w2i[:,0], '--', label = 'Mascons')
plt.xticks(np.arange(0, 21, 5))
plt.tight_layout()
plt.savefig('plots/all_angvel2ix.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('ω2y [rad/sec]')
plt.title('y - ang. vel. 2 (inertial)')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(mmo2_t, mmo2_w2i[:,1], label = 'Order 2')
plt.plot(mmo3_t, mmo3_w2i[:,1], label = 'Order 3')
plt.plot(mmo4_t, mmo4_w2i[:,1], label = 'Order 4')
plt.plot(mme_t, mme_w2i[:,1], '--', label = 'Mascons')
plt.xticks(np.arange(0, 21, 5))
plt.tight_layout()
plt.savefig('plots/all_angvel2iy.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('ω2z [rad/sec]')
plt.title('z - ang. vel. 2 (inertial)')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(mmo2_t, mmo2_w2i[:,2], label = 'Order 2')
plt.plot(mmo3_t, mmo3_w2i[:,2], label = 'Order 3')
plt.plot(mmo4_t, mmo4_w2i[:,2], label = 'Order 4')
plt.plot(mme_t, mme_w2i[:,2], '--', label = 'Mascons')
plt.xticks(np.arange(0, 21, 5))
plt.tight_layout()
plt.savefig('plots/all_angvel2iz.png')

################################################################################

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('| (E[i+1] - E[0])/E[0] |')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(mmo2_t, mmo2_ener_mom_rel_err[:,0], label = 'Order 2')
plt.plot(mmo3_t, mmo3_ener_mom_rel_err[:,0], label = 'Order 3')
plt.plot(mmo4_t, mmo4_ener_mom_rel_err[:,0], label = 'Order 4')
plt.plot(mme_t, mme_ener_mom_rel_err[:,0], label = 'Mascons')
plt.xticks(np.arange(0, 21, 5))
plt.legend(loc = 'best')
plt.tight_layout()
plt.savefig('plots/all_dener.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('| (L[i+1] - L[0])/L[0] |')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(mmo2_t, mmo2_ener_mom_rel_err[:,1], label = 'Order 2')
plt.plot(mmo3_t, mmo3_ener_mom_rel_err[:,1], label = 'Order 3')
plt.plot(mmo4_t, mmo4_ener_mom_rel_err[:,1], label = 'Order 4')
plt.plot(mme_t, mme_ener_mom_rel_err[:,1], label = 'Mascons')
plt.xticks(np.arange(0, 21, 5))
plt.legend(loc = 'best')
plt.tight_layout()
plt.savefig('plots/all_dmom.png')

################################################################################

#plt.show()