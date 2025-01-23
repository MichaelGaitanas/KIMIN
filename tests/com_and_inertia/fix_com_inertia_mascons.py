import numpy as np

#Load an .obj file, exclusively with the format 'v x y z' (mascons).
def loadobjv(path):
    xyz = []
    with open(path,'r') as fp:
        for row in fp:
            temp = row.split()
            if temp[0] == 'v':
                x,y,z = float(temp[1]), float(temp[2]), float(temp[3])
                xyz.append([x,y,z])
    
    return np.asarray(xyz)

#Load an .obj file, exclusively with the format 'v x y z' and 'f i j k' (polyhedron).
"""
def loadobjvf(path):
    xyz = []
    ijk = []
    with open(path,'r') as fp:
        for row in fp:
            temp = row.split()
            if temp[0] == 'v':
                x,y,z = float(temp[1]), float(temp[2]), float(temp[3])
                xyz.append([x,y,z])
            elif temp[0] == 'f':
                i,j,k = int(temp[1]),int(temp[2]),int(temp[3])
                ijk.append([i,j,k])
    
    return np.asarray(xyz), np.asarray(ijk) - 1
"""

#Compute the center of mass of the mascons cloud with constant density.
def get_masc_com(masc):
    return np.sum(masc, axis=0)/len(masc)

#Shift the center of mass of the polyhedron, so that it coincides with O(0,0,0).
def eliminate_com_offset(masc, com):
    masc -= com

#Moment of inertia of a mascon distribution with constant density.
def get_masc_inertia(M, masc):
    m = M/len(masc) #Mass of each mascon.
    Ixx = Iyy = Izz = Ixy = Ixz = Iyz = 0.0

    for x, y, z in masc:
        Ixx += y**2 + z**2  #y^2 + z^2
        Iyy += x**2 + z**2  #x^2 + z^2
        Izz += x**2 + y**2  #x^2 + y^2
        Ixy -= x*y          #-x*y
        Ixz -= x*z          #-x*z
        Iyz -= y*z          #-y*z
        
    return m*np.array([ [Ixx, Ixy, Ixz],
                        [Ixy, Iyy, Iyz],
                        [Ixz, Iyz, Izz] ])

def align_principal_axes_to_basis(I, masc):
    eigenvalues, eigenvectors = np.linalg.eigh(I)
    masc[:] = np.dot(masc, eigenvectors)

def export_masc(masc, path):
    with open(path, 'w') as file:
        for x, y, z in masc:
            file.write(f"v {x:.15f} {y:.15f} {z:.15f}\n")

"""
def export_vf(verts, inds, path):
    with open(path, 'w') as file:
        for x, y, z in verts:
            file.write(f"v {x:.15f} {y:.15f} {z:.15f}\n")
        for i, j, k in inds:
            file.write(f"f {x} {y} {z}\n")
"""

masc = loadobjv('../../obj/mascons/didymain2019_NASA_1229.obj') #Mascons positions ( x [km], y [km], z [km]).
M = 5.320591856403073e11 #Mass of the body [kg].

com = get_masc_com(masc)
print('Initial com : ')
print(com)

eliminate_com_offset(masc, com)
print('Final com : ')
print(get_masc_com(masc))

print('\n')

iner = get_masc_inertia(M, masc)
print('Initial inertia : ')
print(iner)

align_principal_axes_to_basis(iner, masc)
print('Final inertia : ')
print(get_masc_inertia(M, masc))

#export_masc(masc, 'masc_shift_rot.obj')