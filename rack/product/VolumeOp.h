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
/*
 * ProductOp.h
 *
 *  Created on: Mar 7, 2011
 *      Author: mpeura
 */

#ifndef RACKOP_H_
#define RACKOP_H_


//#include <drain/util/DataScaling.h>
//#include <drain/util/StringMapper.h>
//#include <drain/util/Variable.h>
//#include <drain/util/Tree.h>
#include <drain/util/ReferenceMap.h>



//#include "hi5/H52Image.h"
#include "main/rack.h"
#include "hi5/Hi5.h"
#include "data/ODIM.h"
#include "data/DataSelector.h"
#include "data/Data.h"
#include "data/Quantity.h" // NEW

#include "ProductOp.h" // NEW

namespace rack {

using namespace drain::image;

/// Base class for radar data processors.

/// Base class for radar data processors.
/** Input and output as HDF5 data, which has been converted to internal structure, drain::Tree<NodeH5>.
 *
 *  Basically, there are two kinds of polar processing
 *  - Cumulative: the volume is traversed, each sweep contributing to a single accumulation array, out of which the product layer(s) is extracted.
 *  - Sequential: each sweep generates new layer (/dataset) in the product; typically, the lowest only is applied.
 *
 *  TODO: Raise to RackOp
 */





/**
 *   \tparam M - ODIM type corresponding to products type (polar, vertical)
 */
// TODO: generalize for Cart
template <class M>
class VolumeOp : public ProductOp<const PolarODIM, M> {

public:

	VolumeOp(const std::string & name, const std::string &description="") : ProductOp<const PolarODIM, M>(name, description){
	};

	virtual inline
	~VolumeOp(){};


	/// Traverse through given volume and create new, processed data (volume or polar product).
	/**
	 *  This default implementation converts the volume to DataSetMap<PolarSrc>, creates an instance of
	 *  DataSet<DstType<M> >
	 *  and calls processDataSets().
	 */
	virtual
	void processVolume(const Hi5Tree &src, Hi5Tree &dst) const;



protected:



};




template <class M>
void VolumeOp<M>::processVolume(const Hi5Tree &src, Hi5Tree &dst) const {

	drain::Logger mout(__FUNCTION__, __FILE__); //REPL this->name+"(VolumeOp<M>)", __FUNCTION__);

	mout.debug() << "start" << mout.endl;
	mout.debug(2) << *this << mout.endl;
	mout.debug(1) << "DataSelector: "  << this->dataSelector << mout.endl;

	// Step 1: collect sweeps (/datasetN/)
	DataSetMap<PolarSrc> sweeps;

	/// Usually, the operator does not need groups sorted by elevation.
	mout.debug(2) << "collect the applicable paths"  << mout.endl;
	ODIMPathList dataPaths;  // Down to ../dataN/ level, eg. /dataset5/data4
	this->dataSelector.getPaths(src, dataPaths, ODIMPathElem::DATASET); // RE2

	if (dataPaths.empty()){
		mout.warn() << "no dataset's selected" << mout.endl;
	}

	mout.debug(2) << "populate the dataset map, paths=" << dataPaths.size() << mout.endl;
	drain::Variable elangles(typeid(double));
	for (ODIMPathList::const_iterator it = dataPaths.begin(); it != dataPaths.end(); ++it){

		mout.debug(2) << "elangles (this far> "  << elangles << mout.endl;

		//const std::string parent = DataTools::getParent(*it);
		const ODIMPath & parent = *it;
		//parent.pop_back();
		const Hi5Tree & srcDataSet = src(parent);
		const double elangle = srcDataSet["where"].data.attributes["elangle"];  // PATH

		//mout.debug(2) << "check elangle of "  << parent << mout.endl;
		mout.debug() << "testing " << parent << ", elangle=" << elangle << ':' << srcDataSet.data.dataSet << mout.endl;

		//mout.note() << "src dataset1/data2 QTY = " << srcDataSet["data2"]["data"].data.dataSet << mout.endl;

		if (sweeps.find(elangle) == sweeps.end()){
			mout.debug(2) << "add "  << elangle << ':'  << parent << " quantity RegExp:" << this->dataSelector.quantity << mout.endl;
			sweeps.insert(DataSetMap<PolarSrc>::value_type(elangle, DataSet<PolarSrc>(srcDataSet, drain::RegExp(this->dataSelector.quantity) )));  // Something like: sweeps[elangle] = src[parent] .
			elangles << elangle;
			//mout.warn() << "add " <<  DataSet<PolarSrc>(src(parent), drain::RegExp(this->dataSelector.quantity) ) << mout.endl;
		}
		else {
			mout.note() << "elange ="  << elangle << " already added, skipping " << parent << mout.endl;
		}
	}

	//mout.note() << "first elange =" << sweeps.begin()->first << " DS =" << sweeps.begin()->second << mout.endl;
	//mout.note() << "first qty =" << sweeps.begin()->second.begin()->first << " D =" << sweeps.begin()->second.getFirstData() << mout.endl;

	// Copy metadata from the input volume (note that dst may have been cleared above)
	dst["what"]  = src["what"];
	dst["where"] = src["where"];
	dst["how"]   = src["how"];
	dst["what"].data.attributes["object"] = this->odim.object;
	// odim.copyToRoot(dst); NO! Mainly overwrites original data. fgrep 'declare(rootAttribute' odim/*.cpp

	ODIMPathElem dataSetPath(ODIMPathElem::DATASET, 1);
	//if (!DataTools::getNextDescendant(dst, ProductBase::appendResults.getType(), dataSetPath))

	if (ProductBase::appendResults.is(ODIMPathElem::DATASET)){
		if (ProductBase::appendResults.getIndex()){
			dataSetPath.index = ProductBase::appendResults.getIndex();
		}
		else {
			DataSelector::getNextChild(dst, dataSetPath);
		}
	}
	//++dataSetPath.index;

	//mout.warn() << "FAILED: "  << dataSetPath << mout.endl;
		//dataSetPath.push_back(ODIMPathElem(ODIMPathElem::DATASET, 1));


	mout.debug() << "storing product in path: "  << dataSetPath << mout.endl;

	Hi5Tree & dstProduct = dst[dataSetPath];
	DataSet<DstType<M> > dstProductDataset(dstProduct); // PATH

	drain::VariableMap & how = dstProduct["how"].data.attributes;

	/// Main operation
	this->processDataSets(sweeps, dstProductDataset);

	//
	how["software"] = __RACK__;
	how["sw_version"] = __RACK_VERSION__;
	how["elangles"] = elangles;  // This service could be lower in hierarchy (but for PseudoRHI and pCappi ok here)

	//drain::VariableMap & what = dst[dataSetPath]["what"].data.attributes;
	//what["source"] = src["what"].data.attributes["source"];
	//what["object"] = odim.object;

}


}  // namespace rack


#endif /* RACKOP_H_ */

// Rack
 // REP // REP
 // REP // REP // REP // REP
