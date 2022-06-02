#ifndef OBJLOADER_H
#define OBJLOADER_H

bool loadOBJ(
        const char * path,
        std::vector<glm::vec4> & vertices,
        std::vector<glm::vec3> & normals,
        std::vector<GLushort> & elements
);



bool loadAssImp(
	const char * path, 
	std::vector<unsigned short> & indices,
	std::vector<glm::vec3> & vertices,
	std::vector<glm::vec2> & uvs,
	std::vector<glm::vec3> & normals
);

#endif