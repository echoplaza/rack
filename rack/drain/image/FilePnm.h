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
#ifndef FILE_PNM_H_
#define FILE_PNM_H_


#include <string>
#include <iostream>
#include <fstream>
#include <exception>


#include "util/RegExp.h"
#include "Image.h"


namespace drain
{
namespace image
{

// using namespace std;

/// For reading and writing images in PNM format
/** A lighter alternative for Magick++ which supports tens of image formats.
 *  Portable N M ? (PNM) format is an old yet flexible image format
 *  for gray level and color images.
 */
class FilePnm
{
public:

	enum FileType {
		UNDEFINED=0,
		PBM_ASC=1,
		PGM_ASC=2,
		PPM_ASC=3,
		PBM_RAW=4,
		PGM_RAW=5,
		PPM_RAW=6
	};

	/// Syntax for recognising png files.
	static
	const drain::RegExp fileNameRegExp;


	/// Reads a pnm file to an image.
	/**
	 *  Converts indexed (palette) images to RGB or RGBA.
	 *  Scales data to 8 or 16 bits, according to template class.
	 *  Floating point images will be scaled as 16 bit integral (unsigned short int).
	 *
	 *  In addition to reading to Image, supports reading to ImageFrame if type and geometry match.
	 *
	 *  \tparam T - Image or ImageFrame
	 */
	template <class T>
	static
	void read(T & image, const std::string & path);  //(PNG_TRANSFORM_PACKING || PNG_TRANSFORM_EXPAND));  16 >> 8?


	/// Writes image to a png file.
	/** Writes drain::Image to a png image file applying G,GA, RGB or RGBA color model.
	 *  Writes in 8 or 16 bits, according to template class.
	 *  Floating point images will be scaled as 16 bit integral (unsigned short int).
	 *
	 *  \param image - image
	 *  \param pathPrefix -
	 *
	 */
	static
	void write(const ImageFrame &image, const std::string &path);

	/// Writes image to a file, naming it: prefix + index + ".png", using desired number of leading zeros.
	/** Utility function esp. for debugging
	 *
	 *  \param image - image
	 *  \param pathPrefix - leading part of the path: directory and filename prefix.
	 *
	 *
	 */
	static void writeIndexed(const ImageFrame &image, const std::string & pathPrefix, int i=-1, int digits=3);

protected:


	/// Sets target type and geometry
	/**
	 *  In the case of ImageFrame (typically, an image channel), type and geometry have to match already;
	 *  otherwise an exception is thrown.
	 *
	 *  \tparam T - Image or ImageFrame
	 */
	template <class T>
	static
	void initialize(T &, const std::type_info & t, const Geometry & g);

	//void read(ImageFrame & image, std::istream & infile, FileType t);
	static
	void read(ImageFrame & image, std::istream & infile);

	// static
	//int index;


};


template <class T>
void FilePnm::read(T & image, const std::string & path) {

	drain::Logger mout(getImgLog(), __FILE__, __FUNCTION__);

	mout.info() << "path='" << path << "'" << mout.endl;

	std::ifstream infile;
	infile.open(path.c_str(), std::ios::in);

	if (!infile){
		mout.warn() << "opening file '" << path << "' failed" << mout.endl;
		return;
	}

	//std::string magic;
	//infile >> magic;

	if (infile.get() != 'P'){
		mout.warn() << "file does not start with  'P' (magic code)" << mout.endl;
		mout.error() << "not an PNM file" << mout.endl;
		return;
	}

	FileType pt = UNDEFINED;
	int width, height, channels, maxValue;

	char c = infile.get();

	switch (c){
	case '1':
		pt = PBM_ASC;
		channels = 1;
		break;
	case '2':
		pt = PGM_ASC;
		channels = 1;
		break;
	case '3':
		pt = PPM_ASC;
		channels = 3;
		break;
	case '4':
		pt = PBM_RAW;
		channels = 1;
		break;
	case '5':
		pt = PGM_RAW;
		channels = 1;
		break;
	case '6':
		pt = PPM_RAW;
		channels = 3;
		break;
	default:
		mout.error() << "unrecognized PPM type" << mout.endl;
	}

	mout.note() << "PNM type: P" <<  c << " (" << channels  << " channels)" << mout.endl;

	while ((c = infile.get()) != '\n'){
		// ?
	}

	//std::string
	while (infile.peek() == '#'){
		while ((c = infile.get()) !='\n' ){
			std::cout << c;
			if (infile.eof())
				mout.error() << "Premature end of file" << mout.endl;
		}
	}

	infile >> width;
	infile >> height;
	if ((pt != PBM_ASC) && (pt != PBM_RAW))
		infile >> maxValue;

	initialize(image, typeid(unsigned char), Geometry(width, height, channels));

	mout.debug() << image << mout.endl;

	read(image, infile);


}



}

}

#endif /*FILEPng_H_*/

// Drain
