import numpy as np
import matplotlib.pyplot as plt
import os

if not os.path.isdir('plots/'):
    os.mkdir('plots/')

t = np.loadtxt('io/time.txt')/86400.0 #[days]
rpy = np.loadtxt('io/rpy.txt')*180/np.pi #[deg]
rotmat = np.loadtxt('io/rot_mat.txt') #[ ]
quat = np.loadtxt('io/quat.txt') #[ ]
wb = np.loadtxt('io/wb.txt') #[rad/sec]
wi = np.loadtxt('io/wi.txt') #[rad/sec]
ener_mom = np.loadtxt('io/ener_mom.txt') #E [kg*km^2/sec^2] and L [kg*km^2/sec]

E = ener_mom[:,0]
dE = np.abs((E - E[0]))/E[0]
L = np.sqrt(ener_mom[:,1]**2 + ener_mom[:,2]**2 + ener_mom[:,3]**2)
dL = np.abs((L - L[0]))/L[0]

################################################################################

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('roll [deg]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, rpy[:,0])
plt.savefig('plots/roll.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('pitch [deg]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, rpy[:,1])
plt.savefig('plots/pitch.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('yaw [deg]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, rpy[:,2])
plt.savefig('plots/yaw.png')

################################################################################

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('a11 [ ]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, rotmat[:,0])
plt.savefig('plots/mat_a11.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('a12 [ ]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, rotmat[:,1])
plt.savefig('plots/mat_a12.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('a13 [ ]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, rotmat[:,2])
plt.savefig('plots/mat_a13.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('a21 [ ]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, rotmat[:,3])
plt.savefig('plots/mat_a21.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('a22 [ ]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, rotmat[:,4])
plt.savefig('plots/mat_a22.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('a23 [ ]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, rotmat[:,5])
plt.savefig('plots/mat_a23.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('a31 [ ]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, rotmat[:,6])
plt.savefig('plots/mat_a31.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('a32 [ ]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, rotmat[:,7])
plt.savefig('plots/mat_a32.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('a33 [ ]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, rotmat[:,8])
plt.savefig('plots/mat_a33.png')

################################################################################

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('q0 [ ]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, quat[:,0])
plt.savefig('plots/quat0.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('q1 [ ]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, quat[:,1])
plt.savefig('plots/quat1.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('q2 [ ]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, quat[:,2])
plt.savefig('plots/quat2.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('q3 [ ]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, quat[:,3])
plt.savefig('plots/quat3.png')

################################################################################

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('wix [rad/sec]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, wi[:,0])
plt.savefig('plots/wix.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('wiy [rad/sec]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, wi[:,1])
plt.savefig('plots/wiy.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('wiz [rad/sec]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, wi[:,2])
plt.savefig('plots/wiz.png')

################################################################################

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('wbx [rad/sec]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, wb[:,0])
plt.savefig('plots/wbx.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('wby [rad/sec]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, wb[:,1])
plt.savefig('plots/wby.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('wbz [rad/sec]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, wb[:,2])
plt.savefig('plots/wbz.png')

################################################################################

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('E [kg*km^2/sec^2]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t, E)
plt.savefig('plots/ener.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('L [kg*km^2/sec]')
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

plt.show()
