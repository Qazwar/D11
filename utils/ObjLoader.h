#pragma once
#include <string>
#include <map>
#include "..\lib\collection_types.h"
#include <Vector.h>
#include "..\renderer\render_types.h"
#include "Color.h"
#include "..\renderer\QuadBuffer.h"

namespace ds {

	class ObjLoader {

		struct VertexDefinition {

			int vertexIndex;
			int normalIndex;
			int uvIndex;
			Color color;

			VertexDefinition() : vertexIndex(-1), normalIndex(-1), uvIndex(-1), color(Color::WHITE) {}
		};

		struct Material {
			float specularPower;
			Color ambient;
			Color diffuse;
			Color specular;
			int index;
		};

		typedef std::map<IdString, Material> Materials;
		typedef Array<Vector3f> VectorCache;
		typedef Array<VertexDefinition> Faces;
		typedef Array<Vector2f> UVCache;

	public:
		ObjLoader();		
		virtual ~ObjLoader();
		void parse(const char* fileName, Mesh* mesh);
		void parse2(const char* fileName, Mesh* mesh);
		//void read(const std::string& fileName, MeshData& data);
		//void save(const std::string& fileName, const MeshData& data);
	private:
		ObjLoader(const ObjLoader& orig);
		void readMaterials(const char* mtlFileName);
		bool startsWith(const std::string& line, const std::string& txt);
		void readColor(const std::string& line, ds::Color* color);
		void readVector3(const char* line, int offset, v3* v);
		void readVector2(const char* line, int offset, v2* v);
		int readInt(const std::string& line);
		float readFloat(const std::string& line);
		std::string readString(const std::string& line);
		Materials m_Materials;
		VectorCache m_Vertices;
		VectorCache m_Normals;
		UVCache m_UVCache;
		Faces m_Faces;
	};

}