#include <cinchdevel.h>
#include <cinchtest.h>

#include <iostream>
#include <cmath>
#include <mpi.h>

#include "kernels.h"

using namespace std;
using namespace flecsi;
using namespace topology;
using namespace kernels;

namespace flecsi{
namespace execution{
  void driver(int argc, char* argv[]){
  }
}
}

const double h = 2.;
const double step = 0.01;
const double max_step = h * 4.;

const size_t n = max_step / step;
const double start_step = -max_step/2.;


TEST(kernel, cubic_spline) {

  double current = start_step;

  point_t p;

  char name[] = "cubic_spline.csv";

  // Output 
  FILE * output = fopen(name,"w");

  for(size_t i = 0; i < n; ++i){
    p[0] = current;
    fprintf(output,"%.4f;%.4f;%.4f\n",
      current,
      cubic_spline(fabs(current),h),
      gradient_cubic_spline(p,h)[0]);
    current += step; 
  }

  fclose(output);
}

TEST(kernel, gaussian) {

  double current = start_step;

  point_t p;

  char name[] = "gaussian.csv";

  // Output 
  FILE * output = fopen(name,"w");

  for(size_t i = 0; i < n; ++i){
    p[0] = current;
    fprintf(output,"%.4f;%.4f;%.4f\n",
      current,
      gaussian(fabs(current),h),
      gradient_gaussian(p,h)[0]);
    current += step; 
  }

  fclose(output);
}

TEST(kernel, quintic_spline) {

  double current = start_step;

  point_t p;

  char name[] = "quintic_spline.csv";

  // Output 
  FILE * output = fopen(name,"w");

  for(size_t i = 0; i < n; ++i){
    p[0] = current;
    fprintf(output,"%.4f;%.4f;%.4f\n",
      current,
      quintic_spline(fabs(current),h),
      gradient_quintic_spline(p,h)[0]);
    current += step; 
  }

  fclose(output);
}

TEST(kernel, wendland_quintic) {
  
  double current = start_step;

  point_t p;

  char name[] = "wendland_quintic.csv";

  // Output 
  FILE * output = fopen(name,"w");

  for(size_t i = 0; i < n; ++i){
    p[0] = current;
    fprintf(output,"%.4f;%.4f;%.4f\n",
      current,
      wendland_quintic(fabs(current),h),
      gradient_wendland_quintic(p,h)[0]);
    current += step; 
  }

  fclose(output);
}