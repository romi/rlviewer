#include <vector>
#include <stdio.h>
#include <string>
#include <cstring>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include <glm/glm.hpp>
#include <iostream>
#include <fstream>
#include "algorithm"

#include "objloader.hpp"

// VI = 10 10 10 10
// VI_VT = 10/20 10/20 10/20
// VI_VT_VN = 10/20/30 10/20/30 10/20/30
// VI_VN = 10//30 10//30 10//30
enum FACE_DATA_TYPE { VI, VI_VT, VI_VT_VN , VI_VN };



enum FACE_DATA_TYPE face_data_type(std::string line)
{
    FACE_DATA_TYPE faceDataType;
    std::string face;
    std::istringstream line_stream(line);
    std::getline(line_stream, face, ' ');

    int count = std::count(face.begin(), face.end(), '/');
    std::size_t double_found = face.find("//");
    if (count == 0)
        faceDataType = VI;
    else if (count == 1)
        faceDataType = VI_VT;
    else if (double_found != std::string::npos )
        faceDataType = VI_VN;
    else {
        faceDataType = VI_VT_VN;
    }
    return faceDataType;
}


void parse_face(FACE_DATA_TYPE face_date_type, std::string face_data, std::vector<unsigned int>& vertexIndices, std::vector<unsigned int>& uvIndices, std::vector<unsigned int>& normalIndices) {

    char slash;
    std::istringstream face_stream(face_data);
    int vertex = 0, uv = 1, normal = 0;
    // remove any space characters...
    face_data.erase(remove(face_data.begin(), face_data.end(), ' '), face_data.end());
    if (face_date_type == VI_VT){
        face_stream >> vertex >> slash >> uv;
    }
    else if (face_date_type == VI_VT_VN) {
        face_stream >> vertex >> slash >> uv >> slash >> normal;
    }
    else if (face_date_type == VI_VN){
        face_stream >> vertex >>  slash >>  slash >> normal;
    }
    else{
        // face_date_type == VI
        // NO NORMAL.. TBD
    }
    vertexIndices.push_back(vertex); uvIndices.push_back(uv); normalIndices.push_back(normal);
}


void parse_face_line(std::istringstream& line_stream, std::vector<unsigned int>& vertexIndices, std::vector<unsigned int>& uvIndices, std::vector<unsigned int>& normalIndices) {
    auto face_type = face_data_type(line_stream.str());
    std::string face;
    while (std::getline(line_stream, face, ' ')) {
        parse_face(face_type, face, vertexIndices, uvIndices, normalIndices);
    }
}


// Very, VERY simple OBJ loader.
// Here is a short list of features a real function would provide : 
// - Binary files. Reading a model should be just a few memcpy's away, not parsing a file at runtime. In short : OBJ is not very great.
// - Animations & bones (includes bones weights)
// - Multiple UVs
// - All attributes should be optional, not "forced"
// - More stable. Change a line in the OBJ file and it crashes.
// - More secure. Change another line and you can inject code.
// - Loading from memory, stream, etc

bool loadOBJ(
	const char * path, 
	std::vector<glm::vec3> & out_vertices, 
	std::vector<glm::vec2> & out_uvs,
	std::vector<glm::vec3> & out_normals
){
	printf("Loading OBJ file %s...\n", path);

	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<glm::vec3> temp_vertices; 
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;

    std::ifstream input_file(path, std::ios::in);
    if (!input_file) { std::cerr << "Cannot open " << path << std::endl;
        return false;
    }

    std::string line;
    while (getline(input_file, line)) {
        std::string line_header = line.substr(0,2);
        if (line_header == "v ") {
            std::cout << "parsing vertex v" << std::endl;

            std::istringstream line_stream(line.substr(2));
//            glm::vec4 v; s >> v.x; s >> v.y; s >> v.z; v.w = 1.0;
//            mesh->vertices.push_back(v);
            glm::vec3 vertex;
            line_stream >> vertex.x >> vertex.y >> vertex.z;
            temp_vertices.push_back(vertex);
        }
        else if (line_header == "vn") {
            std::cout << "parsing vertex v" << std::endl;
            std::istringstream line_stream(line.substr(2));
            glm::vec3 normal;
            line_stream >> normal.x >> normal.y >> normal.z;
            temp_normals.push_back(normal);
        }
        else if (line_header == "vt") {
            std::cout << "parsing vertex t" << std::endl;
            std::istringstream line_stream(line.substr(2));
            glm::vec2 uv;
            line_stream >> uv.x >> uv.y;
            uv.y = -uv.y;
            temp_uvs.push_back(uv);
        }
        else if (line_header == "f ") {
            std::cout << "parsing face" << std::endl;
            std::istringstream line_stream(line.substr(2));
            parse_face_line(line_stream, vertexIndices, uvIndices, normalIndices);
        }
        else if (line[0] == '#') { /* ignoring this line */ }
        else { /* ignoring this line */ }
    }
    std::cout << "parsed_ob" << std::endl;

	// For each vertex of each triangle
	for( unsigned int i=0; i<vertexIndices.size(); i++ ){

		// Get the indices of its attributes
		unsigned int vertexIndex = vertexIndices[i];
		unsigned int uvIndex = uvIndices[i];
		unsigned int normalIndex = normalIndices[i];
		
		// Get the attributes thanks to the index
		glm::vec3 vertex = temp_vertices[ vertexIndex-1 ];
		glm::vec2 uv = temp_uvs[ uvIndex-1 ];
		glm::vec3 normal = temp_normals[ normalIndex-1 ];
		
		// Put the attributes in buffers
		out_vertices.push_back(vertex);
		out_uvs     .push_back(uv);
		out_normals .push_back(normal);
	
	}
	return true;
}


#ifdef USE_ASSIMP // don't use this #define, it's only for me (it AssImp fails to compile on your machine, at least all the other tutorials still work)

// Include AssImp
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

bool loadAssImp(
	const char * path, 
	std::vector<unsigned short> & indices,
	std::vector<glm::vec3> & vertices,
	std::vector<glm::vec2> & uvs,
	std::vector<glm::vec3> & normals
){

	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(path, 0/*aiProcess_JoinIdenticalVertices | aiProcess_SortByPType*/);
	if( !scene) {
		fprintf( stderr, importer.GetErrorString());
		getchar();
		return false;
	}
	const aiMesh* mesh = scene->mMeshes[0]; // In this simple example code we always use the 1rst mesh (in OBJ files there is often only one anyway)

	// Fill vertices positions
	vertices.reserve(mesh->mNumVertices);
	for(unsigned int i=0; i<mesh->mNumVertices; i++){
		aiVector3D pos = mesh->mVertices[i];
		vertices.push_back(glm::vec3(pos.x, pos.y, pos.z));
	}

	// Fill vertices texture coordinates
	uvs.reserve(mesh->mNumVertices);
	for(unsigned int i=0; i<mesh->mNumVertices; i++){
		aiVector3D UVW = mesh->mTextureCoords[0][i]; // Assume only 1 set of UV coords; AssImp supports 8 UV sets.
		uvs.push_back(glm::vec2(UVW.x, UVW.y));
	}

	// Fill vertices normals
	normals.reserve(mesh->mNumVertices);
	for(unsigned int i=0; i<mesh->mNumVertices; i++){
		aiVector3D n = mesh->mNormals[i];
		normals.push_back(glm::vec3(n.x, n.y, n.z));
	}


	// Fill face indices
	indices.reserve(3*mesh->mNumFaces);
	for (unsigned int i=0; i<mesh->mNumFaces; i++){
		// Assume the model has only triangles.
		indices.push_back(mesh->mFaces[i].mIndices[0]);
		indices.push_back(mesh->mFaces[i].mIndices[1]);
		indices.push_back(mesh->mFaces[i].mIndices[2]);
	}
	
	// The "scene" pointer will be deleted automatically by "importer"
	return true;
}

#endif