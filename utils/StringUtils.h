#pragma once
#include <stdint.h>
#include <string>
#include "..\Common.h"
#include "..\utils\Color.h"
#include "..\math\math.h"
#include "..\lib\collection_types.h"

namespace ds {

	class StringStream {

	typedef Array<char> Buffer;

	public:
		StringStream& operator<<(char c);
		StringStream& operator<<(const char *s);
		StringStream& operator<<(float f);
		StringStream& operator<<(int i);
		StringStream& operator<<(uint32_t i);
		StringStream& operator<<(const v2& v);
		StringStream& operator<<(const v3& v);
		StringStream& operator<<(const Color& v);
		StringStream& operator<<(const Rect& v);
		StringStream& format(const char *format, ...);
		const char *c_str();
	private:
		StringStream& push(const char *data, uint32_t n);
		Buffer _buffer;
	};

namespace string {

	//Array<std::string> split(const std::string& str,const char delimiter = ',');

	int count_delimiters(const char* text, const char delimiter = ',');

	int split(const char* text, float* array, int max, const char delimiter = ',');

	void split(const std::string& str,Array<std::string>& list,const char delimiter = ',');

	void trim(std::string& str);

	void formatPercentage(float percentage, char* ret);

	void trim(std::string& str,const std::string& ws);

	void formatInt(int value, char* buffer, int maxBuffer, int length);

	void formatInt(int value,std::string& str,int length);

	void formatInt(int value, char* str, int length);

	void formatInt(int value,std::string& str);

	void format_number(int n, char * out, const char delimiter = '.');

	int extract_used_characters(const char* text, char* array, int max);

	void formatTime(int minutes,int seconds,std::string& str);

	void file_name(const char* file, char* name);

	IdString murmur_hash(const char* text);

	IdString murmur_hash(const void * key, int len, unsigned int seed);

	void formatDateTime(std::string& str);

	bool isDigit(const char c);

	bool isNumeric(const char* text);

	bool isWhitespace(const char c);

	float strtof(const char* p,char** endPtr);

	bool isCharacter(const char p);


	template <class T>
	inline std::string to_string (const T& t) {
		std::stringstream ss;
		ss << t;
		return ss.str();
	}

	template<class T> 
	T inline from_string(const std::string& s) {
		std::istringstream stream (s);
		T t;
		stream >> t;
		return t;
	}


}

}

