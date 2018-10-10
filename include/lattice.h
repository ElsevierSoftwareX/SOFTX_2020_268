/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2017 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

 /*~--------------------------------------------------------------------------~*
 *
 * /@@@@@@@@  @@           @@@@@@   @@@@@@@@ @@@@@@@  @@      @@
 * /@@/////  /@@          @@////@@ @@////// /@@////@@/@@     /@@
 * /@@       /@@  @@@@@  @@    // /@@       /@@   /@@/@@     /@@
 * /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@@@@@@ /@@@@@@@@@@
 * /@@////   /@@/@@@@@@@/@@       ////////@@/@@////  /@@//////@@
 * /@@       /@@/@@//// //@@    @@       /@@/@@      /@@     /@@
 * /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@      /@@     /@@
 * //       ///  //////   //////  ////////  //       //      //
 *
 *~--------------------------------------------------------------------------~*/

/**
 * @file lattice_type.h
 * @author Alexander Kaltenborn
 * @date July 2018
 * @brief function to choose lattice type and populate initial conditions
 *
 * Module for generating lattices of different types, suitable for a wide variety
 * of problems, making it essential building block for initial data construction.
 *
 * Having dimension set in the user.h file, this function will currently populate
 * either a rectangular lattice, a triangular lattice, an FCC lattice, or a HCP
 * lattice.
 *
 * Inputs:
 *  lattice_type - int value, corresponding to 0:rect, 1:HCP, 2:FCC (1 & 2 are
 *                 triangular in 2D)
 *  domain_type  - int value, 0:box, 1:sphere
 *  bbox_min     - bounding box: bottom-left corner (or a bounding cube if domain)
 *  bbox_max     - bounding box: top-right corner   (is spherical                )
 *  sph_sep      - particle separation within the lattice
 *  posid        - enter the particle ID number from which the function is called
 *                 it is important for when the function is assigning position
 *                 coordinates to the particles
 *  count_only   - boolean for determing particle number (returned for allocating
 *                 proper arrays) or writing positions to those arrays
 *  x,y,z[]      - the arrays to be filled for the positions of each particle
 */

#include <stdlib.h>
#include "user.h"
#include "tree.h"
#include <math.h>

namespace particle_lattice {
#define SQ(x) ((x)*(x))
#define CU(x) ((x)*(x)*(x))

/**
 * @brief      in_domain checks to see if the entered particle position info
 *             is valid within the restrictive domain_type and total domain
 *             Returns boolean: true or false
 *
 * @param      x,y,z       - position of the particle
 *             bbox_min    - minimum position for the total domain
 *             bbox_max    - maximum position for the total domain
 *             domain_type - int value, 0:cube, 1:sphere
 */
bool in_domain_2d(const double x, const double y,
    const point_t& bbox_min, const point_t& bbox_max, const int domain_type) {
  // within_domain checks to see if the position is within the bounding box
  const double xmin = bbox_min[0], xmax = bbox_max[0],
               ymin = bbox_min[1], ymax = bbox_max[1];
  bool within_domain = x>=xmin && x<=xmax && y>=ymin && y<=ymax;

  // extra check for a circular domain
  if(domain_type==1) {
    const double r = 0.5*std::min(xmax-xmin, ymax-ymin);
    const double x0 = 0.5*(xmin + xmax);
    const double y0 = 0.5*(ymin + ymax);
    within_domain &= ((x-x0)*(x-x0)+(y-y0)*(y-y0) <r*r);
  }
  return within_domain;
}

bool in_domain_3d(const double x, const double y, const double z,
    const point_t& bbox_min, const point_t& bbox_max, const int domain_type) {
  // within_domain checks to see if the position is within the total domain
  const double xmin = bbox_min[0], xmax = bbox_max[0],
               ymin = bbox_min[1], ymax = bbox_max[1],
               zmin = bbox_min[2], zmax = bbox_max[2];
  bool within_domain =
       x>=xmin && x<=xmax && y>=ymin && y<=ymax && z>=zmin && z<=zmax;

  // extra check for a spherical domain
  if(domain_type==1) {
    const double r = 0.5*std::min(std::min(xmax-xmin,ymax-ymin),(zmax-zmin));
    const double x0 = 0.5*(xmin + xmax);
    const double y0 = 0.5*(ymin + ymax);
    const double z0 = 0.5*(zmin + zmax);
    within_domain &= ((x-x0)*(x-x0)+(y-y0)*(y-y0)+(z-z0)*(z-z0) < r*r);
  }
  return within_domain;
}

/**
 * @brief      Generate lattice will run through the supplied domain, and--
 *             depending on the count_only switch--count total number of particles
 *             or assign the positions to the position arrays
 *             Returns int64_t: total particle number
 *
 * @param      Refer to inputs section in introduction
 */
int64_t generator_lattice_1d(
   const int lattice_type,
   const int domain_type,
   const point_t& bbox_min,
   const point_t& bbox_max,
   const double sph_sep,
   int64_t posid,
   bool count_only = true,
   double x[] = NULL,
   double y[] = NULL,
   double z[] = NULL)
{
  // Central coordinates: in most cases this should be centered at 0
  double x_c = (bbox_max[0] + bbox_min[0])/2.;

  // Save the starting position id
  const int64_t posid_starting = posid;

  // regular lattice in 1D
  double xmin = bbox_min[0], xmax = bbox_max[0];
  for(double x_p = xmin; x_p < xmax; x_p += sph_sep){
    if(!count_only){
      x[posid] = x_p;
      y[posid] = 0.0;
      z[posid] = 0.0;
    }
    posid++;
  }
  return (posid - posid_starting);
}

int64_t
generator_lattice_2d(
    const int lattice_type,
    const int domain_type,
    const point_t& bbox_min,
    const point_t& bbox_max,
    const double sph_sep,
    int64_t posid,
    bool count_only = true,
    double x[] = NULL,
    double y[] = NULL,
    double z[] = NULL)
{
   // Coordinate extents
   double xmin = bbox_min[0], xmax = bbox_max[0];
   double ymin = bbox_min[1], ymax = bbox_max[1];

   // Lattice spacing in x- and y-directions
   double dx = sph_sep;
   double dy = sph_sep*sqrt(3.)/2.;

   // Save the starting position id
   const int64_t posid_starting = posid;

   if (lattice_type==0) { // rectangular lattice
     for(double y_p=ymin; y_p<ymax; y_p+=dx)
     for(double x_p=xmin; x_p<xmax; x_p+=dx)
     if(in_domain_2d(x_p,y_p, bbox_min,bbox_max, domain_type)){
       if(!count_only){
         x[posid] = x_p;
         y[posid] = y_p;
         z[posid] = 0.0;
       }
       posid++;
     } // if in domain
   }
   else { // triangular lattice
     for(double y_p=ymin,    yo=0; y_p<ymax; y_p+=dy,yo=1-yo)
     for(double x_p=xmin +yo*dx/2; x_p<xmax; x_p+=dx)
     if(in_domain_2d(x_p,y_p, bbox_min,bbox_max, domain_type)) {
       if(!count_only){
         x[posid] = x_p;
         y[posid] = y_p;
         z[posid] = 0.0;
       }
       posid++;
     } // if in domain
   } // lattice
   return (posid - posid_starting);
}


int64_t
generator_lattice_3d(
    const int lattice_type,
    const int domain_type,
    const point_t& bbox_min,
    const point_t& bbox_max,
    const double sph_sep,
    int64_t posid,
    bool count_only = true,
    double x[] = NULL,
    double y[] = NULL,
    double z[] = NULL)
{
   // Save the starting position id
   const int64_t posid_starting = posid;

   // Coordinate extents
   double xmin = bbox_min[0], xmax = bbox_max[0];
   double ymin = bbox_min[1], ymax = bbox_max[1];
   double zmin = bbox_min[2], zmax = bbox_max[2];

   // Lattice spacing
   double dx = sph_sep;
   double dy = sph_sep*sqrt(3.)/2.;
   double dz = sph_sep*sqrt(2./3.);

   // The loop for lattice_type==0 (rectangular)
   if(lattice_type==0){
     for(double z_p=xmin; z_p<zmax; z_p+=dx)
     for(double y_p=ymin; y_p<ymax; y_p+=dx)
     for(double x_p=zmin; x_p<xmax; x_p+=dx)
     if(in_domain_3d(x_p,y_p,z_p, bbox_min,bbox_max, domain_type)) {
       if(!count_only){
         x[posid] = x_p;
         y[posid] = y_p;
         z[posid] = z_p;
       }
       posid++;
     } // if in domain
   }
   else if(lattice_type==1){//hcp lattice in 3D
     for(double z_p=zmin,         zo=0; z_p<zmax; z_p+=dz, zo=1-zo)
     for(double y_p=ymin-zo*dy/3, yo=0; y_p<ymax; y_p+=dy, yo=1-yo)
     for(double x_p=xmin+(yo-zo)*dx/2.; x_p<xmax; x_p+=dx)
     if(in_domain_3d(x_p,y_p,z_p, bbox_min,bbox_max, domain_type)) {
       if(!count_only){
         x[posid] = x_p;
         y[posid] = y_p;
         z[posid] = z_p;
       }
       posid++;
     } // if in domain
   }
   else if(lattice_type==2) {//fcc lattice in 3D
     for(double z_p=zmin,         zl=0; z_p<zmax; z_p+=dz, zl=(zl+1)*(zl<3))
     for(double y_p=ymin-zl*dy/3, yo=0; y_p<ymax; y_p+=dy, yo=1-yo)
     for(double x_p=xmin+(yo-zl)*dx/2.; x_p<xmax; x_p+=dx)
     if(in_domain_3d(x_p,y_p,z_p, bbox_min,bbox_max, domain_type)) {
       if(!count_only){
         x[posid] = x_p;
         y[posid] = y_p;
         z[posid] = z_p;
       }
       posid++;
     } // if in domain
   } // lattice_type

   return (posid - posid_starting);
}


/**
 * @brief  Computes the number of particles in a k-th icosahedral shell
 * @param  k: the shell index (0: central point, 1: first shell etc.
 */
int64_t npart_icosahedral_shell(const int k) {
  int64_t N2 = 1;
  if (k > 0) 
    N2 = 12 + 30*(k-1) + 10*(k-1)*(k-2);
  return N2;
}


/**
 * @brief  Computes the total number of particles in all shells up to k-th
 * @param  k: the shell index (0: central point, 1: first shell etc.
 */
int64_t npart_icosahedral_sphere(const int k) {
  int64_t N3 = 1;
  if (k > 0) 
    N3 = 1 + 3*k*(5*k-1) + 10*k*(k-1)*(k-2)/3;
  return N3;
}


/**
 * @brief      Generates a spherical icosahedral lattice by placing particles in
 *             concentric shells, centered at the origin.
 *             Depending on the count_only switch--count total number of particles
 *             or assign the positions to the position arrays
 *             Returns int64_t: total particle number
 *
 * @param      Refer to inputs section in introduction
 */
int64_t generator_icosahedral_lattice(const int lattice_type,
    const int domain_type, const point_t& bbox_min, const point_t& bbox_max,
    const double sph_sep, int64_t posid, bool count_only = true,
    double x[] = NULL, double y[] = NULL, double z[] = NULL) {
  // sanity check
  assert (lattice_type == 3 and gdimension == 3);

  // save the starting position id
  const int64_t posid_starting = posid;

  // coordinate extents
  const double xmin = bbox_min[0], xmax = bbox_max[0];
  const double ymin = bbox_min[1], ymax = bbox_max[1];
  const double zmin = bbox_min[2], zmax = bbox_max[2];

  // central point
  const double x_c = 0.5*(xmin + xmax);
  const double y_c = 0.5*(ymin + ymax);
  const double z_c = 0.5*(zmin + zmax);

  // lattice spacing
  const double dr = sph_sep;

  // some numeric constants
  const double pi = M_PI;
  const double p5 = pi/5.0;
  const double sinb = sqrt(4.0*SQ(sin(p5)) - 1.0)/(2.0*SQ(sin(p5)));
  const double cosb = sqrt(1.0 - SQ(sinb)),
               side_a = sqrt(9.0*SQ(tan(p5)) - 3.0);
  const double a2= SQ(side_a),
               eta= sqrt(5*sqrt(3.0)/(4.0*pi))*side_a;
    
  int i,k,m,n,v1,v2,v3,NN;
  double x1,x2,y1,y2,z1,z2,x_p,y_p,z_p,r,f,f1,f2,f3;
  double ico_vtx[12][3];
  const int ico_edg[30][2] = {
      {1,2}  ,  {1,3}  ,  {1,4}  ,  {1,5}  ,  {1,6},
      {2,3}  ,  {3,4}  ,  {4,5}  ,  {5,6}  ,  {6,2},
      {2,7}  ,  {7,3}  ,  {3,8}  ,  {8,4}  ,  {4,9},
      {9,5}  ,  {5,10} , {10,6}  ,  {6,11} , {11,2},
      {7,8}  ,  {8,9}  ,  {9,10} , {10,11} , {11,7},
      {7,12} ,  {8,12} ,  {9,12} , {10,12} , {11,12} };
  const int ico_fac[20][3] = {
      { 1,2,3}, { 1,3,4}, { 1,4,5 }, { 1,5,6  }, { 1,6,2 },
      { 7,2,3}, { 8,3,4}, { 9,4,5 }, {10,5,6  }, {11,6,2 },
      { 3,7,8}, { 4,8,9}, { 5,9,10}, {6,10,11 }, {2,11,7 },
      {12,7,8}, {12,8,9}, {12,9,10}, {12,10,11}, {12,11,7} };


  // compute K_rad: number of shells
  const double diag = sqrt(SQ(xmax-xmin) + SQ(ymax-ymin) + SQ(zmax-zmin));
  const double M_shells = 1.0;
  const double R_shells = diag/2.0;
  const int K_rad = (int)(R_shells/dr) + 1;
  const double rho0 = 3.0*M_shells/ (4*pi*CU(R_shells));
  const double m0 = M_shells / npart_icosahedral_sphere(K_rad);

  double rk = 0.0, rk12;
  double rk12p = pow(3.0*m0/(4*pi*rho0), 1./3.);

  for (int NN=0; NN<=K_rad; ++NN) {

    // 
    //-- Vertices
    //
    
    // top vertex
    ico_vtx[0][0]= 0.0;
    ico_vtx[0][1]= 0.0;
    ico_vtx[0][2]= rk;

    // bottom vertex
    ico_vtx[11][0]= 0.0;
    ico_vtx[11][1]= 0.0;
    ico_vtx[11][2]=-rk;

    // upper and lower rings
    for (i=0; i<5; ++i) {
      ico_vtx[i+1][0]= rk*sinb*cos(2*i*p5);
      ico_vtx[i+1][1]= rk*sinb*sin(2*i*p5);
      ico_vtx[i+1][2]= rk*cosb;

      ico_vtx[i+6][0]= rk*sinb*cos((2*i+1)*p5);
      ico_vtx[i+6][1]= rk*sinb*sin((2*i+1)*p5);
      ico_vtx[i+6][2]=-rk*cosb;
    }

    if (NN > 0) {
      // assign vertices
      for (i=0; i<12; ++i) {
        x_p = x_c + ico_vtx[i][0];
        y_p = y_c + ico_vtx[i][1];
        z_p = z_c + ico_vtx[i][2];
        if(in_domain_3d(x_p,y_p,z_p, bbox_min,bbox_max, domain_type)) {
          if(!count_only){
            x[posid] = x_p;
            y[posid] = y_p;
            z[posid] = z_p;
          }
          posid++;
        } // if in domain
      } // for i from 0 to 12
    }
    else {
      // single point at the origin
      if(!count_only){
        x[posid] = x_c;
        y[posid] = y_c;
        z[posid] = z_c;
      }
      posid++;
    } // if NN>0

    //
    //-- Edges
    //
    for (i=0; i<30; ++i) {
      v1= ico_edg[i][0] - 1;
      v2= ico_edg[i][1] - 1;

      x1= ico_vtx[v1][0];
      y1= ico_vtx[v1][1];
      z1= ico_vtx[v1][2];

      x2= ico_vtx[v2][0];
      y2= ico_vtx[v2][1];
      z2= ico_vtx[v2][2];

      for (k=0; k<NN-1; ++k) {
        // Tegmark correction (see Tegmark'96):
        // f= dble(k)/dble(NN-1) - 0.5
        // f1= 0.5 + eta*f*sqrt((1+a2/12)/(1 - a2*f*f + a2/3))
        f1= (double)(k+1)/(double)NN;
        f2= 1.0 - f1;
        x_p= f1*x1 + f2*x2;
        y_p= f1*y1 + f2*y2;
        z_p= f1*z1 + f2*z2;

        r= sqrt(SQ(x_p) + SQ(y_p) + SQ(z_p))/rk;
        x_p= x_c + x_p/r;
        y_p= y_c + y_p/r;
        z_p= z_c + z_p/r;
        if(in_domain_3d(x_p,y_p,z_p, bbox_min,bbox_max, domain_type)) {
          if(!count_only){
            x[posid] = x_p;
            y[posid] = y_p;
            z[posid] = z_p;
          }
          posid++;
        } // if in domain
      } // for k from 1 to NN-1
    } // for i from 0 to 30

    //
    //-- Faces
    //
    for (i=0; i<20; ++i) {
      v1= ico_fac[i][0] - 1;
      v2= ico_fac[i][1] - 1;
      v3= ico_fac[i][2] - 1;
      
      for (m=1; m<=NN-1; ++m) {
        f1= (double)m/(double)NN;
        for (n=1; n<=NN-1-m; ++n) {
          f2= (double)n/(double)NN;
          f3= 1.0 - f1 - f2;

          x_p= f1*ico_vtx[v1][0] + f2*ico_vtx[v2][0] + f3*ico_vtx[v3][0];
          y_p= f1*ico_vtx[v1][1] + f2*ico_vtx[v2][1] + f3*ico_vtx[v3][1];
          z_p= f1*ico_vtx[v1][2] + f2*ico_vtx[v2][2] + f3*ico_vtx[v3][2];

          r= sqrt(SQ(x_p) + SQ(y_p) + SQ(z_p))/rk;
          x_p= x_c + x_p/r;
          y_p= y_c + y_p/r;
          z_p= z_c + z_p/r;
          if(in_domain_3d(x_p,y_p,z_p, bbox_min,bbox_max, domain_type)) {
            if(!count_only){
              x[posid] = x_p;
              y[posid] = y_p;
              z[posid] = z_p;
            }
            posid++;
          } // if in domain

        } // for n
      } // for m from 1 to NN-1
    } // for i from 0 to 20

    // update radius
    rk12 = rk12p*pow(1.0 + 3*m0*npart_icosahedral_shell(i+1)
                            /(4*pi*rho0*CU(rk12p)),1./3.);
    rk = 0.5*(rk12p + rk12);
    rk12p= rk12;

  } //  NN

  return (posid - posid_starting);
}


// wrappers (because function pointers don't accept default parameters)
int64_t generate_lattice_1d(const int lattice_type, const int domain_type,
    const point_t& bbox_min, const point_t& bbox_max, const double sph_sep,
    int64_t posid, double * x, double * y, double * z) {
  return generator_lattice_1d(lattice_type,domain_type,
         bbox_min,bbox_max,sph_sep,posid, false, x,y,z);
}
int64_t count_lattice_1d(const int lattice_type, const int domain_type,
    const point_t& bbox_min, const point_t& bbox_max, const double sph_sep,
    int64_t posid) {
  return generator_lattice_1d(lattice_type,domain_type,
         bbox_min,bbox_max,sph_sep,posid);
}

int64_t generate_lattice_2d(const int lattice_type, const int domain_type,
    const point_t& bbox_min, const point_t& bbox_max, const double sph_sep,
    int64_t posid, double * x, double * y, double * z) {
  return generator_lattice_2d(lattice_type,domain_type,
         bbox_min,bbox_max,sph_sep,posid, false, x,y,z);
}
int64_t count_lattice_2d(const int lattice_type, const int domain_type,
    const point_t& bbox_min, const point_t& bbox_max, const double sph_sep,
    int64_t posid) {
  return generator_lattice_2d(lattice_type,domain_type,
         bbox_min,bbox_max,sph_sep,posid);
}

int64_t generate_lattice_3d(const int lattice_type, const int domain_type,
    const point_t& bbox_min, const point_t& bbox_max, const double sph_sep,
    int64_t posid, double * x, double * y, double * z) {
  return generator_lattice_3d(lattice_type,domain_type,
         bbox_min,bbox_max,sph_sep,posid, false, x,y,z);
}
int64_t count_lattice_3d(const int lattice_type, const int domain_type,
    const point_t& bbox_min, const point_t& bbox_max, const double sph_sep,
    int64_t posid) {
  return generator_lattice_3d(lattice_type,domain_type,
         bbox_min,bbox_max,sph_sep,posid);
}


// pointer types
typedef int64_t (*lattice_generate_function_t)(const int, const int,
    const point_t&, const point_t&, const double, int64_t,
    double*, double*, double*);
typedef int64_t (*particle_count_function_t)(const int, const int,
    const point_t&, const point_t&, const double, int64_t);
lattice_generate_function_t generate;
particle_count_function_t   count;

/**
 * @brief  Installs the 'generate' and 'count' function pointers
 */
void select() {
  switch(gdimension) {
  case 1:
    generate = generate_lattice_1d;
    count = count_lattice_1d;
    break;
  case 2:
    generate = generate_lattice_2d;
    count = count_lattice_2d;
    break;
  case 3:
    generate = generate_lattice_3d;
    count = count_lattice_3d;
    break;
  default:
    std::cerr << "you should not be here" << std::endl;
  }
}


#undef SQ
#undef CU
} // namespace particle_lattice

