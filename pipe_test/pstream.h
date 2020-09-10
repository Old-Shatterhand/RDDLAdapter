#ifndef RDDLPARSER_PSTREAM_H
#define RDDLPARSER_PSTREAM_H

#include<fstream>

class PStream{
public:
	PStream(const char* name);

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
private:
	std::ofstream stream;
};

#endif
