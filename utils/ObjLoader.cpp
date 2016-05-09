#include "ObjLoader.h"
#include <fstream>
#include <vector>
#include "Log.h"
#include "..\io\json.h"

namespace ds {

	ObjLoader::ObjLoader() {
	}

	ObjLoader::ObjLoader(const ObjLoader& orig) {
	}

	ObjLoader::~ObjLoader() {
	}

	bool ObjLoader::startsWith(const std::string& line, const std::string& txt) {
		int len = txt.length();
		for (int i = 0; i < len; ++i) {
			if (line[i] != txt[i]) {
				return false;
			}
		}
		if (line[len] != ' ') {
			return false;
		}
		return true;
	}

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

	void ObjLoader::parse2(const char* fileName, Mesh* mesh) {
		int fileSize = -1;
		FILE *fp = fopen(fileName, "rb");
		if (fp) {
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
			tokenizer.parse(buffer,false);
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
						readMaterials(full);
					}
					else if (strcmp(name, "usemtl") == 0) {
						++n;
						t = tokenizer.get(n);
						strncpy(name, buffer + t.index, t.size);
						name[t.size] = '\0';
						IdString hash = string::murmur_hash(name);
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
				mesh->add(b[3]);
				mesh->add(b[2]);
				mesh->add(b[1]);
				mesh->add(b[0]);
				
			}
		}
		
	}

	void ObjLoader::parse(const char* fileName, Mesh* mesh) {
		std::ifstream InFile(fileName);
		LOG << "reading obj file: " << fileName;
		std::string line;
		Material* m = 0;
		bool hasNormals = false;
		bool hasUV = false;
		while (std::getline(InFile, line)) {
			if (startsWith(line, "mtllib")) {
				std::string mtrlFile = readString(line);
				//readMaterials("content\\data\\" + mtrlFile);
			}
			if (startsWith(line, "o")) {
				std::string name = readString(line);
				LOG << "found object name: " << name;
			}
			else if (startsWith(line, "v")) {
				Vector3f v;
				readVector3(line.c_str(), 2, &v);
				m_Vertices.push_back(v);
			}
			else if (startsWith(line, "vn")) {
				Vector3f vn;
				readVector3(line.c_str(), 3, &vn);
				m_Normals.push_back(vn);
				hasNormals = true;
			}
			else if (startsWith(line, "vt")) {
				Vector2f uv;
				readVector2(line.c_str(), 3, &uv);
				m_UVCache.push_back(uv);
				hasUV = true;
			}
			else if (startsWith(line, "usemtl")) {
				std::string mName = readString(line);
				//m = &m_Materials[mName];
				LOG << "using material: " << mName;
			}
			else if (startsWith(line, "f")) {
				const char* l = line.c_str();
				l += 2;
				int x, y, z;
				//std::string dummy;
				//char c;
				//std::istringstream iss(line);
				//iss >> dummy;
				for (int i = 0; i < 4; ++i) {
					x = -1;
					y = -1;
					z = -1;
					if (hasNormals && hasUV) {
						sscanf(l, "%f/%f/%f", &x, &y, &z);
						//iss >> x >> c >> z >> c >> y;
					}
					else if (hasNormals && !hasUV) {
						sscanf(l, "%f//%f", &x, &y);
						//iss >> x >> c >> c >> y;
					}
					else if (!hasNormals && hasUV) {
						sscanf(l, "%f/&f/", &x, &z);
						//iss >> x >> c >> z >> c;
					}
					else {
						sscanf(l, "%f", &x);
						//iss >> x;// >> c >> c;
					}
					while (*l != ' ') {
						++l;
					}
					++l;
					VertexDefinition f;
					f.vertexIndex = x;
					f.normalIndex = y;
					f.uvIndex = z;
					if (m != 0) {
						f.color = m->diffuse;
					}
					m_Faces.push_back(f);
				}
			}
		}
		LOG << "vertex cache  : " << m_Vertices.size();
		LOG << "normals cache : " << m_Normals.size();
		LOG << "uv cache      : " << m_UVCache.size();
		LOG << "faces         : " << m_Faces.size();
		for (int i = 0; i < m_Faces.size(); ++i) {
			VertexDefinition& current = m_Faces[i];
			if (current.vertexIndex != -1) {
				PNTCVertex v;
				v.position = m_Vertices[current.vertexIndex - 1];
				//v.position.z *= -1.0f;
				if (current.uvIndex != -1) {
					v.texture = m_UVCache[current.uvIndex - 1];
					v.texture.y = 1.0f - v.texture.y;
				}
				else {
					v.texture = Vector2f(0, 0);
				}
				if (hasNormals) {
					v.normal = m_Normals[current.normalIndex - 1];
					//v.normal.z *= -1.0f;
				}
				if (m != 0) {
					v.color = m_Faces[i].color;
				}
				mesh->add(v);
			}
		}
		/*
		for (int i = 0; i < data.size; ++i) {
		LOG << DBG_V3(data.faces[i].v[0]);
		LOG << DBG_V3(data.faces[i].v[1]);
		LOG << DBG_V3(data.faces[i].v[2]);
		LOG << DBG_V3(data.faces[i].v[3]);
		LOG << "n: " << DBG_V3(data.faces[i].n);
		LOG << "c: " << DBG_CLR(data.faces[i].color);

		}
		*/
	}


	void ObjLoader::readVector3(const char* line,int offset, v3* v) {
		const char* l = line;
		l += offset;
		sscanf(l, "%f %f %f", &v->x, &v->y, &v->z);
		//std::string dummy;
		//std::istringstream iss(line);
		//iss >> dummy >> color->x >> color->y >> color->z;
	}

	void ObjLoader::readVector2(const char* line, int offset, v2* v) {
		const char* l = line;
		l += offset;
		sscanf(l, "%f %f", &v->x, &v->y);
		//std::string dummy;
		//std::istringstream iss(line);
		//iss >> dummy >> color->x >> color->y >> color->z;
	}

	void ObjLoader::readColor(const std::string& line, Color* color) {
		std::string dummy;
		std::istringstream iss(line);
		iss >> dummy >> color->r >> color->g >> color->b;
		color->a = 1.0f;
	}

	int ObjLoader::readInt(const std::string& line) {
		std::string dummy;
		std::istringstream iss(line);
		int v = 0;
		iss >> dummy >> v;
		return v;
	}

	float ObjLoader::readFloat(const std::string& line) {
		std::string dummy;
		std::istringstream iss(line);
		float v = 0.0f;
		iss >> dummy >> v;
		return v;
	}

	std::string ObjLoader::readString(const std::string& line) {
		std::string dummy;
		std::istringstream iss(line);
		iss >> dummy >> dummy;
		return dummy;
	}


	void ObjLoader::readMaterials(const char* mtlFileName) {
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
				IdString hash = string::murmur_hash(name.c_str());
				m_Materials[hash] = mtl;
				m = &m_Materials[hash];
			}
			else if (line.find("Ka") != std::string::npos) {
				readColor(line, &m->ambient);
			}
			else if (line.find("Kd") != std::string::npos) {
				readColor(line, &m->diffuse);
				LOG << "diffuse: " << DBG_CLR(m->diffuse);
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
	/*
	void ObjLoader::read(const std::string& fileName, MeshData& data) {
		FILE *file = fopen(fileName.c_str(), "rb");
		int count = 0;
		fread(&count, sizeof(int), 1, file);
		LOG << "number of faces: " << count;
		data.faces = new Surface[count];
		data.size = count;
		for (int i = 0; i < count; ++i) {
			for (int j = 0; j < 4; ++j) {
				fread(&data.faces[i].v[j], sizeof(Vector3f), 1, file);
			}
			for (int j = 0; j < 4; ++j) {
				fread(&data.faces[i].uv[j], sizeof(Vector2f), 1, file);
			}
			fread(&data.faces[i].n, sizeof(Vector3f), 1, file);
			fread(&data.faces[i].color, sizeof(Color), 1, file);
		}
		//fread(&data.aabBox.offset, sizeof(Vector3f), 1, file);
		fread(&data.aabBox.extent, sizeof(Vector3f), 1, file);
		fclose(file);
	}

	void ObjLoader::save(const std::string& fileName, const MeshData& data) {
		FILE *file = fopen(fileName.c_str(), "wb");
		fwrite(&data.size, sizeof(int), 1, file);
		for (int i = 0; i < data.size; ++i) {
			for (int j = 0; j < 4; ++j) {
				fwrite(&data.faces[i].v[j], sizeof(Vector3f), 1, file);
			}
			for (int j = 0; j < 4; ++j) {
				fwrite(&data.faces[i].uv[j], sizeof(Vector2f), 1, file);
			}
			fwrite(&data.faces[i].n, sizeof(Vector3f), 1, file);
			fwrite(&data.faces[i].color, sizeof(Color), 1, file);			
		}
		//fwrite(&data.aabBox.offset, sizeof(Vector3f), 1, file);
		fwrite(&data.aabBox.extent, sizeof(Vector3f), 1, file);
		fclose(file);
	}
	*/
}