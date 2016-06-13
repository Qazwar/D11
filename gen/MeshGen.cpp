#include "MeshGen.h"
#include "..\math\math.h"
#include "..\utils\Log.h"
#include "..\io\json.h"
#include "..\io\FileRepository.h"
#include "..\io\BinaryFile.h"
#include<stdarg.h>

#define EPSILON 0.000001

namespace ds {

	static const v3 CUBE_VERTICES[] = {
		v3(-0.5f,0.5f,-0.5f),
		v3(0.5f,0.5f,-0.5f),
		v3(0.5f,-0.5f,-0.5f),
		v3(-0.5f,-0.5f,-0.5f)
	};

	// float / v3 / int / color
	enum OCDataTypes {
		FLOAT,
		VEC3,
		INT,
		COLOR
	};

	const int OPCODE_DATASIZE[] = { 1, 3, 1, 4 };

	// --------------------------------------------
	// Opcode definition
	// --------------------------------------------
	struct OpcodeDefinition {
	
		gen::OpcodeType type;
		const char* name;
		int args;
		int types[8];
		int var_count;

		OpcodeDefinition(gen::OpcodeType t, const char* n) : type(t), name(n), args(0) , var_count(0) {
		}

		OpcodeDefinition(gen::OpcodeType t, const char* n, int a, ...) : type(t), name(n) , args(a) {
			va_list ap;
			va_start(ap, a);
			for (int i = 0; i < a; ++i) {
				int d = va_arg(ap, int);
				types[i] = d;
				var_count += OPCODE_DATASIZE[d];
			}
			va_end(ap);
		}
	};

	// --------------------------------------------
	// Opcode definitions
	// --------------------------------------------
	const int OPCODE_MAPPING_COUNT = 28;

	const OpcodeDefinition OPCODE_MAPPING[] = {
		{ gen::OpcodeType::ADD_CUBE, "add_cube", 2, OCDataTypes::VEC3, OCDataTypes::VEC3 },
		{ gen::OpcodeType::ADD_CUBE_ROT, "add_cube_rot", 3, OCDataTypes::VEC3, OCDataTypes::VEC3, OCDataTypes::VEC3 },
		{ gen::OpcodeType::SET_COLOR, "set_color", 2, 2, OCDataTypes::COLOR },
		{ gen::OpcodeType::SLICE_UNIFORM, "slice_uniform", 2, OCDataTypes::INT, OCDataTypes::INT },
		{ gen::OpcodeType::SLICE, "slice", 3, OCDataTypes::INT, OCDataTypes::INT, OCDataTypes::INT },
		{ gen::OpcodeType::MOVE_EDGE, "move_edge", 2, 2, OCDataTypes::VEC3 },
		{ gen::OpcodeType::V_SPLIT, "v_split", 2, OCDataTypes::INT, OCDataTypes::FLOAT },
		{ gen::OpcodeType::H_SPLIT, "h_split", 2, OCDataTypes::INT, OCDataTypes::FLOAT },
		{ gen::OpcodeType::MAKE_FACE, "make_face", 4, OCDataTypes::INT, OCDataTypes::INT, OCDataTypes::INT, OCDataTypes::INT },
		{ gen::OpcodeType::ADD_FACE, "add_face", 4, OCDataTypes::VEC3, OCDataTypes::VEC3, OCDataTypes::VEC3, OCDataTypes::VEC3 },
		{ gen::OpcodeType::COMBINE_EDGES, "combine_edges", 2, OCDataTypes::INT, OCDataTypes::INT },
		{ gen::OpcodeType::DEBUG_COLORS, "debug_colors", 0 },
		{ gen::OpcodeType::EXTRUDE_FACE, "extrude_face", 2, OCDataTypes::INT, OCDataTypes::INT },
		{ gen::OpcodeType::SCALE_FACE, "scale_face", 2, OCDataTypes::INT, OCDataTypes::FLOAT },
		{ gen::OpcodeType::ADD_COLOR_CUBE, "add_color_cube", 3, OCDataTypes::VEC3, OCDataTypes::VEC3, OCDataTypes::COLOR },
		{ gen::OpcodeType::ADD_CYLINDER, "add_cylinder", 5, OCDataTypes::VEC3, OCDataTypes::FLOAT, OCDataTypes::FLOAT, OCDataTypes::FLOAT, OCDataTypes::INT },
		{ gen::OpcodeType::ADD_COL_CYLINDER, "add_col_cylinder", 6, OCDataTypes::VEC3, OCDataTypes::FLOAT, OCDataTypes::FLOAT, OCDataTypes::FLOAT, OCDataTypes::INT , OCDataTypes::COLOR },
		{ gen::OpcodeType::MOVE_VERTEX, "move_vertex", 2, OCDataTypes::INT, OCDataTypes::VEC3},
		{ gen::OpcodeType::ADD_HEXAGON, "add_hexagon", 1, OCDataTypes::FLOAT },
		{ gen::OpcodeType::MOVE_FACE, "move_face", 2, OCDataTypes::INT, OCDataTypes::VEC3 },
		{ gen::OpcodeType::EXTRUDE_EDGE_NORMAL, "extrude_edge_normal", 2, OCDataTypes::INT, OCDataTypes::FLOAT},
		{ gen::OpcodeType::ADD_RING, "add_ring", 3, OCDataTypes::FLOAT, OCDataTypes::FLOAT, OCDataTypes::INT },
		{ gen::OpcodeType::SELECT_COLOR, "select_color", 1, OCDataTypes::COLOR },
		{ gen::OpcodeType::START_GROUP, "start_group", 0},
		{ gen::OpcodeType::END_GROUP, "end_group", 0 },
		{ gen::OpcodeType::ROTATE_GROUP, "rotate_group", 2, OCDataTypes::INT, OCDataTypes::VEC3 },
		{ gen::OpcodeType::MOVE_GROUP, "move_group", 2, OCDataTypes::INT, OCDataTypes::VEC3 },
		{ gen::OpcodeType::COPY_GROUP, "copy_group", 2, OCDataTypes::INT, OCDataTypes::VEC3 },
	};

	const OpcodeDefinition UNKNOWN_DEFINITION = OpcodeDefinition(gen::OpcodeType::UNKNOWN, "UNKNOWN");

	// --------------------------------------------
	// translate opcode
	// --------------------------------------------
	const char* translateOpcode(int t) {
		if (t < OPCODE_MAPPING_COUNT) {
			return OPCODE_MAPPING[t].name;
		}
		return UNKNOWN_DEFINITION.name;
	}
	
	// --------------------------------------------
	// find opcode by type
	// --------------------------------------------
	gen::OpcodeType find_opcode_type(const char* name) {
		for (int i = 0; i < OPCODE_MAPPING_COUNT; ++i) {
			if (strcmp(OPCODE_MAPPING[i].name, name) == 0) {
				return OPCODE_MAPPING[i].type;
			}
		}
		return gen::OpcodeType::UNKNOWN;
	}

	// --------------------------------------------
	// find opcode by name
	// --------------------------------------------
	OpcodeDefinition find_opcode(const char* name) {
		for (int i = 0; i < OPCODE_MAPPING_COUNT; ++i) {
			if (strcmp(OPCODE_MAPPING[i].name, name) == 0) {
				return OPCODE_MAPPING[i];
			}
		}
		return UNKNOWN_DEFINITION;
	}

	// --------------------------------------------
	// find opcode by type
	// --------------------------------------------
	OpcodeDefinition find_opcode(int type) {
		for (int i = 0; i < OPCODE_MAPPING_COUNT; ++i) {
			if (OPCODE_MAPPING[i].type == type) {
				return OPCODE_MAPPING[i];
			}
		}
		return UNKNOWN_DEFINITION;
	}

	// --------------------------------------------
	// smooth position
	// --------------------------------------------
	v3 smooth_position(const v3& p) {
		v3 ret;
		for (int i = 0; i < 3; ++i) {
			ret.data[i] = p.data[i];
			if (fabs(p.data[i]) < EPSILON) {
				ret.data[i] = 0.0f;
			}
		}
		return ret;
	}

	// --------------------------------------------
	// check if two vectors are nearly equals
	// --------------------------------------------
	bool equals(const v3& f, const v3& s) {
		v3 d;
		int cnt = 0;
		for (int i = 0; i < 3; ++i) {
			float d = f.data[i] - s.data[i];
			if (fabs(d) < EPSILON) {
				++cnt;
			}
		}
		return cnt == 3;
	}

	// --------------------------------------------
	// check if a list of positions is convex
	// --------------------------------------------
	bool is_clockwise(const v3& n,v3* positions, int num) {
		int cnt = 0;
		for (int i = 0; i < num; ++i) {
			v3 df1 = positions[(i + 1) % num] - positions[i];
			v3 df2 = positions[(i + 2) % num] - positions[(i + 1) % num];
			v3 c = cross(df1, df2);
			float dn = dot(n, c);
			//LOG << "DN:" << dn;
			//float d = cross(df1, df2).z;
			float d = cross(positions[i], positions[(i + 1) % num]).z;
			if (dn < 0.0f) {
				--cnt;
			}
			else {
				++cnt;
			}
		}
		//LOG << "CNT: " << cnt;
		if (cnt == num) {
			return true;
		}
		return false;
	}

	namespace gen {

		MeshGen::MeshGen() : _selectionColor(Color(192, 192, 192, 255)), _selectedColor(Color::WHITE), _currentGroup(-1) , _groupCounter(0) {}

		MeshGen::~MeshGen() {}

		int MeshGen::find_vertices(const v3& pos, uint16_t* ret, int max) {
			int cnt = 0;
			for (uint32_t i = 0; i < _vertices.size(); ++i) {
				v3 p = _vertices[i];
				if (equals(p, pos) && cnt < max) {
					ret[cnt++] = i;
				}
			}
			return cnt;
		}

		int MeshGen::find_edges(const v3& pos, uint16_t* ret, int max) {
			int cnt = 0;
			for (uint32_t i = 0; i < _edges.size(); ++i) {
				const Edge& e = _edges[i];
				v3 p = _vertices[e.vert_index];
				if (equals(p, pos) && cnt < max) {
					ret[cnt++] = i;
				}
			}
			return cnt;
		}

		int MeshGen::find_edge(const v3& start, const v3& end) {
			//LOG << "find_edge: " << DBG_V3(start) << " " << DBG_V3(end);
			for (uint32_t i = 0; i < _edges.size(); ++i) {
				const Edge& e = _edges[i];
				v3 es = _vertices[e.vert_index];
				v3 ee = _vertices[_edges[e.next].vert_index];
				if (start == es && end == ee) {
					return i;
				}
			}
			return -1;
		}

		// ----------------------------------------------
		// add vertex
		// ----------------------------------------------
		int MeshGen::add_vertex(const v3& pos) {
			int ret = _vertices.size();
			_vertices.push_back(pos);
			return ret;
		}

		int intersect_triangle(const ds::Ray& ray, const v3& p0, const v3& p1, const v3& p2, float *t, float *u, float *v) {
			/* find vectors for two edges sharing vert0 */
			v3 edge1 = p1 - p0;
			v3 edge2 = p2 - p0;

			/* begin calculating determinant - also used to calculate U parameter */
			v3 pvec = cross(ray.direction, edge2);

			/* if determinant is near zero, ray lies in plane of triangle */
			float det = dot(edge1, pvec);

			if (det > -EPSILON && det < EPSILON) {
				return 0;
			}
			float inv_det = 1.0f / det;

			/* calculate distance from vert0 to ray origin */
			v3 tvec = ray.origin - p0;

			/* calculate U parameter and test bounds */
			*u = dot(tvec, pvec) * inv_det;
			if (*u < 0.0 || *u > 1.0f) {
				return 0;
			}
			/* prepare to test V parameter */
			v3 qvec = cross(tvec, edge1);

			/* calculate V parameter and test bounds */
			*v = dot(ray.direction, qvec) * inv_det;
			if (*v < 0.0 || *u + *v > 1.0f) {
				return 0;
			}
			/* calculate t, ray intersects triangle */
			*t = dot(edge2, qvec) * inv_det;
			return 1;
		}

		// ----------------------------------------------
		// intersects
		// ----------------------------------------------
		int MeshGen::intersects(const ds::Ray& ray) {
			float tmax = 10000.0f;
			int face = -1;
			for (uint32_t i = 0; i < _faces.size(); ++i) {
				const Face& f = _faces[i];
				const Edge& e1 = _edges[f.edge];
				const Edge& e2 = _edges[e1.next];
				const Edge& e3 = _edges[e2.next];
				const Edge& e4 = _edges[e3.next];
				float u, v, t;
				int ret = intersect_triangle(ray, _vertices[e4.vert_index], _vertices[e1.vert_index], _vertices[e2.vert_index], &t, &u, &v);
				if (ret != 0) {
					//LOG << "1 - FOUND face: " << i << " u: " << u << " v: " << v << " t: " << t;
					if (t < tmax) {
						tmax = t;
						face = i;
					}
				}
				ret = intersect_triangle(ray, _vertices[e2.vert_index], _vertices[e3.vert_index], _vertices[e4.vert_index], &t, &u, &v);
				if (ret != 0) {
					//LOG << "1 - FOUND face: " << i << " u: " << u << " v: " << v << " t: " << t;
					if (t < tmax) {
						tmax = t;
						face = i;
					}
				}
			}
			//LOG << "face: " << face;
			return face;
		}

		// ----------------------------------------------
		// get color
		// ----------------------------------------------
		const Color& MeshGen::get_color(uint16_t face_index) const {
			return _faces[face_index].color;
		}

		// ----------------------------------------------
		// debug
		// ----------------------------------------------
		void MeshGen::debug() {
			LOG << "faces: " << _faces.size();
			for (uint32_t i = 0; i < _faces.size(); ++i) {
				const Face& f = _faces[i];
				LOG << "=> edge: " << f.edge << " normal: " << DBG_V3(f.n) << " color: " << DBG_CLR(f.color) << " group: " << f.group;
				const Edge& e = _edges[f.edge];
				int idx = f.edge;
				for (int j = 0; j < 4; ++j) {
					Edge& e = _edges[idx];
					LOG << "edge: " << idx << " v: " << e.vert_index << " next: " << e.next << " prev: " << e.prev << " vertex (" << e.vert_index<< ") : " << DBG_V3(_vertices[e.vert_index]);
					idx = e.next;
				}
			}
		}

		void MeshGen::debug_face(uint16_t face_index) {
			if (face_index < _faces.size()) {
				const Face& f = _faces[face_index];
				LOG << "=> Face: " << face_index << " edge: " << f.edge << " normal: " << DBG_V3(f.n) << " color: " << DBG_CLR(f.color) << " group: " << f.group;
				const Edge& e = _edges[f.edge];
				int idx = f.edge;
				for (int j = 0; j < 4; ++j) {
					Edge& e = _edges[idx];
					debug_edge(idx);
					idx = e.next;
				}
			}
			else {
				LOG << "Invalid face: " << face_index;
			}
		}

		void MeshGen::debug_edge(uint16_t edgeIndex) {
			Edge& e = _edges[edgeIndex];
			LOG << "edge: " << edgeIndex << " v: " << e.vert_index << " next: " << e.next << " prev: " << e.prev << " vertex (" << e.vert_index << ") : " << DBG_V3(_vertices[e.vert_index]);
		}

		// ----------------------------------------------
		// calculate normal
		// ----------------------------------------------
		void MeshGen::calculate_normal(Face* f) {
			const Edge& e0 = _edges[f->edge];
			const Edge& e1 = _edges[e0.next];
			const Edge& e3 = _edges[e0.prev];
			v3 s = _vertices[e0.vert_index];
			v3 end1 = _vertices[e1.vert_index];
			v3 end2 = _vertices[e3.vert_index];
			v3 a = end1 - s;
			v3 b = end2 - s;
			f->n = normalize(cross(a, b));
		}

		// ----------------------------------------------
		// recalculate all normals
		// ----------------------------------------------
		void MeshGen::recalculate_normals() {
			for (uint32_t i = 0; i < _faces.size(); ++i) {
				Face& f = _faces[i];
				calculate_normal(&f);
			}
		}

		// ----------------------------------------------
		// extrude edge
		// ----------------------------------------------
		uint16_t MeshGen::extrude_edge(uint16_t edgeIndex, const v3& pos) {
			const Edge& e0 = _edges[edgeIndex];
			const Edge& e1 = _edges[e0.next];
			v3 p[4];
			p[0] = _vertices[e0.vert_index];
			p[1] = p[0] + pos;
			p[2] = _vertices[e1.vert_index] + pos;
			p[3] = _vertices[e1.vert_index];
			return add_face(p);
		}

		// ----------------------------------------------
		// extrude edge along normal by factor
		// ----------------------------------------------
		uint16_t MeshGen::extrude_edge(uint16_t edgeIndex, float factor) {
			const Edge& e0 = _edges[edgeIndex];
			const Edge& e1 = _edges[e0.next];
			v3 c = get_center(e0.face_index);
			v3 ce = (_vertices[e0.vert_index] + _vertices[e1.vert_index]) / 2.0f;
			v3 n = normalize(ce - c);
			v3 p[4];
			p[0] = _vertices[e0.vert_index] + n * factor;
			p[1] = _vertices[e1.vert_index] + n * factor;
			p[2] = _vertices[e1.vert_index];
			p[3] = _vertices[e0.vert_index];
			return add_face(p);
		}

		// ----------------------------------------------
		// extrude face
		// ----------------------------------------------
		uint16_t MeshGen::extrude_face(uint16_t face_index, float factor) {
			const Face& f = _faces[face_index];
			int ei = f.edge;
			v3 p[4];
			v3 n = f.n * factor;
			if (factor < 0.0f) {
				n *= -1.0f;
			}
			for (int i = 0; i < 4; ++i) {
				Edge& e = _edges[ei];
				p[i] = _vertices[e.vert_index];
				p[i] += n;
				ei = e.next;
			}
			uint16_t newFace = add_face(p);
			set_color(newFace, f.color);
			const Face& nf = _faces[newFace];
			//LOG << "new face: " << newFace;
			ei = f.edge;
			int nei = nf.edge;
			for (int i = 0; i < 4; ++i) {
				Edge& e = _edges[ei];
				Edge& ne = _edges[nei];
				uint16_t nnf = combine_edges(ei,nei);
				set_color(nnf, f.color);
				ei = e.next;
				nei = ne.next;
			}
			MeshGenOpcode op;
			op.type = OpcodeType::EXTRUDE_FACE;
			op.offset = _store.add_data(face_index);
			_store.add_data(factor);
			record(op);
			return newFace;
		}

		// ----------------------------------------------
		// add face
		// ----------------------------------------------
		uint16_t MeshGen::add_face(const v3& p0, const v3& p1, const v3& p2, const v3& p3) {
			v3 p[] = { p0, p1, p2, p3 };
			MeshGenOpcode op;
			op.type = OpcodeType::ADD_FACE;
			op.offset = _store.add_data(p0);
			_store.add_data(p1);
			_store.add_data(p2);
			_store.add_data(p3);
			record(op);
			return add_face(p);
		}

		// ----------------------------------------------
		// add face
		// ----------------------------------------------
		uint16_t MeshGen::add_face(v3* positions) {
			Face f;
			int idx = _edges.size();
			uint16_t fidx = _faces.size();
			for (int i = 0; i < 4; ++i) {
				v3 start = positions[i];
				v3 end = positions[(i + 1) % 4];
				Edge e;
				e.next = idx + (i + 1) % 4;
				e.prev = idx + (i - 1) % 4;
				if (i == 0) {
					e.prev = idx + 3;
				}
				e.vert_index = add_vertex(positions[i]);
				e.face_index = fidx;
				_edges.push_back(e);
			}
			f.selected = false;
			f.edge = idx;
			f.color = Color::WHITE;
			f.group = _currentGroup;
			calculate_normal(&f);
			uint16_t fi = _faces.size();
			_faces.push_back(f);			
			return fi;
		}

		// ----------------------------------------------
		// get edge index
		// ----------------------------------------------
		uint16_t MeshGen::get_edge_index(uint16_t face_index, int nr) {
			LOG << "FACE: " << face_index;
			const Face& f = _faces[face_index];
			Edge& e = _edges[f.edge];
			LOG << "top: " << f.edge;
			for (int i = 0; i < nr; ++i) {
				e = _edges[e.next];
			}
			LOG << "final: " << f.edge;
			return f.edge;
		}

		// ----------------------------------------------
		// select or unselect face
		// ----------------------------------------------
		bool MeshGen::select_face(uint16_t face_index) {
			bool ret = false;
			if (face_index < _faces.size()) {
				_faces[face_index].selected = !_faces[face_index].selected;
				ret = _faces[face_index].selected;
			}
			return ret;
		}

		// ----------------------------------------------
		// clear all selections
		// ----------------------------------------------
		void MeshGen::clear_selection() {
			for (uint32_t i = 0; i < _faces.size(); ++i) {
				_faces[i].selected = false;
			}
		}

		// ----------------------------------------------
		// texture face
		// ----------------------------------------------
		void MeshGen::texture_face(uint16_t face_index, const Texture& t) {
			if (face_index < _faces.size()) {
				const Face& f = _faces[face_index];
				int idx = f.edge;
				for (int i = 0; i < 4; ++i) {
					Edge& e = _edges[idx];
					e.uv = t.uv[i];
					idx = e.next;
				}
			}
		}

		// ----------------------------------------------
		// combine
		// ----------------------------------------------
		uint16_t MeshGen::make_face(uint16_t* edges) {
			v3 p[4];
			for (int i = 0; i < 4; ++i) {
				p[i] = _vertices[_edges[edges[i]].vert_index];
			}
			MeshGenOpcode op;
			op.type = OpcodeType::MAKE_FACE;
			op.offset = _store.data.size();
			for (int i = 0; i < 4; ++i) {
				_store.add_data(edges[i]);
			}
			record(op);
			return add_face(p);
		}

		// ----------------------------------------------
		// combine edges
		// ----------------------------------------------
		uint16_t MeshGen::combine_edges(uint16_t edge0, uint16_t edge1) {
			v3 p[4];
			const Edge& e0 = _edges[edge0];
			v3 n = _faces[e0.face_index].n;
			const Edge& e1 = _edges[e0.next];
			const Edge& e2 = _edges[edge1];
			const Edge& e3 = _edges[e2.next];
			p[0] = _vertices[e0.vert_index];
			p[1] = _vertices[e1.vert_index];
			p[2] = _vertices[e3.vert_index];
			p[3] = _vertices[e2.vert_index];
			if (!is_clockwise(n,p,4)) {
				v3 t = p[0];
				p[0] = p[1];
				p[1] = t;
			}
			MeshGenOpcode op;
			op.type = OpcodeType::COMBINE_EDGES;
			op.offset = _store.add_data(edge0);
			_store.add_data(edge1);
			record(op);
			uint16_t fi = add_face(p);
			set_color(fi, _selectedColor);
			return fi;
		}

		// ----------------------------------------------
		// subdivide
		// ----------------------------------------------
		void MeshGen::subdivide(uint16_t face_index) {
			const Face& f = _faces[face_index];
			int ei = f.edge;
			uint16_t nf = hsplit_edge(ei);
			vsplit_edge(_edges[ei].prev);
			const Face& next = _faces[nf];
			const Edge& ne = _edges[next.edge];
			vsplit_edge(ne.prev);
		}

		// ----------------------------------------------
		// split edge horizontally
		// ----------------------------------------------
		uint16_t MeshGen::hsplit_edge(uint16_t edgeIndex, float factor) {
			const Edge& e = _edges[edgeIndex];
			const Edge& n = _edges[e.next];
			const Edge& nn = _edges[n.next];
			const Edge& nnn = _edges[nn.next];
			v3 o1 = _vertices[n.vert_index];
			v3 o2 = _vertices[nn.vert_index];
			v3 delta = (_vertices[n.vert_index] - _vertices[e.vert_index]) * (1.0f - factor);
			v3 d2 = (_vertices[nn.vert_index] - _vertices[nnn.vert_index]) * (1.0f - factor);
			_vertices[n.vert_index] -= delta;
			_vertices[nn.vert_index] -= d2;
			v3 p[] = { _vertices[n.vert_index] , o1, o2, _vertices[nn.vert_index] };
			MeshGenOpcode op;
			op.type = OpcodeType::H_SPLIT;
			op.offset = _store.add_data(edgeIndex);
			_store.add_data(factor);
			record(op);
			const Face& face = _faces[e.face_index];
			uint16_t f = add_face(p);
			set_color(f, face.color);
			return f;
		}

		// ----------------------------------------------
		// split edge vertically
		// ----------------------------------------------
		uint16_t MeshGen::vsplit_edge(uint16_t edgeIndex, float factor) {
			const Edge& e = _edges[edgeIndex];
			const Edge& n = _edges[e.next];
			const Edge& nn = _edges[n.next];
			const Edge& nnn = _edges[nn.next];
			v3 o1 = _vertices[n.vert_index];
			v3 o2 = _vertices[nn.vert_index];
			v3 delta = (_vertices[n.vert_index] - _vertices[e.vert_index]) * (1.0f - factor);
			v3 d2 = (_vertices[nn.vert_index] - _vertices[nnn.vert_index]) * (1.0f - factor);
			_vertices[n.vert_index] -= delta;
			_vertices[nn.vert_index] -= d2;
			v3 p[] = { o1, o2, _vertices[nn.vert_index] , _vertices[n.vert_index] };
			MeshGenOpcode op;
			op.type = OpcodeType::V_SPLIT;
			op.offset = _store.add_data(edgeIndex);
			_store.add_data(factor);
			record(op);
			const Face& face = _faces[e.face_index];
			uint16_t f = add_face(p);
			set_color(f, face.color);
			return f;
		}

		// ----------------------------------------------
		// num faces
		// ----------------------------------------------
		int MeshGen::num_faces() const {
			return _faces.size();
		}

		// ----------------------------------------------
		// move vertex
		// ----------------------------------------------
		void MeshGen::move_vertex(uint16_t vert_index, const v3& position) {
			uint16_t connections[16];
			int num = find_vertices(_vertices[vert_index], connections, 16);
			for (int j = 0; j < num; ++j) {
				_vertices[connections[j]] += position;
			}
			MeshGenOpcode op;
			op.type = OpcodeType::MOVE_VERTEX;
			op.offset = _store.add_data(vert_index);
			_store.add_data(position);
			record(op);
		}

		// ----------------------------------------------
		// move edge
		// ----------------------------------------------
		void MeshGen::move_edge(uint16_t edgeIndex, const v3& position) {
			const Edge& e0 = _edges[edgeIndex];
			const Edge& e1 = _edges[e0.next];
			uint16_t connections[16];
			v3 p[] = { _vertices[e0.vert_index], _vertices[e1.vert_index] };
			for (int i = 0; i < 2; ++i) {
				int num = find_vertices(p[i], connections, 16);
				for (int j = 0; j < num; ++j) {
					_vertices[connections[j]] += position;
				}
			}
			MeshGenOpcode op;
			op.type = OpcodeType::MOVE_EDGE;
			op.offset = _store.add_data(edgeIndex);
			_store.add_data(position);
			record(op);
		}

		// ----------------------------------------------
		// move face
		// ----------------------------------------------
		void MeshGen::move_face(uint16_t face_index, const v3& position) {
			Face& f = _faces[face_index];
			int ei = f.edge;
			uint16_t connections[16];
			for (int i = 0; i < 4; ++i) {
				const Edge& e = _edges[ei];
				//move_edge(ei, position);
				int num = find_vertices(_vertices[e.vert_index], connections, 16);
				for (int j = 0; j < num; ++j) {
					_vertices[connections[j]] += position;
				}
				ei = e.next;
			}
			calculate_normal(&f);
		}

		// ----------------------------------------------
		// scale face
		// ----------------------------------------------
		void MeshGen::scale_face(uint16_t face_index, float scale) {
			const Face& f = _faces[face_index];
			int ei = f.edge;
			v3 center = get_center(face_index);
			v3 c[4];
			for (int i = 0; i < 4; ++i) {
				Edge& e0 = _edges[ei];
				Edge& e1 = _edges[e0.next];
				v3 ce = (_vertices[e0.vert_index] + _vertices[e1.vert_index]) / 2.0f;
				v3 cde = ce - center;
				if (scale > 1.0f) {
					c[i] = cde * (1.0f - scale);
				}
				else {
					c[i] = cde * -scale;
				}
				ei = e0.next;
			}
			ei = f.edge;
			uint16_t connections[16];
			for (int i = 0; i < 4; ++i) {
				Edge& e0 = _edges[ei];
				move_edge(ei, c[i]);
				ei = e0.next;
			}
		}

		// ----------------------------------------------
		// get center of face
		// ----------------------------------------------
		v3 MeshGen::get_center(uint16_t face_index) {
			const Face& f = _faces[face_index];
			int ei = f.edge;
			v3 ret;
			for (int i = 0; i < 4; ++i) {
				Edge& e = _edges[ei];
				ret += _vertices[e.vert_index];
				ei = e.next;
			}
			return ret / 4.0f;
		}

		// ----------------------------------------------
		// set color on all selected faces
		// ----------------------------------------------
		void MeshGen::set_color(const Color& color) {
			for (uint32_t i = 0; i < _faces.size(); ++i) {
				if (_faces[i].selected) {
					_faces[i].color = color;
				}
			}
		}

		// ----------------------------------------------
		// set color
		// ----------------------------------------------
		void MeshGen::set_color(uint16_t face_index, const Color& color) {
			if (face_index < _faces.size()) {
				_faces[face_index].color = color;
				MeshGenOpcode op;
				op.type = OpcodeType::SET_COLOR;
				op.offset = _store.add_data(face_index);
				_store.add_data(color);
				record(op);
			}
		}

		void MeshGen::add_face(const v3& position, const v2& size, const v3& normal) {

		}

		// ----------------------------------------------
		// convert geometry into mesh
		// ----------------------------------------------
		void MeshGen::build(Mesh* mesh) {
			for (uint32_t i = 0; i < _faces.size(); ++i) {
				const Face& f = _faces[i];
				const Edge& e = _edges[f.edge];
				int idx = f.edge;
				for (int j = 0; j < 4; ++j) {
					Edge& e = _edges[idx];
					if (f.selected) {
						mesh->add(smooth_position(_vertices[e.vert_index]), f.n, e.uv, _selectionColor);
					}
					else {
						mesh->add(smooth_position(_vertices[e.vert_index]), f.n, e.uv, f.color);
					}
					idx = e.next;
				}
			}
		}

		// ----------------------------------------------
		// add cube
		// ----------------------------------------------
		uint16_t MeshGen::add_cube(const v3 & position, const v3 & size, uint16_t* faces) {
			v3 half_size = size * 0.5f;
			uint16_t my_faces[6];
			v3 p0[4];
			v3 p[] = {
				v3(-half_size.x, half_size.y, half_size.z),
				v3( half_size.x, half_size.y, half_size.z),
				v3( half_size.x, half_size.y,-half_size.z),
				v3(-half_size.x, half_size.y,-half_size.z),
				v3(-half_size.x,-half_size.y, half_size.z),
				v3( half_size.x,-half_size.y, half_size.z),
				v3( half_size.x,-half_size.y,-half_size.z),
				v3(-half_size.x,-half_size.y,-half_size.z)
			};
			int indices[] = { 
				// front
				3, 2, 6, 7, 
				// right
				2, 1, 5, 6,
				// back
				1, 0, 4, 5,
				// left
				0, 3, 7, 4,
				// top
				0, 1, 2, 3 ,
				// bottom
				5, 4, 7, 6
			};
			for (int j = 0; j < 6; ++j) {
				for (int i = 0; i < 4; ++i) {
					p0[i] = p[indices[j * 4 + i]] + position;
				}
				my_faces[j] = add_face(p0);
				set_color(my_faces[j], _selectedColor);
			}
			if (faces != 0) {
				for (int i = 0; i < 6; ++i) {
					faces[i] = my_faces[i];
				}
			}
			MeshGenOpcode op;
			op.type = OpcodeType::ADD_CUBE;
			op.offset = _store.add_data(position);
			_store.add_data(size);
			record(op);
			return my_faces[0];
		}

		// ----------------------------------------------
		// add cube
		// ----------------------------------------------
		uint16_t MeshGen::add_cube(const v3 & position, const v3 & size, const Color& c, uint16_t* faces) {
			v3 half_size = size * 0.5f;
			uint16_t my_faces[6];
			v3 p0[4];
			v3 p[] = {
				v3(-half_size.x, half_size.y, half_size.z),
				v3(half_size.x, half_size.y, half_size.z),
				v3(half_size.x, half_size.y,-half_size.z),
				v3(-half_size.x, half_size.y,-half_size.z),
				v3(-half_size.x,-half_size.y, half_size.z),
				v3(half_size.x,-half_size.y, half_size.z),
				v3(half_size.x,-half_size.y,-half_size.z),
				v3(-half_size.x,-half_size.y,-half_size.z)
			};
			int indices[] = {
				// front
				3, 2, 6, 7,
				// right
				2, 1, 5, 6,
				// back
				1, 0, 4, 5,
				// left
				0, 3, 7, 4,
				// top
				0, 1, 2, 3 ,
				// bottom
				5, 4, 7, 6
			};
			for (int j = 0; j < 6; ++j) {
				for (int i = 0; i < 4; ++i) {
					p0[i] = p[indices[j * 4 + i]] + position;
				}
				my_faces[j] = add_face(p0);
				set_color(my_faces[j], c);
			}
			if (faces != 0) {
				for (int i = 0; i < 6; ++i) {
					faces[i] = my_faces[i];
				}
			}
			MeshGenOpcode op;
			op.type = OpcodeType::ADD_COLOR_CUBE;
			op.offset = _store.add_data(position);
			_store.add_data(size);
			_store.add_data(c);
			record(op);
			return my_faces[0];
		}

		// ----------------------------------------------
		// get connected edges
		// ----------------------------------------------
		int MeshGen::get_connected_edges(uint16_t edge_index, uint16_t* ret, int max) {
			int cnt = 0;
			const Edge& e0 = _edges[edge_index];
			const Edge& e1 = _edges[e0.next];
			uint16_t c[16];
			int num = find_edges(_vertices[e0.vert_index], c, 16);
			for (int i = 0; i < num; ++i) {
				const Edge& ne0 = _edges[c[i]];
				const Edge& ne1 = _edges[ne0.prev];
				if (equals(_vertices[ne1.vert_index], _vertices[e1.vert_index])) {
					ret[cnt++] = c[i];
				}
			}
			return cnt;
		}

		// ----------------------------------------------
		// smooth
		// ----------------------------------------------
		void MeshGen::smooth(const IndexList& list, float radius) {
			v3 center = v3(0, 0, 0); 
			IndexList verts;
			for (uint32_t i = 0; i < list.indices.size(); ++i) {
				const Face& f = _faces[list.indices[i]];
				int ei = f.edge;
				for (int j = 0; j < 4; ++j) {
					const Edge& e = _edges[ei];
					if (!verts.add_unique(ei)) {
						LOG << "Vert already added";
					}
					ei = e.next;
				}
			}
			for (uint32_t i = 0; i < verts.indices.size(); ++i) {
				center += _vertices[verts.indices[i]];
			}
			center /= static_cast<float>(verts.indices.size());
			//LOG << "center: " << DBG_V3(center);
			for (uint32_t i = 0; i < verts.indices.size(); ++i) {
				v3 v = _vertices[verts.indices[i]];
				float x2 = v.x * v.x;
				float y2 = v.y * v.y;
				float z2 = v.z * v.z;
				v3 nv;
				nv.x = v.x * sqrt(1.0f - y2 * 0.5f - z2 * 0.5f + y2 * z2 / 3.0f);
				nv.y = v.y * sqrt(1.0f - z2 * 0.5f - x2 * 0.5f + z2 * x2 / 3.0f);
				nv.z = v.z * sqrt(1.0f - x2 * 0.5f - y2 * 0.5f + x2 * y2 / 3.0f);
				//_vertices[verts.indices[i]] = normalize(_vertices[verts.indices[i]]) * radius;
				_vertices[verts.indices[i]] = nv;
			}
			recalculate_normals();
		}

		// ----------------------------------------------
		// find adjacent faces
		// ----------------------------------------------
		void MeshGen::find_adjacent_faces(uint16_t face_index, IndexList& list) {
			const Face& f = _faces[face_index];
			int ei = f.edge;
			list.add_unique(face_index);
			uint16_t connections[16];
			for (int i = 0; i < 4; ++i) {
				Edge& e0 = _edges[ei];
				int num = get_connected_edges(ei, connections, 16);
				for (int k = 0; k < num; ++k) {
					const Edge& curr = _edges[connections[k]];
					if (list.add_unique(curr.face_index)) {
						find_adjacent_faces(curr.face_index, list);
					}
				}
				ei = e0.next;
			}
		}

		// ----------------------------------------------
		// rotate group
		// ----------------------------------------------
		void MeshGen::rotateGroup(int group, const v3& rotation) {
			mat4 rotY = matrix::mat4RotationY(rotation.y);
			mat4 rotX = matrix::mat4RotationX(rotation.x);
			mat4 rotZ = matrix::mat4RotationZ(rotation.z);
			mat4 world = rotZ * rotY * rotX;
			for (int i = 0; i < _faces.size(); ++i) {
				const Face& f = _faces[i];
				int ei = f.edge;
				if (f.group == group) {
					for (int j = 0; j < 4; ++j) {
						const Edge& e = _edges[ei];
						_vertices[e.vert_index] = _vertices[e.vert_index] * world;
						ei = e.next;
					}
				}
			}
		}

		// ----------------------------------------------
		// copy group
		// ----------------------------------------------
		void MeshGen::copyGroup(int group, const v3& pos) {
			v3 p[4];
			for (int i = 0; i < _faces.size(); ++i) {
				const Face& f = _faces[i];
				int ei = f.edge;
				if (f.group == group) {
					for (int j = 0; j < 4; ++j) {
						const Edge& e = _edges[ei];
						p[j] = _vertices[e.vert_index] + pos;
						ei = e.next;
					}
					uint16_t fi = add_face(p);
					set_color(fi, f.color);
				}
			}
		}

		// ----------------------------------------------
		// move group
		// ----------------------------------------------
		void MeshGen::moveGroup(int group, const v3& pos) {
			for (int i = 0; i < _faces.size(); ++i) {
				const Face& f = _faces[i];
				int ei = f.edge;
				if (f.group == group) {
					for (int j = 0; j < 4; ++j) {
						const Edge& e = _edges[ei];
						_vertices[e.vert_index] += pos;
						ei = e.next;
					}
				}
			}
		}

		// ----------------------------------------------
		// add cube
		// ----------------------------------------------
		uint16_t MeshGen::add_cube(const v3 & position, const v3 & size, const v3 & rotation) {
			mat4 rotY = matrix::mat4RotationY(rotation.y);
			mat4 rotX = matrix::mat4RotationX(rotation.x);
			mat4 rotZ = matrix::mat4RotationZ(rotation.z);
			mat4 t = matrix::mat4Transform(position);
			mat4 s = matrix::mat4Scale(size);
			mat4 world = rotZ * rotY * rotX * s * t;

			v3 p0[4];
			for (int i = 0; i < 4; ++i) {
				p0[i] = world * CUBE_VERTICES[i];
			}
			v3 e[] = {
				v3(0,0,1),
				v3(-1,0,0),
				v3(0,0,-1),
				v3(0,0,1),
				v3(0,0,-1)
			};
			for (int i = 0; i < 5; ++i) {
				e[i] = world * e[i];
			}
			int indices[] = { 1,5,9,0,10 };
			uint16_t faces[6];
			faces[0] = add_face(p0);
			const Face& f = _faces[faces[0]];
			for (int i = 0; i < 5; ++i) {
				faces[i + 1] = extrude_edge(f.edge + indices[i], e[i]);
			}
			MeshGenOpcode op;
			op.type = OpcodeType::ADD_CUBE_ROT;
			op.offset = _store.add_data(position);
			_store.add_data(size);
			_store.add_data(rotation);
			record(op);
			return faces[0];
		}

		// ----------------------------------------------
		// get face
		// ----------------------------------------------
		const Face& MeshGen::get_face(uint16_t face_index) const {
			return _faces[face_index];
		}

		// ----------------------------------------------
		// get edge from specific face
		// ----------------------------------------------
		uint16_t MeshGen::get_edge(uint16_t face_index, uint16_t edge_offset) {
			const Face& f = _faces[face_index];
			int ei = f.edge;
			for (int i = 0; i < edge_offset; ++i) {
				Edge& e = _edges[ei];
				ei = e.next;
			}
			return ei;
		}

		// ----------------------------------------------
		// vertices
		// ----------------------------------------------
		void MeshGen::get_vertices(const Face& face, v3* ret) const {
			int ei = face.edge;
			for (int i = 0; i < 4; ++i) {
				ret[i] = _vertices[ei];
				const Edge& e = _edges[ei];
				ei = e.next;
			}
		}

		int MeshGen::slice(uint16_t face_index, int segments, uint16_t* faces, int max) {
			const Face& f = _faces[face_index];
			const Edge& e0 = _edges[f.edge];
			const Edge& e1 = _edges[e0.next];
			const Edge& e2 = _edges[e1.next];
			const Edge& e3 = _edges[e2.next];
			int cnt = 0;
			if (faces != 0 && cnt < max) {
				faces[cnt++] = face_index;
			}
			v3 n1 = normalize(_vertices[e1.vert_index] - _vertices[e0.vert_index]);
			v3 n2 = normalize(_vertices[e2.vert_index] - _vertices[e1.vert_index]);
			v3 n3 = normalize(_vertices[e3.vert_index] - _vertices[e0.vert_index]);

			float sx = length(_vertices[e1.vert_index] - _vertices[e0.vert_index]) / static_cast<float>(segments);
			float sy = length(_vertices[e2.vert_index] - _vertices[e1.vert_index]) / static_cast<float>(segments);

			_vertices[e1.vert_index] = _vertices[e0.vert_index] + n1 * sx;
			_vertices[e2.vert_index] = _vertices[e1.vert_index] + n2 * sy;
			_vertices[e3.vert_index] = _vertices[e0.vert_index] + n3 * sy;			
			v3 s1 = n1 * sx;
			v3 s2 = n3 * sy;
			v3 p[4];
			v3 org = _vertices[e0.vert_index];
			for (int y = 0; y < segments; ++y) {
				for (int x = 0; x < segments; ++x) {
					if ( x + y != 0 ) {
						v3 sp = org + s1 * static_cast<float>(x);
						sp += s2 * static_cast<float>(y);
						p[0] = sp;
						p[1] = sp + s1;
						p[2] = sp + s1 + s2;
						p[3] = sp + s2;
						uint16_t fi = add_face(p);
						set_color(fi, f.color);
						if (faces != 0 && cnt < max) {
							faces[cnt++] = fi;
						}
					}
				}
			}
			return cnt;
		}

		int MeshGen::slice(uint16_t face_index, int stepsX, int stepsY, uint16_t* faces, int max) {
			const Face& f = _faces[face_index];
			const Edge& e0 = _edges[f.edge];
			const Edge& e1 = _edges[e0.next];
			const Edge& e2 = _edges[e1.next];
			const Edge& e3 = _edges[e2.next];
			int cnt = 0;
			if (faces != 0 && cnt < max) {
				faces[cnt++] = face_index;
			}
			v3 n1 = normalize(_vertices[e1.vert_index] - _vertices[e0.vert_index]);
			v3 n2 = normalize(_vertices[e2.vert_index] - _vertices[e1.vert_index]);
			v3 n3 = normalize(_vertices[e3.vert_index] - _vertices[e0.vert_index]);

			float sx = length(_vertices[e1.vert_index] - _vertices[e0.vert_index]) / static_cast<float>(stepsX);
			float sy = length(_vertices[e2.vert_index] - _vertices[e1.vert_index]) / static_cast<float>(stepsY);

			_vertices[e1.vert_index] = _vertices[e0.vert_index] + n1 * sx;
			_vertices[e2.vert_index] = _vertices[e1.vert_index] + n2 * sy;
			_vertices[e3.vert_index] = _vertices[e0.vert_index] + n3 * sy;
			v3 s1 = n1 * sx;
			v3 s2 = n3 * sy;
			v3 p[4];
			v3 org = _vertices[e0.vert_index];
			for (int y = 0; y < stepsY; ++y) {
				for (int x = 0; x < stepsX; ++x) {
					if (x + y != 0) {
						v3 sp = org + s1 * static_cast<float>(x);
						sp += s2 * static_cast<float>(y);
						p[0] = sp;
						p[1] = sp + s1;
						p[2] = sp + s1 + s2;
						p[3] = sp + s2;
						uint16_t fi = add_face(p);
						set_color(fi, f.color);
						if (faces != 0 && cnt < max) {
							faces[cnt++] = fi;
						}
					}
				}
			}
			MeshGenOpcode op;
			op.type = OpcodeType::SLICE;
			op.offset = _store.add_data(face_index);
			_store.add_data(stepsX);
			_store.add_data(stepsY);
			record(op);
			return cnt;
		}

		// ----------------------------------------------
		// add other
		// ----------------------------------------------
		void MeshGen::add(const MeshGen& other, const v3& position, const v3& scale, const v3& rotation) {
			v3 p[4];
			mat4 rotY = matrix::mat4RotationY(rotation.y);
			mat4 rotX = matrix::mat4RotationX(rotation.x);
			mat4 rotZ = matrix::mat4RotationZ(rotation.z);
			mat4 t = matrix::mat4Transform(position);
			mat4 s = matrix::mat4Scale(scale);
			mat4 world = rotZ * rotY * rotX * s * t;
			for (int i = 0; i < other.num_faces(); ++i) {
				const Face& f = other.get_face(i);
				other.get_vertices(f,p);
				for (int j = 0; j < 4; ++j) {
					p[j] = world * p[j];
				}
				uint16_t ni = add_face(p);
				Face& nf = _faces[ni];
				nf.color = f.color;
			}
			recalculate_normals();
		}

		// ----------------------------------------------
		// fill all faces with random color
		// ----------------------------------------------
		void MeshGen::debug_colors() {
			for (uint32_t i = 0; i < _faces.size(); ++i) {
				set_color(i, ds::Color(math::random(0, 255), math::random(0, 255), math::random(0, 255), 255));
			}
			MeshGenOpcode op;
			op.type = OpcodeType::DEBUG_COLORS;
			op.offset = 0;
			record(op);
		}

		// ----------------------------------------------
		// clear mesh
		// ----------------------------------------------
		void MeshGen::clear() {
			_vertices.clear();
			_edges.clear();
			_faces.clear();
			_store.data.clear();
			_opcodes.clear();
			_groupCounter = 0;
			_currentGroup = -1;
		}

		// ----------------------------------------------
		// record mesh gen opcode
		// ----------------------------------------------
		void MeshGen::record(const MeshGenOpcode& opcode) {
			_opcodes.push_back(opcode);
		}

		int MeshGen::startGroup() {
			_currentGroup = _groupCounter++;
			return _currentGroup;
		}

		void MeshGen::endGroup() {
			_currentGroup = -1;
		}

		// ----------------------------------------------
		// save text format
		// ----------------------------------------------
		void MeshGen::save_text(const char* fileName) {
			char buffer[256];
			sprintf_s(buffer, 256, "resources\\meshes\\%s.txt", fileName);
			FILE* f = fopen(buffer, "w");
			for (uint32_t i = 0; i < _opcodes.size(); ++i) {
				const MeshGenOpcode& op = _opcodes[i];
				OpcodeDefinition def = find_opcode(op.type);
				fprintf(f, "%s ", translateOpcode(op.type));
				//LOG << i << " : " << translateOpcode(op.type) << " args: " << def.args;
				int of = op.offset;
				if (of != -1) {
					for (int j = 0; j < def.args; ++j) {
						int dt = def.types[j];
						//LOG << "dt: " << dt << " of: " << of;
						if (dt == 0) {
							fprintf(f, "%g ", _store.data[of]);
						}
						else if (dt == 1) {
							fprintf(f, "%g,%g,%g ", _store.data[of], _store.data[of + 1], _store.data[of + 2]);
						}
						else if (dt == 2) {
							fprintf(f, "%d ", static_cast<int>(_store.data[of]));
						}
						else if (dt == 3) {
							fprintf(f, "%g,%g,%g,%g ", _store.data[of], _store.data[of + 1], _store.data[of + 2], _store.data[of + 3]);
						}
						of += OPCODE_DATASIZE[dt];
					}
				}
				else {
					LOGE << "No offset defined";
				}
				fprintf(f, "\n");
			}
			fclose(f);
		}

		void MeshGen::executeOpcodes(const Array<MeshGenOpcode>& opcodes, const DataStore& store) {
			for (uint32_t i = 0; i < opcodes.size(); ++i) {
				const MeshGenOpcode& op = opcodes[i];
				switch (op.type) {
				case OpcodeType::ADD_CUBE: {
					v3 p;
					v3 s;
					store.get_data(op, 0, &p);
					store.get_data(op, 3, &s);
					add_cube(p, s);
					break;
				}
				case OpcodeType::ADD_COLOR_CUBE: {
					v3 p;
					v3 s;
					Color clr;
					store.get_data(op, 0, &p);
					store.get_data(op, 3, &s);
					store.get_data(op, 6, &clr);
					add_cube(p, s, clr);
					break;
				}
				case OpcodeType::SLICE_UNIFORM: {
					uint16_t face_index = 0;
					int segments = 0;
					store.get_data(op, 0, &face_index);
					store.get_data(op, 1, &segments);
					slice(face_index, segments);
					break;
				}
				case OpcodeType::SLICE: {
					uint16_t face_index = 0;
					uint16_t stepsX = 0;
					uint16_t stepsY = 0;
					store.get_data(op, 0, &face_index);
					store.get_data(op, 1, &stepsX);
					store.get_data(op, 2, &stepsY);
					slice(face_index, stepsX, stepsY);
					break;
				}
				case OpcodeType::SET_COLOR: {
					uint16_t face_index = 0;
					Color color;
					store.get_data(op, 0, &face_index);
					store.get_data(op, 1, &color);
					set_color(face_index, color);
					break;
				}
				case OpcodeType::MOVE_EDGE: {
					uint16_t edge_index = 0;
					v3 p;
					store.get_data(op, 0, &edge_index);
					store.get_data(op, 1, &p);
					move_edge(edge_index, p);
					break;
				}
				case OpcodeType::V_SPLIT: {
					uint16_t edge_index = 0;
					float factor = 0.5f;
					store.get_data(op, 0, &edge_index);
					store.get_data(op, 1, &factor);
					vsplit_edge(edge_index, factor);
					break;
				}
				case OpcodeType::H_SPLIT: {
					uint16_t edge_index = 0;
					float factor = 0.5f;
					store.get_data(op, 0, &edge_index);
					store.get_data(op, 1, &factor);
					hsplit_edge(edge_index, factor);
					break;
				}
				case OpcodeType::MAKE_FACE: {
					uint16_t edges[4];
					for (int i = 0; i < 4; ++i) {
						store.get_data(op, i, &edges[i]);
					}
					make_face(edges);
					break;
				}
				case OpcodeType::ADD_FACE: {
					v3 p[4];
					for (int i = 0; i < 4; ++i) {
						store.get_data(op, i * 3, &p[i]);
					}
					add_face(p[0], p[1], p[2], p[3]);
					break;
				}
				case OpcodeType::COMBINE_EDGES: {
					uint16_t e0;
					uint16_t e1;
					store.get_data(op, 0, &e0);
					store.get_data(op, 1, &e1);
					combine_edges(e0, e1);
					break;
				}
				case OpcodeType::EXTRUDE_FACE: {
					uint16_t face;
					float factor;
					store.get_data(op, 0, &face);
					store.get_data(op, 1, &factor);
					extrude_face(face, factor);
					break;
				}
				case OpcodeType::SCALE_FACE: {
					uint16_t face;
					float factor;
					store.get_data(op, 0, &face);
					store.get_data(op, 1, &factor);
					scale_face(face, factor);
					break;
				}
				case OpcodeType::ADD_CYLINDER: {
					v3 p;
					float bottom;
					float top;
					float height;
					uint16_t segments;
					store.get_data(op, 0, &p);
					store.get_data(op, 3, &bottom);
					store.get_data(op, 4, &top);
					store.get_data(op, 5, &height);
					store.get_data(op, 6, &segments);
					create_cylinder(p, bottom, top, height, segments);
					break;
				}
				case OpcodeType::ADD_COL_CYLINDER: {
					v3 p;
					float bottom;
					float top;
					float height;
					uint16_t segments;
					Color clr;
					store.get_data(op, 0, &p);
					store.get_data(op, 3, &bottom);
					store.get_data(op, 4, &top);
					store.get_data(op, 5, &height);
					store.get_data(op, 6, &segments);
					store.get_data(op, 7, &clr);
					create_cylinder(p, bottom, top, height, segments, clr);
					break;
				}
				case OpcodeType::DEBUG_COLORS: {
					debug_colors();
					break;
				}
				case OpcodeType::MOVE_VERTEX: {
					uint16_t vert_index;
					v3 p;
					store.get_data(op, 0, &vert_index);
					store.get_data(op, 1, &p);
					move_vertex(vert_index, p);
					break;
				}
				case OpcodeType::EXTRUDE_EDGE_NORMAL: {
					uint16_t edge_index;
					float factor = 0.0f;
					store.get_data(op, 0, &edge_index);
					store.get_data(op, 1, &factor);
					extrude_edge(edge_index, factor);
					break;
				}
				case OpcodeType::MOVE_FACE: {
					uint16_t vert_index;
					v3 p;
					store.get_data(op, 0, &vert_index);
					store.get_data(op, 1, &p);
					move_face(vert_index, p);
					break;
				}
				case OpcodeType::ADD_HEXAGON: {
					float radius;
					store.get_data(op, 0, &radius);
					create_hexagon(radius);
					break;
				}
				case OpcodeType::SELECT_COLOR: {
					store.get_data(op, 0, &_selectedColor);
					break;
				}
				case OpcodeType::ADD_RING: {
					float radius;
					float width;
					uint16_t segments;
					store.get_data(op, 0, &radius);
					store.get_data(op, 1, &width);
					store.get_data(op, 2, &segments);
					create_ring(radius, width, segments);
					break;
				}
				case OpcodeType::START_GROUP: {
					startGroup();
					break;
				}
				case OpcodeType::END_GROUP: {
					endGroup();
					break;
				}
				case OpcodeType::ROTATE_GROUP: {
					int group;
					v3 p;
					store.get_data(op, 0, &group);
					store.get_data(op, 1, &p);
					for (int i = 0; i < 3; ++i) {
						p.data[i] = DEGTORAD(p[i]);
					}
					rotateGroup(group, p);
					break;
				}
				case OpcodeType::MOVE_GROUP: {
					int group;
					v3 p;
					store.get_data(op, 0, &group);
					store.get_data(op, 1, &p);
					moveGroup(group, p);
					break;
				}
				case OpcodeType::COPY_GROUP: {
					int group;
					v3 p;
					store.get_data(op, 0, &group);
					store.get_data(op, 1, &p);
					copyGroup(group, p);
					break;
				}
				}
			}
		}

		// ----------------------------------------------
		// save mesh format
		// ----------------------------------------------
		void MeshGen::save_mesh(const char* fileName) {
			recalculate_normals();
			char buffer[256];
			sprintf_s(buffer, 256, "content\\meshes\\%s.mesh", fileName);
			BinaryFile b;
			if (b.open(buffer,ds::FileMode::WRITE)) {
				b.write(_faces.size() * 4);
				for (uint32_t i = 0; i < _faces.size(); ++i) {
					const Face& face = _faces[i];
					int idx = face.edge;
					for (int j = 0; j < 4; ++j) {
						Edge& e = _edges[idx];
						b.write(smooth_position(_vertices[e.vert_index]));
						b.write(smooth_position(face.n));
						b.write(e.uv);
						b.write(face.color);												
						idx = e.next;
					}
				}
			}
		}

		// ----------------------------------------------
		// load text format
		// ----------------------------------------------
		void MeshGen::load_text(const char* fileName) {
			char buffer[256];
			sprintf_s(buffer, 256, "resources\\meshes\\%s.txt", fileName);
			clear();
			int size = -1;
			char name[256];
			Array<MeshGenOpcode> tmp_opcodes;
			DataStore store;
			const char* txt = repository::load(buffer, &size);
			if (size != -1) {
				Tokenizer t;
				t.parse(txt);
				int cnt = 0;
				while (cnt < t.size()) {
					Token& tk = t.get(cnt);
					if (tk.type == Token::NAME) {
						MeshGenOpcode oc;
						strncpy(name, txt + tk.index, tk.size);
						name[tk.size] = '\0';
						oc.type = find_opcode_type(name);		
						oc.offset = store.data.size();
						++cnt;
						tk = t.get(cnt);
						int vars = 0;
						while ((tk.type == Token::NUMBER) || (tk.type == Token::DELIMITER)) {
							tk = t.get(cnt);
							if (tk.type == Token::NUMBER) {
								store.add_data(tk.value);
								++vars;
							}
							++cnt;
						}
						// check args
						bool valid = true;
						const OpcodeDefinition& def = find_opcode(oc.type);
						if (def.type == OpcodeType::UNKNOWN) {
							LOGE << "Unknown opcode: " << name;
							valid = false;
						}
						if (valid && vars != def.var_count) {
							LOGE << "Missing argument - opcode: " << name << " required: " << def.var_count << " but got: " << vars;
							valid = false;
						}
						if ( valid) {
							tmp_opcodes.push_back(oc);
						}
					}
					else {
						++cnt;
					}
				}
				executeOpcodes(tmp_opcodes, store);
				recalculate_normals();
				delete txt;
			}
		}

		// ----------------------------------------------
		// get connected faces
		// ----------------------------------------------
		int MeshGen::get_connected_faces(uint16_t face_index, uint16_t* ret, int max) {
			const Face& f = _faces[face_index];
			return 0;
		}

		void MeshGen::create_sphere(float radius, int segments, int stacks) {
			float angleStep = TWO_PI / static_cast<float>(segments);
		}
		// ----------------------------------------------
		// create torus
		// ----------------------------------------------
		uint16_t MeshGen::create_torus(const v3& position, float radius, float width, float depth, uint16_t segments) {
			float angleStep = TWO_PI / static_cast<float>(segments);
			v3 p[4];
			v3 pe[4];
			uint16_t ret = 0;
			float angle = 0.0f;
			float next_angle = angleStep;
			float outer_radius = radius + width;
			float half_size = depth * 0.5f;
			for (int i = 0; i < segments; ++i) {
				p[0] = position + v3(radius * cos(next_angle), radius * sin(next_angle), -half_size);
				p[1] = position + v3(outer_radius * cos(next_angle), outer_radius * sin(next_angle), -half_size);
				p[2] = position + v3(outer_radius * cos(angle), outer_radius * sin(angle), -half_size);
				p[3] = position + v3(radius * cos(angle), radius * sin(angle), -half_size);
				uint16_t f1 = add_face(p);
				if (i == 0) {
					ret = f1;
				}
				const Face& fc1 = _faces[f1];
				const Edge& e1 = _edges[fc1.edge];
				uint16_t f2 = extrude_edge(e1.next, v3(0, 0, depth));
				pe[0] = p[1];
				pe[1] = p[0];
				pe[2] = p[3];
				pe[3] = p[2];
				for (int j = 0; j < 4; ++j) {
					pe[j].z += depth;
				}
				uint16_t f3 = add_face(pe);
				const Face& fc3 = _faces[f3];
				const Edge& e2 = _edges[fc3.edge];
				uint16_t f4 = extrude_edge(e2.next, v3(0, 0, -depth));

				angle += angleStep;
				next_angle += angleStep;
			}
			return ret;
		}

		// ----------------------------------------------
		// translate all vertices
		// ----------------------------------------------
		void MeshGen::translate(const v3& position) {
			transform(position, v3(1, 1, 1), v3(0, 0, 0));
		}

		// ----------------------------------------------
		// scale all vertices
		// ----------------------------------------------
		void MeshGen::scale(const v3& scale) {
			transform(v3(0, 0, 0), scale, v3(0, 0, 0));
		}

		// ----------------------------------------------
		// rotate all vertices
		// ----------------------------------------------
		void MeshGen::rotate(const v3& rotation) {
			transform(v3(0, 0, 0), v3(0, 0, 0), rotation);
		}

		// ------------------------------------------------------------
		// transforms all vertices by position, scale and rotation
		// ------------------------------------------------------------
		void MeshGen::transform(const v3& position, const v3& scale, const v3& rotation) {
			mat4 rotY = matrix::mat4RotationY(rotation.y);
			mat4 rotX = matrix::mat4RotationX(rotation.x);
			mat4 rotZ = matrix::mat4RotationZ(rotation.z);
			mat4 t = matrix::mat4Transform(position);
			mat4 s = matrix::mat4Scale(scale);
			mat4 world = rotZ * rotY * rotX * s * t;
			for (uint32_t i = 0; i < _vertices.size(); ++i) {
				_vertices[i] = world * _vertices[i];
			}
		}

		void MeshGen::create_hexagon(float radius) {
			v3 p[6];
			for (int i = 0; i < 6; ++i) {
				float angle = DEGTORAD(static_cast<float>(i) * 60.0f);
				p[i] = v3(radius*cos(angle), 0.0f, radius*sin(angle));
			}
			v3 pp[4];
			pp[0] = p[3];
			pp[1] = p[2];
			pp[2] = p[1];
			pp[3] = p[0];
			add_face(pp);
			pp[0] = p[3];
			pp[1] = p[0];
			pp[2] = p[5];
			pp[3] = p[4];
			add_face(pp);
		}
		// ----------------------------------------------
		// create ring
		// ----------------------------------------------
		void MeshGen::create_cylinder(const v3& pos, float bottomRadius, float topRadius, float height, uint16_t segments, const Color& clr) {
			float angleStep = TWO_PI / static_cast<float>(segments);
			v3 p[4];
			float angle = 0.0f;
			float next_angle = angleStep;
			float hh = height * 0.5f;
			v3* topPoints = new v3[segments];
			v3* bottomPoints = new v3[segments];
			for (int i = 0; i < segments; ++i) {
				p[0] = pos + v3(topRadius * cos(angle), hh, topRadius * sin(angle));
				p[1] = pos + v3(topRadius * cos(next_angle), hh, topRadius * sin(next_angle));
				p[2] = pos + v3(bottomRadius * cos(next_angle), -hh, bottomRadius * sin(next_angle));
				p[3] = pos + v3(bottomRadius * cos(angle), -hh, bottomRadius * sin(angle));
				uint16_t fi = add_face(p);
				set_color(fi, clr);
				topPoints[i] = p[0];
				bottomPoints[i] = p[3];
				angle += angleStep;
				next_angle += angleStep;
			}
			// build center
			v3 topCenter;
			for (int i = 0; i < segments; ++i) {
				topCenter += topPoints[i];
			}
			v3 bottomCenter;
			for (int i = 0; i < segments; ++i) {
				bottomCenter += bottomPoints[i];
			}
			topCenter /= static_cast<float>(segments);
			bottomCenter /= static_cast<float>(segments);
			int faceCount = segments / 2;
			int cnt = 0;
			for (int i = 0; i < faceCount; ++i) {
				p[0] = topPoints[cnt];
				p[1] = topCenter;
				p[2] = topPoints[(cnt + 2) % segments];
				p[3] = topPoints[(cnt + 1) % segments];
				uint16_t fi = add_face(p);
				set_color(fi, clr);
				cnt += 2;
			}
			cnt = 0;
			for (int i = 0; i < faceCount; ++i) {
				p[0] = bottomPoints[cnt];				
				p[1] = bottomPoints[(cnt + 1) % segments];
				p[2] = bottomPoints[(cnt + 2) % segments];				
				p[3] = bottomCenter;
				uint16_t fi = add_face(p);
				set_color(fi, clr);
				cnt += 2;
			}
			delete[] bottomPoints;
			delete[] topPoints;
			MeshGenOpcode op;
			op.type = OpcodeType::ADD_CYLINDER;
			op.offset = _store.add_data(bottomRadius);
			_store.add_data(topRadius);
			_store.add_data(height);
			_store.add_data(segments);
			_store.add_data(clr);
			record(op);
		}

		// ----------------------------------------------
		// create ring
		// ----------------------------------------------
		void MeshGen::create_ring(float radius, float width, uint16_t segments) {
			float angleStep = TWO_PI / static_cast<float>(segments);
			v3 p[4];
			float angle = 0.0f;
			float next_angle = angleStep;
			float outer_radius = radius + width;
			for (int i = 0; i < segments; ++i) {
				p[0] = v3(radius * cos(next_angle), 0.0f, radius * sin(next_angle));
				p[1] = v3(outer_radius * cos(next_angle), 0.0f, outer_radius * sin(next_angle));
				p[2] = v3(outer_radius * cos(angle), 0.0f, outer_radius * sin(angle));
				p[3] = v3(radius * cos(angle), 0.0f, radius * sin(angle));
				uint16_t fi = add_face(p);
				set_color(fi, _selectedColor);
				angle += angleStep;
				next_angle += angleStep;
			}
		}

		// ----------------------------------------------
		// create grid
		// ----------------------------------------------
		void MeshGen::create_grid(const v2& size, int stepsX, int stepsY) {
			v3 p[4];
			float sx = -size.x / 2.0f * stepsX;
			float sy = -size.y / 2.0f * stepsY;
			float hsx = size.x * 0.5f;
			float hsy = size.y * 0.5f;
			float yp = sy + hsy;
			for (int y = 0; y < stepsY; ++y) {
				float xp = sx + hsx;
				for (int x = 0; x < stepsX; ++x) {
					p[0] = v3(xp - hsx, yp + hsy, 0.0f);
					p[1] = v3(xp + hsx, yp + hsy, 0.0f);
					p[2] = v3(xp + hsx, yp - hsy, 0.0f);
					p[3] = v3(xp - hsx, yp - hsy, 0.0f);
					add_face(p);
					xp += size.x;
				}
				yp += size.y;
			}
		}

		void MeshGen::show_edges(uint16_t face_index) {
			Color clr[] = { Color(255, 0, 0, 255), Color(0, 255, 0, 255), Color(0, 0, 255, 255), Color(255, 255, 0, 255) };
			const Face& f = _faces[face_index];
			int ei = f.edge;
			for (int i = 0; i < 4; ++i) {
				const Edge& e0 = _edges[ei];
				const Edge& e1 = _edges[e0.next];
				ei = e0.next;
				v3 p0 = _vertices[e0.vert_index];
				v3 p1 = _vertices[e1.vert_index];
				v3 d = p1 - p0;
				v3 cr = normalize(cross(d, f.n));
				float dt = dot(p0, p1);
				v3 p[4];
				p[0] = v3(p0 + cr * 0.1f);
				p[1] = v3(p1 + cr * 0.1f);
				p[2] = p1;
				p[3] = p0;
				uint16_t f = add_face(p);
				set_color(f,clr[i]);				
			}
		}

	}	
}