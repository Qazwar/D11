#include "FileUtils.h"
#include <fstream>
#include <sstream>
#include <windows.h>
#include <tchar.h> 
#include <stdio.h>
#include <strsafe.h>
#include "Log.h"
#include <assert.h>

namespace ds {

FileUtils::FileUtils() { }

FileUtils::~FileUtils() { }

namespace file {

	bool fileExists(const char* name) {
		if (FILE * file = fopen(name, "r")) {
			fclose(file);
			return true;
		}
		return false;
	}

	bool saveBinary(const char* fileName, char* data, int size) {
		FILE* f = fopen(fileName, "wb");
		if (f) {
			for (int i = 0; i < size; ++i) {
				fputc(data[i], f);
			}
			fclose(f);
			return true;
		}
		return false;
	}

	bool loadTextFile(const char* name,std::vector<std::string>& lines) {    
		std::string line;
		std::ifstream myfile(name);
		if (myfile.is_open()) {
			while ( std::getline(myfile, line)) {
				if (line.find("#", 0) == std::string::npos && line.size() > 0) {					
					lines.push_back(line);                
				}
			}
			myfile.close();    
			return true;
		}		
		return false;
	}

	void getFileTime(const char* fileName,FILETIME& time) {
		WORD ret = -1;
		// no file sharing mode
		HANDLE hFile = CreateFile(fileName, GENERIC_READ, 0, NULL,OPEN_EXISTING, 0, NULL);
		if( hFile != INVALID_HANDLE_VALUE) {		
			// Retrieve the file times for the file.
			GetFileTime(hFile, NULL, NULL, &time);
			CloseHandle(hFile);    
		} 
		else {
			LOGE << "Cannot get filetime for '" << fileName << "'";
		}
	}

	void listDirectory(const char* dir,std::vector<std::string>& files) {
		std::string directory = dir;
		WIN32_FIND_DATAA ffd;
		//LARGE_INTEGER filesize;
		//TCHAR szDir[MAX_PATH];
		//size_t length_of_arg;
		HANDLE hFind = INVALID_HANDLE_VALUE;
		//DWORD dwError = 0;

		if ( directory.find_last_of("\\") != directory.length() ) {
			directory.append("\\");
		}		
		hFind = FindFirstFileA((directory+"*").c_str(), &ffd);		
		if ( INVALID_HANDLE_VALUE != hFind ) {      
			do {
				if ( !(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) {         
					files.push_back(ffd.cFileName);				
				}
			}
			while (FindNextFileA(hFind, &ffd) != 0);
			FindClose(hFind);
		}
		else {
			LOGE << "cannot find directory " << dir;
		}
	}

	// -------------------------------------------------------
	// Checks if the file has changed
	// -------------------------------------------------------
	bool compareFileTime(const char* fileName,const FILETIME& time) {
		FILETIME now;
		getFileTime(fileName,now);
		//SYSTEMTIME stUTC, stLocal;
		//FileTimeToSystemTime(&now, &stUTC);
		//SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);
		//LOG << "file: " << fileName << " now: " << stLocal.wDay << "." << stLocal.wMonth << "." << stLocal.wYear << " " << stLocal.wHour << ":"<< stLocal.wMinute;
		int t = CompareFileTime(&time,&now);		
		if ( t == -1 ) {
			logFileTime(now);
			logFileTime(time);
			return true;
		}
		return false;
	}

	// -------------------------------------------------------
	// Compares filetime of two files and returns true if
	// first one is older
	// -------------------------------------------------------
	bool isOlder(const char* firstFile, const char* secondFile) {
		FILETIME firstTime;
		FILETIME secondTime;
		getFileTime(firstFile, firstTime);
		getFileTime(secondFile, secondTime);
		int t = CompareFileTime(&firstTime, &secondTime);
		if (t == -1) {
			LOG << firstFile << " is older as " << secondFile;
			return true;
		}
		LOG << firstFile << " is NOT older as " << secondFile;
		return false;
	}

	void logFileTime(const FILETIME& time) {
		SYSTEMTIME stUTC, stLocal;
		FileTimeToSystemTime(&time, &stUTC);
		SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);
		LOG << "time: " << stLocal.wDay << "." << stLocal.wMonth << "." << stLocal.wYear << " " << stLocal.wHour << ":" << stLocal.wMinute;
	}

	// -------------------------------------------------------
	// Checks if the file has changed
	// -------------------------------------------------------
	void logFileTime(const char* fileName) {
		FILETIME now;
		getFileTime(fileName,now);
		SYSTEMTIME stUTC, stLocal;
		FileTimeToSystemTime(&now, &stUTC);
		SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);
		LOG << "file: " << fileName << " now: " << stLocal.wDay << "." << stLocal.wMonth << "." << stLocal.wYear << " " << stLocal.wHour << ":"<< stLocal.wMinute;		
	}

	void copyFile(const char* source,const char* dest) {
		if ( !CopyFile(source,dest,false) ) {
			LOGE << "Cannot copy file " << source << " to " << dest;
		}
		/*
		FILE *fd1 = fopen(source, "rb");
		assert( fd1 != 0 );		
		FILE *fd2 = fopen(dest, "wb");
		assert( fd2 != 0 );
		size_t n, m;
		unsigned char buff[8192];
		do {
			n = fread(buff, 1, sizeof buff, fd1);
			if (n) m = fwrite(buff, 1, n, fd2);
			else   m = 0;
		} while ((n > 0) && (n == m));
		fclose(fd1);
		fclose(fd2);
		*/
	}

}

void FileUtils::listDirectory(const std::string& dir,std::vector<std::string>& files,bool appendWildcard) {
	std::string directory = dir;
	WIN32_FIND_DATAA ffd;
	//LARGE_INTEGER filesize;
	//TCHAR szDir[MAX_PATH];
	//size_t length_of_arg;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	//DWORD dwError = 0;
	if ( directory.find_last_of("\\") != directory.length() ) {
		directory.append("\\");
	}
	if ( appendWildcard ) {
		hFind = FindFirstFileA((directory+"*").c_str(), &ffd);
	}
	else {
		hFind = FindFirstFileA(directory.c_str(), &ffd);
	}
	if ( INVALID_HANDLE_VALUE != hFind ) {      
		do {
			if ( !(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) {         
				files.push_back(ffd.cFileName);				
			}
		}
		while (FindNextFileA(hFind, &ffd) != 0);
		FindClose(hFind);
	}
	else {
		LOGE << "cannot find directory " << dir.c_str();
	}
}

std::vector<std::string> FileUtils::listDirectory(const std::string& dir) {
	WIN32_FIND_DATAA ffd;
	LARGE_INTEGER filesize;
	TCHAR szDir[MAX_PATH];
	size_t length_of_arg;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	DWORD dwError=0;
	std::vector<std::string> files;
	hFind = FindFirstFileA(dir.c_str(), &ffd);
	if ( INVALID_HANDLE_VALUE != hFind ) {      
		do {
			if ( !(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) {         
				files.push_back(ffd.cFileName);				
			}
		}
		while (FindNextFileA(hFind, &ffd) != 0);
		FindClose(hFind);
	}
	else {
		LOGE << "cannot find directory " << dir.c_str();
	}
	return files;
}

FILETIME* FileUtils::getFileTime(const std::string& fileName) {
	WORD ret = -1;
	HANDLE hFile = CreateFile(fileName.c_str(), GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL,OPEN_EXISTING, 0, NULL);
    if( hFile != INVALID_HANDLE_VALUE) {
		FILETIME* ftWrite = new FILETIME();	
		// Retrieve the file times for the file.
		if (GetFileTime(hFile, NULL, NULL, ftWrite)) {
			return ftWrite;
		}
		CloseHandle(hFile);    
	}            
	return 0;
}

};
