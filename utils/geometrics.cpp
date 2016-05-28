#include "geometrics.h"
#include "..\math\math.h"
#include "..\utils\Log.h"
#include "..\io\json.h"
#include "..\io\FileRepository.h"

namespace ds {

	static const v3 CUBE_VERTICES[] = {
		v3(-0.5f,0.5f,-0.5f),
		v3(0.5f,0.5f,-0.5f),
		v3(0.5f,-0.5f,-0.5f),
		v3(-0.5f,-0.5f,-0.5f)
	};

	MeshGen::MeshGen() {}

	MeshGen::~MeshGen() {}

		int MeshGen::find_vertex(const v3& pos) {
			for (int i = 0; i < _vertices.size(); ++i) {
				const v3& v = _vertices[i];
				if (v.x == pos.x && v.y == pos.y && v.z == pos.z) {
					return i;
				}
			}
			return -1;
		}

		int MeshGen::find_edge(const v3& start, const v3& end) {
			//LOG << "find_edge: " << DBG_V3(start) << " " << DBG_V3(end);
			for (int i = 0; i < _edges.size(); ++i) {
				const Edge& e = _edges[i];
				v3 es = _vertices[e.start];
				v3 ee = _vertices[e.end];
				//LOG << "es: " << DBG_V3(es) << " ee: " << DBG_V3(ee);
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
			for (int i = 0; i < _vertices.size(); ++i) {
				const v3& v = _vertices[i];
				if (v.x == pos.x && v.y == pos.y && v.z == pos.z) {
					return i;
				}
			}
			int ret = _vertices.size();
			_vertices.push_back(pos);
			return ret;
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
				int idx = e.index;
				for (int j = 0; j < 4; ++j) {
					Edge& e = _edges[idx];
					LOG << "edge: " << e.index << " start: " << e.start << " end: " << e.end << " next: " << e.next << " prev: " << e.prev << " start_pos: " << DBG_V3(_vertices[e.start]) << " end_pos: " << DBG_V3(_vertices[e.end]);
					idx = e.next;
				}
			}
		}

		// ----------------------------------------------
		// calculate normal
		// ----------------------------------------------
		void MeshGen::calculate_normal(Face* f) {
			Edge e0 = _edges[f->edge];
			Edge e3 = _edges[e0.prev];
			v3 s = _vertices[e0.start];
			v3 end1 = _vertices[e0.end];
			v3 end2 = _vertices[e3.start];
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
				Edge e0 = _edges[f.edge];
				Edge e3 = _edges[e0.prev];
				v3 s = _vertices[e0.start];
				v3 end1 = _vertices[e0.end];
				v3 end2 = _vertices[e3.start];
				v3 a = end1 - s;
				v3 b = end2 - s;
				f.n = normalize(cross(a, b));
			}
		}

		// ----------------------------------------------
		// check if edge is in clock wise direction
		// ----------------------------------------------
		bool MeshGen::is_clock_wise(uint16_t index) {
			Edge& e = _edges[index];
			float d = dot(cross(_vertices[e.start], _vertices[e.end]), v3(1, 0, 0));
			LOG << "==> CW - start: " << DBG_V3(_vertices[e.start]) << " end: " << DBG_V3(_vertices[e.end]) << " dot: " << d;
			return d == 0.0f;
		}

		// ----------------------------------------------
		// extrude edge
		// ----------------------------------------------
		uint16_t MeshGen::extrude_edge(uint16_t edgeIndex, const v3& pos) {
			Edge& e = _edges[edgeIndex];
			v3 p[4];	
			if (is_clock_wise(edgeIndex)) {
				p[0] = _vertices[e.start];
				p[1] = p[0] + pos;
				p[2] = _vertices[e.end] + pos;
				p[3] = _vertices[e.end];				
			}
			else {
				p[0] = _vertices[e.end];
				p[1] = _vertices[e.start];
				p[2] = p[1] + pos;
				p[3] = p[0] + pos;
			}
			return add_face(p);
		}

		// ----------------------------------------------
		// combine two edges
		// ----------------------------------------------
		uint16_t MeshGen::combine(uint16_t first, uint16_t second) {
			v3 p[4];
			const Edge& f = _edges[first];
			const Edge& s = _edges[second];
			if (is_clock_wise(first)) {
				p[0] = _vertices[f.end];
				p[2] = _vertices[f.start];
			}
			else {
				p[0] = _vertices[f.start];
				p[2] = _vertices[f.end];
			}
			if (is_clock_wise(second)) {
				p[1] = _vertices[s.end];
				p[3] = _vertices[s.start];
			}
			else {
				p[1] = _vertices[s.start];
				p[3] = _vertices[s.end];
			}
			return add_face(p);
		}

		// ----------------------------------------------
		// add face
		// ----------------------------------------------
		uint16_t MeshGen::add_face(v3* positions) {
			Face f;
			int idx = _edges.size();
			int cnt = idx;
			uint16_t fidx = _faces.size();
			for (int i = 0; i < 4; ++i) {
				v3 start = positions[i];
				v3 end;
				if (i < 3) {
					end = positions[i + 1];
				}
				else {
					end = positions[0];
				}
				Edge e;
				e.index = cnt;
				e.next = cnt + 1;
				e.prev = cnt - 1;
				if (i == 0) {
					e.prev = idx + 3;
				}
				if (i == 3) {
					e.next = idx;
				}
				e.start = add_vertex(positions[i]);
				++cnt;
				if (i < 3) {
					e.end = add_vertex(positions[i + 1]);
				}
				else {
					e.end = add_vertex(positions[0]);
				}				
				e.faceIndex = fidx;
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
			Face& f = _faces[e.faceIndex];
			uint16_t n = e.next;
			Edge& ne = _edges[n];
			return ne.next;
		}

		// ----------------------------------------------
		// get edge index
		// ----------------------------------------------
		uint16_t MeshGen::get_edge_index(uint16_t faceIndex, int nr) {
			LOG << "FACE: " << faceIndex;
			const Face& f = _faces[faceIndex];
			Edge& e = _edges[f.edge];
			LOG << "top: " << e.index;
			for (int i = 0; i < nr; ++i) {
				e = _edges[e.next];
			}
			LOG << "final: " << e.index;
			return e.index;
		}

		// ----------------------------------------------
		// texture face
		// ----------------------------------------------
		void MeshGen::texture_face(uint16_t faceIndex, const Texture& t) {
			const Face& f = _faces[faceIndex];
			int idx = f.edge;
			for (int i = 0; i < 4; ++i) {
				Edge& e = _edges[idx];
				e.uv = t.getUV(i);
				idx = e.next;
			}
		}

		uint16_t MeshGen::make_face(uint16_t* edges) {
			v3 p[4];
			for (int i = 0; i < 4; ++i) {
				p[i] = _vertices[_edges[edges[i]].end];
			}
			return add_face(p);
		}

		// ----------------------------------------------
		// split edge
		// ----------------------------------------------
		uint16_t MeshGen::split_edge(uint16_t edgeIndex, float factor) {
			Edge& e = _edges[edgeIndex];
			Face& f = _faces[e.faceIndex];
			Edge& r = _edges[e.next];
			Edge& o = _edges[r.next];
			v3 delta = (_vertices[e.end] - _vertices[e.start]) * factor;
			v3 oldStart = _vertices[r.start];
			v3 oldEnd = _vertices[r.end];
			move_edge(r.index, -delta);
			v3 p[] = { _vertices[r.start], oldStart, oldEnd, _vertices[r.end] };
			uint16_t newFace = add_face(p);
			int idx = find_edge(_vertices[r.end], _vertices[r.start]);
			if (idx != -1) {			   
				Edge& ne = _edges[idx];
				if (ne.faceIndex != newFace) {
					const Edge& top = _edges[_faces[newFace].edge];
					const Edge& right = _edges[top.next];
					const Edge& bottom = _edges[right.next];
					ne.start = bottom.start;
					ne.end = top.end;
					Edge& nne = _edges[ne.next];
					nne.start = top.end;
					Edge& pe = _edges[ne.prev];
					pe.end = bottom.start;
					Face& f = _faces[ne.faceIndex];
					calculate_normal(&f);
				}
			}
			return newFace;
		}

		// ----------------------------------------------
		// move edge
		// ----------------------------------------------
		void MeshGen::move_edge(uint16_t edgeIndex, const v3& position) {
			Edge& e = _edges[edgeIndex];
			Face& f = _faces[e.faceIndex];
			_vertices[e.start] += position;
			_vertices[e.end] += position;
			calculate_normal(&f);
		}

		// ----------------------------------------------
		// move face
		// ----------------------------------------------
		void MeshGen::move_face(uint16_t faceIndex, const v3& position) {
			Face& f = _faces[faceIndex];
			uint16_t idx = f.edge;			
			for (int i = 0; i < 4; ++i) {
				Edge& e = _edges[idx];
				_vertices[e.start] += position;
				_vertices[e.end] += position;
				idx = e.next;
			}
			calculate_normal(&f);
		}

		// ----------------------------------------------
		// move vertex
		// ----------------------------------------------
		void MeshGen::move_vertex(uint16_t edgeIndex, bool start, const v3& position) {
			Edge& e = _edges[edgeIndex];
			Face& f = _faces[e.faceIndex];
			if (start) {
				_vertices[e.start] += position;
			}
			else {
				_vertices[e.end] += position;
			}
			calculate_normal(&f);
		}

		// ----------------------------------------------
		// set color
		// ----------------------------------------------
		void MeshGen::set_color(uint16_t faceIndex, const Color& color) {
			if (faceIndex < _faces.size()) {
				Face& f = _faces[faceIndex];
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
				int idx = e.index;
				for (int j = 0; j < 4; ++j) {
					Edge& e = _edges[idx];
					mesh->add(_vertices[e.start], f.n, e.uv, f.color);
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

		struct OpCode {

			char name[32];
			float values[16];
			int count;

			const int get_int(int index) const {
				return static_cast<int>(values[index]);
			}

			const v3 get_v3(int index) const {
				return v3(values[index], values[index + 1], values[index + 2]);
			}
			const Color get_color(int index) const {
				return Color(values[index] / 255.0f, values[index + 1] / 255.0f, values[index + 2] / 255.0f, values[index + 3] / 255.0f);
			}
		};

		void MeshGen::parse(const char* fileName) {
			Array<OpCode> opcodes;
			char buffer[256];
			sprintf_s(buffer, 256, "content\\objects\\%s", fileName);
			int size = -1;
			const char* txt = repository::load(buffer, &size);
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
					while ((tk.type == Token::NUMBER)||(tk.type==Token::DELIMITER)) {						
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
				else if (strcmp(oc.name, "split_edge") == 0) {
					// extrude_edge 1 0,0,2
					int idx = oc.get_int(0);
					split_edge(idx);
				}
				else if (strcmp(oc.name, "move_edge") == 0) {
					// extrude_edge 1 0,0,2
					int idx = oc.get_int(0);
					v3 c = oc.get_v3(1);
					move_edge(idx, c);
				}
				
			}
			delete txt;
		}

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
					mesh->add(p, nn, t.getUV(j));
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
					mesh->add(p, nn, t[i].getUV(j));
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
						mesh->add(n, norm, t.getUV(i), color);
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
				mesh->add(n, v3(0, 1, 0), t.getUV(i), color);
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
				mesh->add(n, v3(0, 0, -1), t.getUV(i), color);
			}
		}

	}
}