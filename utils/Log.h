/* 
 * File:   Log.h
 * Author: mecky
 *
 * Created on 3. November 2010, 08:05
 */

#ifndef LOG_H
#define	LOG_H

#include <sstream>
#include <stdio.h>


void init_logger(int width,int height);

void shutdown_logger();

class LogOutputHandler {

public:
	virtual void write(const char* message) = 0;
};

class ConsoleOutputHandler : public LogOutputHandler {

public:
	void write(const char* message);
};

class FileOutputHandler : public LogOutputHandler {

public:
   static FILE*& Stream();
   void write(const char* msg);

};

class Log {
    
public:
    Log();    
    virtual ~Log();
    std::ostringstream& get();
	std::ostringstream& error();
	std::ostringstream& get(const char *file, const unsigned long line);    
	std::ostringstream& error(const char *file, const unsigned long line);
	std::ostringstream& error(const char *file, const unsigned long line,const char* message);
	std::ostringstream& error(const char *file, const unsigned long line, char* format, va_list args);
	std::ostringstream& error(const char *file, const unsigned long line, char* format, ...);
	static LogOutputHandler& handler();        
protected:
    std::ostringstream os;		
private:
	void log_file_line(const char *file, const unsigned long line, bool isError);
    std::string NowTime();
	void NowTime(char* ret, int max);
    Log(const Log&);
    Log& operator =(const Log&);	
	bool _errorFlag;
};

#ifndef FILELOG_MAX_LEVEL
#define FILELOG_MAX_LEVEL logDEBUG
#endif

#define LG \
	Log().get()

#define LOG Log().get(__FILE__,__LINE__)
#define LOGE Log().error(__FILE__,__LINE__)

#define ELOG(M) \
	do {\
		Log().error(__FILE__,__LINE__,M); \
	} while (0)

#endif	/* LOG_H */

