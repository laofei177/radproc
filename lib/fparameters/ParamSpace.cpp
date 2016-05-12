#include "ParamSpace.h"

#include "SpaceIterator.h"
#include "Dimension.h"
#include "SpaceCoord.h"

//#include <fmath/interpolation.h>
//#include <iterator>
//#include <iostream>
//#include <iomanip>
//#include <stdexcept>
//#include <sstream>


void ParamSpace::updateDerivations(const SpaceIterator& i) const {
	for (auto d : derivations) {
		d(i);
	}
}

void ParamSpace::iterate(Parameters& p, std::function<void(const SpaceIterator&)> body, std::initializer_list<int> fixedDimensions) const
{
	SpaceIterator it(*this,p,fixedDimensions);
	while (it.next(&p)) {
		updateDerivations(it);
		body(it);
	}
}

void ParamSpace::iterate(std::function<void(const SpaceIterator&)> body, std::initializer_list<int> fixedDimensions) const
{
	iterate(Parameters(parameters), body, fixedDimensions);
}

void ParamSpace::add(Dimension* dim)
{
	dimensions.push_back(dim);
}

void ParamSpace::addDerivation( const DerivationFun& f )
{
	derivations.push_back(f);
}

size_t ParamSpace::size() const
{
	size_t N = 1;
	for (auto v : dimensions) { N *= v->size(); }
	return N;
}

size_t ParamSpace::dim2ix(const SpaceCoord& coord) const
{
	size_t ix = coord[0];
	size_t o = dimensions[0]->size();
	for (size_t i = 1; i < coord.dims.size(); ++i) {
		ix += coord[i] * o;
		o *= dimensions[i]->size();
	}
	return ix;
}

void ParamSpace::ix2dim(int ix, SpaceCoord& si) const
{
	for (size_t i = 0; i < dimensions.size(); ++i) {
		si[i] = ix % dimensions[i]->size();
	}
}
