#include "pstream.h"

#include <iostream>


PStream::PStream(const char* filename){
	stream.open(filename);
}

PStream::PStream(std::string& filename){
	stream.open(filename);
}

PStream& PStream::operator<<(bool val){
	stream << val << std::endl;
	stream.flush();
	
	return *this;
}

PStream& PStream::operator<<(short val){
	stream << val << std::endl;
	stream.flush();
	
	return *this;
}

PStream& PStream::operator<<(unsigned short val){
	stream << val << std::endl;
	stream.flush();
	
	return *this;
}

PStream& PStream::operator<<(int val){
	stream << val << std::endl;
	stream.flush();
	
	return *this;
}

PStream& PStream::operator<<(unsigned int val){
	stream << val << std::endl;
	stream.flush();
	
	return *this;
}

PStream& PStream::operator<<(long val){
	stream << val << std::endl;
	stream.flush();
	
	return *this;
}

PStream& PStream::operator<<(unsigned long val){
	stream << val << std::endl;
	stream.flush();
	
	return *this;
}

PStream& PStream::operator<<(float val){
	stream << val << std::endl;
	stream.flush();
	
	return *this;
}

PStream& PStream::operator<<(double val){
	stream << val << std::endl;
	stream.flush();
	
	return *this;
}

PStream& PStream::operator<<(long double val){
	stream << val << std::endl;
	stream.flush();
	
	return *this;
}

PStream& PStream::operator<<(void* val){
	stream << val << std::endl;
	stream.flush();

	return *this;
}

void PStream::open(std::string& file){
    stream.open(file);
}

void PStream::flush(){
    stream.flush();
}

void PStream::close(){
    stream.close();
}
