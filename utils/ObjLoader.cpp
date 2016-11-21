#include "ObjLoader.h"
#include <fstream>
#include <vector>
#include "core\log\Log.h"
#include "core\io\json.h"
#include <map>
#include "..\renderer\VertexTypes.h"
#include "core\string\StaticHash.h"

namespace ds {
	
	namespace obj {

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

		typedef std::map<StaticHash, Material> Materials;
		typedef Array<Vector3f> VectorCache;
		typedef Array<VertexDefinition> Faces;
		typedef Array<Vector2f> UVCache;

		int get(Tokenizer& tokenizer, int current, v2* ret) {
			int cnt = current;
			++cnt;
			Token& t = tokenizer.get(cnt);
			ret->x = t.value;
			++cnt;
			t = tokenizer.get(cnt);
			while (t.type != Token::NUMBER) {
				++cnt;
				t = tokenizer.get(cnt);
			}
			ret->y = t.value;
			return cnt;
		}

		int get(Tokenizer& tokenizer, int current, v3* ret) {
			int cnt = current;
			++cnt;
			Token& t = tokenizer.get(cnt);
			ret->x = t.value;
			++cnt;
			t = tokenizer.get(cnt);
			while (t.type != Token::NUMBER) {
				++cnt;
				t = tokenizer.get(cnt);
			}
			ret->y = t.value;
			++cnt;
			t = tokenizer.get(cnt);
			while (t.type != Token::NUMBER) {
				++cnt;
				t = tokenizer.get(cnt);
			}
			ret->z = t.value;
			return cnt;
		}

		void readColor(const std::string& line, Color* color) {
			std::string dummy;
			std::istringstream iss(line);
			iss >> dummy >> color->r >> color->g >> color->b;
			color->a = 1.0f;
		}

		int readInt(const std::string& line) {
			std::string dummy;
			std::istringstream iss(line);
			int v = 0;
			iss >> dummy >> v;
			return v;
		}

		float readFloat(const std::string& line) {
			std::string dummy;
			std::istringstream iss(line);
			float v = 0.0f;
			iss >> dummy >> v;
			return v;
		}

		std::string readString(const std::string& line) {
			std::string dummy;
			std::istringstream iss(line);
			iss >> dummy >> dummy;
			return dummy;
		}

		void readMaterials(const char* mtlFileName, Materials& m_Materials) {
			std::ifstream InFile(mtlFileName);
			LOG << "reading material file: " << mtlFileName;
			std::string line;
			Material* m = 0;
			while (std::getline(InFile, line)) {
				if (line.find("newmtl") != std::string::npos) {
					std::istringstream iss(line);
					std::string name;
					iss >> name >> name;
					LOG << "new mtl: " << name;
					Material mtl;
					StaticHash hash(name.c_str());
					m_Materials[hash] = mtl;
					m = &m_Materials[hash];
				}
				else if (line.find("Ka") != std::string::npos) {
					readColor(line, &m->ambient);
				}
				else if (line.find("Kd") != std::string::npos) {
					readColor(line, &m->diffuse);
					LOG << "diffuse: " << m->diffuse;
				}
				else if (line.find("Ks") != std::string::npos) {
					readColor(line, &m->specular);
				}
				else if (line.find("Ns") != std::string::npos) {
					m->specularPower = readFloat(line);
				}
			}
			LOG << "number of materials: " << m_Materials.size();
		}

		bool parse(const char* fileName, Mesh* mesh, const v3& offset, const v3& scale, const v3& rotation) {
			int fileSize = -1;
			char fullName[256];
			sprintf_s(fullName, 256, "content\\objects\\%s", fileName);
			FILE *fp = fopen(fullName, "rb");
			if (fp) {
				Materials m_Materials;
				VectorCache m_Vertices;
				VectorCache m_Normals;
				UVCache m_UVCache;
				Faces m_Faces;
				LOG << "Loading '" << fileName << "'";
				fseek(fp, 0, SEEK_END);
				int sz = ftell(fp);
				fseek(fp, 0, SEEK_SET);
				LOG << "size: " << sz;
				char* buffer = new char[sz + 1];
				fread(buffer, 1, sz, fp);
				buffer[sz] = '\0';
				fclose(fp);
				char name[128];
				Tokenizer tokenizer;
				tokenizer.parse(buffer, false);
				LOG << "tokens: " << tokenizer.size();
				int n = 0;
				Material* m = 0;
				bool hasNormals = false;
				bool hasUV = false;
				int x, y, z;
				while (n < tokenizer.size()) {
					Token& t = tokenizer.get(n);
					if (t.type == Token::NAME) {
						strncpy(name, buffer + t.index, t.size);
						name[t.size] = '\0';
						if (strcmp(name, "mtllib") == 0) {
							++n;
							t = tokenizer.get(n);
							strncpy(name, buffer + t.index, t.size);
							name[t.size] = '\0';
							LOG << "material: " << name;
							char full[256];
							sprintf_s(full, "content\\objects\\%s.mtl", name);
							readMaterials(full, m_Materials);
						}
						else if (strcmp(name, "usemtl") == 0) {
							++n;
							t = tokenizer.get(n);
							strncpy(name, buffer + t.index, t.size);
							name[t.size] = '\0';
							StaticHash hash(name);
							m = &m_Materials[hash];
							LOG << "using material: " << name;
						}
						else if (strcmp(name, "vt") == 0) {
							v2 vt;
							++n;
							t = tokenizer.get(n);
							vt.x = t.value;
							++n;
							t = tokenizer.get(n);
							vt.y = t.value;
							m_UVCache.push_back(vt);
							hasUV = true;
						}
						else if (strcmp(name, "vn") == 0) {
							v3 vn;
							n = get(tokenizer, n, &vn);
							m_Normals.push_back(vn);
							hasNormals = true;
						}
						else if (strcmp(name, "v") == 0) {
							v3 v;
							++n;
							t = tokenizer.get(n);
							v.x = t.value;
							++n;
							t = tokenizer.get(n);
							v.y = t.value;
							++n;
							t = tokenizer.get(n);
							v.z = t.value;
							m_Vertices.push_back(v);
						}
						else if (strcmp(name, "f") == 0) {
							for (int i = 0; i < 4; ++i) {
								x = -1;
								y = -1;
								z = -1;
								if (hasNormals && hasUV) {
									v3 tmp(-1, -1, -1);
									n = get(tokenizer, n, &tmp);
									x = tmp.x;
									y = tmp.y;
									z = tmp.z;
								}
								else if (hasNormals && !hasUV) {
									v2 tmp(-1, -1);
									n = get(tokenizer, n, &tmp);
									x = tmp.x;
									z = tmp.y;
								}
								else if (!hasNormals && hasUV) {
									++n;
									t = tokenizer.get(n);
									x = t.value;
									++n;
									t = tokenizer.get(n);
									z = t.value;
								}
								else {
									++n;
									t = tokenizer.get(n);
									x = t.value;
								}
								VertexDefinition f;
								f.vertexIndex = x;
								f.normalIndex = z;
								f.uvIndex = y;
								//LOG << "f: " << x << " " << y << " " << z;
								if (m != 0) {
									f.color = m->diffuse;
								}
								else {
									f.color = Color::WHITE;
								}
								m_Faces.push_back(f);
							}
						}
					}
					++n;
					t = tokenizer.get(n);
				}
				delete[] buffer;
				LOG << "vertex cache  : " << m_Vertices.size();
				LOG << "normals cache : " << m_Normals.size();
				LOG << "uv cache      : " << m_UVCache.size();
				LOG << "faces         : " << m_Faces.size();
				PNTCVertex b[4];
				int count = m_Faces.size() / 4;
				for (int i = 0; i < count; ++i) {
					//LOG << "----------------------------------------";
					for (int j = 0; j < 4; ++j) {
						VertexDefinition& current = m_Faces[i * 4 + j];
						if (current.vertexIndex != -1) {
							PNTCVertex& v = b[j];
							v.position = m_Vertices[current.vertexIndex - 1];
							v.position.z *= -1.0f;
							if (current.uvIndex != -1) {
								v.texture = m_UVCache[current.uvIndex - 1];
								v.texture.y = 1.0f - v.texture.y;
							}
							else {
								v.texture = Vector2f(0, 0);
							}
							if (hasNormals) {
								v.normal = m_Normals[current.normalIndex - 1];
								v.normal.z *= -1.0f;
							}
							//if (m != 0) {
							v.color = m_Faces[i * 4 + j].color;
							//}		
							//LOG << "vertex: " << DBG_V3(v.position) << " normal: " << DBG_V3(v.normal) << " color: " << DBG_CLR(v.color);
						}
					}
					mesh->vertices.push_back(b[3]);
					mesh->vertices.push_back(b[2]);
					mesh->vertices.push_back(b[1]);
					mesh->vertices.push_back(b[0]);

				}
				LOG << "Vertices: " << mesh->vertices.size();
				return true;
			}
			else {
				LOGE << "Cannot load '" << fileName << "'";
				return false;
			}
		}
	}
	
}