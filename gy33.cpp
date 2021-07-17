
//          Copyright Marten Elsinga 2021 - 2023.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "gy33.hpp"
#include "hwlib.hpp"
#include <array>
#include <algorithm>

// De constructor voor klasse gy33
gy33::gy33(hwlib::i2c_bus & bus, const uint_fast8_t & address):
	bus( bus ),
	address( address )
	{}

// Met deze functie schrijven we naar de sensor. Door hoe hwlib is opgebouwd kunnen we maar 2x achter elkaar een .write op een i2cbus schrijven,
// dus moeten we eerst het i2c adres schrijven, dan het register waar we naartoe willen schrijven,
// dan opnieuw naar het i2c adres, en vervolgens de waarde die we er naartoe schrijven
void gy33::write(const uint8_t registr, const uint_fast8_t value){
	bus.write(0x29).write(registr);
	bus.write(0x29).write(value);
}


// Dit is de functie om de kleurensensor uit te lezen. Eerst maken we een uint data aan, waar we later naar schrijven
// Hoe de gy33 werkt, is dat we er eerst een register waarde naartoe moeten schrijven, om vervolgens de gegevens uit te lezen.
// Tegenstrijdig met wat je zou verwachten, moet je eerst de registerwaarde schrijven, dan op het i2c-adres nogmaals dezelfde registerwaarde.
uint8_t gy33::read(uint8_t registr){
	uint8_t data = 0x00;
	bus.write(registr);
	bus.write(0x29).write(registr);
	bus.read(0x29).read(data);
	return data;
}


// Deze functie geeft een array van uint8_t's terug met de mediaan van 10 metingen van elke kleur.
std::array<uint8_t, 4> gy33::median_rgbc(){
	//Eerst maken we 4 lijsten - 1 voor elke 'kleur'
	const unsigned int array_size = 10;
	std::array<uint8_t, array_size> clear;
	std::array<uint8_t, array_size> red;
	std::array<uint8_t, array_size> green;
	std::array<uint8_t, array_size> blue;
	//Vervolgens maken we 10 metingen per kleur
	for(unsigned int i = 0; i < array_size; i++){
		uint8_t clearlow = read(0xB4);
		uint8_t clearhigh = read(0xB5);
		//We combineren de meting van de low en high. Dit is omdat zowel de low als high een 8-bit teruggeven vanwege hoe het protocol werkt.
		//Echter, de sensor zit zo in elkaar dat de uiteindelijke gegevens een 16-bit byte is. Hierom combineren we de twee waarden.
		uint16_t cleartotal = ((uint16_t)clearhigh << 8) | clearlow;
		clear[i] = cleartotal;
		uint8_t redlow = read(0xB6);
		uint8_t redhigh = read(0xB7);
		uint16_t redtotal = ((uint16_t)redhigh << 8) | redlow;
		red[i] = redtotal;
		uint8_t greenlow = read(0xB8);
		uint8_t greenhigh = read(0xB9);
		uint16_t greentotal = ((uint16_t)greenhigh << 8) | greenlow;
		green[i] = greentotal;
		uint8_t bluelow = read(0xBA);
		uint8_t bluehigh = read(0xBB);
		uint16_t bluetotal = ((uint16_t)bluehigh << 8) | bluelow;
		blue[i] = bluetotal;
		hwlib::wait_us(2500);
	}
	//Als we dan 4 lijsten hebben kunnen we kijken naar de 5e waarde van elke lijst; de mediaan.
	//We zouden kunnen kijken naar de 5e en 6e en het gemiddelde daarvan nemen, 
	//maar uit mijn ervaringen met deze sensor tot nu toe is dat de enige uitschieters zijn
	//dat je alleen maar nullen terugkrijgt, of dat de waarde ineens veel te hoog is. Een te lage
	//waarde komt eigenlijk niet voor.
	
	//De mediaan waarden zetten we in een lijst met 4 waarden; r,g,b,c
	std::sort (red.begin(), red.end());
	std::sort (green.begin(), green.end());
	std::sort (blue.begin(), blue.end());
	std::sort (clear.begin(), clear.end());
	
	std::array<uint8_t, 4> medians_rgbc;
	medians_rgbc[0] = red[4];
	medians_rgbc[1] = green[4];
	medians_rgbc[2] = blue[4];
	medians_rgbc[3] = clear[4];
	
	//Dan nu wat tests om te kijken of alles klopt en werkt. Ze staan er nog gecomment in voor debugging redenen, als ze later ineens weer nodig zijn.
	//Commentaar wordt niet gecompileerd zo ver ik weet, dus het maakt niet uit dat het er staat.
//	for(unsigned int j = 0; j < 10; j++){
//		hwlib::cout << red[j] << " ";
//	}
//	hwlib::cout << "\n";
//	for(unsigned int j = 0; j < 10; j++){
//		hwlib::cout << green[j] << " ";
//	}
//	hwlib::cout << "\n";
//	for(unsigned int j = 0; j < 10; j++){
//		hwlib::cout << blue[j] << " ";
//	}
//	hwlib::cout << "\n";
//	for(unsigned int j = 0; j < 10; j++){
//		hwlib::cout << clear[j] << " ";
//	}
//	hwlib::cout << "\n";
//	for(unsigned int g = 0; g < 4; g++){
//		hwlib::cout << medians_rgbc[g] << " ";
//	}
//	hwlib::cout << "\n";
	return medians_rgbc;
}


void gy33::startup(uint8_t sensitivity){
	// Write PON naar enable register (Power ON)
	gy33::write(0x00, 0x01);
	hwlib::wait_us(2500);
	// write AEN naar enable register (Enable RGBC)
	gy33::write(0x00, 0x03);
	hwlib::wait_us(2500);
	// write AIEN naar enable register (Enable RGBC interrupts)
	gy33::write(0x00, 0x1B);
	hwlib::wait_us(2500);
	// zet de gain (sensitivity) 
	gy33::write(0x0F, sensitivity);
}
