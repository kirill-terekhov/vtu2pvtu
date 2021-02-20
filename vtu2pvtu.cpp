#include "xml.h"
#include <string>
#include <iostream>
#include <vector>
#include <set>
#include <sstream>
#include <cmath>
#include <limits>
#include <ctime>
#include <cstdlib>
#include <algorithm>

std::string tolower(std::string input)
{
	std::string ret(input);
	for(size_t k = 0; k < ret.size(); ++k)
		ret[k] = tolower(ret[k]);
	return ret;
}

template<typename T>
void convertstr(std::string input, T & output)
{
	std::istringstream s(input.c_str());
	s >> output;
}

template<typename T>
std::string strconvert(const T & input)
{
	std::ostringstream s;
	s << input;
	return s.str();
}

int getdigits(int val)
{
	int digits = 0;
	do
	{
		digits++;
		val /= 10;
	}
	while(val);
	return digits;
}


template<typename dtype>
void * preloaddata(std::istream & f, size_t nelems, int comps, std::vector<dtype> * data)
{
	(*data).resize(nelems*comps);
	size_t reads = 0;
	for(size_t q = 0; q < nelems; ++q)
	{
		for(int c = 0; c < comps; ++c) 
			f >> (*data)[reads++];
	}
	return static_cast<void *>(data);
}

void * preloaddata(std::istream & f, size_t nelems, int comps, std::string dtype)
{
	if( dtype == "Float64" )
		return preloaddata(f,nelems,comps,new std::vector<double>);
	else if( dtype == "Float32" )
		return preloaddata(f,nelems,comps,new std::vector<float>);
	else if( dtype == "Int64" )
		return preloaddata(f,nelems,comps,new std::vector<int64_t>);
	else if( dtype == "UInt64" )
		return preloaddata(f,nelems,comps,new std::vector<uint64_t>);
	else if( dtype == "Int32" )
		return preloaddata(f,nelems,comps,new std::vector<int32_t>);
	else if( dtype == "UInt32" )
		return preloaddata(f,nelems,comps,new std::vector<uint32_t>);
	else if( dtype == "Int16" )
		return preloaddata(f,nelems,comps,new std::vector<int16_t>);
	else if( dtype == "UInt16" )
		return preloaddata(f,nelems,comps,new std::vector<uint16_t>);
	else if( dtype == "Int8" )
		return preloaddata(f,nelems,comps,new std::vector<int8_t>);
	else if( dtype == "UInt8" )
		return preloaddata(f,nelems,comps,new std::vector<uint8_t>);
	else 
	{
		std::cout << "Unknown data type " << dtype << std::endl;
		return NULL;
	}
}

template<typename dtype>
void releasedata(std::vector<dtype> * data) 
{
	delete data;
}

void releasedata(std::string dtype, void * data)
{
	if( dtype == "Float64" )
		return releasedata(static_cast<std::vector<double> *>(data));
	else if( dtype == "Float32" )
		return releasedata(static_cast<std::vector<float> *>(data));
	else if( dtype == "Int64" )
		return releasedata(static_cast<std::vector<int64_t> *>(data));
	else if( dtype == "UInt64" )
		return releasedata(static_cast<std::vector<uint64_t> *>(data));
	else if( dtype == "Int32" )
		return releasedata(static_cast<std::vector<int32_t> *>(data));
	else if( dtype == "UInt32" )
		return releasedata(static_cast<std::vector<uint32_t> *>(data));
	else if( dtype == "Int16" )
		return releasedata(static_cast<std::vector<int16_t> *>(data));
	else if( dtype == "UInt16" )
		return releasedata(static_cast<std::vector<uint16_t> *>(data));
	else if( dtype == "Int8" )
		return releasedata(static_cast<std::vector<int8_t> *>(data));
	else if( dtype == "UInt8" )
		return releasedata(static_cast<std::vector<uint8_t> *>(data));
	else 
	{
		std::cout << "Unknown data type " << dtype << std::endl;
		return;
	}
}

template<typename dtype>
void transferdata(std::ostream & fh, size_t nelems, int comps, const std::vector<int> & epart, const std::set<size_t>& exte, int part, std::vector<dtype> * data)
{
	size_t wr = 0;
	for(size_t q = 0; q < nelems; ++q)
	{
		if( epart[q] == part || exte.count(q) )
		{
			if( wr % (16 / comps) == 0 ) fh << "\t\t\t\t  ";
			for(int c = 0; c < comps; ++c)
				fh << (*data)[q*comps + c] << " ";
			wr++;
			if( wr % (16 / comps) == 0 ) fh << std::endl;
		}
	}
	if( wr % (16 / comps) != 0 ) fh << std::endl;
}

void transferdata(std::ostream & f, size_t nelems, int comps, const std::vector<int> & epart, const std::set<size_t>& exte, int part, std::string dtype, void * data)
{
	if( dtype == "Float64" )
		return transferdata(f,nelems,comps,epart,exte,part,static_cast<std::vector<double> *>(data));
	else if( dtype == "Float32" )
		return transferdata(f,nelems,comps,epart,exte, part,static_cast<std::vector<float> *>(data));
	else if( dtype == "Int64" )
		return transferdata(f,nelems,comps,epart,exte, part,static_cast<std::vector<int64_t> *>(data));
	else if( dtype == "UInt64" )
		return transferdata(f,nelems,comps,epart, exte, part,static_cast<std::vector<uint64_t> *>(data));
	else if( dtype == "Int32" )
		return transferdata(f,nelems,comps,epart, exte, part,static_cast<std::vector<int32_t> *>(data));
	else if( dtype == "UInt32" )
		return transferdata(f,nelems,comps,epart, exte, part,static_cast<std::vector<uint32_t> *>(data));
	else if( dtype == "Int16" )
		return transferdata(f,nelems,comps,epart, exte, part,static_cast<std::vector<int16_t> *>(data));
	else if( dtype == "UInt16" )
		return transferdata(f,nelems,comps,epart, exte, part,static_cast<std::vector<uint16_t> *>(data));
	else if( dtype == "Int8" )
		return transferdata(f,nelems,comps,epart, exte, part,static_cast<std::vector<int8_t> *>(data));
	else if( dtype == "UInt8" )
		return transferdata(f,nelems,comps,epart, exte, part,static_cast<std::vector<uint8_t> *>(data));
	else 
	{
		std::cout << "Unknown data type " << dtype << std::endl;
		return;
	}
}

template<typename dtype>
void transferdata(std::istream & f, std::ostream & fh, size_t nelems, const std::vector<int> & epart, const std::set<size_t> & exte, int part, int comps)
{
	size_t wr = 0;
	for(size_t q = 0; q < nelems; ++q)
	{
		dtype v;
		if( epart[q] == part || exte.count(q) )
		{
			if( wr % (16 / comps) == 0 ) fh << "\t\t\t\t  ";
			for(int c = 0; c < comps; ++c)
			{
				f >> v; 
				fh << v << " ";
			}
			wr++;
			if( wr % (16 / comps) == 0 ) fh << std::endl;
		}
		else  for(int c = 0; c < comps; ++c) f >> v;
	}
	if( wr % (16 / comps) != 0 ) fh << std::endl;
}

int main(int argc, char **argv)
{
	if( argc < 3 )
	{
		std::cout << "Usage:" << argv[0] << " input.vtu partitions [output.pvtu]" << std::endl;
		return -1; 
	}
	else
	{
		const int max_iterations = 30; //maximum iterations for K-means clustering
		std::string input = std::string(argv[1]);
		std::string output = "output.pvtu";
		int parts = atoi(argv[2]);
		if( argc > 3 ) output = std::string(argv[3]);
		if( input.find(".vtu") == std::string::npos )
		{
			std::cout << "input file format should be .vtu" << std::endl;
			return -1;
		}
		if( output.find(".pvtu") == std::string::npos )
		{
			std::cout << "output file format should be .pvtu" << std::endl;
			return -1;
		}
		int dims = 0;
		size_t npoints = 0, ncells = 0;
		//data
		std::vector<std::string> pointdata_name;
		std::vector<std::string> pointdata_type;
		std::vector<int> pointdata_comps;
		std::vector<std::streampos> pointdata_pos;
		std::vector<std::string> celldata_name;
		std::vector<std::string> celldata_type;
		std::vector<int> celldata_comps;
		std::vector<std::streampos> celldata_pos;
		std::vector<std::string> filenames(parts);
		std::vector<int> cpart;
		
		//define file names
		{
			int digits = getdigits(parts), digitsk = 0;
			for(int k = 0; k < parts; ++k)
			{
				digitsk = getdigits(k);
				filenames[k] = output.substr(0,output.size()-5); //strip .pvtu
				for(int j = 0; j < digits - digitsk; ++j) filenames[k] += '0';
				filenames[k] += strconvert(k);
				filenames[k] += ".vtu";
			}
		}
		
		std::ifstream f(input.c_str());
		
		{//scope for node partitioning info
			std::vector<int> ppart;
			std::vector< std::set<size_t> > epoints(parts); //additional nodes that should present on processor
			
			//file positions
			{ //scope to release connectivity information 
				std::streampos pos_connectivity = 0, pos_offsets = 0, pos_types = 0, pos_faces = 0, pos_faceoffsets = 0;
				std::vector<double> pcoords; //read all the coordinates
				const bool load_data = true; //read connectivity information into memory
				bool have_faces = false;
				std::vector<size_t> offsets;
				std::vector<size_t> connectivity;
				std::vector<uint8_t> types;
				std::vector<size_t> faceoffsets;
				std::vector<size_t> faces;
				size_t mone_size_t = -1;
				
				
				std::cout << "Read " << input << " for node coordinates and data positions in file." << std::endl;
				
				{
					XMLReader reader(input, f);
					//~ reader.SetVerbosity(2);
					XMLReader::XMLTag root = reader.OpenTag();
					if( root.name != "VTKFile" )
					{
						reader.Report("Root tag should be VTKFile, not %s. %s is not a valid .vtu file.",root.name.c_str(),input.c_str());
						return -1;
					}
					for(int q = 0; q < root.NumAttrib(); ++q)
					{
						const XMLReader::XMLAttrib & attr = root.GetAttrib(q);
						if( tolower(attr.name) == "type" )
						{
							if( attr.value != "UnstructuredGrid" )
							{
								reader.Report("Unsupported grid type %s, expected UnstructuredGrid.",attr.value.c_str());
								return -1;
							}
						}
						else if( tolower(attr.name) == "compression" )
						{
							reader.Report("Compression is specified, but not supported.");
							return -1;
						}
						//else reader.Report("Unused attribute for %s %s='%s'",root.name.c_str(),attr.name.c_str(),attr.value.c_str());
					}
					
					if( reader.isFailure() ) 
					{
						reader.Report("Failed reading file at %s:%d",__FILE__,__LINE__);
						return -1;
					}
					
					XMLReader::XMLTag subroot = reader.OpenTag();
					
					if( subroot.name != "UnstructuredGrid" )
					{
						reader.Report("Subroot tag should be VTKUnstructuredGrid, not %s. %s is not a valid .vtu file.",subroot.name.c_str(),input.c_str());
						return -1;
					}
					
					if( reader.isFailure() ) 
					{
						reader.Report("Failed reading file at %s:%d",__FILE__,__LINE__);
						return -1;
					}
					for(XMLReader::XMLTag piece = reader.OpenTag(); !piece.Finalize(); piece = reader.OpenTag())
					{
						if( piece.name != "Piece" )
						{
							reader.Report("Expected Piece tag but got %s",piece.name.c_str());
							reader.SkipTag(piece.name);
							continue;
						}
						if( reader.isFailure() ) 
						{
							reader.Report("Failed reading file at %s:%d",__FILE__,__LINE__);
							return -1;
						}
						for(int q = 0; q < piece.NumAttrib(); ++q)
						{
							const XMLReader::XMLAttrib & attr = piece.GetAttrib(q);
							if( tolower(attr.name) == "numberofpoints" ) convertstr(attr.value,npoints);
							else if( tolower(attr.name) == "numberofcells" ) convertstr(attr.value,ncells);
							//else reader.Report("Unused attribute for %s %s='%s'",piece.name.c_str(),attr.name.c_str(),attr.value.c_str());
						}
						if( reader.isFailure() ) 
						{
							reader.Report("Failed reading file at %s:%d",__FILE__,__LINE__);
							return -1;
						}
						if( npoints == 0 )
						{
							reader.Report("NumberOfPoints attribute was not found in %s tag.",piece.name.c_str());
							return -1;
						}
						if( ncells == 0 )
						{
							reader.Report("NumberOfCells attribute was not found in %s tag.",piece.name.c_str());
							return -1;
						}
						
						std::cout << npoints << " points." << std::endl;
						std::cout << ncells << " cells." << std::endl;
						
						for(XMLReader::XMLTag sub = reader.OpenTag(); !sub.Finalize(); sub = reader.OpenTag())
						{
							if( reader.isFailure() ) 
							{
								reader.Report("Failed reading file at %s:%d",__FILE__,__LINE__);
								return -1;
							}
							if( sub.name == "Points" )
							{
								XMLReader::XMLTag data = reader.OpenTag();
								if( data.name != "DataArray" )
								{
									reader.Report("Expected tag DataArray instead of %s.",data.name.c_str());
									return -1;
								}
								for(int q = 0; q < data.NumAttrib(); ++q)
								{
									const XMLReader::XMLAttrib & attr = data.GetAttrib(q);
									if( tolower(attr.name) == "format" )
									{
										if( tolower(attr.value) != "ascii" )
										{
											reader.Report("Data format is %s, but only ascii is supported.",attr.value.c_str());
											return -1;
										}
									}
									else if( tolower(attr.name) == "numberofcomponents" ) convertstr(attr.value,dims);
									//else reader.Report("Unused attribute for %s %s='%s'",sub.name.c_str(),attr.name.c_str(),attr.value.c_str());
								}
								if( dims == 0 )
								{
									reader.Report("NumberOfComponents attribute was not found in %s tag.",data.name.c_str());
									dims = 3;
								}
								
								std::cout << dims << " dimensions." << std::endl;
								
								pcoords.resize(npoints*dims,0.f);
								for(size_t k = 0; k < npoints*dims; ++k)
								{
									std::string tmp = reader.GetWord('<');
									convertstr(tmp,pcoords[k]);
								}
								reader.SkipTag(data.name); //there might be remaining data
								/*
								if( !reader.CloseTag(data) )
								{
									reader.Report("Failed closing XML tag %s in %s:%d.",sub.name.c_str(),__FILE__,__LINE__);
									return -1;
								}
								*/
								if( !reader.CloseTag(sub) )
								{
									reader.Report("Failed closing XML tag %s in %s:%d.",sub.name.c_str(),__FILE__,__LINE__);
									return -1;
								}
							}
							else if( sub.name == "Cells" )
							{
								for(XMLReader::XMLTag data = reader.OpenTag(); !data.Finalize(); data = reader.OpenTag())
								{
									if( reader.isFailure() ) 
									{
										reader.Report("Failed reading file at %s:%d",__FILE__,__LINE__);
										return -1;
									}
									if( data.name != "DataArray" )
									{
										reader.Report("Expected tag DataArray instead of %s.",data.name.c_str());
										return -1;
									}
									for(int q = 0; q < data.NumAttrib(); ++q)
									{
										const XMLReader::XMLAttrib & attr = data.GetAttrib(q);
										if( tolower(attr.name) == "format" )
										{
											if( tolower(attr.value) != "ascii" )
											{
												reader.Report("Data format is %s, but only ascii is supported.",attr.value.c_str());
												return -1;
											}
										}
										else if( tolower(attr.name) == "name" ) 
										{
											if( attr.value == "connectivity" )
												pos_connectivity = f.tellg();
											else if( attr.value == "offsets" )
												pos_offsets = f.tellg();
											else if( attr.value == "types" )
												pos_types = f.tellg();
											else if( attr.value == "faces" )
											{
												pos_faces = f.tellg();
												have_faces = true;
											}
											else if( attr.value == "faceoffsets" )
											{
												pos_faceoffsets = f.tellg();
												have_faces = true;
											}
										}
										//else reader.Report("Unused attribute for %s %s='%s'",data.name.c_str(),attr.name.c_str(),attr.value.c_str());
									}
									reader.SkipTag(data.name);
								}
								if( !reader.CloseTag(sub) )
								{
									reader.Report("Failed closing XML tag %s in %s:%d.",sub.name.c_str(),__FILE__,__LINE__);
									return -1;
								}
							}
							else if( sub.name == "PointData" )
							{
								for(XMLReader::XMLTag data = reader.OpenTag(); !data.Finalize(); data = reader.OpenTag())
								{
									if( reader.isFailure() ) 
									{
										reader.Report("Failed reading file at %s:%d",__FILE__,__LINE__);
										return -1;
									}
									if( data.name != "DataArray" )
									{
										reader.Report("Expected tag DataArray instead of %s.",data.name.c_str());
										return -1;
									}
									pointdata_comps.push_back(1);
									for(int q = 0; q < data.NumAttrib(); ++q)
									{
										const XMLReader::XMLAttrib & attr = data.GetAttrib(q);
										if( tolower(attr.name) == "format" )
										{
											if( tolower(attr.value) != "ascii" )
											{
												reader.Report("Data format is %s, but only ascii is supported.",attr.value.c_str());
												return -1;
											}
										}
										else if( tolower(attr.name) == "name" ) 
											pointdata_name.push_back(attr.value);
										else if( tolower(attr.name) == "type" ) 
											pointdata_type.push_back(attr.value);
										else if( tolower(attr.name) == "numberofcomponents" ) 
											convertstr(attr.value,pointdata_comps.back());
										//else reader.Report("Unused attribute for %s %s='%s'",data.name.c_str(),attr.name.c_str(),attr.value.c_str());
									}
									pointdata_pos.push_back(f.tellg());
									reader.SkipTag(data.name);
								}
								if( !reader.CloseTag(sub) )
								{
									reader.Report("Failed closing XML tag %s in %s:%d.",sub.name.c_str(),__FILE__,__LINE__);
									return -1;
								}
							}
							else if( sub.name == "CellData" )
							{
								for(XMLReader::XMLTag data = reader.OpenTag(); !data.Finalize(); data = reader.OpenTag())
								{
									if( reader.isFailure() ) 
									{
										reader.Report("Failed reading file at %s:%d",__FILE__,__LINE__);
										return -1;
									}
									if( data.name != "DataArray" )
									{
										reader.Report("Expected tag DataArray instead of %s.",data.name.c_str());
										return -1;
									}
									celldata_comps.push_back(1);
									for(int q = 0; q < data.NumAttrib(); ++q)
									{
										const XMLReader::XMLAttrib & attr = data.GetAttrib(q);
										if( tolower(attr.name) == "format" )
										{
											if( tolower(attr.value) != "ascii" )
											{
												reader.Report("Data format is %s, but only ascii is supported.",attr.value.c_str());
												return -1;
											}
										}
										else if( tolower(attr.name) == "name" ) 
											celldata_name.push_back(attr.value);
										else if( tolower(attr.name) == "type" ) 
											celldata_type.push_back(attr.value);
										else if( tolower(attr.name) == "numberofcomponents" ) 
											convertstr(attr.value,celldata_comps.back());
										//else reader.Report("Unused attribute for %s %s='%s'",data.name.c_str(),attr.name.c_str(),attr.value.c_str());
									}
									celldata_pos.push_back(f.tellg());
									reader.SkipTag(data.name);
								}
								if( !reader.CloseTag(sub) )
								{
									reader.Report("Failed closing XML tag %s in %s:%d.",sub.name.c_str(),__FILE__,__LINE__);
									return -1;
								}
							}
							else reader.SkipTag(sub.name);
						}
						if( !reader.CloseTag(piece) )
						{
							reader.Report("Failed closing XML tag %s in %s:%d.",piece.name.c_str(),__FILE__,__LINE__);
							return -1;
						}
					}
					
					reader.CloseTag(subroot);
					reader.CloseTag(root);
				}
				
				if( load_data )
				{
					f.seekg(pos_offsets);
					offsets.resize(ncells);
					for(size_t i = 0; i < offsets.size(); ++i) f >> offsets[i];
					if( f.fail() ) {std::cout << __FILE__ << ":" << __LINE__ << " input stream failure " << std::endl; return -1;}
					f.seekg(pos_types);
					types.resize(ncells);
					for(size_t i = 0; i < types.size(); ++i) 
					{
						int ctype;
						f >> ctype;
						types[i] = static_cast<uint8_t>(ctype);
					}
					if( f.fail() ) {std::cout << __FILE__ << ":" << __LINE__ << " input stream failure " << std::endl; return -1;}
					f.seekg(pos_connectivity);
					connectivity.resize(offsets.back());
					for(size_t i = 0; i < connectivity.size(); ++i) f >> connectivity[i];
					if( f.fail() ) {std::cout << __FILE__ << ":" << __LINE__ << " input stream failure " << std::endl; return -1;}
					if( have_faces )
					{
						f.seekg(pos_faceoffsets);
						faceoffsets.resize(ncells);
						for(size_t i = 0; i < faceoffsets.size(); ++i) f >> faceoffsets[i];
						if( f.fail() ) {std::cout << __FILE__ << ":" << __LINE__ << " input stream failure " << std::endl; return -1;}
						size_t facesreads = 0;
						for(size_t i = faceoffsets.size(); i > 0; i--) { if( faceoffsets[i-1] != mone_size_t ) {facesreads = faceoffsets[i-1]; break;}	}
						f.seekg(pos_faces);
						faces.resize(facesreads);
						for(size_t i = 0; i < faces.size(); ++i) f >> faces[i];
						if( f.fail() ) {std::cout << __FILE__ << ":" << __LINE__ << " input stream failure " << std::endl; return -1;}
					}
				}
				
				std::cout << "Calculate centers of cells." << std::endl;
				
				{ //scope for ccoords memory release
					std::vector<double> ccoords(ncells*dims,0.f);
					{
						std::streampos pos_connectivity_running = pos_connectivity;
						std::streampos pos_offsets_running = pos_offsets;
						size_t offset, offset0 = 0, size, point;
						for(size_t i = 0; i < ncells; ++i)
						{
							if( load_data )
								offset = offsets[i];
							else
							{
								f.seekg(pos_offsets_running);
								if( f.fail() ) 	{std::cout << __FILE__ << ":" << __LINE__ << " input stream failure " << std::endl;	return -1;}
								f >> offset;
								pos_offsets_running = f.tellg();
								f.seekg(pos_connectivity_running);
								if( f.fail() ) {std::cout << __FILE__ << ":" << __LINE__ << " input stream failure " << std::endl; return -1;}
							}
							size = offset-offset0;
							for(size_t k = 0; k < size; ++k)
							{
								if( load_data )
									point = connectivity[offset0+k];
								else f >> point;
								for(int q = 0; q < dims; ++q)
									ccoords[i*dims+q] += pcoords[point*dims+q];					
							}
							offset0 = offset;
							for(int q = 0; q < dims; ++q)
								ccoords[i*dims+q] /= static_cast<double>(size);
							if( !load_data )
							{
								pos_connectivity_running = f.tellg();
								if( f.fail() ) { std::cout << __FILE__ << ":" << __LINE__ << " input stream failure " << std::endl; return -1;}
							}
						}
					}
					
					std::cout << "Cluster cell centers into " << parts << " parts." << std::endl;
					
					//separate points by processors with K-means
					cpart.resize(ncells,-1);
					{
						std::vector< double > cluster_coords(parts*dims,0.0);
						std::vector< int > cluster_npoints(parts,0);
						//select seed points for clusters
						{
							srand((unsigned int)time(NULL));
							for(int i = 0; i < parts; i++) if( cluster_npoints[i] == 0 )
							{
								while(true)
								{
									size_t index = (rand()*rand()) % ncells;
									if(cpart[index] == -1)
									{
										for(int k = 0; k < dims; ++k)
											cluster_coords[i*dims + k] = ccoords[index*dims + k];
										cpart[index] = i;
										break;
									}
								}
							}
						}
						
						int iter = 1;
						while(true)
						{
							int changed = 0;
							// associates each point to the nearest center
#pragma omp parallel for reduction(+:changed)
							for(int64_t i = 0; i < static_cast<int64_t>(ncells); i++)
							{
								int id_old_cluster = cpart[i];
								int id_nearest_center = -1;
								
								double lmin = std::numeric_limits<double>::max();
								
								for(int j = 0; j < parts; ++j)
								{
									double l = 0;
									for(int k = 0; k < dims; ++k)
										l += pow(ccoords[i*dims+k] - cluster_coords[j*dims+k],2);
									if( l < lmin )
									{
										lmin = l;
										id_nearest_center = j;
									}
								}					
								if(id_old_cluster != id_nearest_center)
								{
									cpart[i] = id_nearest_center;
									changed++;
								}
							}
							std::cout << "iter " << iter << " changed " << changed << std::endl;
							//no change in cluster positions
							if(changed == 0 || iter >= max_iterations)
								break;
							for(int i = 0; i < parts; i++)
							{
								for(int k = 0; k < dims; ++k)
									cluster_coords[i*dims+k] = 0;
								cluster_npoints[i] = 0;
							}
							// recalculating the center of each cluster
#pragma omp parallel
							{
								std::vector< double > local_cluster_coords(parts*dims,0.0);
								std::vector< int > local_npoints(parts,0);
#pragma omp for
								for(int64_t j = 0; j < static_cast<int64_t>(ncells); ++j)
								{
									for(int k = 0; k < dims; ++k)
										local_cluster_coords[cpart[j]*dims+k] += ccoords[j*dims+k];
									local_npoints[cpart[j]]++;
								}
#pragma omp critical
								{
									for(int i = 0; i < parts; ++i)
									{
										for(int k = 0; k < dims; ++k)
											cluster_coords[i*dims+k] += local_cluster_coords[i*dims+k];
										cluster_npoints[i] += local_npoints[i];
									}
								}
							}
							for(int i = 0; i < parts; i++)
							{
								for(int k = 0; k < dims; ++k)
									cluster_coords[i*dims+k] /= static_cast<double>(cluster_npoints[i]);
							}
							iter++;
						}
					}
				}
				
				std::cout << "Collect nodes for each part." << std::endl;
				
				//read cell by cell and setup partitioning for nodes
				ppart.resize(npoints,-1);
				{
					std::streampos pos_connectivity_running = pos_connectivity;
					std::streampos pos_offsets_running = pos_offsets;
					size_t offset, offset0 = 0, size, point;
					for(size_t i = 0; i < ncells; ++i)
					{
						if( load_data )
							offset = offsets[i];
						else
						{
							f.seekg(pos_offsets_running);
							if( f.fail() ) {std::cout << __FILE__ << ":" << __LINE__ << " input stream failure " << std::endl; return -1;}
							f >> offset;
							pos_offsets_running = f.tellg();
							f.seekg(pos_connectivity_running);
							if( f.fail() ) 	{std::cout << __FILE__ << ":" << __LINE__ << " input stream failure " << std::endl;	return -1;}
						}
						size = offset-offset0;
						for(size_t k = 0; k < size; ++k)
						{
							if( load_data )
								point = connectivity[offset0+k];
							else f >> point;
							ppart[point] = std::max(ppart[point],cpart[i]);
						}
						offset0 = offset;
						if( !load_data )
						{
							pos_connectivity_running = f.tellg();
							if( f.fail() ) { std::cout << __FILE__ << ":" << __LINE__ << " input stream failure " << std::endl; return -1;}
						}
					}
				}
				
				std::cout << "Compute extra nodes that belong to each partition." << std::endl;
				
				
				//calculate dependence of cells on nodes and mark partitioning
				{
					std::streampos pos_connectivity_running = pos_connectivity;
					std::streampos pos_offsets_running = pos_offsets;
					size_t offset, offset0 = 0, size, point;
					for(size_t i = 0; i < ncells; ++i)
					{
						if( load_data )
							offset = offsets[i];
						else
						{
							f.seekg(pos_offsets_running);
							if( f.fail() ) {std::cout << __FILE__ << ":" << __LINE__ << " input stream failure " << std::endl; return -1;}
							f >> offset;
							pos_offsets_running = f.tellg();
							f.seekg(pos_connectivity_running);
							if( f.fail() ) { std::cout << __FILE__ << ":" << __LINE__ << " input stream failure " << std::endl; return -1;}
						}
						size = offset-offset0;
						for(size_t k = 0; k < size; ++k)
						{
							if( load_data )
								point = connectivity[offset0+k];
							else f >> point;
							if( cpart[i] != ppart[point] )
								epoints[cpart[i]].insert(point);
						}
						offset0 = offset;
						if( !load_data )
						{
							pos_connectivity_running = f.tellg();
							if( f.fail() ) { std::cout << __FILE__ << ":" << __LINE__ << " input stream failure " << std::endl; return -1;}
						}
					}
				}
			
				std::vector<size_t> pnum(npoints); //index of point
				size_t wr;
				for(int k = 0; k < parts; ++k)
				{
					//mark nodes that should be in the part
					std::cout << "Prepare nodes for partition " << k << std::endl;
					size_t npointsk = 0;
					size_t ncellsk = 0;

					std::cout << "Enumerate nodes for partition " << k << std::endl;
					for(size_t m = 0; m < npoints; ++m)
						if( ppart[m] == k || epoints[k].count(m) ) pnum[m] = npointsk++;
						else pnum[m] = mone_size_t;
						
					for(size_t m = 0; m < ncells; ++m)
						if( cpart[m] == k ) ncellsk++;
					
					std::cout << "Output file " << filenames[k] << " for part " << k << std::endl;
					
					
					std::cout << npointsk << " nodes" << std::endl;
					std::cout << ncellsk << " cells" << std::endl;
					
					std::ofstream fo(filenames[k].c_str());
					
					fo << "<VTKFile type=\"UnstructuredGrid\">" << std::endl;
					fo << "\t<UnstructuredGrid>" << std::endl;
					fo << "\t\t<Piece NumberOfPoints=\"" << npointsk << "\" NumberOfCells=\"" << ncellsk << "\">" << std::endl;
					
					std::cout << "Write points coordinates for partition " << k << std::endl;
					
					fo << "\t\t\t<Points>" << std::endl;
					fo << "\t\t\t\t<DataArray type=\"Float64\" NumberOfComponents=\"" << dims << "\" Format=\"ascii\">" << std::endl;
					wr = 0;
					for(size_t m = 0; m < npoints; ++m)	
					{
						if( ppart[m] == k || epoints[k].count(m) )
						{
							if( wr % (16/dims) == 0 ) fo << "\t\t\t\t  ";
							for(int d = 0; d < dims; ++d)
								fo << pcoords[m*dims + d] << " ";
							wr++;
							if( wr % (16/dims) == 0 ) fo << std::endl;
						}
					}
					if( wr % (16/dims) != 0 ) fo << std::endl;
					fo << "\t\t\t\t</DataArray>" << std::endl;
					fo << "\t\t\t</Points>" << std::endl;
					
					std::cout << "Write cells connectivity for partition " << k << std::endl;
					
					fo << "\t\t\t<Cells>" << std::endl;
					fo << "\t\t\t\t<DataArray type=\"UInt64\" Name=\"connectivity\" Format=\"ascii\">" << std::endl;
					{
						std::streampos pos_connectivity_running = pos_connectivity;
						std::streampos pos_offsets_running = pos_offsets;
						size_t offset, offset0 = 0, size, point;
						wr = 0;
						for(size_t m = 0; m < ncells; ++m)
						{
							if( load_data )
								offset = offsets[m];
							else
							{
								f.seekg(pos_offsets_running);
								if( f.fail() ) { std::cout << __FILE__ << ":" << __LINE__ << " input stream failure " << std::endl; return -1;}
								f >> offset;
								pos_offsets_running = f.tellg();
								f.seekg(pos_connectivity_running);
								if( f.fail() ) { std::cout << __FILE__ << ":" << __LINE__ << " input stream failure " << std::endl; return -1;}
							}
							size = offset-offset0;
							if( cpart[m] == k )
							{
								for(size_t q = 0; q < size; ++q)
								{
									if( load_data )
										point = connectivity[offset0+q];
									else f >> point;
									if( wr % 16 == 0 ) fo << "\t\t\t\t  ";
									fo << pnum[point] << " ";
									wr++;
									if( wr % 16 == 0 ) fo << std::endl;
									if( !(ppart[point] == k || epoints[k].count(point)) )
										std::cout << "Error, connection to point " << point << " from cell " << m << " didn't get correct part number, got " << ppart[point] << std::endl;
									if( pnum[point] == mone_size_t )
										std::cout << "Error, connection to point " << point << " from cell " << m << " was not enumerated" << std::endl;
								}
							}
							else if( !load_data ) for(size_t q = 0; q < size; ++q) f >> point;
							offset0 = offset;
							if( !load_data )
							{
								pos_connectivity_running = f.tellg();
								if( f.fail() ) { std::cout << __FILE__ << ":" << __LINE__ << " input stream failure " << std::endl; return -1;}
							}
						}
						if( wr % 16 != 0 ) fo << std::endl;
					}
					fo << "\t\t\t\t</DataArray>" << std::endl;
					
					std::cout << "Write cells offsets for partition " << k << std::endl;
					
					fo << "\t\t\t\t<DataArray type=\"UInt64\" Name=\"offsets\" Format=\"ascii\">" << std::endl;
					wr = 0;
					{
						size_t offset, offset0 = 0, size, offsetr = 0;
						if( !load_data )
							f.seekg(pos_offsets);
						for(size_t m = 0; m < ncells; ++m)
						{
							if( load_data )
								offset = offsets[m];
							else f >> offset;
							size = offset - offset0;
							offset0 = offset;
							if( cpart[m] == k )
							{
								offsetr += size;
								if( wr % 16 == 0 ) fo << "\t\t\t\t  ";
								fo << offsetr << " ";
								wr++;
								if( wr % 16 == 0 ) fo << std::endl;
							}
						}
						if( wr % 16 != 0 ) fo << std::endl;
					}
					fo << "\t\t\t\t</DataArray>" << std::endl;
					
					std::cout << "Write cells types for partition " << k << std::endl;
					
					fo << "\t\t\t\t<DataArray type=\"UInt8\" Name=\"types\" Format=\"ascii\">" << std::endl;
					wr = 0;
					{
						int ctype;
						if( !load_data )
							f.seekg(pos_types);
						for(size_t m = 0; m < ncells; ++m)	
						{
							if( load_data )
								ctype = types[m];
							else f >> ctype;
							if( cpart[m] == k )
							{
								if( wr % 16 == 0 ) fo << "\t\t\t\t  ";
								fo << ctype << " ";
								wr++;
								if( wr % 16 == 0 ) fo << std::endl;
							}
						}
						if( wr % 16 != 0 ) fo << std::endl;
					}
					fo << "\t\t\t\t</DataArray>" << std::endl;
					if( have_faces )
					{
						std::cout << "Write cells faces offsets for partition " << k << std::endl;
						size_t offset, offset0 = 0, size, offsetr = 0;
						if( !load_data )
							f.seekg(pos_faceoffsets);
						fo << "\t\t\t\t<DataArray type=\"UInt64\" Name=\"faceoffsets\" Format=\"ascii\">" << std::endl;
						wr = 0;
						for(size_t m = 0; m < ncells; ++m)
						{
							if( load_data )
								offset = faceoffsets[m];
							else
								f >> offset;
							if( offset == mone_size_t )
							{
								if( cpart[m] == k )
								{
									if( wr % 16 == 0 ) fo << "\t\t\t\t  ";
									fo << -1 << " ";
									wr++;
									if( wr % 16 == 0 ) fo << std::endl;
								}
								continue;
							}
							size = offset - offset0;
							offset0 = offset;
							if( cpart[m] == k )
							{
								offsetr += size;
								if( wr % 16 == 0 ) fo << "\t\t\t\t  ";
								fo << offsetr << " ";
								wr++;
								if( wr % 16 == 0 ) fo << std::endl;
							}
						}
						if( wr % 16 != 0 ) fo << std::endl;
						fo << "\t\t\t\t</DataArray>" << std::endl;
						std::streampos pos_faces_running = pos_faces;
						std::streampos pos_faceoffsets_running = pos_faceoffsets;
						size_t point, nfaces, npoints, reads;
						offset0 = 0;
						std::cout << "Write cells faces connectivity for partition " << k << std::endl;
						fo << "\t\t\t\t<DataArray type=\"UInt64\" Name=\"faces\" Format=\"ascii\">" << std::endl;
						wr = 0;
						for(size_t m = 0; m < ncells; ++m)
						{
							if( load_data )
								offset = faceoffsets[m];
							else
							{
								f.seekg(pos_faceoffsets_running);
								if( f.fail() ) {std::cout << __FILE__ << ":" << __LINE__ << " input stream failure " << std::endl; return -1;}
								f >> offset;
								pos_faceoffsets_running = f.tellg();
							}
							if( offset == mone_size_t ) continue;
							size = offset-offset0;
							if( !load_data )
							{
								f.seekg(pos_faces_running);
								if( f.fail() ) {std::cout << __FILE__ << ":" << __LINE__ << " input stream failure " << std::endl; return -1; }
							}
							if( cpart[m] == k )
							{
								if( load_data )
									nfaces = faces[offset0];
								else f >> nfaces;
								if( nfaces+1 > size ) std::cout << "Error, something is inconsistent, total reads " << size << " but number of faces " << nfaces << std::endl;
								reads = 1;
								fo << "\t\t\t\t  ";
								fo << nfaces << " ";
								//~ std::cout << "nfaces " << nfaces;
								for(size_t q = 0; q < nfaces; ++q)
								{
									if( load_data )
										npoints = faces[offset0+reads];
									else f >> npoints;
									reads++;
									fo << npoints << " ";
									//~ std::cout << " " << npoints;
									for(size_t p = 0; p < npoints; ++p)
									{
										if( load_data )
											point = faces[offset0+reads];
										else f >> point;
										reads++;
										fo << pnum[point] << " ";
										if( !(ppart[point] == k || epoints[k].count(point)) )
											std::cout << "Error, connection to point " << point << " from cell " << m << " didn't get correct part number, got " << ppart[point] << std::endl;
										if( pnum[point] == mone_size_t )
											std::cout << "Error, connection to point " << point << " from cell " << m << " was not enumerated" << std::endl;
									}
									if( reads > size ) std::cout << "Error, something is inconsistent, total reads " << size << " but current reads " << reads << " number of faces " << nfaces << " points in face " << npoints << std::endl;
								}
								fo << std::endl;
								//~ std::cout << std::endl;
								if( reads != size )
									std::cout << "Error, inconsistent number of reads " << reads << " with offset size " << size << std::endl;
							}
							else if( !load_data ) for(size_t q = 0; q < size; ++q) f >> point;
							offset0 = offset;
							if( !load_data )
							{
								pos_faces_running = f.tellg();
								if( f.fail() ) {std::cout << __FILE__ << ":" << __LINE__ << " input stream failure " << std::endl; return -1; }
							}
						}
						fo << "\t\t\t\t</DataArray>" << std::endl;
						
						std::cout << "Done with cells faces for partition " << k << std::endl;
					}
					fo << "\t\t\t</Cells>" << std::endl;	
				}
			}
			
			std::cout << "Write nodes data" << std::endl;
//#pragma omp parallel for
			for(int k = 0; k < parts; ++k)
			{
				std::ofstream fo(filenames[k].c_str(),std::ios::app); //append to the end
				fo << "\t\t\t<PointData>" << std::endl;
				
				
				std::cout << "Write nodes part data for partition " << k << std::endl;
				
				fo << "\t\t\t\t<DataArray";
				fo << " type=\"Int32\"";
				fo << " Name=\"part\"";
				fo << " Format=\"ascii\"";
				fo << ">" << std::endl;
				size_t wr = 0;
				for(size_t m = 0; m < npoints; ++m)
				{
					if( ppart[m] == k || epoints[k].count(m) )
					{
						if( wr % 16 == 0 ) fo << "\t\t\t\t  ";
						fo << ppart[m] << " ";
						wr++;
						if( wr % 16 == 0 ) fo << std::endl;
					}
				}
				if( wr % 16 != 0 ) fo << std::endl;
				fo << "\t\t\t\t</DataArray>" << std::endl;
			}
			
			
			for(size_t m = 0; m < pointdata_name.size(); ++m)
			{
				std::cout << "Writing node data " << pointdata_name[m] << std::endl;
				f.seekg(pointdata_pos[m]);
				if( f.fail() ) {std::cout << __FILE__ << ":" << __LINE__ << " input stream failure " << std::endl; return -1;}
				//preload data here
				void * data = preloaddata(f,npoints,pointdata_comps[m],pointdata_type[m]);
//#pragma omp parallel for	
				for(int k = 0; k < parts; ++k)
				{
					std::cout << "Prepare nodes for partition " << k << std::endl;
					std::ofstream fo(filenames[k].c_str(),std::ios::app); //append to the end
					
					std::cout << "Write node data " << pointdata_name[m] << " for partition " << k << std::endl;
					
					fo << "\t\t\t\t<DataArray";
					fo << " type=\"" << pointdata_type[m] << "\"";
					fo << " Name=\"" << pointdata_name[m] << "\"";
					fo << " Format=\"ascii\"";
					if( pointdata_comps[m] != 1 )
						fo << " NumberOfComponents=\"" << pointdata_comps[m] << "\"";
					fo << ">" << std::endl;
					
					f.seekg(pointdata_pos[m]);
					if( f.fail() ) 
					{
						std::cout << __FILE__ << ":" << __LINE__ << " input stream failure " << std::endl;
						return -1;
					}
					transferdata(fo,npoints,pointdata_comps[m],ppart,epoints[k],k,pointdata_type[m],data);
					
					fo << "\t\t\t\t</DataArray>" << std::endl;
				}
				
				releasedata(pointdata_type[m],data);
			}
			
//#pragma omp parallel for
			for(int k = 0; k < parts; ++k)
			{
				std::ofstream fo(filenames[k].c_str(),std::ios::app); //append to the end
				fo << "\t\t\t</PointData>" << std::endl;
			}
		}
		
		std::cout << "Write cells data" << std::endl;
		
//#pragma omp parallel for
		for(int k = 0; k < parts; ++k)
		{	
			std::cout << "Write cell part data for partition " << k << std::endl;
			std::ofstream fo(filenames[k].c_str(),std::ios::app); //append to the end
			fo << "\t\t\t<CellData>" << std::endl;
			fo << "\t\t\t\t<DataArray";
			fo << " type=\"Int32\"";
			fo << " Name=\"part\"";
			fo << " Format=\"ascii\"";
			fo << ">" << std::endl;
			size_t wr = 0;
			for(size_t m = 0; m < ncells; ++m)
			{
				if( cpart[m] == k )
				{
					if( wr % 16 == 0 ) fo << "\t\t\t\t  ";
					fo << k << " ";
					wr++;
					if( wr % 16 == 0 ) fo << std::endl;
				}
			}
			if( wr % 16 != 0 ) fo << std::endl;
			fo << "\t\t\t\t</DataArray>" << std::endl;
		}
		
			
		for(size_t m = 0; m < celldata_name.size(); ++m)
		{
			std::cout << "Writing cell data " << celldata_name[m] << std::endl;
			f.seekg(celldata_pos[m]);
			if( f.fail() ) {std::cout << __FILE__ << ":" << __LINE__ << " input stream failure " << std::endl; return -1;}
			//preload data here
			void * data = preloaddata(f,ncells,celldata_comps[m],celldata_type[m]);
//#pragma omp parallel for
			for(int k = 0; k < parts; ++k)
			{
				std::cout << "Write cell data " << celldata_name[m] << " for partition " << k << std::endl;
				std::ofstream fo(filenames[k].c_str(),std::ios::app); //append to the end
				fo << "\t\t\t\t<DataArray";
				fo << " type=\"" << celldata_type[m] << "\"";
				fo << " Name=\"" << celldata_name[m] << "\"";
				fo << " Format=\"ascii\"";
				if( celldata_comps[m] != 1 )
					fo << " NumberOfComponents=\"" << celldata_comps[m] << "\"";
				fo << ">" << std::endl;
				
				
				transferdata(fo,ncells,celldata_comps[m],cpart,std::set<size_t>(),k,celldata_type[m],data);
				fo << "\t\t\t\t</DataArray>" << std::endl;
			}
			releasedata(celldata_type[m],data);
		}
		
		std::cout << "Finalize cells data" << std::endl;
//#pragma omp parallel for
		for(int k = 0; k < parts; ++k)
		{
			std::ofstream fo(filenames[k].c_str(),std::ios::app); //append to the end
			fo << "\t\t\t</CellData>" << std::endl;
			fo << "\t\t</Piece>" << std::endl;
			fo << "\t</UnstructuredGrid>" << std::endl;
			fo << "</VTKFile>" << std::endl;
		}
		
		
		std::cout << "Output header file " << output << std::endl;
		std::ofstream fh(output.c_str());
		
		fh << "<VTKFile type=\"PUnstructuredGrid\">" << std::endl;
		fh << "\t<PUnstructuredGrid GhostLevel=\"0\">" << std::endl;
		fh << "\t\t<PPoints>" << std::endl;
		fh << "\t\t\t<PDataArray";
		fh << " type=\"Float64\"";
		fh << " NumberOfComponents=\"" << dims << "\"";
		fh << " Format=\"ascii\"";
		fh << "/>" << std::endl;
		fh << "\t\t</PPoints>" << std::endl;
		fh << "\t\t<PPointData>" << std::endl;
		fh << "\t\t\t<PDataArray";
		fh << " type=\"Int32\"";
		fh << " Name=\"part\"";
		fh << " Format=\"ascii\"";
		fh << "/>" << std::endl;
		for(size_t k = 0; k < pointdata_name.size(); ++k)
		{
			fh << "\t\t\t<PDataArray";
			fh << " type=\"" << pointdata_type[k] << "\"";
			fh << " Name=\"" << pointdata_name[k] << "\"";
			fh << " Format=\"ascii\"";
			if( pointdata_comps[k] != 1 )
				fh << " NumberOfComponents=\"" << pointdata_comps[k] << "\"";
			fh << "/>" << std::endl;
		}
		fh << "\t\t</PPointData>" << std::endl;
		fh << "\t\t<PCellData>" << std::endl;
		fh << "\t\t\t<PDataArray";
		fh << " type=\"Int32\"";
		fh << " Name=\"part\"";
		fh << " Format=\"ascii\"";
		fh << "/>" << std::endl;
		for(size_t k = 0; k < celldata_name.size(); ++k)
		{
			fh << "\t\t\t<PDataArray";
			fh << " type=\"" << celldata_type[k] << "\"";
			fh << " Name=\"" << celldata_name[k] << "\"";
			fh << " Format=\"ascii\"";
			if( celldata_comps[k] != 1 )
				fh << " NumberOfComponents=\"" << celldata_comps[k] << "\"";
			fh << "/>" << std::endl;
		}
		fh << "\t\t</PCellData>" << std::endl;
		
		
		for(int k = 0; k < parts; ++k)
			fh << "\t\t<Piece Source=\"" << filenames[k].substr(filenames[k].find_last_of("/\\")+1,filenames[k].length()) << "\"/>" << std::endl; 
		
		fh << "\t</PUnstructuredGrid>" << std::endl;
		fh << "</VTKFile>" << std::endl;
		
		fh.close();
		
		
	}
	return 0;
}
