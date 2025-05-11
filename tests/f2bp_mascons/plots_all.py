import numpy as np
import matplotlib.pyplot as plt
import os

if not os.path.isdir('plots/'):
    os.mkdir('plots/')

mme_t = np.loadtxt('io/mme_time.txt') #[days]
mme_pos = np.loadtxt('io/mme_rel_pos.txt') #[km]
mme_rpy1 = np.loadtxt('io/mme_euler_rpy1.txt') #[deg]
mme_rpy2 = np.loadtxt('io/mme_euler_rpy2.txt') #[deg]
mme_ener_mom_rel_err = np.loadtxt('io/mme_ener_mom_rel_error.txt') #dE/E0 [ ] and dL/L0 [ ]

mme_x = mme_pos[:,0]
mme_y = mme_pos[:,1]
mme_yaw2 = mme_rpy2[:,2]
mme_theta = (np.degrees(np.arctan2(mme_y, mme_x)) + 180)%360 - 180
mme_relyaw2 = mme_yaw2 - mme_theta

mmo2_t = np.loadtxt('io/mmo2_time.txt') #[days]
mmo2_pos = np.loadtxt('io/mmo2_rel_pos.txt') #[km]
mmo2_rpy1 = np.loadtxt('io/mmo2_euler_rpy1.txt') #[deg]
mmo2_rpy2 = np.loadtxt('io/mmo2_euler_rpy2.txt') #[deg]
mmo2_ener_mom_rel_err = np.loadtxt('io/mmo2_ener_mom_rel_error.txt') #dE/E0 [ ] and dL/L0 [ ]

mmo3_t = np.loadtxt('io/mmo3_time.txt') #[days]
mmo3_pos = np.loadtxt('io/mmo3_rel_pos.txt') #[km]
mmo3_rpy1 = np.loadtxt('io/mmo3_euler_rpy1.txt') #[deg]
mmo3_rpy2 = np.loadtxt('io/mmo3_euler_rpy2.txt') #[deg]
mmo3_ener_mom_rel_err = np.loadtxt('io/mmo3_ener_mom_rel_error.txt') #dE/E0 [ ] and dL/L0 [ ]

mmo4_t = np.loadtxt('io/mmo4_time.txt') #[days]
mmo4_pos = np.loadtxt('io/mmo4_rel_pos.txt') #[km]
mmo4_rpy1 = np.loadtxt('io/mmo4_euler_rpy1.txt') #[deg]
mmo4_rpy2 = np.loadtxt('io/mmo4_euler_rpy2.txt') #[deg]
mmo4_ener_mom_rel_err = np.loadtxt('io/mmo4_ener_mom_rel_error.txt') #dE/E0 [ ] and dL/L0 [ ]

################################################################################

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('rel z [km]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(mmo2_t, mmo2_pos[:,2], label = 'Order 2')
plt.plot(mmo3_t, mmo3_pos[:,2], label = 'Order 3')
plt.plot(mmo4_t, mmo4_pos[:,2], label = 'Order 4')
plt.plot(mme_t, mme_pos[:,2], label = 'Mascons')
plt.legend(loc = 'best')
plt.savefig('plots/relz.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('rel dist [km]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(mmo2_t, mmo2_pos[:,3], label = 'Order 2')
plt.plot(mmo3_t, mmo3_pos[:,3], label = 'Order 3')
plt.plot(mmo4_t, mmo4_pos[:,3], label = 'Order 4')
plt.plot(mme_t, mme_pos[:,3], label = 'Mascons')
plt.legend(loc = 'best')
plt.savefig('plots/rel_dist.png')

################################################################################

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('roll1 [deg]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(mmo2_t, mmo2_rpy1[:,0], label = 'Order 2')
plt.plot(mmo3_t, mmo3_rpy1[:,0], label = 'Order 3')
plt.plot(mmo4_t, mmo4_rpy1[:,0], label = 'Order 4')
plt.plot(mme_t, mme_rpy1[:,0], label = 'Mascons')
plt.legend(loc = 'best')
plt.savefig('plots/roll1.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('pitch1 [deg]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(mmo2_t, mmo2_rpy1[:,1], label = 'Order 2')
plt.plot(mmo3_t, mmo3_rpy1[:,1], label = 'Order 3')
plt.plot(mmo4_t, mmo4_rpy1[:,1], label = 'Order 4')
plt.plot(mme_t, mme_rpy1[:,1], label = 'Mascons')
plt.legend(loc = 'best')
plt.savefig('plots/pitch1.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('yaw1 [deg]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(mmo2_t, mmo2_rpy1[:,2], label = 'Order 2')
plt.plot(mmo3_t, mmo3_rpy1[:,2], label = 'Order 3')
plt.plot(mmo4_t, mmo4_rpy1[:,2], label = 'Order 4')
plt.plot(mme_t, mme_rpy1[:,2], label = 'Mascons')
plt.legend(loc = 'best')
plt.savefig('plots/yaw1.png')

################################################################################

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('roll2 [deg]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(mmo2_t, mmo2_rpy2[:,0], label = 'Order 2')
plt.plot(mmo3_t, mmo3_rpy2[:,0], label = 'Order 3')
plt.plot(mmo4_t, mmo4_rpy2[:,0], label = 'Order 4')
plt.plot(mme_t, mme_rpy2[:,0], label = 'Mascons')
plt.legend(loc = 'best')
plt.savefig('plots/roll2.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('pitch2 [deg]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(mmo2_t, mmo2_rpy2[:,1], label = 'Order 2')
plt.plot(mmo3_t, mmo3_rpy2[:,1], label = 'Order 3')
plt.plot(mmo4_t, mmo4_rpy2[:,1], label = 'Order 4')
plt.plot(mme_t, mme_rpy2[:,1], label = 'Mascons')
plt.legend(loc = 'best')
plt.savefig('plots/pitch2.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('yaw2 [deg]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(mmo2_t, mmo2_rpy2[:,2], label = 'Order 2')
plt.plot(mmo3_t, mmo3_rpy2[:,2], label = 'Order 3')
plt.plot(mmo4_t, mmo4_rpy2[:,2], label = 'Order 4')
plt.plot(mme_t, mme_rpy2[:,2], label = 'Mascons')
plt.legend(loc = 'best')
plt.savefig('plots/yaw2.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('relyaw2 [deg]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(mme_t, mme_relyaw2, label = 'Mascons')
plt.legend(loc = 'best')
plt.savefig('plots/relyaw2.png')

################################################################################

plt.figure()
plt.xlabel('t [sec]')
plt.ylabel('dE/E0')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(mmo2_t, mmo2_ener_mom_rel_err[:,0], label = 'Order 2')
plt.plot(mmo3_t, mmo3_ener_mom_rel_err[:,0], label = 'Order 3')
plt.plot(mmo4_t, mmo4_ener_mom_rel_err[:,0], label = 'Order 4')
plt.plot(mme_t, mme_ener_mom_rel_err[:,0], label = 'Mascons')
plt.legend(loc = 'best')
plt.savefig('plots/dener.png')

plt.figure()
plt.xlabel('t [sec]')
plt.ylabel('dL/L0')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(mmo2_t, mmo2_ener_mom_rel_err[:,1], label = 'Order 2')
plt.plot(mmo3_t, mmo3_ener_mom_rel_err[:,1], label = 'Order 3')
plt.plot(mmo4_t, mmo4_ener_mom_rel_err[:,1], label = 'Order 4')
plt.plot(mme_t, mme_ener_mom_rel_err[:,1], label = 'Mascons')
plt.legend(loc = 'best')
plt.savefig('plots/dmom.png')

################################################################################

#plt.show()