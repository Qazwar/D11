#pragma once

namespace ds {

	struct BaseScore {

		BaseScore() {}
		virtual int compare(const BaseScore& other) = 0;

	};

	template<class T, int SIZE = 10>
	class HighscoreService {

	public:
		HighscoreService() : _count(0) {
			_scores = new T[SIZE];
		}
		~HighscoreService() {
			delete[] _scores;
		}
		int add(const T& score) {
			int idx = -1;
			for (int i = 0; i < _count; ++i) {
				if (idx == -1) {
					int state = score.compare(_scores[i]);
					if (state >= 0) {
						idx = i;
					}
				}
			}
			if (idx != -1) {
				// FIXME: move the rest on further down
				_scores[idx] = score;
				++_count;
			}
			return idx;
		}
		bool load() {
			FILE* f = fopen("score.dat", "rb");
			if (f) {
				int count = 0;
				fread(&count, sizeof(int), 1, f);
				for (int i = 0; i < count; ++i) {
					fread(&_scores[i], sizeof(T), 1, f);
				}
				_count = count;
				fclose(f);
				return true;
			}
			return false;
		}
		bool save() {
			FILE* f = fopen("score.dat", "wb");
			if (f) {
				fwrite(&_count, sizeof(int), 1, f);
				for (int i = 0; i < _count; ++i) {
					fwrite(&_scores[i], sizeof(T), 1, f);
				}
				fclose(f);
				return true;
			}
			return false;
		}
		int get(T* list, int max) {
			int cnt = max;
			if (cnt > _count) {
				cnt = _count;
			}
			for (int i = 0; i < cnt; ++i) {
				list[i] = _scores[i];
			}
			return cnt;
		}
	private:
		int _count;
		T* _scores;
	};

}