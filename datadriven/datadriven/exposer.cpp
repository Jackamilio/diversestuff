#include "exposing.h"


template<> TwType Exposer::getTwType<bool>()			{ return TW_TYPE_BOOLCPP; }
template<> TwType Exposer::getTwType<char>()			{ return TW_TYPE_INT8; }
template<> TwType Exposer::getTwType<unsigned char>()	{ return TW_TYPE_UINT8; }
template<> TwType Exposer::getTwType<short>()			{ return TW_TYPE_INT16; }
template<> TwType Exposer::getTwType<unsigned short>()	{ return TW_TYPE_UINT16; }
template<> TwType Exposer::getTwType<int>()				{ return TW_TYPE_INT32; }
template<> TwType Exposer::getTwType<unsigned int>()	{ return TW_TYPE_UINT32; }
template<> TwType Exposer::getTwType<float>()			{ return TW_TYPE_FLOAT; }
template<> TwType Exposer::getTwType<double>()			{ return TW_TYPE_DOUBLE; }
template<> TwType Exposer::getTwType<std::string>()		{ return TW_TYPE_STDSTRING; }
TwBar* exposureBar;
