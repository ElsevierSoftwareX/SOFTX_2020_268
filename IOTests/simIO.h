#ifndef _SIM_IO_H_
#define _SIM_IO_H_

#include <string>
#include <vector>


namespace Flecsi_Sim_IO
{

enum VariableType
{ 
	point               = 0,
	grid_cellCentered   = 1,
	grid_vertexCentered = 2,
	grid_faceCentered   = 3
};

enum TreeType
{
	octree = 0,
	kdtree = 1
};

enum Endianness
{
	little  = 0,
	big = 1
};


struct dataOrganization
{	
	bool available;
	TreeType type;
	void *data;

	dataOrganization(){ data=NULL; }
	~dataOrganization(){ }
};


struct Variable
{
	std::string name;				// Variable name
	int numDims;					// 1 or 2 or 3 ...
	std::vector<int> gridDims;		// Size of each dimension
	std::vector<double> extents;	// (min, max) pair for each dimension
	VariableType varType;			// point or ...
	std::string dataType;			// datatype: int, char, float, ...
	void *data;						// the actual data

	Variable(){ data = NULL; }
	~Variable(){
		if (data != NULL) return;
		
		if ( dataType == "float")			delete [] (float *) data;
		else if ( dataType == "double")		delete [] (double *) data;
		else if ( dataType == "int8_t")		delete [] (int8_t *) data;
		else if ( dataType == "int16_t")	delete [] (int16_t *) data;
		else if ( dataType == "int32_t")	delete [] (int32_t *) data;
		else if ( dataType == "int64_t")	delete [] (int64_t *) data;
		else if ( dataType == "uint8_t")	delete [] (uint8_t *) data;
		else if ( dataType == "uint16_t")	delete [] (uint16_t *) data;
		else if ( dataType == "uint32_t")	delete [] (uint32_t *) data;
		else if ( dataType == "uint64_t")	delete [] (uint64_t *) data;
		else { }
		
	}
};


struct TimeStep
{
	std::vector<Variable> vars;
	int timestep;
	double timeStamp;
};



class SimIO
{
  public:  
	std::string outputFileName;
	Endianness endian;
	std::vector<TimeStep> timeVariables;
	dataOrganization layout;		// storing the organization as just one of the variables

  public:
  	SimIO(){ endian = little; }
	SimIO(std::string _outputFileName):outputFileName(_outputFileName){ endian = little; }
	~SimIO();

	void setFilename(std::string _name){ outputFileName = _name; }
	void setEndianness(Endianness _en){ endian = _en; }
	void setLayout(dataOrganization _layout){ layout = _layout; }

	virtual int createDataset() = 0;
	virtual int writeGridData(int _timeStep) = 0;
	virtual int writePointData(int _timeStep) = 0;
};

inline SimIO::~SimIO()
{
	for (int i=0; i<timeVariables.size(); i++)
	{

	}
}

} // end Flecsi_Sim_IO namespace

#endif