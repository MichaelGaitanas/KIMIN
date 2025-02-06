import numpy as np
import matplotlib.pyplot as plt
import os

t_masc_ord2 = np.loadtxt('../masc_masc_ord2/io/time.txt')/86400.0 #[days]
rv_masc_ord2 = np.loadtxt('../masc_masc_ord2/io/pos_vel.txt') #[km]
EL_masc_ord2 = np.loadtxt('../masc_masc_ord2/io/energy_momentum.txt') #E [kg*km^2/sec^2] and L [kg*km^2/sec]
rpy1_masc_ord2 = np.loadtxt('../masc_masc_ord2/io/roll_pitch_yaw_1.txt')*180/np.pi #[deg]
rpy2_masc_ord2 = np.loadtxt('../masc_masc_ord2/io/roll_pitch_yaw_2.txt')*180/np.pi #[deg]

t_masc_ord3 = np.loadtxt('../masc_masc_ord3/io/time.txt')/86400.0 #[days]
rv_masc_ord3 = np.loadtxt('../masc_masc_ord3/io/pos_vel.txt') #[km]
EL_masc_ord3 = np.loadtxt('../masc_masc_ord3/io/energy_momentum.txt') #E [kg*km^2/sec^2] and L [kg*km^2/sec]
rpy1_masc_ord3 = np.loadtxt('../masc_masc_ord3/io/roll_pitch_yaw_1.txt')*180/np.pi #[deg]
rpy2_masc_ord3 = np.loadtxt('../masc_masc_ord3/io/roll_pitch_yaw_2.txt')*180/np.pi #[deg]

t_masc_ord4 = np.loadtxt('../masc_masc_ord4/io/time.txt')/86400.0 #[days]
rv_masc_ord4 = np.loadtxt('../masc_masc_ord4/io/pos_vel.txt') #[km]
EL_masc_ord4 = np.loadtxt('../masc_masc_ord4/io/energy_momentum.txt') #E [kg*km^2/sec^2] and L [kg*km^2/sec]
rpy1_masc_ord4 = np.loadtxt('../masc_masc_ord4/io/roll_pitch_yaw_1.txt')*180/np.pi #[deg]
rpy2_masc_ord4 = np.loadtxt('../masc_masc_ord4/io/roll_pitch_yaw_2.txt')*180/np.pi #[deg]

t_masc_exact = np.loadtxt('../masc_masc_exact/io/time.txt')/86400.0 #[days]
rv_masc_exact = np.loadtxt('../masc_masc_exact/io/pos_vel.txt') #[km]
EL_masc_exact = np.loadtxt('../masc_masc_exact/io/energy_momentum.txt') #E [kg*km^2/sec^2] and L [kg*km^2/sec]
rpy1_masc_exact = np.loadtxt('../masc_masc_exact/io/roll_pitch_yaw_1.txt')*180/np.pi #[deg]
rpy2_masc_exact = np.loadtxt('../masc_masc_exact/io/roll_pitch_yaw_2.txt')*180/np.pi #[deg]

###############################################################################################

t_poly_ord2 = np.loadtxt('../poly_poly_ord2/io/time.txt')/86400.0 #[days]
rv_poly_ord2 = np.loadtxt('../poly_poly_ord2/io/pos_vel.txt') #[km]
EL_poly_ord2 = np.loadtxt('../poly_poly_ord2/io/energy_momentum.txt') #E [kg*km^2/sec^2] and L [kg*km^2/sec]
rpy1_poly_ord2 = np.loadtxt('../poly_poly_ord2/io/roll_pitch_yaw_1.txt')*180/np.pi #[deg]
rpy2_poly_ord2 = np.loadtxt('../poly_poly_ord2/io/roll_pitch_yaw_2.txt')*180/np.pi #[deg]

t_poly_ord3 = np.loadtxt('../poly_poly_ord3/io/time.txt')/86400.0 #[days]
rv_poly_ord3 = np.loadtxt('../poly_poly_ord3/io/pos_vel.txt') #[km]
EL_poly_ord3 = np.loadtxt('../poly_poly_ord3/io/energy_momentum.txt') #E [kg*km^2/sec^2] and L [kg*km^2/sec]
rpy1_poly_ord3 = np.loadtxt('../poly_poly_ord3/io/roll_pitch_yaw_1.txt')*180/np.pi #[deg]
rpy2_poly_ord3 = np.loadtxt('../poly_poly_ord3/io/roll_pitch_yaw_2.txt')*180/np.pi #[deg]

t_poly_ord4 = np.loadtxt('../poly_poly_ord4/io/time.txt')/86400.0 #[days]
rv_poly_ord4 = np.loadtxt('../poly_poly_ord4/io/pos_vel.txt') #[km]
EL_poly_ord4 = np.loadtxt('../poly_poly_ord4/io/energy_momentum.txt') #E [kg*km^2/sec^2] and L [kg*km^2/sec]
rpy1_poly_ord4 = np.loadtxt('../poly_poly_ord4/io/roll_pitch_yaw_1.txt')*180/np.pi #[deg]
rpy2_poly_ord4 = np.loadtxt('../poly_poly_ord4/io/roll_pitch_yaw_2.txt')*180/np.pi #[deg]

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('rel dist [km]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t_masc_ord2, np.sqrt(rv_masc_ord2[:,0]**2 + rv_masc_ord2[:,1]**2 + rv_masc_ord2[:,2]**2), label='masc_masc_ord2')
plt.plot(t_masc_ord3, np.sqrt(rv_masc_ord3[:,0]**2 + rv_masc_ord3[:,1]**2 + rv_masc_ord3[:,2]**2), label='masc_masc_ord3')
plt.plot(t_masc_ord4, np.sqrt(rv_masc_ord4[:,0]**2 + rv_masc_ord4[:,1]**2 + rv_masc_ord4[:,2]**2), label='masc_masc_ord4')
plt.plot(t_masc_exact, np.sqrt(rv_masc_exact[:,0]**2 + rv_masc_exact[:,1]**2 + rv_masc_exact[:,2]**2), label='masc_masc_exact')
plt.plot(t_poly_ord2, np.sqrt(rv_poly_ord2[:,0]**2 + rv_poly_ord2[:,1]**2 + rv_poly_ord2[:,2]**2), label='poly_poly_ord2')
plt.plot(t_poly_ord3, np.sqrt(rv_poly_ord3[:,0]**2 + rv_poly_ord3[:,1]**2 + rv_poly_ord3[:,2]**2), label='poly_poly_ord3')
plt.plot(t_poly_ord4, np.sqrt(rv_poly_ord4[:,0]**2 + rv_poly_ord4[:,1]**2 + rv_poly_ord4[:,2]**2), label='poly_poly_ord4')
plt.legend()
plt.savefig('rel_dist.png')


plt.figure()
plt.xlabel('t [days]')
plt.ylabel('energy [kg*km^2/sec^2]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t_masc_ord2,  EL_masc_ord2[:,0],  label='masc_masc_ord2')
plt.plot(t_masc_ord3,  EL_masc_ord3[:,0],  label='masc_masc_ord3')
plt.plot(t_masc_ord4,  EL_masc_ord4[:,0],  label='masc_masc_ord4')
plt.plot(t_masc_exact, EL_masc_exact[:,0], label='masc_masc_exact')
plt.plot(t_poly_ord2,  EL_poly_ord2[:,0],  label='poly_poly_ord2')
plt.plot(t_poly_ord3,  EL_poly_ord3[:,0],  label='poly_poly_ord3')
plt.plot(t_poly_ord4,  EL_poly_ord4[:,0],  label='poly_poly_ord4')
plt.legend()
plt.savefig('energy.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('roll1 [deg]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t_masc_ord2,  rpy1_masc_ord2[:,0],  label='masc_masc_ord2')
plt.plot(t_masc_ord3,  rpy1_masc_ord3[:,0],  label='masc_masc_ord3')
plt.plot(t_masc_ord4,  rpy1_masc_ord4[:,0],  label='masc_masc_ord4')
plt.plot(t_masc_exact, rpy1_masc_exact[:,0], label='masc_masc_exact')
plt.plot(t_poly_ord2,  rpy1_poly_ord2[:,0],  label='poly_poly_ord2')
plt.plot(t_poly_ord3,  rpy1_poly_ord3[:,0],  label='poly_poly_ord3')
plt.plot(t_poly_ord4,  rpy1_poly_ord4[:,0],  label='poly_poly_ord4')
plt.legend()
plt.savefig('roll1.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('pitch1 [deg]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t_masc_ord2,  rpy1_masc_ord2[:,1],  label='masc_masc_ord2')
plt.plot(t_masc_ord3,  rpy1_masc_ord3[:,1],  label='masc_masc_ord3')
plt.plot(t_masc_ord4,  rpy1_masc_ord4[:,1],  label='masc_masc_ord4')
plt.plot(t_masc_exact, rpy1_masc_exact[:,1], label='masc_masc_exact')
plt.plot(t_poly_ord2,  rpy1_poly_ord2[:,1],  label='poly_poly_ord2')
plt.plot(t_poly_ord3,  rpy1_poly_ord3[:,1],  label='poly_poly_ord3')
plt.plot(t_poly_ord4,  rpy1_poly_ord4[:,1],  label='poly_poly_ord4')
plt.legend()
plt.savefig('pitch1.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('yaw1 [deg]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t_masc_ord2,  rpy1_masc_ord2[:,2],  label='masc_masc_ord2')
plt.plot(t_masc_ord3,  rpy1_masc_ord3[:,2],  label='masc_masc_ord3')
plt.plot(t_masc_ord4,  rpy1_masc_ord4[:,2],  label='masc_masc_ord4')
plt.plot(t_masc_exact, rpy1_masc_exact[:,2], label='masc_masc_exact')
plt.plot(t_poly_ord2,  rpy1_poly_ord2[:,2],  label='poly_poly_ord2')
plt.plot(t_poly_ord3,  rpy1_poly_ord3[:,2],  label='poly_poly_ord3')
plt.plot(t_poly_ord4,  rpy1_poly_ord4[:,2],  label='poly_poly_ord4')
plt.legend()
plt.savefig('yaw1.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('roll2 [deg]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t_masc_ord2,  rpy2_masc_ord2[:,0],  label='masc_masc_ord2')
plt.plot(t_masc_ord3,  rpy2_masc_ord3[:,0],  label='masc_masc_ord3')
plt.plot(t_masc_ord4,  rpy2_masc_ord4[:,0],  label='masc_masc_ord4')
plt.plot(t_masc_exact, rpy2_masc_exact[:,0], label='masc_masc_exact')
plt.plot(t_poly_ord2,  rpy2_poly_ord2[:,0],  label='poly_poly_ord2')
plt.plot(t_poly_ord3,  rpy2_poly_ord3[:,0],  label='poly_poly_ord3')
plt.plot(t_poly_ord4,  rpy2_poly_ord4[:,0],  label='poly_poly_ord4')
plt.legend()
plt.savefig('roll2.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('pitch2 [deg]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t_masc_ord2,  rpy2_masc_ord2[:,1],  label='masc_masc_ord2')
plt.plot(t_masc_ord3,  rpy2_masc_ord3[:,1],  label='masc_masc_ord3')
plt.plot(t_masc_ord4,  rpy2_masc_ord4[:,1],  label='masc_masc_ord4')
plt.plot(t_masc_exact, rpy2_masc_exact[:,1], label='masc_masc_exact')
plt.plot(t_poly_ord2,  rpy2_poly_ord2[:,1],  label='poly_poly_ord2')
plt.plot(t_poly_ord3,  rpy2_poly_ord3[:,1],  label='poly_poly_ord3')
plt.plot(t_poly_ord4,  rpy2_poly_ord4[:,1],  label='poly_poly_ord4')
plt.legend()
plt.savefig('pitch2.png')

plt.figure()
plt.xlabel('t [days]')
plt.ylabel('yaw2 [deg]')
plt.gcf().subplots_adjust(left = 0.2)
plt.plot(t_masc_ord2,  rpy2_masc_ord2[:,2],  label='masc_masc_ord2')
plt.plot(t_masc_ord3,  rpy2_masc_ord3[:,2],  label='masc_masc_ord3')
plt.plot(t_masc_ord4,  rpy2_masc_ord4[:,2],  label='masc_masc_ord4')
plt.plot(t_masc_exact, rpy2_masc_exact[:,2], label='masc_masc_exact')
plt.plot(t_poly_ord2,  rpy2_poly_ord2[:,2],  label='poly_poly_ord2')
plt.plot(t_poly_ord3,  rpy2_poly_ord3[:,2],  label='poly_poly_ord3')
plt.plot(t_poly_ord4,  rpy2_poly_ord4[:,2],  label='poly_poly_ord4')
plt.legend()
plt.savefig('yaw2.png')

plt.show()