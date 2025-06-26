////////////////////////////////////////////////////////////////////////////////
//
//   CellGrid.hh
//
//   This class stores the state of each LGAD cell.
//
//   Authors: Hoyong Jeong (hoyong5419@korea.ac.kr)
//            Kyungmin Lee (  railroad@korea.ac.kr)
//
////////////////////////////////////////////////////////////////////////////////



#pragma once



//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include <vector>
#include <string>
#include <iostream>



//------------------------------------------------------------------------------
// Class declaration
//------------------------------------------------------------------------------
class CellGrid
{
public:
	//------------------------------------------------
	// Constructors and destructors
	//------------------------------------------------
	// Default constructor: 16x16
	CellGrid();

	// Custom size
	CellGrid(int rows, int cols);


	//------------------------------------------------
	// Methods
	//------------------------------------------------
	// Init
	void Initialize(bool value = false);

	// Get cell state
	bool Get(int row, int col) const;
	bool Get(int index) const;

	// Set cell state
	void Set(int row, int col, bool value);
	void Set(int index, bool value);

	// JSON handling
	std::string ToJSONString() const;
	bool FromJSONString(const std::string& json);

	void Print(std::ostream& os = std::cout) const;

	int GetRows()  const { return rows;        }
	int GetCols()  const { return cols;        }
	int GetTotal() const { return rows * cols; }


private:
	int rows;
	int cols;
	std::vector<bool> cells;

	// Inspectors
	bool IsValidIndex(int index) const;
	bool IsValidCoord(int row, int col) const;
};
