#include "Huffmann.h"
#include <string>
#include "..\utils\Log.h"
#include "..\lib\collection_types.h"

namespace ds {

	namespace huffmann {

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

		int prepare_tree(Node* nodes, int* frequencies) {
			int num = 0;
			for (int i = 0; i < 256; ++i) {
				if (frequencies[i] > 0) {
					LOG << i << " " << (char)i << " = " << frequencies[i];
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

		void reverse(char* src, char* dest, int num) {
			int h = num / 2;
			dest[h] = src[h];
			for (int i = 0; i < h; ++i) {
				char t = src[i];
				dest[i] = src[num - i - 1];
				dest[num - i - 1] = t;
			}

		}

		int build_tree(Node* nodes,int num) {
			int count = num;
			int total = num;
			while (count > 1) {
				int f = find_min(nodes, total, -1);
				int s = find_min(nodes, total, f);
				if (f != -1 && s != -1) {
					LOG << "combining " << f << " and " << s;
					Node& n = nodes[total++];
					n.left = f;
					n.right = s;
					n.used = false;
					n.letter = 0;
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

		int find_node(Node* nodes, int num, char c) {
			for (int i = 0; i < num; ++i) {
				if (nodes[i].letter == c) {
					return i;
				}
			}
			return -1;
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

			for (int i = 0; i < total; ++i) {
				const Node& n = nodes[i];
				char l = ' ';
				char r = ' ';
				char letter = n.letter;
				if (letter == 0) {
					letter = ' ';
				}
				LOG << i << " : freq: " << n.frequency << " parent: " << n.parent << " letter: '" << letter << "' left: " << n.left << " right: " << n.right;
			}
			const Node& root = nodes[total - 1];
			char b[8];			
			for (int i = 0; i < total; ++i) {
				Node& n = nodes[i];
				if (n.letter > 0) {
					for (int j = 0; j < 8; ++j) {
						b[j] = 0;
						n.d[j] = 0;
					}
					//int nd = get_bits(nodes, i, total - 1, 1, &n.bits);
					int nd = get_bits(nodes, i, total - 1, 0, b);
					reverse(b, n.d, nd);
					LOG << i << " letter: " << n.letter << " nd: " << nd << " org: " << b << " bits: " << n.d;
				}
			}

			std::string str;
			for (int i = 0; i < len; ++i) {
				int idx = find_node(nodes, num, text[i]);
				if (idx != -1) {
					const Node& n = nodes[idx];
					str += n.d;
				}
			}
			LOG << "stream: " << str;
		}
	}

}