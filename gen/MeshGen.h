#pragma once
#include "..\renderer\MeshBuffer.h"

namespace ds {	

	namespace gen {

	// ---------------------------------------
	// UnqiueIndexList
	// ---------------------------------------
	struct IndexList {

		Array<uint16_t> indices;

		bool add_unique(uint16_t idx) {
			if (!contains(idx)) {
				indices.push_back(idx);
				return true;
			}
			return false;
		}

		bool contains(uint16_t idx) const {
			for (uint32_t i = 0; i < indices.size(); ++i) {
				if (indices[i] == idx) {
					return true;
				}
			}
			return false;
		}

		void clear() {
			indices.clear();
		}
	};

	// ---------------------------------------
	// Edge
	// ---------------------------------------
	struct Edge {
		uint16_t next;
		uint16_t prev;
		uint16_t vert_index;
		uint16_t face_index;
		uint16_t opposite;
		v2 uv;
	};

	// ---------------------------------------
	// Face
	// ---------------------------------------
	struct Face {
		uint16_t edge;
		v3 n;
		Color color;
		bool selected;
	};

	enum OpcodeType {
		ADD_CUBE,
		ADD_CUBE_ROT,
		SET_COLOR,
		SLICE_UNIFORM,
		SLICE,
		MOVE_EDGE,
		V_SPLIT,
		H_SPLIT,
		MAKE_FACE,
		ADD_FACE,
		COMBINE_EDGES,
		DEBUG_COLORS,
		UNKNOWN
	};

	// ---------------------------------------
	// MeshGenOpcode
	// ---------------------------------------
	struct MeshGenOpcode {

		int type;
		int offset;

		MeshGenOpcode() : type(OpcodeType::UNKNOWN), offset(-1) {}
	};

	// ---------------------------------------
	// DataStore
	// ---------------------------------------
	struct DataStore {

		Array<float> data;

		uint32_t add_data(const v3& v) {
			int cnt = data.size();
			data.push_back(v.x);
			data.push_back(v.y);
			data.push_back(v.z);
			return cnt;
		}

		uint32_t add_data(int v) {
			int cnt = data.size();
			data.push_back(static_cast<float>(v));
			return cnt;
		}

		uint32_t add_data(float v) {
			int cnt = data.size();
			data.push_back(v);
			return cnt;
		}

		uint32_t add_data(uint16_t v) {
			int cnt = data.size();
			data.push_back(static_cast<float>(v));
			return cnt;
		}

		uint32_t add_data(MeshGenOpcode& op, const Color& v) {
			int cnt = data.size();
			data.push_back(v.r);
			data.push_back(v.g);
			data.push_back(v.b);
			data.push_back(v.a);
			return cnt;
		}

		void get_data(const MeshGenOpcode& op, int index, v3* ret) const {
			int offset = op.offset + index;
			for (int i = 0; i < 3; ++i) {
				ret->data[i] = data[offset + i];
			}
		}

		void get_data(const MeshGenOpcode& op, int index, uint16_t* ret) const {
			int offset = op.offset + index;
			*ret = static_cast<uint16_t>(data[offset]);
		}

		void get_data(const MeshGenOpcode& op, int index, int* ret) const {
			int offset = op.offset + index;
			*ret = static_cast<int>(data[offset]);
		}

		void get_data(const MeshGenOpcode& op, int index, float* ret) const {
			int offset = op.offset + index;
			*ret = data[offset];
		}

		void get_data(const MeshGenOpcode& op, int index, Color* ret) const {
			int offset = op.offset + index;
			for (int i = 0; i < 4; ++i) {
				float v = data[offset + i];
				if (v > 1.0f) {
					v /= 255.0f;
				}
				ret->values[i] = v;
			}			
		}

	};

	
	// ---------------------------------------
	// MeshGen
	// ---------------------------------------
	class MeshGen {

	public:
		MeshGen();
		~MeshGen();
		void build(Mesh* mesh);
		int num_faces() const;
		const Face& get_face(uint16_t face_index) const;
		uint16_t get_edge(uint16_t face_index, uint16_t edge_offset);
		void get_vertices(const Face& face,v3* ret) const;
		uint16_t add_cube(const v3& position, const v3& size, uint16_t* faces = 0);
		uint16_t add_cube(const v3& position, const v3& size, const v3& rotation);
		void set_color(uint16_t faceIndex, const Color& color);
		void set_color(const Color& color);
		uint16_t add_face(v3* positions);
		uint16_t add_face(const v3& p0, const v3& p1, const v3& p2, const v3& p3);
		void add_face(const v3& position, const v2& size, const v3& normal);
		void subdivide(uint16_t face_index);
		void move_edge(uint16_t edgeIndex, const v3& position);
		void move_face(uint16_t faceIndex, const v3& position);
		void texture_face(uint16_t faceIndex, const Texture& t);
		void scale_face(uint16_t faceIndex, float scale);
		v3 get_center(uint16_t faceIndex);
		int slice(uint16_t face_index, int segments, uint16_t* faces = 0, int max = 0);
		int slice(uint16_t face_index, int stepsX, int stepsY, uint16_t* faces = 0, int max = 0);
		uint16_t hsplit_edge(uint16_t edgeIndex, float factor = 0.5f);
		uint16_t vsplit_edge(uint16_t edgeIndex, float factor = 0.5f);
		uint16_t get_edge_index(uint16_t faceIndex, int nr);
		uint16_t make_face(uint16_t* edges);
		uint16_t combine_edges(uint16_t edge0, uint16_t edge1);
		uint16_t extrude_edge(uint16_t edgeIndex, const v3& pos);
		uint16_t extrude_face(uint16_t faceIndex,float factor);
		const Color& get_color(uint16_t face_index) const;
		void debug();
		void recalculate_normals();
		// objects
		void create_ring(float radius, float width, uint16_t segments);
		void create_cylinder(float radius, float height, uint16_t segments);
		uint16_t create_torus(const v3& position,float radius, float width, float depth, uint16_t segments);
		void create_grid(const v2& size, int stepsX, int stepsY);
		void create_sphere(float radius, int segments, int stacks);
		
		void debug_edge(uint16_t edgeIndex);
		void debug_face(uint16_t faceIndex);
		int intersects(const ds::Ray& ray);
		void clear();		
		int get_connected_faces(uint16_t face_index, uint16_t* ret, int max);
		int get_connected_edges(uint16_t edge_index, uint16_t* ret, int max);
		void translate(const v3& position);
		void scale(const v3& scale);
		void rotate(const v3& rotation);
		void transform(const v3& position, const v3& scale, const v3& rotation);
		void add(const MeshGen& other, const v3& position,const v3& scale = v3(1,1,1),const v3& rotation = v3(0,0,0));
		void find_adjacent_faces(uint16_t face_index, IndexList& list);
		void smooth(const IndexList& list, float radius);
		void debug_colors();
		// selection
		bool select_face(uint16_t face_index);
		void clear_selection();
		
		void save_bin(const char* fileName);
		void save_text(const char* fileName);
		void save_mesh(const char* fileName);
		void load_bin(const char* fileName);
		void load_text(const char* fileName);
	private:
		void executeOpcodes(const Array<MeshGenOpcode>& opcodes, const DataStore& store);
		// recording
		void record(const MeshGenOpcode& opcode);
		MeshGen(const MeshGen& other) {}
		void calculate_normal(Face* f);
		int add_vertex(const v3& pos);
		int find_edges(const v3& pos, uint16_t* ret, int max);
		int find_vertices(const v3& pos, uint16_t* ret, int max);
		int find_edge(const v3& start, const v3& end);
		Array<v3> _vertices;
		Array<Edge> _edges;
		Array<Face> _faces;
		Color _selectionColor;
		DataStore _store;
		Array<MeshGenOpcode> _opcodes;
	};

	}
}

