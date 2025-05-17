import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import os

if not os.path.isdir('plots/'):
    os.mkdir('plots/')

mmes_t = np.loadtxt('io/mmes_time.txt') #[days]
pos_spacecraft = np.loadtxt('io/mmes_pos_spacecraft.txt') #[km]
mmes_ener_mom_rel_err = np.loadtxt('io/mmes_ener_mom_rel_error.txt') #dE/E0 [ ] and dL/L0 [ ]

fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')
ax.plot(pos_spacecraft[:,0], pos_spacecraft[:,1], pos_spacecraft[:,2])
ax.set_xlabel('x [km]')
ax.set_ylabel('y [km]')
ax.set_zlabel('z [km]')
ax.set_title('Spacecraft 3D trajectory')
plt.savefig('plots/trajectory.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('dE/E0')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(mmes_t, mmes_ener_mom_rel_err[:,0])
plt.savefig('plots/dener.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('dL/L0')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(mmes_t, mmes_ener_mom_rel_err[:,1])
plt.savefig('plots/dmom.png')