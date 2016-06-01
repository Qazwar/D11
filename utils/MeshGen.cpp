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

	namespace gen {

		MeshGen::MeshGen() {}

		MeshGen::~MeshGen() {
			LOG << "====> BYE BYE!!!";
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

		int intersect_triangle(const ds::Ray& ray, const v3& p0, const v3& p1, const v3& p2, double *t, double *u, double *v) {
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
		// add face
		// ----------------------------------------------
		uint16_t MeshGen::add_face(const v3& p0, const v3& p1, const v3& p2, const v3& p3) {
			v3 p[] = { p0, p1, p2, p3 };
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
			f.edge = idx;
			f.color = Color::WHITE;
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
			return add_face(p);
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
			return add_face(p);
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
			return add_face(p);
		}

		// ----------------------------------------------
		// num faces
		// ----------------------------------------------
		int MeshGen::num_faces() const {
			return _faces.size();
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

		void MeshGen::smooth(const IndexList& list, float radius) {
			v3 center = v3(0, 0, 0); // FIXME: get center
			IndexList verts;
			for (int i = 0; i < list.indices.size(); ++i) {
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
			for (int i = 0; i < verts.indices.size(); ++i) {
				center += _vertices[verts.indices[i]];
			}
			center /= static_cast<float>(verts.indices.size());
			LOG << "center: " << DBG_V3(center);
			for (int i = 0; i < verts.indices.size(); ++i) {
				/*
				v3 v = _vertices[verts.indices[i]];// *2.0f / 1.0f - v3(1, 1, 1);
				float x2 = v.x * v.x;
				float y2 = v.y * v.y;
				float z2 = v.z * v.z;
				float t = sqrt(x2 + y2 + z2);
				v3 s;
				s.x = v.x / t;
				s.y = v.y / t;
				s.z = v.z / t;
				*/
				_vertices[verts.indices[i]] = normalize(_vertices[verts.indices[i]]) * radius;


				/*

				v3 d = _vertices[verts.indices[i]] - center;
				v3 dn = normalize(d);
				float l = length(d);
				float factor = radius / l;
				float t = _vertices[verts.indices[i]].x * _vertices[verts.indices[i]].y * _vertices[verts.indices[i]].z;
				float vx = _vertices[verts.indices[i]].x * sqrt(1.0f - t * 0.5f);
				float vy = _vertices[verts.indices[i]].y * sqrt(1.0f - t * 0.5f);
				float vz = _vertices[verts.indices[i]].z * sqrt(1.0f - t * 0.5f);
				//_vertices[verts.indices[i]] *= factor;
				_vertices[verts.indices[i]] = v3(vx,vy,vz);
				*/
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
			return faces[0];
		}

		// ----------------------------------------------
		// get face
		// ----------------------------------------------
		const Face& MeshGen::get_face(uint16_t face_index) const {
			return _faces[face_index];
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
		// clear mesh
		// ----------------------------------------------
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
					else if (strcmp(oc.name, "vsplit_edge") == 0) {
						// extrude_edge 1 0,0,2
						int idx = oc.get_int(0);
						float factor = oc.get_float(1);
						vsplit_edge(idx, factor);
					}
					else if (strcmp(oc.name, "hsplit_edge") == 0) {
						// extrude_edge 1 0,0,2
						int idx = oc.get_int(0);
						float factor = oc.get_float(1);
						hsplit_edge(idx, factor);
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
			for (int i = 0; i < _vertices.size(); ++i) {
				_vertices[i] = world * _vertices[i];
			}
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

	}	
}