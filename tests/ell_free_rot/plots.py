import numpy as np
import matplotlib.pyplot as plt
import os

if not os.path.isdir('plots/'):
    os.mkdir('plots/')

t = np.loadtxt('io/time.txt') #[sec]
rpy = np.loadtxt('io/euler_rpy.txt') #[deg]
quat = np.loadtxt('io/quaternion.txt') #[ ]
wb = np.loadtxt('io/ang_vel_wb.txt') #[rad/sec]
wi = np.loadtxt('io/ang_vel_wi.txt') #[rad/sec]
ener_mom = np.loadtxt('io/energy_momentum.txt') #E [kg*km^2/sec^2] and L [kg*km^2/sec]
ener_mom_rel_err = np.loadtxt('io/energy_momentum_rel_error.txt') #dE/E0 [ ] and dLx/Lx0, dLy/Ly0, dLz/Lz0 [ ]

################################################################################

plt.figure()
plt.xlabel('t [sec]')
plt.ylabel('roll [deg]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, rpy[:,0])
plt.savefig('plots/roll.png')

plt.figure()
plt.xlabel('t [sec]')
plt.ylabel('pitch [deg]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, rpy[:,1])
plt.savefig('plots/pitch.png')

plt.figure()
plt.xlabel('t [sec]')
plt.ylabel('yaw [deg]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, rpy[:,2])
plt.savefig('plots/yaw.png')

################################################################################

plt.figure()
plt.xlabel('t [sec]')
plt.ylabel('q0 [ ]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, quat[:,0])
plt.savefig('plots/quat0.png')

plt.figure()
plt.xlabel('t [sec]')
plt.ylabel('q1 [ ]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, quat[:,1])
plt.savefig('plots/quat1.png')

plt.figure()
plt.xlabel('t [sec]')
plt.ylabel('q2 [ ]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, quat[:,2])
plt.savefig('plots/quat2.png')

plt.figure()
plt.xlabel('t [sec]')
plt.ylabel('q3 [ ]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, quat[:,3])
plt.savefig('plots/quat3.png')

################################################################################

plt.figure()
plt.xlabel('t [sec]')
plt.ylabel('wix [rad/sec]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, wi[:,0])
plt.savefig('plots/wix.png')

plt.figure()
plt.xlabel('t [sec]')
plt.ylabel('wiy [rad/sec]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, wi[:,1])
plt.savefig('plots/wiy.png')

plt.figure()
plt.xlabel('t [sec]')
plt.ylabel('wiz [rad/sec]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, wi[:,2])
plt.savefig('plots/wiz.png')

################################################################################

plt.figure()
plt.xlabel('t [sec]')
plt.ylabel('wbx [rad/sec]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, wb[:,0])
plt.savefig('plots/wbx.png')

plt.figure()
plt.xlabel('t [sec]')
plt.ylabel('wby [rad/sec]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, wb[:,1])
plt.savefig('plots/wby.png')

plt.figure()
plt.xlabel('t [sec]')
plt.ylabel('wbz [rad/sec]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, wb[:,2])
plt.savefig('plots/wbz.png')

################################################################################

plt.figure()
plt.xlabel('t [sec]')
plt.ylabel('E [kg*km^2/sec^2]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, ener_mom[:,0])
plt.savefig('plots/ener.png')

plt.figure()
plt.xlabel('t [sec]')
plt.ylabel('Lx [kg*km^2/sec]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, ener_mom[:,1])
plt.savefig('plots/momx.png')

plt.figure()
plt.xlabel('t [sec]')
plt.ylabel('Ly [kg*km^2/sec]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, ener_mom[:,2])
plt.savefig('plots/momy.png')

plt.figure()
plt.xlabel('t [sec]')
plt.ylabel('Lz [kg*km^2/sec]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, ener_mom[:,3])
plt.savefig('plots/momz.png')

plt.figure()
plt.xlabel('t [sec]')
plt.ylabel('dE/E0')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, ener_mom_rel_err[:,0])
plt.savefig('plots/dener.png')

plt.figure()
plt.xlabel('t [sec]')
plt.ylabel('dLx/Lx0')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, ener_mom_rel_err[:,1])
plt.savefig('plots/dmomx.png')

plt.figure()
plt.xlabel('t [sec]')
plt.ylabel('dLy/Ly0')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, ener_mom_rel_err[:,2])
plt.savefig('plots/dmomy.png')

plt.figure()
plt.xlabel('t [sec]')
plt.ylabel('dLz/Lz0')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, ener_mom_rel_err[:,3])
plt.savefig('plots/dmomz.png')

################################################################################

#plt.show()
