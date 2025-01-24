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

def inertia_is_diagonal(I, tolerance = 1.0e-12):
    #Extract the matrix elements.
    Ixx = abs(I[0,0])
    Iyy = abs(I[1,1])
    Izz = abs(I[2,2])
    Ixy = abs(I[0,1])
    Ixz = abs(I[0,2])
    Iyz = abs(I[1,2])

    #Find the maximum diagonal element.
    max_diag = max(Ixx, Iyy, Izz)

    #Compare off-diagonal elements against the tolerance-scaled max diagonal element.
    if Ixy >= tolerance*max_diag or Ixz >= tolerance*max_diag or Iyz >= tolerance*max_diag:
        return False

    #If all checks pass, the matrix is considered diagonal.
    return True

#Compute analytically the eigenvalues of an inertia matrix (real and symmetric).
def inertia_eigvals(I):    
    #Extract the matrix elements.
    Ixx = I[0,0]
    Ixy = I[0,1]
    Ixz = I[0,2]
    Iyy = I[1,1]
    Iyz = I[1,2]
    Izz = I[2,2]

    #Characteristic polynomial coefficients (P(x) = -x^3 + b*x^2 + c*x + d)
    b = Ixx + Iyy + Izz
    c = Ixy**2 + Ixz**2 - Ixx*Iyy + Iyz**2 - Ixx*Izz - Iyy*Izz
    d = -Ixz**2 * Iyy + 2.0*Ixy*Ixz*Iyz - Ixx*Iyz**2 - Ixy**2*Izz + Ixx*Iyy*Izz

    return np.roots([-1.0, b, c, d])

#Compute analytically the eigenvectors of an inertia matrix (real and symmetric).
def inertia_eigvecs(I):

    def calculate_eigenvector(Ixx, Ixy, Ixz, Iyy, Iyz, Izz, eigval):
        a = -Ixy**2 + (Ixx - eigval)*(Iyy - eigval)
        b = -Ixy*Ixz + (Ixx - eigval)*Iyz
        c = -Ixz**2 + (Ixx - eigval)*(Izz - eigval)

        if a + b == 0:
            raise ValueError("Eigenvector calculation encountered a division by zero.")

        return np.array([ (Ixy*(b+c)/(a+b) - Ixz)/(Ixx - eigval),  -(b+c)/(a+b), 1.0 ])



    #Extract the matrix elements.
    Ixx = I[0, 0]
    Ixy = I[0, 1]
    Ixz = I[0, 2]
    Iyy = I[1, 1]
    Iyz = I[1, 2]
    Izz = I[2, 2]

    eigvals = np.sort(inertia_eigvals(I))
    
    v0 = v1 = v2 = None

    if inertia_is_diagonal(I):
        #If diagonal, eigenvectors are standard basis vectors.
        v0 = np.array([1.0, 0.0, 0.0])
        v1 = np.array([0.0, 1.0, 0.0])
        v2 = np.array([0.0, 0.0, 1.0])
    else:
        #Calculate eigenvectors analytically.
        v0 = calculate_eigenvector(Ixx, Ixy, Ixz, Iyy, Iyz, Izz, eigvals[0])
        v1 = calculate_eigenvector(Ixx, Ixy, Ixz, Iyy, Iyz, Izz, eigvals[1])
        v2 = calculate_eigenvector(Ixx, Ixy, Ixz, Iyy, Iyz, Izz, eigvals[2])

        v0 /= np.linalg.norm(v0)
        v1 /= np.linalg.norm(v1)
        v2 /= np.linalg.norm(v2)

    #Construct a 3x3 matrix with rows as the eigenvectors.
    eigmat = np.array([v0, v1, v2])

    return eigmat.T

#Compute the center of mass of the mascon distribution with constant density.
def get_masc_com(masc):
    return np.sum(masc, axis = 0)/len(masc)

#Shift the center of mass of the mascon distribution with constant density, so that it coincides with O(0,0,0).
def eliminate_com_offset(masc, com):
    masc -= com

#Compute the moment of inertia of a mascon distribution with constant density.
def get_masc_inertia(masc, M):
    m = M/len(masc) #Mass of each mascon.
    Ixx = Iyy = Izz = Ixy = Ixz = Iyz = 0.0

    for x, y, z in masc:
        Ixx += y**2 + z**2
        Iyy += x**2 + z**2
        Izz += x**2 + y**2
        Ixy -= x*y        
        Ixz -= x*z        
        Iyz -= y*z        
        
    return m*np.array([ [Ixx, Ixy, Ixz],
                        [Ixy, Iyy, Iyz],
                        [Ixz, Iyz, Izz] ])

#Rotate the mascon distribution with constant density, so that its local axes coincide with the principal axes of inertia.
def align_principal_axes_to_basis(masc, I):
    eigenvalues, eigenvectors = np.linalg.eigh(I)
    #eigenvectors = inertia_eigvecs(I)
    masc[:] = np.dot(masc, eigenvectors)

def export_masc_to_obj(masc, path):
    with open(path, 'w') as file:
        for x, y, z in masc:
            file.write(f"v {x:.15f} {y:.15f} {z:.15f}\n")

masc = loadobjv('../../obj/mascons/didymain2019_NASA_1229.obj') #Mascons positions ( x [km], y [km], z [km] ).
M = 5.320591856403073e11 #Mass of the body [kg].

com = get_masc_com(masc)
#print('Initial com : ')
#print(com)

eliminate_com_offset(masc, com)
com = get_masc_com(masc)
#print('Final com : ')
#print(com)

iner = get_masc_inertia(masc, M)
#print('\nInitial inertia : ')
#print(iner)

eigvals = np.sort(inertia_eigvals(iner))
print(f"{eigvals[0]:.16f} {eigvals[1]:.16f} {eigvals[2]:.16f}")

eigenvalues, eigenvectors = np.linalg.eigh(iner)
print(f"{eigenvalues[0]:.16f} {eigenvalues[1]:.16f} {eigenvalues[2]:.16f}")

#align_principal_axes_to_basis(masc, iner)
#iner = get_masc_inertia(masc, M)
#print('\nFinal inertia : ')
#print(iner)

#export_masc_to_obj(masc, 'masc_shift_rot.obj')