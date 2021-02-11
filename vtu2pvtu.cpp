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
void transferdata(std::istream & f, std::ostream & fh, size_t nelems, std::vector<int> & epart, int part, int comps)
{
	size_t wr = 0;
	for(size_t q = 0; q < nelems; ++q)
	{
		dtype v;
		if( epart[q] == part )
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
		//read all the coordinates
		size_t npoints = 0, ncells = 0;
		int dims = 0;
		std::vector<float> pcoords;
		//file positions
		std::streampos pos_connectivity = 0, pos_offsets = 0, pos_types = 0, pos_faces = 0, pos_faceoffsets = 0;
		bool have_faces = false;
		//data
		std::vector<std::string> pointdata_name;
		std::vector<std::string> pointdata_type;
		std::vector<int> pointdata_comps;
		std::vector<std::streampos> pointdata_pos;
		std::vector<std::string> celldata_name;
		std::vector<std::string> celldata_type;
		std::vector<int> celldata_comps;
		std::vector<std::streampos> celldata_pos;
		
		std::cout << "Read " << input << " for node coordinates and data positions in file." << std::endl;
		
		std::ifstream f(input.c_str());
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
		
		std::cout << "Calculate centers of cells." << std::endl;
		
		std::vector<float> ccoords(ncells*dims,0.f);
		{
			std::streampos pos_connectivity_running = pos_connectivity;
			std::streampos pos_offsets_running = pos_offsets;
			size_t offset, offset0 = 0, size, point;
			for(size_t i = 0; i < ncells; ++i)
			{
				f.seekg(pos_offsets_running);
				if( f.fail() ) 
				{
					std::cout << __FILE__ << ":" << __LINE__ << " input stream failure " << std::endl;
					return -1;
				}
				f >> offset;
				size = offset-offset0;
				offset0 = offset;
				pos_offsets_running = f.tellg();
				f.seekg(pos_connectivity_running);
				if( f.fail() ) 
				{
					std::cout << __FILE__ << ":" << __LINE__ << " input stream failure " << std::endl;
					return -1;
				}
				for(size_t k = 0; k < size; ++k)
				{
					f >> point;
					for(int q = 0; q < dims; ++q)
						ccoords[i*dims+q] += pcoords[point*dims+q];					
				}
				for(int q = 0; q < dims; ++q)
					ccoords[i*dims+q] /= static_cast<float>(size);
				pos_connectivity_running = f.tellg();
			}
		}
		
		std::cout << "Cluster cell centers into " << parts << " parts." << std::endl;
		
		//separate points by processors with K-means
		std::vector< int > cpart(ncells,-1);
		{
			std::vector< float > cluster_coords(parts*dims,0.0);
			std::vector< int > cluster_npoints(parts,0);
			//select seed points for clusters
			{
				srand(time(NULL));
				for(int i = 0; i < parts; i++) if( cluster_npoints[i] == 0 )
				{
					while(true)
					{
						size_t index = rand() % ncells;
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
					
					float lmin = std::numeric_limits<float>::max();
					
					for(int j = 0; j < parts; ++j)
					{
						float l = 0;
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
					std::vector< float > local_cluster_coords(parts*dims,0.0);
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
						cluster_coords[i*dims+k] /= static_cast<float>(cluster_npoints[i]);
				}
				iter++;
			}
		}
		
		std::cout << "Collect nodes for each part." << std::endl;
		
		//read cell by cell and setup partitioning for nodes
		std::vector<int> ppart(npoints,-1);
		{
			std::streampos pos_connectivity_running = pos_connectivity;
			std::streampos pos_offsets_running = pos_offsets;
			size_t offset, offset0 = 0, size, point;
			for(size_t i = 0; i < ncells; ++i)
			{
				f.seekg(pos_offsets_running);
				if( f.fail() ) 
				{
					std::cout << __FILE__ << ":" << __LINE__ << " input stream failure " << std::endl;
					return -1;
				}
				f >> offset;
				size = offset-offset0;
				offset0 = offset;
				pos_offsets_running = f.tellg();
				f.seekg(pos_connectivity_running);
				if( f.fail() ) 
				{
					std::cout << __FILE__ << ":" << __LINE__ << " input stream failure " << std::endl;
					return -1;
				}
				for(size_t k = 0; k < size; ++k)
				{
					f >> point;
					ppart[point] = std::max(ppart[point],cpart[i]);
				}
				pos_connectivity_running = f.tellg();
			}
		}
		
		std::cout << "Compute extra nodes that belong to each partition." << std::endl;
		
		std::vector< std::set<size_t> > epoints(parts); //additional nodes that should present on processor
		//calculate dependence of cells on nodes and mark partitioning
		{
			std::streampos pos_connectivity_running = pos_connectivity;
			std::streampos pos_offsets_running = pos_offsets;
			size_t offset, offset0 = 0, size, point;
			for(size_t i = 0; i < ncells; ++i)
			{
				f.seekg(pos_offsets_running);
				if( f.fail() ) 
				{
					std::cout << __FILE__ << ":" << __LINE__ << " input stream failure " << std::endl;
					return -1;
				}
				f >> offset;
				size = offset-offset0;
				offset0 = offset;
				pos_offsets_running = f.tellg();
				f.seekg(pos_connectivity_running);
				if( f.fail() ) 
				{
					std::cout << __FILE__ << ":" << __LINE__ << " input stream failure " << std::endl;
					return -1;
				}
				for(size_t k = 0; k < size; ++k)
				{
					f >> point;
					if( cpart[i] != ppart[point] )
						epoints[cpart[i]].insert(point);
				}
				pos_connectivity_running = f.tellg();
			}
		}
		
		std::cout << "Output header file " << output << std::endl;
		std::ofstream fh(output.c_str());
		
		fh << "<VTKFile type=\"PUnstructuredGrid\">" << std::endl;
		fh << "\t<PUnstructuredGrid GhostLevel=\"0\">" << std::endl;
		fh << "\t\t<PPointData>" << std::endl;
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
		fh << "\t\t\t<PDataArray";
		fh << " type=\"Int32\"";
		fh << " Name=\"part\"";
		fh << " Format=\"ascii\"";
		fh << "/>" << std::endl;
		fh << "\t\t</PPointData>" << std::endl;
		fh << "\t\t<PCellData>" << std::endl;
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
		fh << "\t\t\t<PDataArray";
		fh << " type=\"Int32\"";
		fh << " Name=\"part\"";
		fh << " Format=\"ascii\"";
		fh << "/>" << std::endl;
		fh << "\t\t</PCellData>" << std::endl;
		fh << "\t\t<PPoints>" << std::endl;
		fh << "\t\t\t<PDataArray";
		fh << " type=\"Float32\"";
		fh << " NumberOfComponents=\"" << dims << "\"";
		fh << " Format=\"ascii\"";
		fh << "/>" << std::endl;
		fh << "\t\t</PPoints>" << std::endl;
		
		std::vector<size_t> pnum(npoints); //index of point
		std::vector<int> ppartk(npoints);
		std::string filename;
		int digits = getdigits(parts), digitsk = 0;
		size_t wr;
		for(int k = 0; k < parts; ++k)
		{
			//mark nodes that should be in the part
			std::cout << "Prepare nodes for partition " << k << std::endl;
			size_t npointsk = 0;
			size_t ncellsk = 0;
			ppartk = ppart;
			for(std::set<size_t>::iterator it = epoints[k].begin(); it != epoints[k].end(); ++it)
				ppartk[*it] = k;
				
			std::cout << "Enumerate nodes for partition " << k << std::endl;
			for(size_t m = 0; m < npoints; ++m)
				if( ppartk[m] == k ) pnum[m] = npointsk++;
				else pnum[m] = -1;
				
			for(size_t m = 0; m < ncells; ++m)
				if( cpart[m] == k ) ncellsk++;
			
			digitsk = getdigits(k);
			filename = output.substr(0,output.size()-5); //strip .pvtu
			for(int j = 0; j < digits - digitsk; ++j) filename += '0';
			filename += strconvert(k);
			filename += ".vtu";
			std::cout << "Output file " << filename << " for part " << k << std::endl;
			
			fh << "\t\t<Piece Source=\"" << filename << "\"/>" << std::endl; 
			std::cout << npointsk << " nodes" << std::endl;
			std::cout << ncellsk << " cells" << std::endl;
			
			std::ofstream fo(filename.c_str());
			
			fo << "<VTKFile type=\"UnstructuredGrid\">" << std::endl;
			fo << "\t<UnstructuredGrid>" << std::endl;
			fo << "\t\t<Piece NumberOfPoints=\"" << npointsk << "\" NumberOfCells=\"" << ncellsk << "\">" << std::endl;
			
			std::cout << "Write points data for partition " << k << std::endl;
			
			fo << "\t\t\t<PointData>" << std::endl;
			for(size_t m = 0; m < pointdata_name.size(); ++m)
			{
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
				if( pointdata_type[m] == "Float64" )
					transferdata<double>(f,fo,npoints,ppartk,k,pointdata_comps[m]);
				else if( pointdata_type[m] == "Float32" )
					transferdata<float>(f,fo,npoints,ppartk,k,pointdata_comps[m]);
				else if( pointdata_type[m] == "Int64" )
					transferdata<int64_t>(f,fo,npoints,ppartk,k,pointdata_comps[m]);
				else if( pointdata_type[m] == "UInt64" )
					transferdata<uint64_t>(f,fo,npoints,ppartk,k,pointdata_comps[m]);
				else if( pointdata_type[m] == "Int32" )
					transferdata<int32_t>(f,fo,npoints,ppartk,k,pointdata_comps[m]);
				else if( pointdata_type[m] == "UInt32" )
					transferdata<uint32_t>(f,fo,npoints,ppartk,k,pointdata_comps[m]);
				else if( pointdata_type[m] == "Int16" )
					transferdata<int16_t>(f,fo,npoints,ppartk,k,pointdata_comps[m]);
				else if( pointdata_type[m] == "UInt16" )
					transferdata<uint16_t>(f,fo,npoints,ppartk,k,pointdata_comps[m]);
				else if( pointdata_type[m] == "Int8" )
					transferdata<int8_t>(f,fo,npoints,ppartk,k,pointdata_comps[m]);
				else if( pointdata_type[m] == "UInt8" )
					transferdata<uint8_t>(f,fo,npoints,ppartk,k,pointdata_comps[m]);
				
				fo << "\t\t\t\t</DataArray>" << std::endl;
			}
			fo << "\t\t\t\t<DataArray";
			fo << " type=\"Int32\"";
			fo << " Name=\"part\"";
			fo << " Format=\"ascii\"";
			fo << ">" << std::endl;
			wr = 0;
			for(size_t m = 0; m < npoints; ++m)
			{
				if( ppartk[m] == k )
				{
					if( wr % 16 == 0 ) fo << "\t\t\t\t  ";
					fo << ppart[m] << " ";
					wr++;
					if( wr % 16 == 0 ) fo << std::endl;
				}
			}
			if( wr % 16 != 0 ) fo << std::endl;
			fo << "\t\t\t\t</DataArray>" << std::endl;
			fo << "\t\t\t</PointData>" << std::endl;
			
			std::cout << "Write cell data for partition " << k << std::endl;
			
			fo << "\t\t\t<CellData>" << std::endl;
			
			for(size_t m = 0; m < celldata_name.size(); ++m)
			{
				fo << "\t\t\t\t<DataArray";
				fo << " type=\"" << celldata_type[m] << "\"";
				fo << " Name=\"" << celldata_name[m] << "\"";
				fo << " Format=\"ascii\"";
				if( celldata_comps[m] != 1 )
					fo << " NumberOfComponents=\"" << celldata_comps[m] << "\"";
				fo << ">" << std::endl;
				
				f.seekg(celldata_pos[m]);
				if( f.fail() ) 
				{
					std::cout << __FILE__ << ":" << __LINE__ << " input stream failure " << std::endl;
					return -1;
				}
				if( celldata_type[m] == "Float64" )
					transferdata<double>(f,fo,ncells,cpart,k,celldata_comps[m]);
				else if( celldata_type[m] == "Float32" )
					transferdata<float>(f,fo,ncells,cpart,k,celldata_comps[m]);
				else if( celldata_type[m] == "Int64" )
					transferdata<int64_t>(f,fo,ncells,cpart,k,celldata_comps[m]);
				else if( celldata_type[m] == "UInt64" )
					transferdata<uint64_t>(f,fo,ncells,cpart,k,celldata_comps[m]);
				else if( celldata_type[m] == "Int32" )
					transferdata<int32_t>(f,fo,ncells,cpart,k,celldata_comps[m]);
				else if( celldata_type[m] == "UInt32" )
					transferdata<uint32_t>(f,fo,ncells,cpart,k,celldata_comps[m]);
				else if( celldata_type[m] == "Int16" )
					transferdata<int16_t>(f,fo,ncells,cpart,k,celldata_comps[m]);
				else if( celldata_type[m] == "UInt16" )
					transferdata<uint16_t>(f,fo,ncells,cpart,k,celldata_comps[m]);
				else if( celldata_type[m] == "Int8" )
					transferdata<int8_t>(f,fo,ncells,cpart,k,celldata_comps[m]);
				else if( celldata_type[m] == "UInt8" )
					transferdata<uint8_t>(f,fo,ncells,cpart,k,celldata_comps[m]);
				
				fo << "\t\t\t\t</DataArray>" << std::endl;
			}
			fo << "\t\t\t\t<DataArray";
			fo << " type=\"Int32\"";
			fo << " Name=\"part\"";
			fo << " Format=\"ascii\"";
			fo << ">" << std::endl;
			wr = 0;
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
			
			fo << "\t\t\t</CellData>" << std::endl;
			
			std::cout << "Write points coordinates for partition " << k << std::endl;
			
			fo << "\t\t\t<Points>" << std::endl;
			fo << "\t\t\t\t<DataArray type=\"Float32\" NumberOfComponents=\"" << dims << "\" Format=\"ascii\">" << std::endl;
			wr = 0;
			for(size_t m = 0; m < npoints; ++m)	
			{
				if( ppartk[m] == k )
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
			//~ std::vector<size_t> offsets;
			{
				std::streampos pos_connectivity_running = pos_connectivity;
				std::streampos pos_offsets_running = pos_offsets;
				size_t offset, offset0 = 0, size, point;
				wr = 0;
				for(size_t m = 0; m < ncells; ++m)
				{
					f.seekg(pos_offsets_running);
					if( f.fail() ) 
					{
						std::cout << __FILE__ << ":" << __LINE__ << " input stream failure " << std::endl;
						return -1;
					}
					f >> offset;
					size = offset-offset0;
					offset0 = offset;
					pos_offsets_running = f.tellg();
					f.seekg(pos_connectivity_running);
					if( f.fail() ) 
					{
						std::cout << __FILE__ << ":" << __LINE__ << " input stream failure " << std::endl;
						return -1;
					}
					if( cpart[m] == k )
					{
						for(size_t q = 0; q < size; ++q)
						{
							f >> point;
							if( wr % 16 == 0 ) fo << "\t\t\t\t  ";
							fo << pnum[point] << " ";
							wr++;
							if( wr % 16 == 0 ) fo << std::endl;
							if( ppartk[point] != k )
								std::cout << "Error, connection to point " << point << " from cell " << m << " didn't get correct part number, got " << ppartk[point] << std::endl;
							if( pnum[point] == -1 )
								std::cout << "Error, connection to point " << point << " from cell " << m << " was not enumerated" << std::endl;
						}
						//~ fo << std::endl;
						//~ offsets.push_back(wr);
					}
					else for(size_t q = 0; q < size; ++q)
						f >> point;
					pos_connectivity_running = f.tellg();
				}
				if( wr % 16 != 0 ) fo << std::endl;
			}
			//~ if( offsets.size() != ncellsk )
				//~ std::cout << "Error, number of offsets " << offsets.size() << " does not match number of local cells " << std::endl;
			fo << "\t\t\t\t</DataArray>" << std::endl;
			
			std::cout << "Write cells offsets for partition " << k << std::endl;
			
			fo << "\t\t\t\t<DataArray type=\"UInt64\" Name=\"offsets\" Format=\"ascii\">" << std::endl;
			wr = 0;
			{
				size_t offset, offset0 = 0, size, offsetr = 0;
				f.seekg(pos_offsets);
				for(size_t m = 0; m < ncells; ++m)
				{
					//~ fo << offsets[m] << " ";
					f >> offset;
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
			f.seekg(pos_types);
			int ctype;
			wr = 0;
			for(size_t m = 0; m < ncells; ++m)	
			{
				f >> ctype;
				if( f.fail() ) 
				{
					std::cout << __FILE__ << ":" << __LINE__ << " input stream failure " << std::endl;
					return -1;
				}
				if( cpart[m] == k )
				{
					if( wr % 16 == 0 ) fo << "\t\t\t\t  ";
					fo << ctype << " ";
					wr++;
					if( wr % 16 == 0 ) fo << std::endl;
				}
			}
			if( wr % 16 != 0 ) fo << std::endl;
			fo << "\t\t\t\t</DataArray>" << std::endl;
			if( have_faces )
			{
				size_t offset, offset0 = 0, size, offsetr = 0;
				f.seekg(pos_faceoffsets);
				std::cout << "Write cells faces offsets for partition " << k << std::endl;
				fo << "\t\t\t\t<DataArray type=\"UInt64\" Name=\"faceoffsets\" Format=\"ascii\">" << std::endl;
				wr = 0;
				for(size_t m = 0; m < ncells; ++m)
				{
					f >> offset;
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
					f.seekg(pos_faceoffsets_running);
					if( f.fail() ) 
					{
						std::cout << __FILE__ << ":" << __LINE__ << " input stream failure " << std::endl;
						return -1;
					}
					f >> offset;
					pos_faceoffsets_running = f.tellg();
					if( offset == -1 ) continue;
					size = offset-offset0;
					offset0 = offset;
					f.seekg(pos_faces_running);
					if( f.fail() ) 
					{
						std::cout << __FILE__ << ":" << __LINE__ << " input stream failure " << std::endl;
						return -1;
					}
					if( cpart[m] == k )
					{
						f >> nfaces;
						if( nfaces+1 > size ) std::cout << "Error, something is inconsistent, total reads " << size << " but number of faces " << nfaces << std::endl;
						reads = 1;
						fo << "\t\t\t\t  ";
						fo << nfaces << " ";
						//~ std::cout << "nfaces " << nfaces;
						for(size_t q = 0; q < nfaces; ++q)
						{
							f >> npoints;
							reads++;
							fo << npoints << " ";
							//~ std::cout << " " << npoints;
							for(size_t p = 0; p < npoints; ++p)
							{
								f >> point;
								reads++;
								fo << pnum[point] << " ";
								if( ppartk[point] != k )
									std::cout << "Error, connection to point " << point << " from cell " << m << " didn't get correct part number, got " << ppartk[point] << std::endl;
								if( pnum[point] == -1 )
									std::cout << "Error, connection to point " << point << " from cell " << m << " was not enumerated" << std::endl;
							}
							if( reads > size ) std::cout << "Error, something is inconsistent, total reads " << size << " but current reads " << reads << " number of faces " << nfaces << " points in face " << npoints << std::endl;
						}
						fo << std::endl;
						//~ std::cout << std::endl;
						if( reads != size )
							std::cout << "Error, inconsistent number of reads " << reads << " with offset size " << size << std::endl;
					}
					else for(size_t q = 0; q < size; ++q)
						f >> point;
					pos_faces_running = f.tellg();
				}
				fo << "\t\t\t\t</DataArray>" << std::endl;
				
				std::cout << "Done with cells faces for partition " << k << std::endl;
			}
			fo << "\t\t\t</Cells>" << std::endl;
			fo << "\t\t</Piece>" << std::endl;
			fo << "\t</UnstructuredGrid>" << std::endl;
			fo << "</VTKFile>" << std::endl;
			
			fo.close();
			
			std::cout << "Closed " << filename << std::endl;
		}
		
		fh << "\t</PUnstructuredGrid>" << std::endl;
		fh << "</VTKFile>" << std::endl;
		
		fh.close();
		
		
	}
	return 0;
}
