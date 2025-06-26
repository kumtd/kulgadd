////////////////////////////////////////////////////////////////////////////////
//
//   PinGrid.hh
//
//   This class stores the state of each switch pin.
//
//   Authors: Hoyong Jeong (hoyong5419@korea.ac.kr)
//            Kyungmin Lee (  railroad@korea.ac.kr)
//            Changi Jeong (  jchg3876@korea.ac.kr)
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
class PinGrid
{
	public:
	//------------------------------------------------
	// Constructors and destructors
	//------------------------------------------------
	// Default constructor: 16 by 16
	PinGrid();

	// Custom size
	PinGrid(unsigned short int rows, unsigned short int cols);


	//------------------------------------------------
	// Methods
	//------------------------------------------------
	// Init
	void Initialize(bool value = false);

	// Get pin state
	bool Get(unsigned short int index)                       const;
	bool Get(unsigned short int row, unsigned short int col) const;

	// Set pin state
	void Set(unsigned short int index, bool value);
	void Set(unsigned short int row, unsigned short int col, bool value);

	// JSON handling
	std::string ToJSONString() const;
	bool FromJSONString(const std::string& json);

	void Print(std::ostream& os = std::cout) const;

	// Get grid size
	unsigned short int GetRows()  const { return mRows;         }
	unsigned short int GetCols()  const { return mCols;         }
	unsigned short int GetTotal() const { return mRows * mCols; }


	private:
	unsigned short int mRows;
	unsigned short int mCols;
	std::vector<bool> mPins;

	// Inspectors
	bool IsValidCoord(unsigned short int row, unsigned short int col) const;
	bool IsValidIndex(unsigned short int index)                       const;
};
