/*

MIT License

Copyright (c) 2017 FMI Open Development / Markus Peura, first.last@fmi.fi

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/
/*
Part of Rack development has been done in the BALTRAD projects part-financed
by the European Union (European Regional Development Fund and European
Neighbourhood Partnership Instrument, Baltic Sea Region Programme 2007-2013)
*/
#ifndef TEXTREADER_H_
#define TEXTREADER_H_

#include <iostream>
#include <sstream>
#include <list>
#include <string>



namespace drain
{

/// Utility for scanning text segments
/**
 *
 *
 */
class TextReader {

public:

	/// Read input stream until any char in \c endChars is encountered. The end char will not be included, but passed in input stream.
	/**
	 *  Typically, used for reading variable values, including nested segments, hence
	 *  recognizing closing chars.
	 *
	 */
	static inline
	std::string scanSegment(std::istream & istr, const std::string & endChars){
		std::stringstream sstr;
		scanSegment(istr, endChars, sstr);
		return sstr.str();
	}

	/// Read input stream until any char in \c endChars is encountered. The end char will not be included, but passed in input stream.
	/**
	 *  \return - \c true if end char was found; \c false at end-of-file
	 */
	static
	bool scanSegment(std::istream & istr, const std::string & endChars, std::ostream & ostr);

	//static	bool scanSegment(std::istream & istr, const std::string & endChars, std::strin & ostr);

	template <class T>
	static
	bool scanSegmentToValue(std::istream & istr, const std::string & endChars, T & dst);

	/// Read input stream until a char not in \c skipChars is encountered.
	static
	void skipChars(std::istream & istr, const std::string skipChars); //  = " \t\n\r"

	static inline
	void skipWhiteSpace(std::istream & istr){
		skipChars(istr, " \t\n\r");
	}

};

// Specified implementation
template <>
bool TextReader::scanSegmentToValue(std::istream & istr, const std::string & endChars, std::string & dst);

// Default implementation for basic (numeric) types
template <class T>
bool TextReader::scanSegmentToValue(std::istream & istr, const std::string & endChars, T & dst){
	std::stringstream sstr;
	bool result = scanSegment(istr, endChars, sstr);
	sstr >> dst;
	return result;
}



} // ::drain

#endif
