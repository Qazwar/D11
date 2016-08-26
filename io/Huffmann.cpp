#include "Huffmann.h"
#include <string>
#include "..\utils\Log.h"
#include "..\lib\collection_types.h"
#include "BinaryFile.h"
#include "..\math\Bitset.h"

namespace ds {

	namespace huffmann {

		// for debuging
		const char EMPTY_LETTER = '$';
		// for real
		//const char EMPTY_LETTER = 0;

		// ---------------------------------------------
		// reverse string
		// ---------------------------------------------
		void reverse(char* src, char* dest, int num) {
			int h = num / 2;
			dest[h] = src[h];
			for (int i = 0; i < h; ++i) {
				char t = src[i];
				dest[i] = src[num - i - 1];
				dest[num - i - 1] = t;
			}

		}

		// ---------------------------------------------
		// internal Node
		// ---------------------------------------------
		struct Node {
			int left;
			int right;
			char letter;
			int frequency;
			bool used;
			int parent;
			int bits;
			char d[8];
		};

		// ---------------------------------------------
		// Bitstream
		// ---------------------------------------------
		struct BitStream {

			int* data;
			int index;
			int total;
			int totalBits;

			BitStream() : data(0), index(0) , total(0) , totalBits(0) {}

			~BitStream() {
				if (data) {
					delete[] data;
				}
			}

			void push(bool flag) {
				if (index + 1 >= totalBits) {
					allocate(totalBits * 2 + 32);
				}
				int offset = index % 32;
				int idx = index / 32;
				if (flag) {
					bit::set(data + idx, offset);
				}
				++index;
			}

			void push(const char* t,int num) {
				for (int i = 0; i < num; ++i) {
					if (t[i] == '1') {
						push(true);
					}
					else {
						push(false);
					}
				}
			}

			int numBits() {
				return index;
			}

			bool isSet(int index) const {
				int offset = index % 32;
				int idx = index / 32;
				return bit::is_set(*(data + idx), offset);
			}

			void allocate(int numBits) {
				if (numBits == 0) {
					numBits = 32;
				}
				if (numBits > totalBits) {
					int size = numBits / 32 + 1;
					if (data != 0) {
						int* tmp = new int[size];
						for (int i = 0; i < size; ++i) {
							if (i < total) {
								tmp[i] = data[i];
							}
							else {
								tmp[i] = 0;
							}
						}
						delete[] data;
						data = tmp;
						totalBits = size * 32;
						total = size;
					}
					else {
						data = new int[size];
						for (int i = 0; i < size; ++i) {
							data[i] = 0;
						}
						totalBits = size * 32;
						total = size;
					}
				}
			}

			void load(const BinaryFile& f) {
				f.read(&index);
				int l = 0;
				f.read(&l);
				allocate(l * 32);
				for (int i = 0; i < l; ++i) {
					f.read(&data[i]);
				}
			}

			void save(const BinaryFile& f) {
				f.write(index);
				f.write(total);
				for (int i = 0; i < total; ++i) {
					f.write(data[i]);
				}
			}

			void debug() {
				char* test = new char[totalBits];
				for (int i = 0; i < totalBits; ++i) {
					test[i] = '\0';
				}
				int idx = 0;
				for (int i = 0; i < index; ++i) {
					if (isSet(i)) {
						test[idx++] = '1';
					}
					else {
						test[idx++] = '0';
					}
				}
				LOG << "bitstream: " << test;
				delete[] test;
			}
		};

		// ---------------------------------------------
		// find node with lowest frequency
		// ---------------------------------------------
		int find_min(Node* nodes, int num, int self) {
			int c = 1000;
			int idx = -1;
			for (int i = 0; i < num; ++i) {
				if (self == -1 || self != i) {
					const Node& n = nodes[i];
					if (!n.used && n.frequency < c) {
						idx = i;
						c = n.frequency;
					}
				}
			}
			return idx;
		}

		// ---------------------------------------------
		// determine bits
		// ---------------------------------------------
		int get_bits(Node* nodes, int current, int root, int index, int* bits) {
			const Node& c = nodes[current];
			const Node& r = nodes[root];
			if (c.parent != -1 && c.parent != root) {
				const Node& p = nodes[c.parent];
				if (p.right == current) {
					*bits |= 1 << index;
				}
				return get_bits(nodes, c.parent, root, index + 1, bits);
			}
			return index;
		}

		// ---------------------------------------------
		// determine bits
		// ---------------------------------------------
		int get_bits(Node* nodes, int current, int root, int index, char* bits) {
			const Node& c = nodes[current];
			const Node& r = nodes[root];
			if (c.parent != -1 && current != root) {
				const Node& p = nodes[c.parent];
				if (p.right == current) {
					bits[index] = '1';
				}
				else {
					bits[index] = '0';
				}
				return get_bits(nodes, c.parent, root, index + 1, bits);
			}
			return index;
		}

		// ---------------------------------------------
		// prepare tree using frequencies
		// ---------------------------------------------
		int prepare_tree(Node* nodes, int* frequencies) {
			int num = 0;
			for (int i = 0; i < 256; ++i) {
				if (frequencies[i] > 0) {
					Node& n = nodes[num++];
					n.letter = i;
					n.frequency = frequencies[i];
					n.left = -1;
					n.right = -1;
					n.parent = -1;
					n.bits = 0;
					n.used = false;
				}
			}
			return num;
		}

		// ---------------------------------------------
		// build final tree
		// ---------------------------------------------
		int build_tree(Node* nodes,int num) {
			int count = num;
			int total = num;
			while (count > 1) {
				int f = find_min(nodes, total, -1);
				int s = find_min(nodes, total, f);
				if (f != -1 && s != -1) {
					Node& n = nodes[total++];
					n.left = f;
					n.right = s;
					n.used = false;
					n.letter = EMPTY_LETTER;
					n.parent = -1;
					n.bits = 0;
					n.frequency = nodes[f].frequency + nodes[s].frequency;
					nodes[f].used = true;
					nodes[s].used = true;
					nodes[f].parent = total - 1;
					nodes[s].parent = total - 1;
				}
				--count;
			}
			return total;
		}

		// ---------------------------------------------
		// find node by character
		// ---------------------------------------------
		int find_node(Node* nodes, int num, char c) {
			for (int i = 0; i < num; ++i) {
				if (nodes[i].letter == c) {
					return i;
				}
			}
			return -1;
		}

		// ---------------------------------------------
		// log tree
		// ---------------------------------------------
		void log_tree(Node* nodes, int id, int ident) {
			const Node& n = nodes[id];
			std::string str = "";
			for (int i = 0; i < ident; ++i) {
				str += " ";
			}
			LOG << str << id << " :  " << n.letter << "(" << n.frequency << ")";
			if (n.left != -1) {
				log_tree(nodes, n.left, ident + 2);
			}
			if (n.right != -1) {
				log_tree(nodes, n.right, ident + 2);
			}
		}

		// https://www.programmingalgorithms.com/algorithm/huffman-compress?lang=C%2B%2B
		void first(const char* text) {
			int frequencies[256] = { 0 };
			int len = strlen(text);
			for (int i = 0; i < len; ++i) {
				++frequencies[text[i]];
			}
			Node nodes[512];
			int num = prepare_tree(nodes, frequencies);
			LOG << "symbols: " << num;
			
			int total = build_tree(nodes,num);
			LOG << "tree: " << total;
	
			const Node& root = nodes[total - 1];
			log_tree(nodes, total - 1, 0);

			BitStream stream;
			char b[8];			
			for (int i = 0; i < total; ++i) {
				Node& n = nodes[i];
				if (n.letter != EMPTY_LETTER) {
					for (int j = 0; j < 8; ++j) {
						b[j] = 0;
						n.d[j] = 0;
					}
					int nd = get_bits(nodes, i, total - 1, 0, b);
					reverse(b, n.d, nd);
				}
			}

			for (int i = 0; i < len; ++i) {
				int idx = find_node(nodes, num, text[i]);
				if (idx != -1) {
					const Node& n = nodes[idx];
					stream.push(n.d, strlen(n.d));
				}
			}
			stream.debug();

			BinaryFile f;
			if (f.open("test.huf", FileMode::WRITE)) {
				f.write(num);
				for (int i = 0; i < num; ++i) {
					const Node& n = nodes[i];
					f.write(n.letter);
					f.write(n.frequency);
				}
				f.write(len);
				stream.save(f);
			}
		}

		char decode(Node* nodes,int nidx,int bitIndex,BitStream* stream) {
			// FIXME: send over root
			const Node& p = nodes[nidx];
			bool found = false;
			char c = 0;
			while (!found) {
				// mive right
				if (stream->isSet(bitIndex)) {

				}
				else {

				}
				found = true;
			}
			return c;
		}

		void decompress(const char* file) {
			Node nodes[512];
			BinaryFile f;
			int num = 0;
			int l = 0;
			BitStream stream;
			if (f.open(file, FileMode::READ)) {				
				f.read(&num);
				for (int i = 0; i < num; ++i) {
					Node& n = nodes[i];
					n.left = -1;
					n.right = -1;
					n.parent = -1;
					n.bits = 0;
					n.used = false;
					f.read(&n.letter);
					f.read(&n.frequency);
				}
				f.read(&l);		
				stream.load(f);
			}
			LOG << "stream size: " << stream.index;
			stream.debug();

			int total = build_tree(nodes, num);
			LOG << "tree: " << total;

			const Node& root = nodes[total - 1];
			//log_node(nodes, total - 1, 0);
			char* txt = new char[l];
			char test[1024] = { 0 };
			int idx = 0;
			for (int i = 0; i < l; ++i) {
				test[l] = decode(nodes, total - 1, 0,&stream);
			}

			LOG << "decompress: " << test;
		}
	}

}