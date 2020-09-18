#ifndef RDDLPARSER_PSTREAM_H
#define RDDLPARSER_PSTREAM_H

#include<fstream>
#include<string>

class PStream{
public:
	PStream() = default;
	PStream(const char* name);
	PStream(std::string& name);
	PStream& operator<<(bool val);
	PStream& operator<<(short val);
	PStream& operator<<(unsigned short val);
	PStream& operator<<(int val);
	PStream& operator<<(unsigned int val);
	PStream& operator<<(long val);
	PStream& operator<<(unsigned long val);
	PStream& operator<<(float val);
	PStream& operator<<(double val);
	PStream& operator<<(long double val);
	PStream& operator<<(void* val);

	void open(std::string&);
	void flush();
	void close();
private:
	std::ofstream stream;
};

#endif
