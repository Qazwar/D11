#pragma once
#include "..\renderer\MeshBuffer.h"

namespace ds {	

	namespace gen {

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
			for (int i = 0; i < indices.size(); ++i) {
				if (indices[i] == idx) {
					return true;
				}
			}
			return false;
		}
	};

	struct Edge {
		uint16_t next;
		uint16_t prev;
		uint16_t vert_index;
		uint16_t face_index;
		uint16_t opposite;
		v2 uv;
	};

	struct Face {
		uint16_t edge;
		v3 n;
		Color color;
	};

	class MeshGen {

	public:
		MeshGen();
		~MeshGen();
		void build(Mesh* mesh);
		int num_faces() const;
		const Face& get_face(uint16_t face_index) const;
		void get_vertices(const Face& face,v3* ret) const;
		uint16_t add_cube(const v3& position, const v3& size, uint16_t* faces = 0);
		uint16_t add_cube(const v3& position, const v3& size, const v3& rotation);
		void set_color(uint16_t faceIndex, const Color& color);
		uint16_t add_face(v3* positions);
		uint16_t add_face(const v3& p0, const v3& p1, const v3& p2, const v3& p3);
		void add_face(const v3& position, const v2& size, const v3& normal);
		void subdivide(uint16_t face_index);
		void move_edge(uint16_t edgeIndex, const v3& position);
		void move_face(uint16_t faceIndex, const v3& position);
		void texture_face(uint16_t faceIndex, const Texture& t);
		void scale_face(uint16_t faceIndex, float scale);
		v3 get_center(uint16_t faceIndex);
		uint16_t hsplit_edge(uint16_t edgeIndex, float factor = 0.5f);
		uint16_t vsplit_edge(uint16_t edgeIndex, float factor = 0.5f);
		uint16_t get_edge_index(uint16_t faceIndex, int nr);
		uint16_t make_face(uint16_t* edges);
		uint16_t extrude_edge(uint16_t edgeIndex, const v3& pos);
		uint16_t extrude_face(uint16_t faceIndex,float factor);
		void debug();
		void recalculate_normals();
		void parse(const char* fileName);
		void create_ring(float radius, float width, uint16_t segments);
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
	private:
		MeshGen(const MeshGen& other) {}
		//void find_adjacent_faces(uint16_t face_index, IndexList& list);
		void calculate_normal(Face* f);
		int add_vertex(const v3& pos);
		int find_edges(const v3& pos, uint16_t* ret, int max);
		int find_edge(const v3& start, const v3& end);
		Array<v3> _vertices;
		Array<Edge> _edges;
		Array<Face> _faces;
	};

	}
}

