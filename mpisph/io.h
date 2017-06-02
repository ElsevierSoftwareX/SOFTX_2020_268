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
 * @file io.cc
 * @author Julien Loiseau
 * @date April 2017
 * @brief Simple implementation for Input Output for serial and distributed
 */

#ifndef _mpisph_io_h_
#define _mpisph_io_h_

#include <cstdlib>
#include <iostream>
#include <vector>

//#include <hdf5.h>

#include "tree.h"

#include "H5hut.h"
#include "hdf5ParticleIO.h"

namespace io{

// Read data from txt file with ranges
// This is not the most beautiful way, but enough for testing
void inputDataTxtRange(
    std::vector<std::pair<entity_key_t,body>>& bodies, 
    int& nbodies,
    int& totalnbodies,
    int rank, 
    int size,
    const char * filename
  ){
  nbodies = 0; 
  totalnbodies = 0;
  int mpi_err;
  MPI_File file; 
  MPI_Offset filesize;
  MPI_Offset nelemperproc;
  MPI_Offset startposition; 
  MPI_Offset stopposition; 
  double x, y, z, velX, velY, velZ, velHX, velHY, velHZ;
  double accX, accY, accZ, smoothinglength, pressure;
  double entropy, density, mass, tmp, angularMoment;
  int tmpD;

  //int overlap = 100;

  mpi_err = MPI_File_open(MPI_COMM_WORLD,filename,
      MPI_MODE_RDONLY,MPI_INFO_NULL, &file);
  if(mpi_err){
    fprintf(stderr,"Could not open file %s\n",filename);
    MPI_Finalize(); 
    exit(1); 
  }

  MPI_File_get_size(file,&filesize);
  nelemperproc = filesize/size;
  startposition = rank * nelemperproc; 
  stopposition = startposition + nelemperproc-1;
  if(rank == size-1)
    stopposition = filesize;
  //else 
  //  stopposition += overlap; 
  // Read in a buffer of a double and or int 
  char buffer[2048]; 
  //int iintbuffer; 
 
  //printf("%d/%d from %d to %d",rank,size,startposition,stopposition);

  // Read a line in char buffer
  MPI_File_read_at(file,startposition,
      &buffer,2048,MPI_CHAR,MPI_STATUS_IGNORE); 
  // Print the read value 
  //std::cout << "Just read: "<< buffer<< std::endl;
  
  // For all but the first process, go to the end of this line 
  if(rank != 0){
    int pos = 0;
    while(buffer[pos]!='\n'){
      startposition++;
      pos++;
    }
    startposition++;
    // Read another line from new starting point
    MPI_File_read_at(file,startposition,
      &buffer,2048,MPI_CHAR,MPI_STATUS_IGNORE); 
    // Print the read value 
    //std::cout << "Second read: "<< buffer<< std::endl;
  }

  // Interpret the buffer 
  sscanf(buffer,
      "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf"
      " %lf %lf %lf %lf %lf %lf %d %lf %lf %lf %lf %lf \n",
      &x,&y,&z,                       // positions
      &velX,&velY,&velZ,              // velocity
      &velHX,&velHY,&velHZ,           // velHalf 
      &accX,&accY,&accZ,              // acceleration
      &density,&pressure,&entropy,    // density, pressure, entropy 
      &mass,&smoothinglength,&tmp,    // mass, smoothing length, dt 
      &tmpD,&tmp,&tmp,                // step, totalTime, tmp
      &tmp,&tmp,&angularMoment        // tmp, tmp, angularMoment
  );
  point_t position = {x,y,z}; 
  point_t velocity = {velX,velY,velZ};
  point_t velocityhalf = {velHX,velHY,velHZ};
  point_t acceleration = {accX, accY, accZ};
  
  auto bi = body(position,velocity,velocityhalf,
      acceleration,density,pressure,entropy,mass,smoothinglength);
  bodies.push_back(std::make_pair(entity_key_t::null(),bi));
  ++nbodies;

  // Move at the end of this line 
  int pos = 0;
  while(buffer[pos]!='\n'){
    startposition++;
    pos++;
  }
  startposition++;

  // Loop for all the lines of the file 
  while(startposition < stopposition){
    // Read new line  
    MPI_File_read_at(file,startposition,
      &buffer,2048,MPI_CHAR,MPI_STATUS_IGNORE); 
    // Interpret
    sscanf(buffer,
      "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf"
      " %lf %lf %lf %lf %lf %lf %d %lf %lf %lf %lf %lf \n",
      &x,&y,&z,                       // positions
      &velX,&velY,&velZ,              // velocity
      &velHX,&velHY,&velHZ,           // velHalf 
      &accX,&accY,&accZ,              // acceleration
      &density,&pressure,&entropy,    // density, pressure, entropy 
      &mass,&smoothinglength,&tmp,    // mass, smoothing length, dt 
      &tmpD,&tmp,&tmp,                // step, totalTime, tmp
      &tmp,&tmp,&angularMoment        // tmp, tmp, angularMoment
    );
    position = {x,y,z}; 
    velocity = {velX,velY,velZ};
    velocityhalf = {velHX,velHY,velHZ};
    acceleration = {accX, accY, accZ};

    auto bi = body(position,velocity,velocityhalf,
      acceleration,density,pressure,entropy,mass,smoothinglength);
    bodies.push_back(std::make_pair(entity_key_t::null(),bi));
    
    ++nbodies;
    //std::cout << *bi << std::endl;
    // Move at the end of this line 
    int pos = 0;
    while(buffer[pos]!='\n'){
      startposition++;
      pos++;
    }
    startposition++;
  }
  // Reduction over the bodies 
  MPI_Allreduce(&nbodies,&totalnbodies,1,MPI_INT,MPI_SUM,MPI_COMM_WORLD);
  std::cout << rank<<"/"<<size<<" nbodies: "<<nbodies<<"/"<<totalnbodies<<std::endl; 

} // inputDataTxtRange

// Input data fro HDF5 File
void inputDataHDF5(
  std::vector<std::pair<entity_key_t,body>>& bodies,
  const char * filename,
  int& totalnbodies,
  int& nbodies)
{
  
  int rank, size; 
  MPI_Comm_size(MPI_COMM_WORLD,&size); 
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);

  MPI_Barrier(MPI_COMM_WORLD);
  if(rank==0){
    std::cout<<"Input particles";
  }
  MPI_Barrier(MPI_COMM_WORLD); 

  auto dataFile = H5OpenFile(filename,H5_O_RDWR,MPI_COMM_WORLD);
  // Set the step to 0 for first reading
  H5SetStep(dataFile,0);
  // Set number of data to read to one
  // \TODO change to attribute 
  H5PartSetNumParticles(dataFile,1);
  
  // Get the number of particles 
  int64_t nparticles = 0UL; 
  H5ReadFileAttribInt64(dataFile,"nparticles",&nparticles);
  
  //H5PartReadDataInt64(dataFile,"nparticles",&nparticles);
  if(rank == 0){
    std::cout<<"Total of "<<nparticles<<" particles"<<std::endl;
  }
  int64_t nparticlesproc = nparticles/size;
  // Handle the number of particles for the last one
  if(size-1==rank){
    nparticlesproc = nparticles - nparticlesproc*(size-1);
  }

  // Register for main 
  totalnbodies = nparticles;
  nbodies = nparticlesproc;


  // Resize the bodies vector 
  bodies.clear();
  bodies.resize(nparticlesproc); 

  // Set the number of particles for each process 
  H5PartSetNumParticles(dataFile,nparticlesproc);

  // Read the dataset and fill the particles data 
  double* dataX = new double[nparticlesproc];
  double* dataY = new double[nparticlesproc];
  double* dataZ = new double[nparticlesproc];

  // Positions
  H5PartReadDataFloat64(dataFile,"x",dataX);
  H5PartReadDataFloat64(dataFile,"y",dataY);
  H5PartReadDataFloat64(dataFile,"z",dataZ);
  for(int64_t i=0; i<nparticlesproc; ++i){
    point_t position = point_t{dataX[i]/*,dataY[i],dataZ[i]*/};
    bodies[i].second.setPosition(position);
  }

  // Velocity
  H5PartReadDataFloat64(dataFile,"vx",dataX);
  H5PartReadDataFloat64(dataFile,"vy",dataY);
  H5PartReadDataFloat64(dataFile,"vz",dataZ);
  for(int64_t i=0; i<nparticlesproc; ++i){
    point_t velocity = point_t{dataX[i]/*,dataY[i],dataZ[i]*/};
    bodies[i].second.setVelocity(velocity);
  }

  // Acceleration
  H5PartReadDataFloat64(dataFile,"ax",dataX);
  H5PartReadDataFloat64(dataFile,"ay",dataY);
  H5PartReadDataFloat64(dataFile,"az",dataZ);
  for(int64_t i=0; i<nparticlesproc; ++i){
    point_t acceleration = point_t{dataX[i]/*,dataY[i],dataZ[i]*/};
    bodies[i].second.setVelocity(acceleration);
  }

  // Smoothing Length 
  H5PartReadDataFloat64(dataFile,"h",dataX);
  for(int64_t i=0; i<nparticlesproc; ++i){
    bodies[i].second.setSmoothinglength(dataX[i]);
  }

  // Density   
  H5PartReadDataFloat64(dataFile,"rho",dataX);
  for(int64_t i=0; i<nparticlesproc; ++i){
    bodies[i].second.setDensity(dataX[i]);
  }
  // Internal Energy   
  H5PartReadDataFloat64(dataFile,"u",dataX);
  for(int64_t i=0; i<nparticlesproc; ++i){
    bodies[i].second.setInternalenergy(dataX[i]);
  }
  // Pressure  
  H5PartReadDataFloat64(dataFile,"P",dataX);
  for(int64_t i=0; i<nparticlesproc; ++i){
    bodies[i].second.setPressure(dataX[i]);
  }
  // Mass  
  H5PartReadDataFloat64(dataFile,"m",dataX);
  for(int64_t i=0; i<nparticlesproc; ++i){
    bodies[i].second.setMass(dataX[i]);
  }
  // Id   
  H5PartReadDataFloat64(dataFile,"id",dataX);
  for(int64_t i=0; i<nparticlesproc; ++i){
    bodies[i].second.setId(dataX[i]);
  }
  // delta T   
  H5PartReadDataFloat64(dataFile,"dt",dataX);
  for(int64_t i=0; i<nparticlesproc; ++i){
    bodies[i].second.setDt(dataX[i]);
  }

  delete[] dataX;
  delete[] dataY;
  delete[] dataZ;

  H5CloseFile(dataFile);

  MPI_Barrier(MPI_COMM_WORLD);
  if(rank==0){
    std::cout<<".done"<<std::endl;
  }

}// inputDataHDF5

// Output data in HDF5 format 
// Generate the associate XDMF file 
void outputDataHDF5(
    std::vector<std::pair<entity_key_t,body>>& bodies,
    const char* filename,
    int step)
{

  int size, rank;
  MPI_Comm_size(MPI_COMM_WORLD,&size);
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);

  MPI_Barrier(MPI_COMM_WORLD);
  if(rank == 0){
    std::cout<<"Output particles";
  }

  int64_t nparticlesproc = bodies.size();

  // open the file 
  // auto dataFile = H5OpenFile(filename,H5_O_RDWR,MPI_COMM_WORLD);
  // Set the step 
  // H5SetStep(dataFile,step);
  // Set the number of particles 
  // H5PartSetNumParticles(dataFile,nparticlesproc);

  Flecsi_Sim_IO::HDF5ParticleIO simio;
  simio.createDataset(filename,MPI_COMM_WORLD);

  // If iteration 0 put the global header data 
  // Or check each time if they exists and if no put them  

  // Put the step header
  simio.setTimeStep(step);

  // 4 buffers, 3 double and 1 int64
  double* b1 = new double[nparticlesproc];
  double* b2 = new double[nparticlesproc];
  double* b3 = new double[nparticlesproc];
  int64_t* bi = new int64_t[nparticlesproc];

  // Position
  int64_t pos = 0L;
  // Extract data from bodies 
  for(auto bi: bodies){
    b1[pos] = bi.second.getPosition()[0];
    b2[pos] = bi.second.getPosition()[1];
    b3[pos++] = bi.second.getPosition()[2];
  }

  // Add variable  
  simio.addVariable( Flecsi_Sim_IO::Variable("x",Flecsi_Sim_IO::point, 
        "double", nparticlesproc,b1));
  simio.addVariable( Flecsi_Sim_IO::Variable("y",Flecsi_Sim_IO::point, 
        "double", nparticlesproc,b2));
  simio.addVariable( Flecsi_Sim_IO::Variable("z",Flecsi_Sim_IO::point, 
        "double", nparticlesproc,b3));
  // Push to file 
  simio.writeVariables();

  // Velocity
  pos = 0L;
  // Extract data from bodies 
  for(auto bi: bodies){
    b1[pos] = bi.second.getVelocity()[0];
    b2[pos] = bi.second.getVelocity()[1];
    b3[pos++] = bi.second.getVelocity()[2];
  }

  simio.addVariable( Flecsi_Sim_IO::Variable("vx",Flecsi_Sim_IO::point, 
        "double", nparticlesproc,b1));
  simio.addVariable( Flecsi_Sim_IO::Variable("vy",Flecsi_Sim_IO::point, 
        "double", nparticlesproc,b2));
  simio.addVariable( Flecsi_Sim_IO::Variable("vz",Flecsi_Sim_IO::point, 
        "double", nparticlesproc,b3));

  //H5PartWriteDataFloat64(dataFile,"vx",dataX);
  //H5PartWriteDataFloat64(dataFile,"vy",dataY);
  //H5PartWriteDataFloat64(dataFile,"vz",dataZ);

  simio.writeVariables();

  // Acceleration 
  pos = 0L;
  // Extract data from bodies 
  for(auto bi: bodies){
    b1[pos] = bi.second.getAcceleration()[0];
    b2[pos] = bi.second.getAcceleration()[1];
    b3[pos++] = bi.second.getAcceleration()[2];
  }

  simio.addVariable( Flecsi_Sim_IO::Variable("ax",Flecsi_Sim_IO::point, 
        "double", nparticlesproc,b1));
  simio.addVariable( Flecsi_Sim_IO::Variable("ay",Flecsi_Sim_IO::point, 
        "double", nparticlesproc,b2));
  simio.addVariable( Flecsi_Sim_IO::Variable("az",Flecsi_Sim_IO::point, 
        "double", nparticlesproc,b3));

  simio.writeVariables();

  // H5PartWriteDataFloat64(dataFile,"vx",dataX);
  // H5PartWriteDataFloat64(dataFile,"vy",dataY);
  // H5PartWriteDataFloat64(dataFile,"vz",dataZ);

  // Smoothing length, Density, Internal Energy 
  pos = 0L;
  // Extract data from bodies 
  for(auto bi: bodies){
    b1[pos] = bi.second.getSmoothinglength();
    b2[pos] = bi.second.getDensity();
    b3[pos++] = bi.second.getInternalenergy();
  }

  simio.addVariable( Flecsi_Sim_IO::Variable("h",Flecsi_Sim_IO::point, 
        "double", nparticlesproc,b1));
  simio.addVariable( Flecsi_Sim_IO::Variable("rho",Flecsi_Sim_IO::point, 
        "double", nparticlesproc,b2));
  simio.addVariable( Flecsi_Sim_IO::Variable("u",Flecsi_Sim_IO::point, 
        "double", nparticlesproc,b3));

  simio.writeVariables();

  // H5PartWriteDataFloat64(dataFile,"h",dataX);
  // H5PartWriteDataFloat64(dataFile,"rho",dataY);
  // H5PartWriteDataFloat64(dataFile,"u",dataZ);

 // Pressure, Mass, Id, timestep
  pos = 0L;
  // Extract data from bodies 
  for(auto bid: bodies){
    b1[pos] = bid.second.getPressure();
    b2[pos] = bid.second.getMass();
    b3[pos] = bid.second.getDt();
    bi[pos++] = bid.second.getId();
  }
  
  simio.addVariable( Flecsi_Sim_IO::Variable("P",Flecsi_Sim_IO::point, 
        "double", nparticlesproc,b1));
  simio.addVariable( Flecsi_Sim_IO::Variable("m",Flecsi_Sim_IO::point, 
        "double", nparticlesproc,b2));
  simio.addVariable( Flecsi_Sim_IO::Variable("dt",Flecsi_Sim_IO::point, 
        "double", nparticlesproc,b3));
  simio.addVariable( Flecsi_Sim_IO::Variable("id",Flecsi_Sim_IO::point, 
        "double", nparticlesproc,bi));

  simio.writeVariables();
  simio.closeFile();
  // H5PartWriteDataFloat64(dataFile,"P",dataX);
  // H5PartWriteDataFloat64(dataFile,"m",dataY);
  // H5PartWriteDataFloat64(dataFile,"dt",dataZ);
  // H5PartWriteDataInt64(dataFile,"id",dataInt);

  //int64_t nparticles;
  //MPI_Allreduce(&nparticles,&nparticlesproc,1,MPI_INT64_T,MPI_COMM_WORLD);

  // Also output nparticles and timestep 
  //H5PartWriteDataInt64(dataFile,"nparticles",&nparticles);

  // H5CloseFile(dataFile);

  delete[] b1;
  delete[] b2;
  delete[] b3;
  delete[] bi;

  MPI_Barrier(MPI_COMM_WORLD);
  if(rank == 0){
    std::cout<<".done"<<std::endl;
  }

}// outputDataHDF5

} // namespace io

#endif

