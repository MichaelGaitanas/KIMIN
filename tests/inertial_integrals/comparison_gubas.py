import math as ma
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

#Load an .obj file, exclusively with the format 'v x y z' and 'f i j k' (polyhedron, no normals).
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
    
    return np.array(xyz, dtype=np.float64), np.array(ijk, dtype=np.int64) - 1

#Generate the (non normalized) inertial integrals tensor of order 'ord', of a mascons distribution 'masc', of total mass 'M' with constant density.
def masc_integrals(M,masc,ord):
    m = M/len(masc) #Mass of each mascon.
    J = np.zeros((ord+1,ord+1,ord+1), dtype = np.float64)
    for i in range(ord+1):
        for j in range(ord+1):
            for k in range(ord+1):
                if i + j + k <= ord:
                    J[i,j,k] = m*np.sum((masc[:,0]**i)*(masc[:,1]**j)*(masc[:,2]**k)) #m*(x[n]^i)*(y[n]^j)*(z[n]^k), where n is the mascon's enumeration.
    return J

#Computes the total volume of a polyhedron assuming that the latter is a bunch of
#attached tetrahedra with common vertex at the O(0,0,0). The total volume then, is the sum of
#the individual tetrahedra volumes.
def poly_vol_tet(verts,faces):
    vol = 0.0
    for f in faces:
        p0 = verts[f[0]]
        p1 = verts[f[1]]
        p2 = verts[f[2]]
        vol += abs(np.dot(p0, np.cross(p1,p2)))/6.0
    return vol

#GUBAS
#Computes Q parameter as defined by Hou, where i,j,k are indices for calculation.
def Q_ijk(i,j,k):
	return float(ma.factorial(i)*ma.factorial(j)*ma.factorial(k))/float(ma.factorial(i + j + k + 3))

#GUBAS
#Computes summation over a tetrahedron defined by 4 vertices, where the 4th is assumed to be at (0,0,0).
def tet_sums(l,m,n, x1,x2,x3, y1,y2,y3, z1,z2,z3):
	sum_val = 0.0
	for i1 in range(l+1): #loops through index constraints as defined in Hou
		for j1 in range(l-i1+1):
			for i2 in range(m+1):
				for j2 in range(m-i2+1):
					for i3 in range(n+1):
						for j3 in range(n-i3+1):
							sum_val += (float(ma.factorial(l))/float(ma.factorial(i1)*ma.factorial(j1)\
								             *ma.factorial(l-i1-j1)))\
								             *(float(ma.factorial(m))/float(ma.factorial(i2)*ma.factorial(j2)\
								             *ma.factorial(m-i2-j2)))\
								             *(float(ma.factorial(n))/float(ma.factorial(i3)*ma.factorial(j3)\
								             *ma.factorial(n-i3-j3)))\
								             *x1**i1*x2**j1*x3**(l-i1-j1)*y1**i2*y2**j2*y3**(m-i2-j2)\
								             *z1**i3*z2**j3*z3**(n-i3-j3)\
								             *Q_ijk(i1+i2+i3,j1+j2+j3,l+m+n-i1-i2-i3-j1-j2-j3)
	return sum_val

#GUBAS
#Computes inertial integrals of a polyhedron (verts,faces) of arbitrary order 'ord'.
#'rho' is the mass density of the polyhedron.
def poly_integrals(rho,verts,faces,ord):
	faces = faces[:, ~np.all(np.isnan(faces), axis = 0)]
	J = np.zeros([ord+1, ord+1, ord+1], dtype=np.float64)
	for l in range(ord+1):
		for m in range(ord+1-l):
			for n in range(ord+1-m-l):
				for a in range(np.shape(faces)[0]):
					x1 = verts[int(faces[a,0]), 0:3]
					x2 = verts[int(faces[a,1]), 0:3]
					x3 = verts[int(faces[a,2]), 0:3]
					Ta = np.abs(np.linalg.det(np.c_[x1,x2,x3]))
					J[l,m,n] += rho*Ta*tet_sums(l,m,n,x1[0],x2[0],x3[0],x1[1],x2[1],x3[1],x1[2],x2[2],x3[2])
	return J

masc = loadobjv('../../obj/mascons/patroclus_ellipsoid_48723_fixed.obj') #masc -> ([km],[km],[km])

verts, faces = loadobjvf('../../obj/patroclus_ellipsoid.obj') #verts -> ([km],[km],[km])

M = 123456.0 #Total mass of the body [kg].
ord = 3 #Order of the inertial integrals expansion.

Jmasc = masc_integrals(M,masc,ord)
Jpoly = poly_integrals(M/poly_vol_tet(verts,faces), verts, faces, ord)

for i in range(Jpoly.shape[0]):
    for j in range(Jpoly.shape[1]):
        for k in range(Jpoly.shape[2]):
            print(f"J[{i}][{j}][{k}] : {Jpoly[i, j, k]:.10e}, {Jmasc[i, j, k]:.10e}")