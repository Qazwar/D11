#include "json.h"
#include <stdio.h>
#include <string.h>
#include <stack>
#include "..\io\FileRepository.h"
#include "..\utils\Log.h"

namespace ds {

	void Tokenizer::parse(const char* text, bool skipComments) {
		int cnt = 0;
		const char* p = text;
		while (*p != 0) {
			Token token(Token::EMPTY);
			if (string::isDigit(*p)) {
				char *out;
				token = Token(Token::NUMBER, string::strtof(p, &out));
				p = out;
			}
			else if ((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || (*p == '_')) {
				const char *identifier = p;
				while ((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || (*p == '_') || (*p >= '0' && *p <= '9'))
					p++;
				token = Token(Token::NAME, identifier - text, p - identifier);
			}
			else if (*p == '"') {
				const char *identifier = p + 1;
				++p;
				while (*p != '"') {
					p++;
				}
				token = Token(Token::STRING, identifier - text, p - identifier);
				++p;
			}
			else if (*p == '/' && skipComments) {
				++p;
				if (*p == '/') {
					++p;
					while (*p != '\n') {
						++p;
						if (*p == 0) {
							break;
						}
					}
					++p;
					if (*p == 0) {
						break;
					}
				}
			}
			else {
				switch (*p) {
					case '{': token = Token(Token::OPEN_BRACES); break;
					case '}': token = Token(Token::CLOSE_BRACES); break;
					case '(': token = Token(Token::OPEN_BRACKET); break;
					case ')': token = Token(Token::CLOSE_BRACKET); break;
					case '/': token = Token(Token::SLASH); break;
					case ' ': case '\t': case '\r': break;
					case '\n': token = Token(Token::NEWLINE); break;
					case ':': token = Token(Token::SEPARATOR); break;
					case '=': token = Token(Token::ASSIGN); break;
					case ';': token = Token(Token::SEMICOLON); break;
					case ',': token = Token(Token::DELIMITER); break;
				}
				++p;
			}
			if (token.type != Token::EMPTY) {
				_tokens.push_back(token);
			}
		}
	}

	const char* Tokenizer::name(int index) const {
		const Token& token = _tokens[index];
		switch (token.type) {
			case Token::NAME: return "NAME"; break;
			case Token::NUMBER: return "NUMBER"; break;
			case Token::DELIMITER: return "DELIMITER"; break;
			case Token::SEPARATOR: return "SEPARATOR"; break;
			case Token::OPEN_BRACES: return "OPEN_BRACES"; break;
			case Token::CLOSE_BRACES: return "CLOSE_BRACES"; break;
			case Token::STRING: return "STRING"; break;
			case Token::ASSIGN: return "ASSIGN"; break;
			case Token::SEMICOLON: return "SEMICOLON"; break;
			case Token::OPEN_BRACKET: return "OPEN_BRACKET"; break;
			case Token::CLOSE_BRACKET: return "CLOSE_BRACKET"; break;
			default: return "UNKNOWN"; break;
		}
	}

// -------------------------------------------
//
// -------------------------------------------
JSONReader::JSONReader() : _text(0) {
	int sizes[] = { sizeof(unsigned int), sizeof(int), sizeof(int) };
	_category_buffer.init(sizes, 3);
	int data_sizes[] = { sizeof(unsigned int), sizeof(int), sizeof(int), sizeof(int) };
	_data_buffer.init(data_sizes, 4);
}

// -------------------------------------------
//
// -------------------------------------------
JSONReader::~JSONReader() {
	if (_category_buffer.data != 0) {
		//gDefaultMemory->deallocate(_category_buffer.data);
		DEALLOC(_category_buffer.data);
	}
	if (_data_buffer.data != 0) {
		//gDefaultMemory->deallocate(_data_buffer.data);
		DEALLOC(_data_buffer.data);
	}
	if (_text != 0) {
		delete[] _text;
	}
}

void save_buffer(CharBuffer* buffer, FILE* fp) {
	fwrite(&buffer->size, sizeof(int), 1, fp);
	fwrite(&buffer->capacity, sizeof(int), 1, fp);
	for (int i = 0; i < buffer->capacity; ++i) {
		fwrite(&buffer->data[i], sizeof(char), 1, fp);
	}
}

void load_buffer(CharBuffer* buffer, FILE* fp) {
	int size = 0;
	int capacity = 0;
	fread(&size, sizeof(int), 1, fp);
	fread(&capacity, sizeof(int), 1, fp);
	buffer->resize(capacity);
	for (int i = 0; i < capacity; ++i) {
		fread(&buffer->data[i], sizeof(char), 1, fp);
	}
	buffer->size = size;
}

// -------------------------------------------
// allocate category buffer
// -------------------------------------------
void JSONReader::allocCategoryBuffer(int size) {
	if (_category_buffer.resize(size)) {
		_hashes = (unsigned int*)_category_buffer.get_ptr(0);
		_parents = (int*)_category_buffer.get_ptr(1);
		_indices = (int*)_category_buffer.get_ptr(2);
	}
}

// -------------------------------------------
// add category
// -------------------------------------------
int JSONReader::add_category(const char* name, int parent) {
	if (_category_buffer.size + 1 > _category_buffer.capacity) {
		allocCategoryBuffer(_category_buffer.size * 2 + 8);
	}
	_hashes[_category_buffer.size] = string::murmur_hash(name);
	_parents[_category_buffer.size] = parent;
	_indices[_category_buffer.size] = _name_buffer.size;
	++_category_buffer.size;
	int l = strlen(name);
	if (_name_buffer.size + l > _name_buffer.capacity) {
		_name_buffer.resize(_name_buffer.capacity + 256);
	}
	_name_buffer.append(name, l);
	return _category_buffer.size - 1;
}

// -------------------------------------------
// save binary
// -------------------------------------------
void JSONReader::save_binary(const char* fileName) {

	FILE* fp = fopen(fileName, "wb");
	if (fp) {
		// buffer
		fwrite(&_category_buffer.size, sizeof(int), 1, fp);
		for (int i = 0; i < _category_buffer.size; ++i) {
			fwrite(&_hashes[i], sizeof(unsigned int), 1, fp);
		}
		for (int i = 0; i < _category_buffer.size; ++i) {
			fwrite(&_parents[i], sizeof(int), 1, fp);
		}
		for (int i = 0; i < _category_buffer.size; ++i) {
			fwrite(&_indices[i], sizeof(int), 1, fp);
		}
		save_buffer(&_name_buffer, fp);

		fwrite(&_data_buffer.size, sizeof(int), 1, fp);
		for (int i = 0; i < _values.size; ++i) {
			fwrite(&_data_keys[i], sizeof(unsigned int), 1, fp);
		}
		for (int i = 0; i < _data_buffer.size; ++i) {
			fwrite(&_data_categories[i], sizeof(int), 1, fp);
		}
		for (int i = 0; i < _data_buffer.size; ++i) {
			fwrite(&_data_indices[i], sizeof(int), 1, fp);
		}
		for (int i = 0; i < _data_buffer.size; ++i) {
			fwrite(&_data_sizes[i], sizeof(int), 1, fp);
		}
	
		save_buffer(&_values, fp);
		fclose(fp);
	}
}

// -------------------------------------------
// load binary
// -------------------------------------------
bool JSONReader::load_binary(const char* fileName) {
	FILE* fp = fopen(fileName, "rb");
	if (fp) {
		int size = 0;
		fread(&size, sizeof(int), 1, fp);
		allocCategoryBuffer(size);
		for (int i = 0; i < size; ++i) {
			fread(&_hashes[i], sizeof(unsigned int), 1, fp);
		}
		for (int i = 0; i < size; ++i) {
			fread(&_parents[i], sizeof(int), 1, fp);
		}
		for (int i = 0; i < size; ++i) {
			fread(&_indices[i], sizeof(int), 1, fp);
		}
		_category_buffer.size = size;

		load_buffer(&_name_buffer, fp);

		fread(&size, sizeof(int), 1, fp);
		alloc(size);
		for (int i = 0; i < size; ++i) {
			fread(&_data_keys[i], sizeof(unsigned int), 1, fp);
		}
		for (int i = 0; i < size; ++i) {
			fread(&_data_categories[i], sizeof(int), 1, fp);
		}
		for (int i = 0; i < size; ++i) {
			fread(&_data_indices[i], sizeof(int), 1, fp);
		}
		for (int i = 0; i < size; ++i) {
			fread(&_data_sizes[i], sizeof(int), 1, fp);
		}
		_data_buffer.size = size;
		load_buffer(&_values, fp);
		return true;
	}
	return false;
}

// -------------------------------------------
// parse
// -------------------------------------------
bool JSONReader::parse(const char* fileName) {
	//Token tokens[1024];
	int fileSize = -1;
	_text = repository::load(fileName, &fileSize);
	if (fileSize == -1) {
		return false;
	}
	//tokenize(_text);// , tokens, 1024);
	Tokenizer tokenizer;
	tokenizer.parse(_text);
	char name[128];
	int n = 0;
	int category_index = 0;
	int cat = -1;
	Stack<int> cat_stack;
	while (n < tokenizer.size()) {
		Token& t = tokenizer.get(n);
		if (t.type == Token::NAME) {
			if (tokenizer.get(n + 1).type == Token::OPEN_BRACES) {
				++n;
				strncpy(name, _text + t.index, t.size);
				name[t.size] = '\0';
				++category_index;
				int parent = -1;
				if (!cat_stack.empty()) {
					parent = cat_stack.top();
				}
				cat = add_category(name, parent);
				cat_stack.push(cat);
				++n;
			}
			else if (tokenizer.get(n + 1).type == Token::SEPARATOR) {
				++n;
				strncpy(name, _text + t.index, t.size);
				name[t.size] = '\0';
				int parent = -1;
				if (!cat_stack.empty()) {
					parent = cat_stack.top();
				}
				int p = create_property(name, parent);
				++n;
				Token& v = tokenizer.get(n);
				if (v.type == Token::STRING) {
					int start = v.index;
					int end = v.index + v.size;
					add(p, _text + start, v.size);
					++n;
				}
				else if (v.type == Token::NAME) {
					strncpy(name, _text + v.index, v.size);
					name[v.size] = '\0';
					if (strcmp(name, "true") == 0) {
						add(p, 1.0f);
					}
					else {
						add(p, 0.0f);
					}
					++n;
				}
				else {
					bool parsing = true;
					while (parsing) {
						if (v.type == Token::NUMBER) {
							add(p, v.value);
							if (tokenizer.get(n + 1).type != Token::DELIMITER) {
								parsing = false;
							}
						}
						++n;
						v = tokenizer.get(n);

					}
				}
			}
		}

		else if (t.type == Token::CLOSE_BRACES) {
			if (!cat_stack.empty()) {
				cat_stack.pop();
			}
			--category_index;
			++n;
		}
		else {
			++n;
		}
		t = tokenizer.get(n);
	}
	return true;
}

// -------------------------------------------
// get categories
// -------------------------------------------
int JSONReader::get_categories(int* result, int max, int parent) const {
	int cnt = 0;
	for (int i = 0; i < _category_buffer.size; ++i) {
		if (_parents[i] == parent && cnt < max) {
			result[cnt++] = i;
		}
	}
	return cnt;
}

// -------------------------------------------
// find category
// -------------------------------------------
int JSONReader::find_category(const char* name, int parent) const {
	unsigned int hash = string::murmur_hash(name);
	for (int i = 0; i < _category_buffer.size; ++i) {
		if (_parents[i] == parent && _hashes[i] == hash) {
			return i;
		}
	}
	return -1;
}

// -------------------------------------------
// get category name
// -------------------------------------------
const char* JSONReader::get_category_name(int category_id) const {
	if (category_id >= 0 && category_id < _category_buffer.size) {
		int index = _indices[category_id];
		return _name_buffer.data + index;
	}
	return '\0';
}

// -------------------------------------------
// matches
// -------------------------------------------
bool JSONReader::matches(int category_id, const char* name) const {
	if (category_id >= 0 && category_id < _category_buffer.size) {
		unsigned int hash = string::murmur_hash(name);
		if (_hashes[category_id] == hash) {
			return true;
		}
	}
	return false;
}

// -------------------------------------------
// get bool
// ------------------------------------------- 
bool JSONReader::get(int category_id, const char* name, bool* ret) const {
	int idx = get_index(category_id, name);
	if (idx != -1) {
		int v = static_cast<int>(get(_data_indices[idx]));
		if (v == 1) {
			*ret = true;
		}
		else {
			*ret = false;
		}
		return true;		
	}
	return false;
}

// -------------------------------------------
// get int
// -------------------------------------------
bool JSONReader::get_int(int category_id, const char* name, int* ret) const {
	int idx = get_index(category_id, name);
	if (idx != -1) {
		*ret = static_cast<int>(get(_data_indices[idx]));
		return true;		
	}
	return false;
}

// -------------------------------------------
// get uint
// -------------------------------------------
bool JSONReader::get_uint(int category_id, const char* name, uint32_t* ret) const {
	int idx = get_index(category_id, name);
	if (idx != -1) {
		*ret = static_cast<uint32_t>(get(_data_indices[idx]));
		return true;
	}
	return false;
}

// -------------------------------------------
// get uint
// -------------------------------------------
bool JSONReader::get(int category_id, const char* name, uint16_t* ret) const {
	int idx = get_index(category_id, name);
	if (idx != -1) {
		*ret = static_cast<uint16_t>(get(_data_indices[idx]));
		return true;
	}
	return false;
}

// -------------------------------------------
// get uint
// -------------------------------------------
bool JSONReader::get(int category_id, const char* name, uint32_t* ret) const {
	int idx = get_index(category_id, name);
	if (idx != -1) {
		*ret = static_cast<uint32_t>(get(_data_indices[idx]));
		return true;
	}
	return false;
}

// -------------------------------------------
// get float
// -------------------------------------------
bool JSONReader::get_float(int category_id, const char* name, float* ret) const {
	int idx = get_index(category_id, name);
	if (idx != -1) {
		*ret = get(_data_indices[idx]);
		return true;		
	}
	return false;
}

bool JSONReader::get(int category_id, const char* name, float* ret) const {
	int idx = get_index(category_id, name);
	if (idx != -1) {
		*ret = get(_data_indices[idx]);
		return true;
	}
	return false;
}


// -------------------------------------------
// get v2
// -------------------------------------------
bool JSONReader::get_vec2(int category_id, const char* name, v2* ret) const {
	int idx = get_index(category_id, name);
	if (idx != -1) {
		ret->x = get(_data_indices[idx]);
		ret->y = get(_data_indices[idx] + 1);
		return true;	
	}
	return false;
}

bool JSONReader::get(int category_id, const char* name, v2* ret) const {
	int idx = get_index(category_id, name);
	if (idx != -1) {
		ret->x = get(_data_indices[idx]);
		ret->y = get(_data_indices[idx] + 1);
		return true;
	}
	return false;
}

// -------------------------------------------
// get v3
// -------------------------------------------
bool JSONReader::get_vec3(int category_id, const char* name, v3* ret) const {
	int idx = get_index(category_id, name);
	if (idx != -1) {
		ret->x = get(_data_indices[idx]);
		ret->y = get(_data_indices[idx] + 1);
		ret->z = get(_data_indices[idx] + 2);
		return true;
	}
	return false;
}

// -------------------------------------------
// get v3
// -------------------------------------------
bool JSONReader::get(int category_id, const char* name, v3* ret) const {
	int idx = get_index(category_id, name);
	if (idx != -1) {
		ret->x = get(_data_indices[idx]);
		ret->y = get(_data_indices[idx] + 1);
		ret->z = get(_data_indices[idx] + 2);
		return true;
	}
	return false;
}

int JSONReader::num_properties(int category_id) const {
	int cnt = 0;
	for (int i = 0; i < _data_buffer.size; ++i) {
		if (_data_categories[i] == category_id) {
			++cnt;
		}
	}
	return cnt;
}
// -------------------------------------------
// get index
// -------------------------------------------
int JSONReader::get_index(int category_id, const char* name) const {
	if (category_id >= 0 && category_id < _category_buffer.size) {
		unsigned int key = string::murmur_hash(name);
		for (int i = 0; i < _data_buffer.size; ++i) {
			if (_data_categories[i] == category_id && _data_keys[i] == key) {
				return i;
			}
		}
	}
	return -1;
}

// -------------------------------------------
// contains property
// -------------------------------------------
bool JSONReader::contains_property(int category_id, const char* name) const {
	return get_index(category_id, name) != -1;
}

// -------------------------------------------
// get color
// -------------------------------------------
bool JSONReader::get_color(int category_id, const char* name, Color* ret) const {
	int idx = get_index(category_id, name);
	if (idx != -1) {
		ret->r = get(_data_indices[idx]) / 255.0f;
		ret->g = get(_data_indices[idx] + 1) / 255.0f;
		ret->b = get(_data_indices[idx] + 2) / 255.0f;
		ret->a = get(_data_indices[idx] + 3) / 255.0f;
		return true;
	}
	return false;
}

bool JSONReader::get(int category_id, const char* name, Color* ret) const {
	int idx = get_index(category_id, name);
	if (idx != -1) {
		ret->r = get(_data_indices[idx]) / 255.0f;
		ret->g = get(_data_indices[idx] + 1) / 255.0f;
		ret->b = get(_data_indices[idx] + 2) / 255.0f;
		ret->a = get(_data_indices[idx] + 3) / 255.0f;
		return true;
	}
	return false;
}

// -------------------------------------------
// get rect
// -------------------------------------------
bool JSONReader::get(int category_id, const char* name, Rect* ret) const {
	int idx = get_index(category_id, name);
	if (idx != -1) {
		ret->top = get(_data_indices[idx]);
		ret->left = get(_data_indices[idx] + 1);
		ret->right = ret->left + get(_data_indices[idx] + 2);
		ret->bottom = ret->top + get(_data_indices[idx] + 3);
		return true;
	}
	return false;
}

// -------------------------------------------
// get string
// ------------------------------------------- 
const char* JSONReader::get_string(int category_id, const char* name) const {
	int idx = get_index(category_id, name);
	if (idx != -1) {
		return get_char(_data_indices[idx]);
	}
	return '\0';
}

// -------------------------------------------
// get color path
// -------------------------------------------
bool JSONReader::get_color_path(int category_id, const char* name, ds::ColorPath* path) const {
	int idx = get_index(category_id, name);
	if (idx != -1) {
		int entries = _data_sizes[idx];
		int steps = entries / 5;
		if ((entries % 5) == 0) {
			int current = 0;
			for (int i = 0; i < steps; ++i) {
				float step = get(_data_indices[idx] + current);
				++current;
				Color c;
				c.r = get(_data_indices[idx] + current) / 255.0f;
				++current;
				c.g = get(_data_indices[idx] + current) / 255.0f;
				++current;
				c.b = get(_data_indices[idx] + current) / 255.0f;
				++current;
				c.a = get(_data_indices[idx] + current) / 255.0f;
				++current;
				path->add(step, c);
			}
		}
		else {
			return false;
		}
	}
	return true;
}
// -------------------------------------------
// get v2 path
// -------------------------------------------
bool JSONReader::get_vec2_path(int category_id, const char* name, ds::Vector2fPath* path) const {
	int idx = get_index(category_id, name);
	if (idx != -1) {
		int entries = _data_sizes[idx];
		int steps = entries / 3;
		if ((entries % 3) == 0) {
			int current = 0;
			for (int i = 0; i < steps; ++i) {
				float step = get(_data_indices[idx] + current);
				++current;
				v2 c;
				c.x = get(_data_indices[idx] + current);
				++current;
				c.y = get(_data_indices[idx] + current);
				++current;
				path->add(step, c);
			}
		}
		else {
			return false;
		}
	}
	return true;
}
// -------------------------------------------
// get float path
// -------------------------------------------
bool JSONReader::get_float_path(int category_id, const char* name, ds::FloatArray* path) const {
	int idx = get_index(category_id, name);
	if (idx != -1) {
		int entries = _data_sizes[idx];
		int steps = entries / 2;
		if ((entries % 2) == 0) {
			int current = 0;
			for (int i = 0; i < steps; ++i) {
				float step = get(_data_indices[idx] + current);
				++current;
				float c = get(_data_indices[idx] + current);
				++current;
				path->add(step, c);
			}
		}
		else {
			return false;
		}
	}
	return true;
}
// -------------------------------------------
// get array
// -------------------------------------------
int JSONReader::get_array(int category_id, const char* name, float* values, int max) const {
	int idx = get_index(category_id, name);
	int cnt = 0;
	if (idx != -1) {
		int entries = _data_sizes[idx];
		for (int i = 0; i < entries; ++i) {
			if (cnt < max) {
				values[cnt++] = get(_data_indices[idx] + i);
			}
		}
	}
	return cnt;
}

// -------------------------------------------
// get int array
// -------------------------------------------
int JSONReader::get_array(int category_id, const char* name, int* values, int max) const {
	int idx = get_index(category_id, name);
	int cnt = 0;
	if (idx != -1) {
		int entries = _data_sizes[idx];
		for (int i = 0; i < entries; ++i) {
			if (cnt < max) {
				values[cnt++] = static_cast<int>(get(_data_indices[idx] + i));
			}
		}
	}
	return cnt;
}

// -------------------------------------------
// allocate data buffer
// -------------------------------------------
void JSONReader::alloc(int elements) {
	if (_data_buffer.resize(elements)) {
		_data_keys = (unsigned int*)_data_buffer.get_ptr(0);
		_data_categories = (int*)_data_buffer.get_ptr(1);
		_data_indices = (int*)_data_buffer.get_ptr(2);
		_data_sizes = (int*)_data_buffer.get_ptr(3);
	}
}

// -------------------------------------------
// create property
// -------------------------------------------
int JSONReader::create_property(const char* name, int category) {
	if (_data_buffer.size + 1 > _data_buffer.capacity) {
		alloc(_data_buffer.size * 2 + 8);
	}
	_data_keys[_data_buffer.size] = string::murmur_hash(name);
	_data_categories[_data_buffer.size] = category;
	_data_indices[_data_buffer.size] = _values.num;
	_data_sizes[_data_buffer.size] = 0;
	++_data_buffer.size;
	return _data_buffer.size - 1;
}

// -------------------------------------------
// add value to property
// -------------------------------------------
void JSONReader::add(int pIndex, float value) {
	float* v = (float*)_values.alloc(sizeof(float));
	*v = value;
	++_data_sizes[pIndex];
}

// -------------------------------------------
// add string to property
// -------------------------------------------
void JSONReader::add(int pIndex, const char* c, int len) {
	int sz = ((len + 4) / 4) * 4;
	char* v = (char*)_values.alloc(sz * sizeof(char));
	for (int i = 0; i < len; ++i) {
		*v = c[i];
		++v;
	}
	*v = '\0';
	_data_sizes[pIndex] = sz / 4;
}

// -------------------------------------------
// add char to property
// -------------------------------------------
void JSONReader::add(int pIndex, char c) {
	char* v = (char*)_values.alloc(sizeof(char));
	*v = c;
	++_data_sizes[pIndex];
}

// -------------------------------------------
// get float value
// -------------------------------------------
float JSONReader::get(int index) const {
	char* p = _values.data + index * sizeof(float);
	float* v = (float*)(p);
	return *v;
}

// -------------------------------------------
// get string from property
// -------------------------------------------
const char* JSONReader::get_char(int index) const {
	return _values.data + index * sizeof(float);
}


JSONWriter::JSONWriter() {
	f = 0;
	_ident = 0;
}


JSONWriter::~JSONWriter() {
	if (f != 0) {
		fclose(f);
	}
}

// ----------------------------------------------------------
// open
// ----------------------------------------------------------
bool JSONWriter::open(const char* fileName) {
	f = fopen(fileName, "w");
	_started = true;
	return true;
}

// ----------------------------------------------------------
// start category
// ----------------------------------------------------------
void JSONWriter::startCategory(const char* name) {
	if (!_started) {
		fprintf_s(f, "\n");
	}
	writeIdent();
	fprintf(f, "%s { ", name);
	_open = true;
	_items = 0;
	_ident += 4;
	_started = false;
}

// ----------------------------------------------------------
// write int
// ----------------------------------------------------------
void JSONWriter::write(const char* name, int value) {

	writeLineIdent();
	fprintf_s(f, "%s : %d", name, value);
	++_items;
}

// ----------------------------------------------------------
// write uint32_t
// ----------------------------------------------------------
void JSONWriter::write(const char* name, uint32_t value) {

	writeLineIdent();
	fprintf_s(f, "%s : %d", name, value);
	++_items;
}

// ----------------------------------------------------------
// write float
// ----------------------------------------------------------
void JSONWriter::write(const char* name, float value) {

	writeLineIdent();
	fprintf(f, "%s : %g", name, value);
	++_items;
}

// ----------------------------------------------------------
// write string
// ----------------------------------------------------------
void JSONWriter::write(const char* name, const char* value) {

	writeLineIdent();
	fprintf(f, "%s : \"%s\"", name, value);
	++_items;
}

// ----------------------------------------------------------
// write boolean
// ----------------------------------------------------------
void JSONWriter::write(const char* name, bool value) {

	writeLineIdent();
	if (value) {
		fprintf(f, "%s : true", name);
	}
	else {
		fprintf(f, "%s : false", name);
	}
	++_items;
}

// ----------------------------------------------------------
// write v2
// ----------------------------------------------------------
void JSONWriter::write(const char* name, const v2& value) {

	writeLineIdent();
	fprintf(f, "%s : %g,%g", name, value.x, value.y);
	++_items;

}

// ----------------------------------------------------------
// write v3
// ----------------------------------------------------------
void JSONWriter::write(const char* name, const v3& value) {

	writeLineIdent();
	fprintf(f, "%s : %g,%g,%g", name, value.x, value.y, value.z);
	++_items;

}

// ----------------------------------------------------------
// write color
// ----------------------------------------------------------
void JSONWriter::write(const char* name, const Color& value) {

	writeLineIdent();
	int r = static_cast<int>(value.r * 255.0f);
	int g = static_cast<int>(value.g * 255.0f);
	int b = static_cast<int>(value.b * 255.0f);
	int a = static_cast<int>(value.a * 255.0f);
	fprintf(f, "%s : %d,%d,%d,%d", name, r, g, b, a);
	++_items;
}

// ----------------------------------------------------------
// write rect
// ----------------------------------------------------------
void JSONWriter::write(const char* name, const Rect& value) {

	writeLineIdent();
	int top = static_cast<int>(value.top);
	int left = static_cast<int>(value.left);
	int w = static_cast<int>(value.width());
	int h = static_cast<int>(value.height());
	fprintf(f, "%s : %d,%d,%d,%d", name, top, left, w, h);
	++_items;
}

// ----------------------------------------------------------
// end category
// ----------------------------------------------------------
void JSONWriter::endCategory() {
	_items = 0;
	fprintf_s(f, "\n");
	_ident -= 4;
	writeIdent();
	fprintf_s(f, "}");
	_open = false;
}

// ----------------------------------------------------------
// write line ident
// ----------------------------------------------------------
void JSONWriter::writeLineIdent() {
	fprintf(f, "\n");
	for (int i = 0; i < _ident; ++i) {
		fprintf(f, " ");
	}
}

// ----------------------------------------------------------
// write color path
// ----------------------------------------------------------
void JSONWriter::write(const char* name, const ds::ColorPath& path) {
	for (int i = 0; i < path.size(); ++i) {
		const ds::Color& value = path.value(i);
		float key = path.key(i);
		writeLineIdent();
		int r = static_cast<int>(value.r * 255.0f);
		int g = static_cast<int>(value.g * 255.0f);
		int b = static_cast<int>(value.b * 255.0f);
		int a = static_cast<int>(value.a * 255.0f);
		fprintf(f, "\"%g\" : \"%d,%d,%d,%d\"", key, r, g, b, a);
		++_items;
	}
}
// ----------------------------------------------------------
// write color path
// ----------------------------------------------------------
void JSONWriter::write(const char* name, const ds::Vector2fPath& path) {
	writeLineIdent();
	fprintf(f, "%s : ", name);
	for (int i = 0; i < path.size(); ++i) {
		const v2& value = path.value(i);
		float key = path.key(i);
		fprintf(f, "%g,%g,%g", key, value.x, value.y);
		writeLineIdent();
		++_items;
	}
}
// ----------------------------------------------------------
// write color path
// ----------------------------------------------------------
void JSONWriter::write(const char* name, const ds::FloatArray& path) {
	writeLineIdent();
	fprintf(f, "%s : ", name);
	for (int i = 0; i < path.size(); ++i) {
		float value = path.value(i);
		float key = path.key(i);		
		fprintf(f, "%g,%g", key, value);
		writeLineIdent();
		++_items;
	}
}

void JSONWriter::write(const char* name, const int* values, int count) {
	writeLineIdent();
	fprintf(f, "%s : ", name);
	for (int i = 0; i < count; ++i) {
		if (i > 0) {
			fprintf(f, ",%d", values[i]);
		}
		else {
			fprintf(f, "%d", values[i]);
		}
		++_items;
	}
	writeLineIdent();
}

// ----------------------------------------------------------
// write ident
// ----------------------------------------------------------
void JSONWriter::writeIdent() {
	for (int i = 0; i < _ident; ++i) {
		fprintf(f, " ");
	}
}

FlatJSONReader::FlatJSONReader() : _text(0) {
	int data_sizes[] = { sizeof(unsigned int), sizeof(int), sizeof(int), sizeof(int) };
	_data_buffer.init(data_sizes, 4);
}
// -------------------------------------------
// add category
// -------------------------------------------
int FlatJSONReader::add_category(const char* name) {
	int idx = _name_buffer.size;
	int l = strlen(name);
	if (_name_buffer.size + l > _name_buffer.capacity) {
		_name_buffer.resize(_name_buffer.capacity + 256);
	}
	_name_buffer.append(name, l);
	return idx;
}

void FlatJSONReader::buildName(const Stack<CategoryEntry>& stack, char* buffer) {
	std::string tmp;
	char b[128];
	for (int i = stack.size() - 1; i >= 0; --i) {
		const CategoryEntry& c = stack.at(i);
		strncpy(b, _name_buffer.data + c.text_index, c.text_length);
		b[c.text_length] = '\0';
		tmp.append(b);
		tmp.append(".");
	}
	strcpy(buffer, tmp.c_str());
}

// -------------------------------------------
// allocate data buffer
// -------------------------------------------
void FlatJSONReader::alloc(int elements) {
	if (_data_buffer.resize(elements)) {
		_data_keys = (unsigned int*)_data_buffer.get_ptr(0);
		_data_indices = (int*)_data_buffer.get_ptr(1);
		_data_sizes = (int*)_data_buffer.get_ptr(2);
	}
}

// -------------------------------------------
// create property
// -------------------------------------------
int FlatJSONReader::create_property(const char* name) {
	if (_data_buffer.size + 1 > _data_buffer.capacity) {
		alloc(_data_buffer.size * 2 + 8);
	}
	_data_keys[_data_buffer.size] = string::murmur_hash(name);
	_data_indices[_data_buffer.size] = _values.num;
	_data_sizes[_data_buffer.size] = 0;
	++_data_buffer.size;
	LOG << "property: " << name;
	return _data_buffer.size - 1;
}

// -------------------------------------------
// add value to property
// -------------------------------------------
void FlatJSONReader::add(int pIndex, float value) {
	float* v = (float*)_values.alloc(sizeof(float));
	*v = value;
	++_data_sizes[pIndex];
}

// -------------------------------------------
// add string to property
// -------------------------------------------
void FlatJSONReader::add(int pIndex, const char* c, int len) {
	int sz = ((len + 4) / 4) * 4;
	char* v = (char*)_values.alloc(sz * sizeof(char));
	for (int i = 0; i < len; ++i) {
		*v = c[i];
		++v;
	}
	*v = '\0';
	_data_sizes[pIndex] = sz / 4;
}

// -------------------------------------------
// add char to property
// -------------------------------------------
void FlatJSONReader::add(int pIndex, char c) {
	char* v = (char*)_values.alloc(sizeof(char));
	*v = c;
	++_data_sizes[pIndex];
}

// -------------------------------------------
// get index
// -------------------------------------------
int FlatJSONReader::get_index(const char* name) const {
	unsigned int key = string::murmur_hash(name);
	for (int i = 0; i < _data_buffer.size; ++i) {
		if (_data_keys[i] == key) {
			return i;
		}
	}
	return -1;
}

// -------------------------------------------
// get float value
// -------------------------------------------
float FlatJSONReader::get(int index) const {
	char* p = _values.data + index * sizeof(float);
	float* v = (float*)(p);
	return *v;
}
// -------------------------------------------
// get float
// -------------------------------------------
bool FlatJSONReader::get_float(const char* name, float* ret) const {
	int idx = get_index(name);
	if (idx != -1) {
		*ret = get(_data_indices[idx]);
		return true;
	}
	return false;
}

// -------------------------------------------
// get float
// -------------------------------------------
bool FlatJSONReader::get(const char* name, float* ret) const {
	int idx = get_index(name);
	if (idx != -1) {
		*ret = get(_data_indices[idx]);
		return true;
	}
	return false;
}

// -------------------------------------------
// get v2 path
// -------------------------------------------
bool FlatJSONReader::get(const char* name, Vector2fPath* path) const {
	int idx = get_index(name);
	if (idx != -1) {
		int entries = _data_sizes[idx];
		int steps = entries / 3;
		if ((entries % 3) == 0) {
			int current = 0;
			for (int i = 0; i < steps; ++i) {
				float step = get(_data_indices[idx] + current);
				++current;
				v2 c;
				c.x = get(_data_indices[idx] + current);
				++current;
				c.y = get(_data_indices[idx] + current);
				++current;
				path->add(step, c);
			}
		}
		else {
			return false;
		}
	}
	return true;
}
// -------------------------------------------
// get int
// -------------------------------------------
bool FlatJSONReader::get(const char* name, int* ret) const {
	int idx = get_index(name);
	if (idx != -1) {
		*ret = static_cast<int>(get(_data_indices[idx]));
		return true;
	}
	return false;
}

bool FlatJSONReader::get(const char* name, Color* ret) const {
	int idx = get_index(name);
	if (idx != -1) {
		ret->r = get(_data_indices[idx]) / 255.0f;
		ret->g = get(_data_indices[idx] + 1) / 255.0f;
		ret->b = get(_data_indices[idx] + 2) / 255.0f;
		ret->a = get(_data_indices[idx] + 3) / 255.0f;
		return true;
	}
	return false;
}

bool FlatJSONReader::get(const char* name, v2* ret) const {
	int idx = get_index(name);
	if (idx != -1) {
		ret->x = get(_data_indices[idx]);
		ret->y = get(_data_indices[idx] + 1);
		return true;
	}
	return false;
}

// -------------------------------------------
// get rect
// -------------------------------------------
bool FlatJSONReader::get(const char* name, Rect* ret) const {
	int idx = get_index(name);
	if (idx != -1) {
		ret->top = get(_data_indices[idx]);
		ret->left = get(_data_indices[idx] + 1);
		ret->right = ret->left + get(_data_indices[idx] + 2);
		ret->bottom = ret->top + get(_data_indices[idx] + 3);
		return true;
	}
	return false;
}

// -------------------------------------------
// contains
// -------------------------------------------
bool FlatJSONReader::contains(const char* name) const {
	return get_index(name) != -1;
}
// -------------------------------------------
// parse
// -------------------------------------------
bool FlatJSONReader::parse(const char* fileName) {
	int fileSize = -1;
	_text = repository::load(fileName, &fileSize);
	if (fileSize == -1) {
		return false;
	}
	Tokenizer tokenizer;
	tokenizer.parse(_text);
	char name[128];
	char fullName[256];
	int n = 0;
	int category_index = 0;
	int cat = -1;
	Stack<CategoryEntry> cat_stack;
	char cat_name[256];
	while (n < tokenizer.size()) {
		Token& t = tokenizer.get(n);
		if (t.type == Token::NAME) {
			if (tokenizer.get(n + 1).type == Token::OPEN_BRACES) {
				++n;
				strncpy(name, _text + t.index, t.size);
				name[t.size] = '\0';
				CategoryEntry cat;
				cat.text_index = add_category(name);
				cat.text_length = strlen(name);
				cat_stack.push(cat);
				buildName(cat_stack, cat_name);
				//LOG << "cat_name: " << cat_name;
				++n;
			}
			else if (tokenizer.get(n + 1).type == Token::SEPARATOR) {
				++n;
				strncpy(name, _text + t.index, t.size);
				name[t.size] = '\0';
				sprintf(fullName, "%s%s", cat_name, name);
				LOG << "fullname: " << fullName;
				int p = create_property(fullName);
				++n;
				Token& v = tokenizer.get(n);
				if (v.type == Token::STRING) {
					int start = v.index;
					int end = v.index + v.size;
					add(p, _text + start, v.size);
					++n;
				}
				else if (v.type == Token::NAME) {
					strncpy(name, _text + v.index, v.size);
					name[v.size] = '\0';
					if (strcmp(name, "true") == 0) {
						add(p, 1.0f);
					}
					else {
						add(p, 0.0f);
					}
					++n;
				}
				else {
					bool parsing = true;
					while (parsing) {
						if (v.type == Token::NUMBER) {
							add(p, v.value);
							if (tokenizer.get(n + 1).type != Token::DELIMITER) {
								parsing = false;
							}
						}
						++n;
						v = tokenizer.get(n);

					}
				}
			}
		}

		else if (t.type == Token::CLOSE_BRACES) {
			if (!cat_stack.empty()) {
				cat_stack.pop();
			}
			buildName(cat_stack, cat_name);
			//LOG << "cat_name: " << cat_name;
			++n;
		}
		else {
			++n;
		}
		t = tokenizer.get(n);
	}
	return true;
}


}
