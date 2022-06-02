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
#include <GL/glew.h>
#include "algorithm"

#include "objloader.hpp"

// VI (elements)= 10 10 10 10
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


void parse_face(FACE_DATA_TYPE face_date_type, std::string face_data, std::vector<int>& face_normal, std::vector<GLushort> & elements) {

    char slash;
    std::istringstream face_stream(face_data);
    GLushort element_vertex = 0;
    int normal = 0, uv = 0;
    // remove any space characters...
    face_data.erase(remove(face_data.begin(), face_data.end(), ' '), face_data.end());
    if (face_date_type == VI_VT){
        face_stream >> element_vertex >> slash >> uv;
        elements.push_back(element_vertex);
    }
    else if (face_date_type == VI_VT_VN) {
        face_stream >> element_vertex >> slash >> uv >> slash >> normal;
        elements.push_back(element_vertex);
//        face_normal.push_back(normal);
    }
    else if (face_date_type == VI_VN){
        face_stream >> element_vertex >>  slash >>  slash >> normal;
        elements.push_back(element_vertex);
//        face_normal.push_back(normal);
    }
    else{
        // face_date_type == VI
        // NO NORMAL.. TBD
    }
}


void parse_face_line(std::istringstream& line_stream, std::vector<glm::vec3>& normals, std::vector<GLushort> & elements) {
    auto face_type = face_data_type(line_stream.str());
    std::string face;
    int max = 3; // Only supporting 3 vertex faces for now.
    int current = 0;
    std::vector<int> face_normal;
    while (std::getline(line_stream, face, ' ') && current++<max) {
        parse_face(face_type, face, face_normal, elements);
    }
//    normals.emplace_back(face_normal[0], face_normal[1], face_normal[2]);
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

void calc_normals(	std::vector<glm::vec4> & vertices,
                      std::vector<glm::vec3> & normals,
                      std::vector<GLushort> & elements) {
    std::vector<int> nb_seen;
    normals.resize(vertices.size(), glm::vec3(0.0, 0.0, 0.0));
    nb_seen.resize(vertices.size(), 0);
    for (unsigned int i = 0; i < elements.size(); i+=3) {
        GLushort ia = elements[i];
        GLushort ib = elements[i+1];
        GLushort ic = elements[i+2];
        glm::vec3 normal = glm::normalize(glm::cross(
                glm::vec3(vertices[ib]) - glm::vec3(vertices[ia]),
                glm::vec3(vertices[ic]) - glm::vec3(vertices[ia])));

        int v[3];  v[0] = ia;  v[1] = ib;  v[2] = ic;
        for (int j = 0; j < 3; j++) {
            GLushort cur_v = v[j];
            nb_seen[cur_v]++;
            if (nb_seen[cur_v] == 1) {
                normals[cur_v] = normal;
            } else {
                // average
                normals[cur_v].x = normals[cur_v].x * (1.0 - 1.0/nb_seen[cur_v]) + normal.x * 1.0/nb_seen[cur_v];
                normals[cur_v].y = normals[cur_v].y * (1.0 - 1.0/nb_seen[cur_v]) + normal.y * 1.0/nb_seen[cur_v];
                normals[cur_v].z = normals[cur_v].z * (1.0 - 1.0/nb_seen[cur_v]) + normal.z * 1.0/nb_seen[cur_v];
                normals[cur_v] = glm::normalize(normals[cur_v]);
            }
        }
    }
}


bool loadOBJ(
	const char * path, 
	std::vector<glm::vec4> & vertices,
	std::vector<glm::vec3> & normals,
    std::vector<GLushort> & elements
){
	printf("Loading OBJ file %s...\n", path);
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
            glm::vec4 vertex;
            line_stream >> vertex.x >> vertex.y >> vertex.z; vertex.w = 1.0;
            vertices.push_back(vertex);
        }
        else if (line_header == "f ") {
            std::cout << "parsing face" << std::endl;
            std::istringstream line_stream(line.substr(2));
            parse_face_line(line_stream, normals, elements);
        }
        else if (line[0] == '#') { /* ignoring this line */ }
        else { /* ignoring this line */ }
    }
    std::cout << "parsed_ob" << std::endl;
    calc_normals(vertices, normals, elements);
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
	vertices.reserve(mNumVertices);
	for(unsigned int i=0; i<mNumVertices; i++){
		aiVector3D pos = mVertices[i];
		vertices.push_back(glm::vec3(pos.x, pos.y, pos.z));
	}

	// Fill vertices texture coordinates
	uvs.reserve(mNumVertices);
	for(unsigned int i=0; i<mNumVertices; i++){
		aiVector3D UVW = mTextureCoords[0][i]; // Assume only 1 set of UV coords; AssImp supports 8 UV sets.
		uvs.push_back(glm::vec2(UVW.x, UVW.y));
	}

	// Fill vertices normals
	normals.reserve(mNumVertices);
	for(unsigned int i=0; i<mNumVertices; i++){
		aiVector3D n = mNormals[i];
		normals.push_back(glm::vec3(n.x, n.y, n.z));
	}


	// Fill face indices
	indices.reserve(3*mNumFaces);
	for (unsigned int i=0; i<mNumFaces; i++){
		// Assume the model has only triangles.
		indices.push_back(mFaces[i].mIndices[0]);
		indices.push_back(mFaces[i].mIndices[1]);
		indices.push_back(mFaces[i].mIndices[2]);
	}
	
	// The "scene" pointer will be deleted automatically by "importer"
	return true;
}

#endif