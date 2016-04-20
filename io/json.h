#pragma once
#include <string.h>
#include "..\utils\StringUtils.h"
#include "..\utils\Color.h"
#include "..\memory\DefaultAllocator.h"
#include "..\lib\BlockArray.h"
#include "..\math\FloatArray.h"

namespace ds {

	struct Token {

		enum TokenType { EMPTY, NUMBER, NAME, DELIMITER, SEPARATOR, OPEN_BRACES, CLOSE_BRACES, STRING, ASSIGN, SEMICOLON,OPEN_BRACKET,CLOSE_BRACKET };

		Token() {}
		Token(TokenType type) : type(type) {}
		Token(TokenType type, float v) : type(type), value(v) {}
		Token(TokenType type, int i, int s) : type(type), index(i), size(s) {}

		TokenType type;
		float value;
		int index;
		int size;
	};

	class Tokenizer {

	public:
		Tokenizer() : UNKNOWN_TOKEN(Token::EMPTY) {}
		~Tokenizer() {}
		void parse(const char* text);
		int size() const {
			return _tokens.size();
		}
		const Token& get(uint32_t index) const {
			if (index < _tokens.size()) {
				return _tokens[index];
			}
			else {
				return UNKNOWN_TOKEN;
			}
		}

		Token& get(uint32_t index) {
			if (index < _tokens.size()) {
				return _tokens[index];
			}
			else {
				return UNKNOWN_TOKEN;
			}
		}

		const char* name(int index) const;

	private:
		Array<Token> _tokens;
		Token UNKNOWN_TOKEN;
	};

	class JSONReader {

	public:
		JSONReader();
		~JSONReader();
		bool parse(const char* fileName);
		void save_binary(const char* fileName);
		bool load_binary(const char* fileName);
		int get_categories(int* result, int max, int parent = -1) const;
		int find_category(const char* name, int parent = -1) const;
		bool matches(int category_id, const char* name) const;
		bool contains_property(int category_id, const char* name) const;
		bool get_int(int category_id, const char* name, int* ret) const;
		bool get(int category_id, const char* name, bool* ret) const;
		bool get_uint(int category_id, const char* name, uint32_t* ret) const;
		bool get(int category_id, const char* name, uint16_t* ret) const;
		bool get(int category_id, const char* name, uint32_t* ret) const;
		bool get_float(int category_id, const char* name, float* ret) const;
		bool get_vec2(int category_id, const char* name, v2* ret) const;
		bool get_vec3(int category_id, const char* name, v3* ret) const;
		bool get_color(int category_id, const char* name, Color* ret) const;
		bool get(int category_id, const char* name, Rect* ret) const;
		bool get_color_path(int category_id, const char* name, ds::ColorPath* path) const;
		bool get_vec2_path(int category_id, const char* name, ds::Vector2fPath* path) const;
		bool get_float_path(int category_id, const char* name, ds::FloatArray* path) const;
		int get_array(int category_id, const char* name, float* values, int max) const;
		int get_array(int category_id, const char* name, int* values, int max) const;
		const char* get_string(int category_id, const char* name) const;
		const char* get_category_name(int category_id) const;
	private:
		int get_index(int category_id, const char* name) const;
		void allocCategoryBuffer(int size);
		int add_category(const char* name, int parent);
		void alloc(int elements);
		int create_property(const char* name, int category);
		void add(int pIndex, float value);
		void add(int pIndex, const char* c, int len);
		void add(int pIndex, char c);
		float get(int index) const;
		const char* get_char(int index) const;
		char* _text;

		BlockArray _category_buffer;
		unsigned int* _hashes;
		int* _parents;
		int* _indices;

		BlockArray _data_buffer;
		unsigned int* _data_keys;
		int* _data_categories;
		int* _data_indices;
		int* _data_sizes;

		CharBuffer _name_buffer;
		CharBuffer _values;

	};

	class JSONWriter {

	public:
		JSONWriter();
		~JSONWriter();
		bool open(const char* fileName);
		void startCategory(const char* name);
		void write(const char* name, int value);
		void write(const char* name, uint32_t value);
		void write(const char* name, float value);
		void write(const char* name, bool value);
		void write(const char* name, const v2& value);
		void write(const char* name, const v3& value);
		void write(const char* name, const Color& value);
		void write(const char* name, const Rect& value);
		void write(const char* name, const char* value);
		void write(const char* name, const ds::ColorPath& path);
		void write(const char* name, const ds::Vector2fPath& path);
		void write(const char* name, const ds::FloatArray& path);
		void write(const char* name, const int* values,int count);
		void endCategory();
	private:
		FILE* f;
		bool _started;
		void writeIdent();
		void writeLineIdent();
		int _ident;
		char _temp[128];
		bool _open;
		int _items;
	};

}