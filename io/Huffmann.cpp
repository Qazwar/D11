#include "Huffmann.h"
#include <string>
#include "..\utils\Log.h"
#include "..\lib\collection_types.h"

namespace ds {

	namespace huffmann {

		struct Node {
			Node* left;
			Node* right;
			char letter;
			int frequency;
		};

		int find_min(int* frequencies, int num) {
			int c = 1000;
			int idx = -1;
			for (int i = 0; i < num; ++i) {
				if (frequencies[i] >= 0 && frequencies[i] < c && idx == -1) {
					idx = i;
					c = frequencies[i];
				}
			}
			return idx;
		}

		void first(const char* text) {
			int frequencies[256] = { 0 };
			int len = strlen(text);
			for (int i = 0; i < len; ++i) {
				++frequencies[text[i]];
			}
			int num = 0;
			for (int i = 0; i < 256; ++i) {
				if (frequencies[i] > 0) {
					LOG << i << " = " << frequencies[i];
					++num;
				}
			}
			Array<Node*> nodes;
			int idx = 0;
			for (int i = 0; i < 256; ++i) {
				if (frequencies[i] > 0) {
					Node* n = new Node;
					n->letter = i;
					n->frequency = frequencies[i];
					n->left = 0;
					n->right = 0;
					nodes.push_back(n);
				}
			}
			while (nodes.size() > 1) {

			}
			nodes.destroy_all();
		}
	}

}