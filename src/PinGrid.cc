////////////////////////////////////////////////////////////////////////////////
//
//   PinGrid.cc
//
//   The definition of PinGrid class.
//
//   Authors: Hoyong Jeong (hoyong5419@korea.ac.kr)
//            Kyungmin Lee (  railroad@korea.ac.kr)
//            Changi Jeong (  jchg3876@korea.ac.kr)
//
////////////////////////////////////////////////////////////////////////////////



//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <nlohmann/json.hpp>

#include "PinGrid.hh"



//------------------------------------------------------------------------------
// JSON namespace
//------------------------------------------------------------------------------
using json = nlohmann::json;



//------------------------------------------------------------------------------
// Constructors and destructors
//------------------------------------------------------------------------------
//------------------------------------------------
// Default
//------------------------------------------------
PinGrid::PinGrid() : mRows(16), mCols(16), mPins(mRows * mCols, false)
{
}


//------------------------------------------------
// Custom size
//------------------------------------------------
PinGrid::PinGrid(unsigned short int r, unsigned short int c) : mRows(r), mCols(c), mPins(r * c, false)
{
}



//------------------------------------------------------------------------------
// Methods
//------------------------------------------------------------------------------
//------------------------------------------------
// Initialize
//------------------------------------------------
void PinGrid::Initialize(bool value)
{
	// Fill member vector pins with given value
	std::fill(mPins . begin(), mPins . end(), value);
}


//------------------------------------------------
// Get pin state
//------------------------------------------------
// By (row, column)
bool PinGrid::Get(unsigned short int row, unsigned short int col) const
{
	if ( !IsValidCoord(row, col) ) throw std::out_of_range("[kumtdd] PinGrid::Get: Invalid row or column");

	return mPins[row * mCols + col];
}

// By index
bool PinGrid::Get(unsigned short int index) const
{
	if ( !IsValidIndex(index) ) throw std::out_of_range("[kumtdd] PinGrid::Get: Invalid index");

	return mPins[index];
}


//------------------------------------------------
// Set pin state
//------------------------------------------------
// By (row, column)
void PinGrid::Set(unsigned short int row, unsigned short int col, bool value)
{
	if ( !IsValidCoord(row, col) ) throw std::out_of_range("[kumtdd] PinGrid::Set: invalid row or column");

	mPins[row * mCols + col] = value;
}

// By index
void PinGrid::Set(unsigned short int index, bool value)
{
	if ( !IsValidIndex(index) ) throw std::out_of_range("[kumtdd] PinGrid::Set: Invalid index");

	mPins[index] = value;
}


//------------------------------------------------
// JSON handling
//------------------------------------------------
// Vector to JSON
std::string PinGrid::ToJSONString() const
{
	json j;
	j["rows"] = mRows;
	j["cols"] = mCols;
	j["pins"] = mPins;

	return j . dump();
}

// JSON to vector
bool PinGrid::FromJSONString(const std::string& jsonStr)
{
	try
	{
		json j = json::parse(jsonStr);
		if ( !j . contains("rows") || !j . contains("cols") || !j . contains("pins") ) return false;

		unsigned short int r = j["rows"];
		unsigned short int c = j["cols"];
		std::vector<bool> newPins = j["pins"].get<std::vector<bool>>();
		if ( (unsigned short int) newPins . size() != r * c ) return false;

		mRows = r;
		mCols = c;
		mPins = std::move(newPins);
		return true;
	}
	catch ( ... )
	{
		return false;
	}
}


//------------------------------------------------
// Print
//------------------------------------------------
void PinGrid::Print(std::ostream& os) const
{
	for ( unsigned short int i = 0; i < mRows; i++ )
	{
		for ( unsigned short int j = 0; j < mCols; j++ )
		{
			os << (Get(i, j) ? "1" : "0");
		}
		os << '\n';
	}
}


//------------------------------------------------
// Inspectors
//------------------------------------------------
bool PinGrid::IsValidIndex(unsigned short int index) const
{
	return index >= 0 && index < mRows * mCols;
}

bool PinGrid::IsValidCoord(unsigned short int row, unsigned short int col) const
{
	return row >= 0 && row < mRows && col >= 0 && col < mCols;
}
