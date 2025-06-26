////////////////////////////////////////////////////////////////////////////////
//
//   CellGrid.cc
//
//   The definition of CellGrid class.
//
//   Authors: Hoyong Jeong (hoyong5419@korea.ac.kr)
//            Kyungmin Lee (  railroad@korea.ac.kr)
//
////////////////////////////////////////////////////////////////////////////////



//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "CellGrid.hh"
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <nlohmann/json.hpp>



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
CellGrid::CellGrid() : rows(16), cols(16), cells(rows * cols, false)
{
}


//------------------------------------------------
// Custom size
//------------------------------------------------
CellGrid::CellGrid(int r, int c) : rows(r), cols(c), cells(r * c, false)
{
}



//------------------------------------------------------------------------------
// Methods
//------------------------------------------------------------------------------
//------------------------------------------------
// Initialize
//------------------------------------------------
void CellGrid::Initialize(bool value)
{
	std::fill(cells . begin(), cells . end(), value);
}


//------------------------------------------------
// Get cell state
//------------------------------------------------
// By (row, column)
bool CellGrid::Get(int row, int col) const
{
	if ( !IsValidCoord(row, col) ) throw std::out_of_range("Invalid row or column");

	return cells[row * cols + col];
}

// By index
bool CellGrid::Get(int index) const
{
	if ( !IsValidIndex(index) ) throw std::out_of_range("Invalid index");

	return cells[index];
}


//------------------------------------------------
// Set cell state
//------------------------------------------------
// By (row, column)
void CellGrid::Set(int row, int col, bool value)
{
	if ( !IsValidCoord(row, col) ) throw std::out_of_range("Invalid row or column");

	cells[row * cols + col] = value;
}

// By index
void CellGrid::Set(int index, bool value)
{
	if ( !IsValidIndex(index) ) throw std::out_of_range("Invalid index");

	cells[index] = value;
}


//------------------------------------------------
// JSON handling
//------------------------------------------------
// Vector to JSON
std::string CellGrid::ToJSONString() const
{
	json j;
	j["rows"]  = rows;
	j["cols"]  = cols;
	j["cells"] = cells;

	return j . dump();
}

// JSON to vector
bool CellGrid::FromJSONString(const std::string& jsonStr)
{
	try {
		json j = json::parse(jsonStr);
		if (!j.contains("rows") || !j.contains("cols") || !j.contains("cells"))
			return false;

		int r = j["rows"];
		int c = j["cols"];
		std::vector<bool> newCells = j["cells"].get<std::vector<bool>>();
		if ((int)newCells.size() != r * c)
			return false;

		rows = r;
		cols = c;
		cells = std::move(newCells);
		return true;
	} catch (...) {
		return false;
	}
}


//------------------------------------------------
// Print
//------------------------------------------------
void CellGrid::Print(std::ostream& os) const
{
	for ( int i = 0; i < rows; ++i )
	{
		for ( int j = 0; j < cols; ++j )
		{
			os << (Get(i, j) ? "1" : "0");
		}
		os << '\n';
	}
}


//------------------------------------------------
// Inspectors
//------------------------------------------------
bool CellGrid::IsValidIndex(int index) const
{
	return index >= 0 && index < rows * cols;
}

bool CellGrid::IsValidCoord(int row, int col) const
{
	return row >= 0 && row < rows && col >= 0 && col < cols;
}
