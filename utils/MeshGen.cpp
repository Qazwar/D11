#include "MeshGen.h"
#include "..\math\math.h"
#include "..\utils\Log.h"
#include "..\io\json.h"
#include "..\io\FileRepository.h"

#define EPSILON 0.000001

namespace ds {

	static const v3 CUBE_VERTICES[] = {
		v3(-0.5f,0.5f,-0.5f),
		v3(0.5f,0.5f,-0.5f),
		v3(0.5f,-0.5f,-0.5f),
		v3(-0.5f,-0.5f,-0.5f)
	};

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

	MeshGen::MeshGen() {}

	MeshGen::~MeshGen() {}

		int MeshGen::find_vertex(const v3& pos) {
			for (int i = 0; i < _vertices.size(); ++i) {
				const v3& v = _vertices[i];
				if (equals(v,pos)) {
					return i;
				}
			}
			return -1;
		}

		int MeshGen::find_edges(const v3& pos, uint16_t* ret, int max) {
			int cnt = 0;
			for (int i = 0; i < _edges.size(); ++i) {
				const Edge& e = _edges[i];
				v3 p = _vertices[e.vert_index];
				if (equals(p, pos) && cnt < max) {
					ret[cnt++] = i;
				}
			}
			return cnt;
		}

		int MeshGen::find_edge(const v3& start, const v3& end) {
			LOG << "find_edge: " << DBG_V3(start) << " " << DBG_V3(end);
			for (int i = 0; i < _edges.size(); ++i) {
				const Edge& e = _edges[i];
				v3 es = _vertices[e.vert_index];
				v3 ee = _vertices[_edges[e.next].vert_index];
				//LOG << "es: " << DBG_V3(es) << " ee: " << DBG_V3(ee);
				if (start == es && end == ee) {
					return i;
				}
				/*
				v3 en = _vertices[_edges[e.prev].vert_index];
				//LOG << "es: " << DBG_V3(es) << " ee: " << DBG_V3(ee);
				if (start == es && end == en) {
					return i;
				}
				*/
			}
			return -1;
		}

		// ----------------------------------------------
		// add edge
		// ----------------------------------------------
		uint16_t MeshGen::add_edge(const v3& pos) {
			Edge e;
			e.vert_index = _vertices.size();
			_vertices.push_back(pos);
			int ret = _edges.size();
			_edges.push_back(e);
			return ret;
		}

		// ----------------------------------------------
		// add vertex
		// ----------------------------------------------
		int MeshGen::add_vertex(const v3& pos) {
			/*
			for (int i = 0; i < _vertices.size(); ++i) {
				const v3& v = _vertices[i];
				if (equals(v,pos)) {
					return i;
				}
			}
			*/
			int ret = _vertices.size();
			_vertices.push_back(pos);
			return ret;
		}

		int intersect_triangle(const ds::Ray& ray,const v3& p0, const v3& p1, const v3& p2,double *t, double *u, double *v)	{
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
			float inv_det = 1.0 / det;

			/* calculate distance from vert0 to ray origin */
			v3 tvec = ray.origin - p0;

			/* calculate U parameter and test bounds */
			*u = dot(tvec, pvec) * inv_det;
			if (*u < 0.0 || *u > 1.0) {
				return 0;
			}
			/* prepare to test V parameter */
			v3 qvec = cross(tvec, edge1);

			/* calculate V parameter and test bounds */
			*v = dot(ray.direction, qvec) * inv_det;
			if (*v < 0.0 || *u + *v > 1.0) {
				return 0;
			}
			/* calculate t, ray intersects triangle */
			*t = dot(edge2, qvec) * inv_det;
			return 1;
		}


		int MeshGen::intersects(const ds::Ray& ray) {
			float tmax = 10000.0f;
			int face = -1;
			for (int i = 0; i < _faces.size(); ++i) {
				const Face& f = _faces[i];
				const Edge& e1 = _edges[f.edge];
				const Edge& e2 = _edges[e1.next];
				const Edge& e3 = _edges[e2.next];
				const Edge& e4 = _edges[e3.next];
				double u, v, t;
				int ret = intersect_triangle(ray, _vertices[e1.vert_index], _vertices[e2.vert_index], _vertices[e3.vert_index], &t, &u, &v);
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
		// debug
		// ----------------------------------------------
		void MeshGen::debug() {
			LOG << "faces: " << _faces.size();
			for (int i = 0; i < _faces.size(); ++i) {
				const Face& f = _faces[i];
				LOG << "=> edge: " << f.edge << " normal: " << DBG_V3(f.n) << " color: " << DBG_CLR(f.color);
				const Edge& e = _edges[f.edge];
				int idx = f.edge;
				for (int j = 0; j < 4; ++j) {
					Edge& e = _edges[idx];
					LOG << "edge: " << idx << " v: " << e.vert_index << " next: " << e.next << " prev: " << e.prev << " pos: " << DBG_V3(_vertices[e.vert_index]);
					idx = e.next;
				}
			}
		}

		void MeshGen::debug_face(uint16_t face_index) {
			const Face& f = _faces[face_index];
			LOG << "=> Face: " << face_index << " edge: " << f.edge << " normal: " << DBG_V3(f.n) << " color: " << DBG_CLR(f.color);
			const Edge& e = _edges[f.edge];
			int idx = f.edge;
			for (int j = 0; j < 4; ++j) {
				Edge& e = _edges[idx];
				debug_edge(idx);
				idx = e.next;
			}
		}

		void MeshGen::debug_edge(uint16_t edgeIndex) {
			Edge& e = _edges[edgeIndex];
			LOG << "edge: " << edgeIndex << " v: " << e.vert_index << " next: " << e.next << " prev: " << e.prev << " pos: " << DBG_V3(_vertices[e.vert_index]);
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
			for (int i = 0; i < _faces.size(); ++i) {
				Face& f = _faces[i];
				calculate_normal(&f);
			}
		}

		// ----------------------------------------------
		// check if edge is in clock wise direction
		// ----------------------------------------------
		bool MeshGen::is_clock_wise(uint16_t index) {
			const Edge& e0 = _edges[index];
			const Edge& e1 = _edges[e0.next];
			const Face& f = _faces[e0.face_index];
			v3 c = cross(_vertices[e0.vert_index],_vertices[e1.vert_index]);
			LOG << "==> CW - start: " << DBG_V3(_vertices[e0.vert_index]) << " end: " << DBG_V3(_vertices[e1.vert_index]) << "  NC : " << DBG_V3(c);
			return c.z >= 0.0f;
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
		// extrude face
		// ----------------------------------------------
		uint16_t MeshGen::extrude_face(uint16_t face_index, float factor) {
			const Face& f = _faces[face_index];
			int ei = f.edge;
			v3 p[4];
			v3 n = f.n * factor;
			for (int i = 0; i < 4; ++i) {
				Edge& e = _edges[ei];
				p[i] = _vertices[e.vert_index];
				p[i] += n;
				ei = e.next;
			}
			uint16_t newFace = add_face(p);
			const Face& nf = _faces[newFace];
			LOG << "new face: " << newFace;
			ei = f.edge;
			int nei = nf.edge;
			for (int i = 0; i < 4; ++i) {
				Edge& e = _edges[ei];
				Edge& ne = _edges[nei];
				//h_combine(ne.index, e.index);
				ei = e.next;
				nei = ne.next;
			}
			return newFace;
		}

		// ----------------------------------------------
		// combine two edges
		// ----------------------------------------------
		uint16_t MeshGen::combine(uint16_t first, uint16_t second) {
			/*
			const Edge& f = _edges[first];
			const Edge& s = _edges[second];
			float d = dot(_vertices[f.end] - _vertices[f.start], _vertices[s.end] - _vertices[s.start]);
			LOG << "DOT: " << d;
			return add_face(_vertices[f.start], _vertices[s.end], _vertices[s.start], _vertices[f.end]);
			*/
			return 0;
		}

		uint16_t MeshGen::v_combine(uint16_t first, uint16_t second) {
			/*
			const Edge& f = _edges[first];
			const Edge& s = _edges[second];
			float d = dot(_vertices[f.end] - _vertices[f.start], _vertices[s.end] - _vertices[s.start]);
			LOG << "V-DOT: " << d;
			return add_face(_vertices[f.start], _vertices[s.end], _vertices[s.start], _vertices[f.end]);
			*/
			return 0;
		}

		uint16_t MeshGen::h_combine(uint16_t first, uint16_t second) {
			/*
			const Edge& f = _edges[first];
			const Edge& s = _edges[second];
			debug_edge(first);
			debug_edge(second);
			float d = dot(_vertices[f.end] - _vertices[f.start], _vertices[s.end] - _vertices[s.start]);
			LOG << "H-DOT: " << d;
			if (d < 0.0f) {
				return add_face(_vertices[f.end], _vertices[f.start], _vertices[s.end], _vertices[s.start]);
			}
			else {
				return add_face(_vertices[f.end], _vertices[f.start], _vertices[s.start], _vertices[s.end]);
			}
			*/
			return 0;
		}

		uint16_t MeshGen::add_face(const v3& p0, const v3& p1, const v3& p2, const v3& p3) {
			v3 p[] = { p0, p1, p2, p3 };
			return add_face(p);
		}

		bool MeshGen::verify(uint16_t face_index) {
			v3 p[4];
			const Face& f = _faces[face_index];
			int ei = f.edge;
			for (int i = 0; i < 4; ++i) {
				Edge& e0 = _edges[ei];
				Edge& e1 = _edges[e0.next];
				p[i] = _vertices[e1.vert_index] - _vertices[e0.vert_index];
				ei = e0.next;
			}
			int cnt = 0;
			for (int i = 0; i < 4; ++i) {
				v3 a = p[i];
				v3 b = p[(i + 1) % 4];
				v3 c = cross(a, b);
				if (c.z > 0.0f) {
					++cnt;
				}
				else if ( c.z < 0.0f ) {
					--cnt;
				}
			}
			LOG << "CNT: " << cnt;
			return cnt == 0;
		}

		// http://debian.fmi.uni-sofia.bg/~sergei/cgsr/docs/clockwise.htm
		bool MeshGen::verify(v3* positions) {
			int cnt = 0;
			for (int i = 0; i < 4; ++i) {
				v3 a = positions[i];
				v3 b = positions[(i + 1) % 4];
				v3 c = cross(a, b);
				if (c.z > 0.0f) {
					++cnt;
				}
				else if (c.z < 0.0f) {
					--cnt;
				}
			}
			LOG << "CNT: " << cnt;
			return cnt == 0;
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
			f.edge = idx;
			f.color = Color::WHITE;
			calculate_normal(&f);
			uint16_t fi = _faces.size();
			_faces.push_back(f);
			return fi;
		}

		int MeshGen::find_opposite_edge(uint16_t edgeIndex) {
			Edge& e = _edges[edgeIndex];
			Face& f = _faces[e.face_index];
			uint16_t n = e.next;
			Edge& ne = _edges[n];
			return ne.next;
		}

		int MeshGen::find_opposite(uint16_t edgeIndex) {
			const Edge& e = _edges[edgeIndex];
			const Edge& next = _edges[e.next];
			return find_edge(_vertices[next.vert_index], _vertices[e.vert_index]);
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

		uint16_t MeshGen::make_face(uint16_t* edges) {
			v3 p[4];
			for (int i = 0; i < 4; ++i) {
				p[i] = _vertices[_edges[edges[i]].vert_index];
			}
			return add_face(p);
		}

		void MeshGen::connect(uint16_t f, uint16_t s, uint16_t t) {
			Edge& fe = _edges[f];
			Edge& se = _edges[s];
			Edge& te = _edges[t];
			fe.next = s;
			se.prev = f;
			se.next = t;
			te.prev = s;
		}

		// ----------------------------------------------
		// split edge
		// ----------------------------------------------
		uint16_t MeshGen::split_edge(uint16_t edgeIndex, float factor) {
			const Edge& e = _edges[edgeIndex];
			const Edge& n = _edges[e.next];
			const Edge& nn = _edges[n.next];
			const Edge& nnn = _edges[nn.next];
			Edge& prev = _edges[e.prev];
			v3 delta = (_vertices[n.vert_index] - _vertices[e.vert_index]) * factor;
			v3 np1 = _vertices[e.vert_index] + delta;
			uint16_t na1 = add_edge(np1);
			v3 d2 = (_vertices[nn.vert_index] - _vertices[nnn.vert_index]) * factor;
			v3 np2 = _vertices[nnn.vert_index] + d2;
			uint16_t na2 = add_edge(np2);
			connect(edgeIndex, na1, na2);
			connect(na1, na2, nn.next);
			_edges[nn.next].next = edgeIndex;

			uint16_t na3 = add_edge(np1);
			Face f;
			f.edge = na3;
			int fi = _faces.size();
			_faces.push_back(f);
			uint16_t na4 = add_edge(_vertices[nn.vert_index]);// +(_vertices[nn.vert_index] - _vertices[nnn.vert_index]) * (1.0f - factor));
			uint16_t na5 = add_edge(np2);
			_edges[na3].face_index = fi;
			_edges[na4].face_index = fi;
			_edges[na5].face_index = fi;
			_edges[e.next].face_index = fi;

			connect(na3, e.next, na4);
			connect(e.next, na4, na5);
			connect(na5, na3, e.next);
			
			return 0;
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
				int num = find_edges(p[i], connections, 16);
				for (int j = 0; j < num; ++j) {
					const Edge& curr = _edges[connections[j]];
					if (connections[j] != edgeIndex && connections[j] != e0.next) {
						_vertices[curr.vert_index] += position;
					}
				}
			}
			_vertices[e0.vert_index] += position;
			_vertices[e1.vert_index] += position;
		}

		// ----------------------------------------------
		// move face
		// ----------------------------------------------
		void MeshGen::move_face(uint16_t face_index, const v3& position) {
			Face& f = _faces[face_index];
			int ei = f.edge;
			for (int i = 0; i < 4; ++i) {
				const Edge& e = _edges[ei];
				move_edge(ei, position);
				ei = e.next;
			}
			calculate_normal(&f);
		}

		// ----------------------------------------------
		// scale face
		// ----------------------------------------------
		void MeshGen::scale_face(uint16_t face_index, float scale) {
			const Face& f = _faces[face_index];
			uint16_t connections[16];
			int ei = f.edge;
			float dir = 1.0f;
			if (scale < 1.0f) {
				dir = -1.0f;
			}
			v3 center = get_center(face_index);
			for (int i = 0; i < 4; ++i) {
				Edge& e0 = _edges[ei];
				v3 diff = _vertices[e0.vert_index] - center;
				v3 c = normalize(diff);
				float l = length(diff);
				c *= (l * scale);
				c *= dir;
				int num = find_edges(_vertices[e0.vert_index], connections, 16);
				for (int j = 0; j < num; ++j) {
					const Edge& curr = _edges[connections[j]];
					if (connections[j] != ei) {
						_vertices[curr.vert_index] += c;
					}
				}
				_vertices[e0.vert_index] += c;
				ei = e0.next;
			}
		}

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
		// move vertex
		// ----------------------------------------------
		void MeshGen::move_vertex(uint16_t edgeIndex, bool start, const v3& position) {
			Edge& e = _edges[edgeIndex];
			Face& f = _faces[e.face_index];
			_vertices[e.vert_index] += position;
			calculate_normal(&f);
		}

		// ----------------------------------------------
		// set color
		// ----------------------------------------------
		void MeshGen::set_color(uint16_t face_index, const Color& color) {
			if (face_index < _faces.size()) {
				Face& f = _faces[face_index];
				f.color = color;
			}
		}

		void MeshGen::add_face(const v3& position, const v2& size, const v3& normal) {

		}

		// ----------------------------------------------
		// convert geometry into mesh
		// ----------------------------------------------
		void MeshGen::build(Mesh* mesh) {
			for (int i = 0; i < _faces.size(); ++i) {
				const Face& f = _faces[i];
				const Edge& e = _edges[f.edge];
				int idx = f.edge;
				for (int j = 0; j < 4; ++j) {
					Edge& e = _edges[idx];
					mesh->add(_vertices[e.vert_index], f.n, e.uv, f.color);
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
			v3 p0[] = { 
				v3(-half_size.x,half_size.y,-half_size.z),
				v3(half_size.x,half_size.y,-half_size.z),
				v3(half_size.x,-half_size.y,-half_size.z),
				v3(-half_size.x,-half_size.y,-half_size.z)
			};
			for (int i = 0; i < 4; ++i) {
				p0[i] += position;
			}
			my_faces[0] = add_face(p0);
			const Face& f = _faces[my_faces[0]];
			// left
			my_faces[1] = extrude_edge(f.edge + 1, v3(0.0f, 0.0f, size.z));
			// back
			my_faces[2] = extrude_edge(f.edge + 5, v3(-size.x, 0, 0));
			// right
			my_faces[3] = extrude_edge(f.edge + 9, v3(0, 0, -size.z));
			// top
			my_faces[4] = extrude_edge(f.edge + 8, v3(0, 0, -size.z));
			// bottom
			my_faces[5] = extrude_edge(f.edge + 2, v3(0, 0, size.z));
			if (faces != 0) {
				for (int i = 0; i < 6; ++i) {
					faces[i] = my_faces[i];
				}
			}
			return my_faces[0];
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
				faces[i+1] = extrude_edge(f.edge + indices[i], e[i]);
			}
			return faces[0];
		}

		void MeshGen::clear() {
			_vertices.clear();
			_edges.clear();
			_faces.clear();
		}

		struct OpCode {

			char name[32];
			float values[16];
			int count;

			const int get_int(int index) const {
				return static_cast<int>(values[index]);
			}

			const float get_float(int index) const {
				return values[index];
			}

			const v3 get_v3(int index) const {
				return v3(values[index], values[index + 1], values[index + 2]);
			}

			const v2 get_v2(int index) const {
				return v2(values[index], values[index + 1]);
			}

			const Color get_color(int index) const {
				return Color(values[index] / 255.0f, values[index + 1] / 255.0f, values[index + 2] / 255.0f, values[index + 3] / 255.0f);
			}

			const Rect get_rect(int index) const {
				return Rect(values[index], values[index + 1], values[index + 2], values[index + 3]);
			}
		};

		void MeshGen::parse(const char* fileName) {
			clear();
			Array<OpCode> opcodes;
			char buffer[256];
			sprintf_s(buffer, 256, "content\\meshes\\%s", fileName);
			int size = -1;
			const char* txt = repository::load(buffer, &size);
			if (size != -1) {
				Tokenizer t;
				t.parse(txt);
				LOG << "tokens: " << t.size();
				int cnt = 0;
				while (cnt < t.size()) {
					Token& tk = t.get(cnt);
					if (tk.type == Token::NAME) {
						OpCode oc;
						strncpy(oc.name, txt + tk.index, tk.size);
						oc.name[tk.size] = '\0';
						oc.count = 0;
						++cnt;
						tk = t.get(cnt);
						while ((tk.type == Token::NUMBER) || (tk.type == Token::DELIMITER)) {
							tk = t.get(cnt);
							if (tk.type == Token::NUMBER) {
								oc.values[oc.count++] = tk.value;
							}
							++cnt;
						}
						--cnt;
						opcodes.push_back(oc);
					}
					else {
						++cnt;
					}
				}
				LOG << "opcodes: " << opcodes.size();
				for (int i = 0; i < opcodes.size(); ++i) {
					const OpCode& oc = opcodes[i];
					LOG << "oc: " << oc.name << " values: " << oc.count;
					if (strcmp(oc.name, "add_face") == 0) {
						v3 p[4];
						p[0] = oc.get_v3(0);
						p[1] = oc.get_v3(3);
						p[2] = oc.get_v3(6);
						p[3] = oc.get_v3(9);
						add_face(p);
					}
					else if (strcmp(oc.name, "add_cube") == 0) {
						// add_cube 0,0,0 1.0,1.0,0.1
						v3 pos = oc.get_v3(0);
						v3 size = oc.get_v3(3);
						add_cube(pos, size);
					}
					else if (strcmp(oc.name, "set_color") == 0) {
						int idx = oc.get_int(0);
						Color c = oc.get_color(1);
						set_color(idx, c);
					}
					else if (strcmp(oc.name, "extrude_edge") == 0) {
						// extrude_edge 1 0,0,2
						int idx = oc.get_int(0);
						v3 c = oc.get_v3(1);
						extrude_edge(idx, c);
					}
					else if (strcmp(oc.name, "texture_face") == 0) {
						// texture_face 5 260,650 324,650 324,714 260,714
						int idx = oc.get_int(0);
						v2 uv[4];
						for (int i = 0; i < 4; ++i) {
							uv[i] = oc.get_v2(i * 2 + 1);
						}
						texture_face(idx, math::buildTexture(uv));
					}
					else if (strcmp(oc.name, "texture_face_rect") == 0) {
						// texture_face 5 260,650 324,650 324,714 260,714
						int idx = oc.get_int(0);
						Rect r = oc.get_rect(1);
						texture_face(idx, math::buildTexture(r));
					}
					else if (strcmp(oc.name, "split_edge") == 0) {
						// extrude_edge 1 0,0,2
						int idx = oc.get_int(0);
						float factor = oc.get_float(1);
						split_edge(idx, factor);
					}
					else if (strcmp(oc.name, "move_edge") == 0) {
						// move_edge 1 0,0,2
						int idx = oc.get_int(0);
						v3 c = oc.get_v3(1);
						move_edge(idx, c);
					}
					else if (strcmp(oc.name, "move_face") == 0) {
						// move_face 1 0,0,2
						int idx = oc.get_int(0);
						v3 c = oc.get_v3(1);
						move_face(idx, c);
					}

				}
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
		// find connections
		// ----------------------------------------------
		int MeshGen::find_connections(uint16_t face_index, EdgeConnection* connections, int max) {
			const Face& f = _faces[face_index];
			int ei = f.edge;
			int cnt = 0;
			for (int i = 0; i < 4; ++i) {
				Edge& e = _edges[ei];
				Edge& n = _edges[e.next];
				int idx = find_edge(_vertices[n.vert_index], _vertices[e.vert_index]);
				if (idx != -1 && cnt < max) {
					EdgeConnection& c = connections[cnt++];
					c.first = ei;
					c.second = idx;
				}
				ei = e.next;
			}
			return cnt;
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
				p[0] = v3(radius * cos(next_angle), radius * sin(next_angle), 0.0f);
				p[1] = v3(outer_radius * cos(next_angle), outer_radius * sin(next_angle), 0.0f);
				p[2] = v3(outer_radius * cos(angle), outer_radius * sin(angle), 0.0f);
				p[3] = v3(radius * cos(angle), radius * sin(angle), 0.0f);
				add_face(p);
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
			float yp = sy;
			for (int y = 0; y < stepsY; ++y) {
				float xp = sx;
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

		// ----------------------------------------------
		// create cube ring
		// ----------------------------------------------
		void MeshGen::create_cube_ring(float radius, float width, uint16_t segments) {
			float angleStep = TWO_PI / static_cast<float>(segments);
			v3 p[4];
			float angle = 0.0f;
			float next_angle = angleStep;
			float outer_radius = radius + width;
			float half_size = width * 0.5f;
			for (int i = 0; i < segments; ++i) {
				p[0] = v3(radius * cos(next_angle), radius * sin(next_angle), -half_size);
				p[1] = v3(outer_radius * cos(next_angle), outer_radius * sin(next_angle), -half_size);
				p[2] = v3(outer_radius * cos(angle), outer_radius * sin(angle), -half_size);
				p[3] = v3(radius * cos(angle), radius * sin(angle), -half_size);
				add_face(p);
				angle += angleStep;
				next_angle += angleStep;
			}
			angle = 0.0f;
			next_angle = angleStep;
			for (int i = 0; i < segments; ++i) {
				p[0] = v3(outer_radius * cos(next_angle), outer_radius * sin(next_angle), half_size);
				p[1] = v3(radius * cos(next_angle), radius * sin(next_angle), half_size);	
				p[2] = v3(radius * cos(angle), radius * sin(angle), half_size);
				p[3] = v3(outer_radius * cos(angle), outer_radius * sin(angle), half_size);				
				add_face(p);
				//uint16_t ne = get_edge_index(nf, 1);
				//extrude_edge(ne, v3(0, 0, width));
				angle += angleStep;
				next_angle += angleStep;
			}
			combine(1, 17);
		}

	namespace geometrics {

		void createCube(Mesh* mesh, const Rect& textureRect, const v3& center, const v3& size,const v3& rotation) {
			float px = 0.5f * size.x;
			float py = 0.5f * size.y;
			float pz = 0.5f * size.z;
			mat3 RX = matrix::mat3RotationX(rotation.x);
			mat3 RY = matrix::mat3RotationY(rotation.y);
			mat3 RZ = matrix::mat3RotationZ(rotation.z);
			ds::Texture t = math::buildTexture(textureRect);
			v3 points[] = {
				v3(-px, py, pz),v3(px, py, pz),v3(px, py, -pz),v3(-px, py, -pz),
				v3(-px, -py, pz),v3(px, -py, pz),v3(px, -py, -pz),v3(-px, -py, -pz)
			};
			// front right top left back bottom
			v3 norms[] = { v3(0, 0, -1), v3(1,0,0), v3(0,1,0), v3(-1,0,0), v3(0,0,1) , v3(0,-1,0)};
			int indices[] = { 3, 2, 6, 7, 2, 1, 5, 6, 0, 1, 2, 3, 0, 3, 7, 4, 1, 0, 4, 5, 7, 6, 5, 4 };
			for (int i = 0; i < 6; ++i) {
				for (int j = 0; j < 4; ++j) {
					v3 p = points[indices[i * 4 + j]];
					p = RZ * RY * RX * p + center;
					v3 nn = RZ * RY * RX * norms[i];
					mesh->add(p, nn, t.uv[i]);
				}
			}
		}

		void createCube(Mesh* mesh, Rect* textureRects, const v3& center, const v3& size, const v3& rotation) {
			float px = 0.5f * size.x;
			float py = 0.5f * size.y;
			float pz = 0.5f * size.z;
			mat3 RX = matrix::mat3RotationX(rotation.x);
			mat3 RY = matrix::mat3RotationY(rotation.y);
			mat3 RZ = matrix::mat3RotationZ(rotation.z);
			ds::Texture t[6];
			for (int i = 0; i < 6; ++i) {
				t[i] = math::buildTexture(textureRects[i]);
			}
			v3 points[] = {
				v3(-px, py, pz), v3(px, py, pz), v3(px, py, -pz), v3(-px, py, -pz),
				v3(-px, -py, pz), v3(px, -py, pz), v3(px, -py, -pz), v3(-px, -py, -pz)
			};
			// front right top left back bottom
			v3 norms[] = { v3(0, 0, -1), v3(1, 0, 0), v3(0, 1, 0), v3(-1, 0, 0), v3(0, 0, 1), v3(0, -1, 0) };
			int indices[] = { 3, 2, 6, 7, 2, 1, 5, 6, 0, 1, 2, 3, 0, 3, 7, 4, 1, 0, 4, 5, 7, 6, 5, 4 };
			for (int i = 0; i < 6; ++i) {
				for (int j = 0; j < 4; ++j) {
					v3 p = points[indices[i * 4 + j]];
					p = RZ * RY * RX * p + center;
					v3 nn = RZ * RY * RX * norms[i];
					mesh->add(p, nn, t[i].uv[j]);
				}
			}
		}

		// ---------------------------------------------------------------
		// create XZ grid
		// ---------------------------------------------------------------
		void createGrid(Mesh* mesh, float cellSize, int countX, int countY, const Rect& textureRect, const v3& offset,const Color& color)  {
			v3 center = offset;
			center.z = offset.z + cellSize * 0.5f;
			float px = 0.5f * cellSize;
			float py = 0.0f;
			float pz = 0.5f * cellSize;
			ds::Texture t = math::buildTexture(textureRect);
			v3 norm = v3(0, 1, 0);
			v3 points[] = {
				v3(-px, offset.y,  pz),
				v3( px, offset.y,  pz),
				v3( px, offset.y, -pz),
				v3(-px, offset.y, -pz)
			};
			for (int y = 0; y < countY; ++y) {
				center.x = offset.x + cellSize * 0.5f;
				for (int x = 0; x < countX; ++x) {
					for (int i = 0; i < 4; ++i) {
						v3 n = points[i] + center;
						mesh->add(n, norm, t.uv[i], color);
					}
					center.x += cellSize;
				}
				center.z += cellSize;
			}
		}

		// ---------------------------------------------------------------
		// create XZ plane
		// ---------------------------------------------------------------
		void createPlane(Mesh* mesh, const v3& position, const Rect& textureRect, const v2& size, float rotation, const Color& color)  {
			v3 center = position;
			mat3 R = matrix::mat3RotationY(rotation);
			float px = 0.5f * size.x;
			float py = 0.0f;
			float pz = 0.5f * size.y;
			ds::Texture t = math::buildTexture(textureRect);
			v3 points[] = {
				v3( -px, 0.0f, pz),
				v3(px, 0.0f, pz),
				v3(px, 0.0f, -pz),
				v3(-px, 0.0f, -pz)
			};
			for (int i = 0; i < 4; ++i) {
				v3 n = R * points[i];
				n += center;
				mesh->add(n, v3(0, 1, 0), t.uv[i], color);
			}
		}

		// ---------------------------------------------------------------
		// create XZ plane
		// ---------------------------------------------------------------
		void createXYPlane(Mesh* mesh, const v3& position, const Rect& textureRect, const v2& size, float rotation, const Color& color) {
			v3 center = position;
			mat3 R = matrix::mat3RotationY(rotation);
			float px = 0.5f * size.x;
			float py = 0.5f * size.y;
			float pz = position.z;
			ds::Texture t = math::buildTexture(textureRect);
			v3 points[] = {
				v3(-px,  py, pz),
				v3( px,  py, pz),
				v3( px, -py, pz),
				v3(-px, -py, pz)
			};
			for (int i = 0; i < 4; ++i) {
				v3 n = R * points[i];
				n += center;
				mesh->add(n, v3(0, 0, -1), t.uv[i], color);
			}
		}

	}
}