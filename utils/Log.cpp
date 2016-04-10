/* 
 * File:   Log.cpp
 * Author: mecky
 * 
 * Created on 3. November 2010, 08:05
 */

#include "Log.h"
#include <windows.h>
#include <stdio.h>
#pragma warning(disable: 4996)

void file_name(const char* file, char* name) {
	char n[256];
	char e[16];
	_splitpath(file,0,0,name,e);
}

void ConsoleOutputHandler::write(const char* message) {
	OutputDebugStringA(message);
}

FILE*& FileOutputHandler::Stream() {
    static FILE* pStream = stderr;
    return pStream;
}

void FileOutputHandler::write(const char* message) {
	OutputDebugStringA(message);
	FILE* pStream = Stream();
	if ( pStream ) {
		fprintf(pStream, "%s", message);
		fflush(pStream);
	}
}

Log::Log() {}

Log::Log(const Log& orig) {}

std::ostringstream& Log::get() {
    os << NowTime();
    os << " : ";
    return os;
}

void Log::log_file_line(const char *file, const unsigned long line, bool isError) {
	char buffer[128];
	NowTime(buffer, 128);
	os << buffer;
	os << " ";
	if (isError) {
		os << " -- ERROR -- ";
	}
	file_name(file, buffer);
	os << " [";
	os << buffer;
	os << ":";
	os << line;
	os << "] : ";
}

std::ostringstream& Log::get(const char *file, const unsigned long line) {
	log_file_line(file, line, false);
	return os;
}

std::ostringstream& Log::error(const char *file, const unsigned long line) {
	log_file_line(file, line, true);
	return os;
}

std::ostringstream& Log::error(const char *file, const unsigned long line,const char* message) {
	log_file_line(file, line,true);
	os << message;
	return os;
}

std::ostringstream& Log::error(const char *file, const unsigned long line, char* format, va_list args) {
	log_file_line(file, line, true);
	char buffer[1024];
	memset(buffer, 0, sizeof(buffer));
	int written = vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, format, args);
	os << buffer;
	return os;
}

std::ostringstream& Log::error(const char *file, const unsigned long line, char* format, ...) {
	va_list args;
	va_start(args, format);
	error(file, line, format, args);
	va_end(args);
	return os;
}

std::ostringstream& Log::error() {
	os << NowTime();
	os << " [ERROR] : ";
	return os;
}

Log::~Log() {
    os << std::endl;
	OutputDebugStringA(os.str().c_str());
	//handler().write(os.str());
}

LogOutputHandler& Log::handler() {
	static ConsoleOutputHandler coh;
	return coh;	
}

void Log::NowTime(char* ret, int max) {
	char buffer[200];
	if (GetTimeFormatA(LOCALE_USER_DEFAULT, 0, 0, "HH':'mm':'ss", buffer, 200) == 0) {
		//return "Error in NowTime()";
	}

	static DWORD first = GetTickCount();
	sprintf_s(ret, max, "%s.%03ld", buffer, (long)(GetTickCount() - first) % 1000);
}

std::string Log::NowTime() {
    const int MAX_LEN = 200;
    char buffer[MAX_LEN];
    if (GetTimeFormatA(LOCALE_USER_DEFAULT, 0, 0,
            "HH':'mm':'ss", buffer, MAX_LEN) == 0)
        return "Error in NowTime()";

    char result[100] = {0};
    static DWORD first = GetTickCount();
    std::sprintf(result, "%s.%03ld", buffer, (long)(GetTickCount() - first) % 1000);
    return result;
}
