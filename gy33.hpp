///@file

//          Copyright Marten Elsinga 2021 - 2023.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)


#ifndef GY33_HPP
#define GY33_HPP

#include "hwlib.hpp"
#include <array>
#include <algorithm>

class gy33{
private:
	hwlib::i2c_bus & bus;
	const uint_fast8_t & address;
public:
///\brief
///default constructor
///\details
///This constructor initialises the gy33 object
	gy33(hwlib::i2c_bus & bus, const uint_fast8_t & address);
///\brief
///Write to gy33
///\details
///This method writes to the gy33 sensor. It expects a uint8_t register, and a value to write to the register
	void write(const uint8_t registr, const uint_fast8_t value);
///\brief
///Reads from uint8_t register
///\details
///This method reads from uint8_t register. It returns a uint8_t containing data. Read from 0xB4 to 0xBB for desired results
	uint8_t read(uint8_t registr);
///\brief
///Reads all registers and returns medians
///\details
///This method reads all 8 registers containing color data. It returns a std::array containing 4 uint8_t's, ordered red/green/blue/clear
	std::array<uint8_t, 4> median_rgbc();
///\brief
///Starts up the sensor
///\details
///Call this method at the start of your code. Writes PowerON, enables all registers, and sets sensitivity value. Sensitivity options are 0x00, 0x01, 0x02 and 0x03 for 1x, 4x, 16x and 60x gain respectively.
	void startup(uint8_t sensitivity);
};

#endif //GY33_HPP