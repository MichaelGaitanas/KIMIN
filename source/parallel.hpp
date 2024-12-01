/*




Forget this header for now...


//Include omp.h

dmatnx3 poly_perps_parallel(const dmatnx3 &verts, const imatnx3 &faces, const int nthreads)
{
    dmatnx3 perps(faces.size());

    int i;
    int nperps = perps.size();
    omp_set_dynamic(false);
    omp_set_num_threads(nthreads);
    #pragma omp parallel for firstprivate(nperps)\
                             private(i)\
                             shared(perps,verts,faces)\
                             schedule(static)
    for (i = 0; i < nperps; ++i)
    {
        dvec3 p0 = verts[faces[i][0]];
        dvec3 p1 = verts[faces[i][1]];
        dvec3 p2 = verts[faces[i][2]];
        perps[i] = cross(p1-p0, p2-p1);
    }

    return perps;
}

dmatnx3 poly_norms_parallel(const dmatnx3 &verts, const imatnx3 &faces, const int nthreads)
{
    dmatnx3 norms(faces.size());

    int i;
    int nnorms = norms.size();
    omp_set_dynamic(false);
    omp_set_num_threads(nthreads);
    #pragma omp parallel for firstprivate(nnorms)\
                             private(i)\
                             shared(norms,verts,faces)\
                             schedule(static)
    for (i = 0; i < nnorms; ++i)
    {
        dvec3 p0 = verts[faces[i][0]];
        dvec3 p1 = verts[faces[i][1]];
        dvec3 p2 = verts[faces[i][2]];
        dvec3 perp = cross(p1-p0, p2-p1);
        norms[i] = perp/length(perp);
    }

    return norms;
}

bool in_poly_parallel(const dvec3 &r, const dmatnx3 &verts, const imatnx3 &faces, const int nthreads)
{
    int intersections = 0;
    dmatnx3 norms = poly_perps_parallel(verts, faces, nthreads);
    //dmatnx3 norms = poly_norms_parallel(verts, faces, nthreads);

    int j;
    int nfaces = faces.size();
    omp_set_dynamic(false);
    omp_set_num_threads(nthreads);
    #pragma omp parallel for firstprivate(nfaces)\
                             private(j)\
                             shared(r,verts,faces,norms)\
                             schedule(static)\
                             reduction(+:intersections)
    for (j = 0; j < nfaces; ++j)
    {
        dvec3 p0 = verts[faces[j][0]];
        dvec3 p1 = verts[faces[j][1]];
        dvec3 p2 = verts[faces[j][2]];
        dvec3 pj = { p0[0] + (p0[1]*norms[j][1] + p0[2]*norms[j][2] - r[1]*norms[j][1] - r[2]*norms[j][2])/norms[j][0], r[1], r[2] };

        double Aj01 = 0.5*length(cross(p0-pj, p1-p0));
        double Aj12 = 0.5*length(cross(p1-pj, p2-p1));
        double Aj20 = 0.5*length(cross(p2-pj, p0-p2));
        double A012 = 0.5*length(cross(p1-p0, p2-p1));

        if ( fabs(Aj01 + Aj12 + Aj20 - A012) <= machine_zero && pj[0] > r[0])
            intersections++;
    }

    if (intersections%2 == 1)
        return true;
    return false;
}

dmatnx3 fill_poly_with_masc_parallel(const dmatnx3 &verts, const imatnx3 &faces, const ivec3 &grid_reso, const int nthreads)
{
    dvec3 rmin = verts[0], rmax = verts[0];
    for (int i = 1; i < verts.size(); ++i)
    {
        if (verts[i][0] < rmin[0]) rmin[0] = verts[i][0];
        if (verts[i][1] < rmin[1]) rmin[1] = verts[i][1];
        if (verts[i][2] < rmin[2]) rmin[2] = verts[i][2];

        if (verts[i][0] > rmax[0]) rmax[0] = verts[i][0];
        if (verts[i][1] > rmax[1]) rmax[1] = verts[i][1];
        if (verts[i][2] > rmax[2]) rmax[2] = verts[i][2];
    }

    double xmin = rmin[0];
    double ymin = rmin[1];
    double zmin = rmin[2];

    double xmax = rmax[0];
    double ymax = rmax[1];
    double zmax = rmax[2];

    dmatnx3 masc;
    for (int i = 0; i < grid_reso[0]; ++i)
    {
        double x = xmin + i*(xmax - xmin)/((double)grid_reso[0] - 1.0);
        for (int j = 0; j < grid_reso[1]; ++j)
        {
            double y = ymin + j*(ymax - ymin)/((double)grid_reso[1] - 1.0);
            for (int k = 0; k < grid_reso[2]; ++k)
            {
                double z = zmin + k*(zmax - zmin)/((double)grid_reso[2] - 1.0);
                dvec3 r = {x,y,z}; //current point of the grid
                if (in_poly_parallel(r, verts,faces, nthreads))
                {
                    masc.push_back(r);
                }
            }
        }
    }

    return masc;
}

double mut_pot_masc_parallel(const dvec3 &r, const double M1, const dmatnx3 &masc1, const dmat3 &A1,
                                             const double M2, const dmatnx3 &masc2, const dmat3 &A2,
                             const int nthreads)
{
    double sum = 0.0;
    double x = r[0], y = r[1], z = r[2];
    double A1_00 = A1[0][0], A1_01 = A1[0][1], A1_02 = A1[0][2],
           A1_10 = A1[1][0], A1_11 = A1[1][1], A1_12 = A1[1][2],
           A1_20 = A1[2][0], A1_21 = A1[2][1], A1_22 = A1[2][2];
    double A2_00 = A2[0][0], A2_01 = A2[0][1], A2_02 = A2[0][2],
           A2_10 = A2[1][0], A2_11 = A2[1][1], A2_12 = A2[1][2],
           A2_20 = A2[2][0], A2_21 = A2[2][1], A2_22 = A2[2][2];
    int nmasc1 = masc1.size(), nmasc2 = masc2.size();
    int i,j;

    omp_set_dynamic(false);
    omp_set_num_threads(nthreads);
    #pragma omp parallel for collapse(2)\
                             firstprivate(nmasc1,nmasc2)\
                             private(i,j)\
                             shared(masc1,masc2)\
                             schedule(static)\
                             reduction(+:sum)
    for (i = 0; i < nmasc1; ++i)
    {
        for (j = 0; j < nmasc2; ++j)
        {
            double x1i = masc1[i][0], y1i = masc1[i][1], z1i = masc1[i][2];
            double x2j = masc2[j][0], y2j = masc2[j][1], z2j = masc2[j][2];

            double dijx = x + A2_00*x2j + A2_01*y2j + A2_02*z2j - (A1_00*x1i + A1_01*y1i + A1_02*z1i);
            double dijy = y + A2_10*x2j + A2_11*y2j + A2_12*z2j - (A1_10*x1i + A1_11*y1i + A1_12*z1i);
            double dijz = z + A2_20*x2j + A2_21*y2j + A2_22*z2j - (A1_20*x1i + A1_21*y1i + A1_22*z1i);

            sum += 1.0/sqrt(dijx*dijx + dijy*dijy + dijz*dijz);
        }
    }
    return -G*M1*M2*sum/(nmasc1*nmasc2);
}

dvec3 mut_force_masc_parallel(const dvec3 &r, const double M1, const dmatnx3 &masc1, const dmat3 &A1,
                                              const double M2, const dmatnx3 &masc2, const dmat3 &A2,
                              const int nthreads)
{
    double sumx = 0.0, sumy = 0.0, sumz = 0.0;
    double x = r[0], y = r[1], z = r[2];
    double A1_00 = A1[0][0], A1_01 = A1[0][1], A1_02 = A1[0][2],
           A1_10 = A1[1][0], A1_11 = A1[1][1], A1_12 = A1[1][2],
           A1_20 = A1[2][0], A1_21 = A1[2][1], A1_22 = A1[2][2];
    double A2_00 = A2[0][0], A2_01 = A2[0][1], A2_02 = A2[0][2],
           A2_10 = A2[1][0], A2_11 = A2[1][1], A2_12 = A2[1][2],
           A2_20 = A2[2][0], A2_21 = A2[2][1], A2_22 = A2[2][2];
    int nmasc1 = masc1.size(), nmasc2 = masc2.size();
    int i,j;

    omp_set_dynamic(false);
    omp_set_num_threads(nthreads);
    #pragma omp parallel for collapse(2)\
                             firstprivate(nmasc1,nmasc2)\
                             private(i,j)\
                             shared(masc1,masc2)\
                             schedule(static)\
                             reduction(+:sumx,sumy,sumz)
    for (i = 0; i < nmasc1; ++i)
    {
        for (j = 0; j < nmasc2; ++j)
        {
            double x1i = masc1[i][0], y1i = masc1[i][1], z1i = masc1[i][2];
            double x2j = masc2[j][0], y2j = masc2[j][1], z2j = masc2[j][2];

            double dijx = x + A2_00*x2j + A2_01*y2j + A2_02*z2j - (A1_00*x1i + A1_01*y1i + A1_02*z1i);
            double dijy = y + A2_10*x2j + A2_11*y2j + A2_12*z2j - (A1_10*x1i + A1_11*y1i + A1_12*z1i);
            double dijz = z + A2_20*x2j + A2_21*y2j + A2_22*z2j - (A1_20*x1i + A1_21*y1i + A1_22*z1i);

            double len = sqrt(dijx*dijx + dijy*dijy + dijz*dijz);
            double invlen3 = 1.0/(len*len*len);

            sumx += dijx*invlen3;
            sumy += dijy*invlen3;
            sumz += dijz*invlen3;
        }
    }
    return G*M1*M2*dvec3{sumx, sumy, sumz}/(nmasc1*nmasc2);
}

dvec3 mut_torque_masc_parallel(const dvec3 &r, const double M1, const dmatnx3 &masc1, const dmat3 &A1,
                                               const double M2, const dmatnx3 &masc2, const dmat3 &A2,
                               const int nthreads)
{
    double sumx = 0.0, sumy = 0.0, sumz = 0.0;
    double x = r[0], y = r[1], z = r[2];
    double A1_00 = A1[0][0], A1_01 = A1[0][1], A1_02 = A1[0][2],
           A1_10 = A1[1][0], A1_11 = A1[1][1], A1_12 = A1[1][2],
           A1_20 = A1[2][0], A1_21 = A1[2][1], A1_22 = A1[2][2];
    double A2_00 = A2[0][0], A2_01 = A2[0][1], A2_02 = A2[0][2],
           A2_10 = A2[1][0], A2_11 = A2[1][1], A2_12 = A2[1][2],
           A2_20 = A2[2][0], A2_21 = A2[2][1], A2_22 = A2[2][2];
    int nmasc1 = masc1.size(), nmasc2 = masc2.size();
    int i,j;

    omp_set_dynamic(false);
    omp_set_num_threads(nthreads);
    #pragma omp parallel for collapse(2)\
                             firstprivate(nmasc1,nmasc2)\
                             private(i,j)\
                             shared(masc1,masc2)\
                             schedule(static)\
                             reduction(+:sumx,sumy,sumz)
    for (i = 0; i < nmasc1; ++i)
    {
        for (j = 0; j < nmasc2; ++j)
        {
            double x1i = masc1[i][0], y1i = masc1[i][1], z1i = masc1[i][2];
            double x2j = masc2[j][0], y2j = masc2[j][1], z2j = masc2[j][2];

            double a1ix = A1_00*x1i + A1_01*y1i + A1_02*z1i;
            double a1iy = A1_10*x1i + A1_11*y1i + A1_12*z1i;
            double a1iz = A1_20*x1i + A1_21*y1i + A1_22*z1i;

            double a2jx = A2_00*x2j + A2_01*y2j + A2_02*z2j;
            double a2jy = A2_10*x2j + A2_11*y2j + A2_12*z2j;
            double a2jz = A2_20*x2j + A2_21*y2j + A2_22*z2j;

            double dijx = x + a2jx - a1ix;
            double dijy = y + a2jy - a1iy;
            double dijz = z + a2jz - a1iz;

            double a1i_cross_dij_x = a1iy*dijz - a1iz*dijy;
            double a1i_cross_dij_y = a1iz*dijx - a1ix*dijz;
            double a1i_cross_dij_z = a1ix*dijy - a1iy*dijx;

            double len = sqrt(dijx*dijx + dijy*dijy + dijz*dijz);
            double invlen3 = 1.0/(len*len*len);

            sumx += a1i_cross_dij_x*invlen3;
            sumy += a1i_cross_dij_y*invlen3;
            sumz += a1i_cross_dij_z*invlen3;
        }
    }
    return G*M1*M2*dvec3{sumx, sumy, sumz}/(nmasc1*nmasc2);
}

dvec6 mut_force_torque_masc_parallel(const dvec3 &r, const double M1, const dmatnx3 &masc1, const dmat3 &A1,
                                                     const double M2, const dmatnx3 &masc2, const dmat3 &A2,
                                     const int nthreads)
{
    double sumfx = 0.0, sumfy = 0.0, sumfz = 0.0;
    double sumtx = 0.0, sumty = 0.0, sumtz = 0.0;
    double x = r[0], y = r[1], z = r[2];
    double A1_00 = A1[0][0], A1_01 = A1[0][1], A1_02 = A1[0][2],
           A1_10 = A1[1][0], A1_11 = A1[1][1], A1_12 = A1[1][2],
           A1_20 = A1[2][0], A1_21 = A1[2][1], A1_22 = A1[2][2];
    double A2_00 = A2[0][0], A2_01 = A2[0][1], A2_02 = A2[0][2],
           A2_10 = A2[1][0], A2_11 = A2[1][1], A2_12 = A2[1][2],
           A2_20 = A2[2][0], A2_21 = A2[2][1], A2_22 = A2[2][2];
    int nmasc1 = masc1.size(), nmasc2 = masc2.size();
    int i,j;

    omp_set_dynamic(false);
    omp_set_num_threads(nthreads);
    #pragma omp parallel for collapse(2)\
                             firstprivate(nmasc1,nmasc2)\
                             private(i,j)\
                             shared(masc1,masc2)\
                             schedule(static)\
                             reduction(+:sumfx,sumfy,sumfz,sumtx,sumty,sumtz)
    for (i = 0; i < nmasc1; ++i)
    {
        for (j = 0; j < nmasc2; ++j)
        {
            double x1i = masc1[i][0], y1i = masc1[i][1], z1i = masc1[i][2];
            double x2j = masc2[j][0], y2j = masc2[j][1], z2j = masc2[j][2];

            double a1ix = A1_00*x1i + A1_01*y1i + A1_02*z1i;
            double a1iy = A1_10*x1i + A1_11*y1i + A1_12*z1i;
            double a1iz = A1_20*x1i + A1_21*y1i + A1_22*z1i;

            double a2jx = A2_00*x2j + A2_01*y2j + A2_02*z2j;
            double a2jy = A2_10*x2j + A2_11*y2j + A2_12*z2j;
            double a2jz = A2_20*x2j + A2_21*y2j + A2_22*z2j;

            double dijx = x + a2jx - a1ix;
            double dijy = y + a2jy - a1iy;
            double dijz = z + a2jz - a1iz;

            double a1i_cross_dij_x = a1iy*dijz - a1iz*dijy;
            double a1i_cross_dij_y = a1iz*dijx - a1ix*dijz;
            double a1i_cross_dij_z = a1ix*dijy - a1iy*dijx;

            double len = sqrt(dijx*dijx + dijy*dijy + dijz*dijz);
            double invlen3 = 1.0/(len*len*len);

            sumfx += dijx*invlen3;
            sumfy += dijy*invlen3;
            sumfz += dijz*invlen3;

            sumtx += a1i_cross_dij_x*invlen3;
            sumty += a1i_cross_dij_y*invlen3;
            sumtz += a1i_cross_dij_z*invlen3;
        }
    }
    dvec3 sumf = G*M1*M2*dvec3{sumfx, sumfy, sumfz}/(nmasc1*nmasc2);
    dvec3 sumt = G*M1*M2*dvec3{sumtx, sumty, sumtz}/(nmasc1*nmasc2);
    return dvec6{sumf[0], sumf[1], sumf[2], sumt[0], sumt[1], sumt[2]};
}

dmat3 masc_inertia_parallel(const double M, const dmatnx3 &masc, const int nthreads)
{
    int nmasc = masc.size();
    double m = (double)M/nmasc;

    double Ixx,Iyy,Izz,Ixy,Ixz,Iyz;
    Ixx = Iyy = Izz = Ixy = Ixz = Iyz = 0.0;
    int i;

    omp_set_dynamic(false);
    omp_set_num_threads(nthreads);
    #pragma omp parallel for firstprivate(nmasc)\
                             private(i)\
                             shared(masc)\
                             schedule(static)\
                             reduction(+:Ixx,Iyy,Izz)\
                             reduction(-:Ixy,Ixz,Iyz)
    for (i = 0; i < nmasc; ++i)
    {
        double xi = masc[i][0];
        double yi = masc[i][1];
        double zi = masc[i][2];

        Ixx += yi*yi + zi*zi;
        Iyy += xi*xi + zi*zi;
        Izz += xi*xi + yi*yi;
        Ixy -= xi*yi;
        Ixz -= xi*zi;
        Iyz -= yi*zi;
    }

    return {{{m*Ixx, m*Ixy, m*Ixz},
             {m*Ixy, m*Iyy, m*Iyz},
             {m*Ixz, m*Iyz, m*Izz}}};
}

dtens masc_integrals_parallel(const double M, const dmatnx3 &masc, const int ord, const int nthreads)
{
    omp_set_dynamic(false);
    omp_set_num_threads(nthreads);
    int nmasc = masc.size();
    int n;

    double m = (double)M/nmasc;
    dtens J(ord+1, dmat(ord+1, dvec(ord+1, 0.0) ) );
    
    for (int i = 0; i < ord + 1; ++i)
        for (int j = 0; j < ord + 1; ++j)
            for (int k = 0; k < ord + 1; ++k)
            {
                double sum = 0.0;
                #pragma omp parallel for firstprivate(nmasc)\
                                         private(n)\
                                         shared(masc,J,i,j,k)\
                                         schedule(static)\
                                         reduction(+:sum)
                for (n = 0; n < nmasc; ++n)
                {
                    sum += m*pow(masc[n][0], i)*pow(masc[n][1], j)*pow(masc[n][2], k);
                }
                J[i][j][k] = sum;
            }

    return J;
}

*/