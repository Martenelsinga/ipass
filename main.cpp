
//          Copyright Marten Elsinga 2021 - 2023.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "hwlib.hpp"
#include "gy33.hpp"
#include <array>
#include <stdlib.h>

// Deze functie maakt een array van ints, precies 10 lang. 
// Deze array gebruiken we als 'levels'. Elk getal van de array is 0, 1, of 2.
// Een 0 staat voor rood, 1 voor groen, 2 voor blauw.
// Elke keer dat je een sequentie goed hebt, moet je hem opnieuw doorgaan, maar dan 1 verder.
std::array<unsigned int, 10> generate(){
	std::array<unsigned int, 10> numbers;
	for(unsigned int i = 0; i < 10; i++){
		int randomnumber = rand() % 3;
		numbers[i] = randomnumber;
	}
	return numbers;
}


// Deze functie laat de lampjes 1 voor 1 oplichten, om zo de 'code' 
// aan de speler te geven. 
void lights(std::array<unsigned int, 10> numbers, unsigned int level){
	auto redled = hwlib::target::pin_out( hwlib::target::pins::d26 );
	auto greenled = hwlib::target::pin_out( hwlib::target::pins::d28 );
	auto blueled = hwlib::target::pin_out( hwlib::target::pins::d32 );
	for(unsigned int i = 0; i < level; i++){
		if(numbers[i] == 0){
			redled.write(1);
			greenled.write(0);
			blueled.write(0);
		}else if(numbers[i] == 1){
			redled.write(0);
			greenled.write(1);
			blueled.write(0);
		}else if(numbers[i] == 2){
			redled.write(0);
			greenled.write(0);
			blueled.write(1);
		}
		hwlib::wait_ms(500);
		redled.write(0);
		greenled.write(0);
		blueled.write(0);
		hwlib::wait_ms(250);
	}
}


// Dit is een heel primitieve manier om te kijken welke kleurwaarde hoger is. Er zijn absoluut betere opties,
// maar voor nu koos ik voor deze. Deze functie kijkt dus naar welke kleur het hoogst is, en geeft op basis daarvan een
// 0 voor rood, 1 voor groen, 2 voor blauw, 3 bij 2 of meer gelijke waarden, en 4 bij een error terug.
// Dit getal vergelijken we in functie spelletje() met de gevraagde waarde. 
unsigned int readcolors(){
	auto scl = hwlib::target::pin_oc( hwlib::target::pins::d21 );
	auto sda = hwlib::target::pin_oc( hwlib::target::pins::d20 );
	auto i2c = hwlib::i2c_bus_bit_banged_scl_sda( scl, sda );
	
	auto redled = hwlib::target::pin_out( hwlib::target::pins::d26 );
	auto greenled = hwlib::target::pin_out( hwlib::target::pins::d28 );
	auto blueled = hwlib::target::pin_out( hwlib::target::pins::d32 );
	
	gy33 gy(i2c, 0x29);
	gy.startup(0x02);
	
	std::array<uint8_t, 4> medianarray = gy.median_rgbc();
	// Reboot on error. Het rebooten doet niet enorm veel voor zover ik weet, en mijn handmatige reboots bestaan uit
	// ik die de draadjes loskoppelt om de sensor even opnieuw op te starten,
	// Maar ik vind het ook gek dat er een 'power off' bitje is die je kan schrijven die niet veel doet,
	// dus ik roep hem voor de zekerheid maar aan.
	if(medianarray[3] == 0 || medianarray[3] == 255){
		gy.write(0x00, 0x00);
		hwlib::wait_ms(250);
		gy.startup(0x02);
		redled.write(1);
		greenled.write(1);
		blueled.write(1);
		return 4;
	// Als blauw het hoogste is, schrijven we blauw naar de ledjes. Zo ook de andere else ifs hieronder.
	}else if(medianarray[2] > medianarray[0] && medianarray[2] > medianarray[1]){
		redled.write(0);
		greenled.write(0);
		blueled.write(1);
		return 2;
	}
	else if(medianarray[0] > medianarray[1] && medianarray[0] > medianarray[2]){
		redled.write(1);
		greenled.write(0);
		blueled.write(0);
		return 0;
	}else if(medianarray[1] > medianarray[0] && medianarray[1] > medianarray[2]){
		redled.write(0);
		greenled.write(1);
		blueled.write(0);
		return 1;
	// Deze else komt niet voor, maar heb ik er voor de zekerheid ingedaan, 
	// Als er somehow iets onvoorziend fout gaat.
	}else{
		redled.write(1);
		greenled.write(1);
		blueled.write(1);
		return 3;
	}
}


// In deze functie ga ik een voor een door de getallen van de array heen
// en kijk ik, zodra de knop ingedrukt is, of de waarde van de ingestopte kleur overeenkomt met
// de waarde die we willen hebben.
unsigned int spelletje(std::array<unsigned int, 10> numbers, unsigned int level){
	auto submitbutton = hwlib::target::pin_in( hwlib::target::pins::d24 );
	auto redled = hwlib::target::pin_out( hwlib::target::pins::d26 );
	auto greenled = hwlib::target::pin_out( hwlib::target::pins::d28 );
	auto blueled = hwlib::target::pin_out( hwlib::target::pins::d32 );
	unsigned int counter = 0;
	while(1){
		if(counter == level){
			return 1;
		}else{
			if(submitbutton.read() == 1){
				unsigned int kleuruitkomst = readcolors();
				if(kleuruitkomst == 3){
					redled.write(1);
					greenled.write(1);
					blueled.write(1);
					hwlib::wait_ms(50);
					redled.write(0);
					greenled.write(0);
					blueled.write(0);
				}else if(kleuruitkomst == 4){
					hwlib::cout << "error";
					return 3;
				}else if(kleuruitkomst == numbers[counter]){
					counter++;
					hwlib::wait_ms(250);
				}else{
					hwlib::wait_ms(250);
					return 0;
				}
				
			}
		}
	}
}

void kitt(unsigned int aantal){
	auto redled = hwlib::target::pin_out( hwlib::target::pins::d26 );
	auto greenled = hwlib::target::pin_out( hwlib::target::pins::d28 );
	auto blueled = hwlib::target::pin_out( hwlib::target::pins::d32 );
	for(unsigned int i = 0; i < aantal; i++){
		hwlib::wait_ms(50);
		redled.write(1);
		greenled.write(0);
		hwlib::wait_ms(50);
		redled.write(0);
		greenled.write(1);
		hwlib::wait_ms(50);
		greenled.write(0);
		blueled.write(1);
		hwlib::wait_ms(50);
		greenled.write(1);
		blueled.write(0);
		hwlib::wait_ms(50);
		redled.write(1);
		greenled.write(0);
	}
}


// Deze functie is de functie die de hele applicatie draait. 
// Het is een aparte functie van main() omdat hij opnieuw aangeroepen moet kunnen worden in geval van een win of verlies.
// De functie maakt gebruik van alle andere functies om het samen te voegen.
// Als we het spel meer of minder 'waves' willen geven, of de 'starting difficulty' willen aanpassen,
// kan dat door in de for-loop de start waarde van i aan te passen, of de eindwaarde van i voor meer waves.
unsigned int hetspel(){
	auto redled = hwlib::target::pin_out( hwlib::target::pins::d26 );
	auto greenled = hwlib::target::pin_out( hwlib::target::pins::d28 );
	auto blueled = hwlib::target::pin_out( hwlib::target::pins::d32 );
	auto port1 = hwlib::port_out_from(redled, greenled, blueled);
	std::array<unsigned int, 10> numbers = generate();
	lights(numbers, 1);
	for(unsigned int i = 1; i < 11; i++){
		unsigned int uitkomst = spelletje(numbers, i);
		if(uitkomst == 1){
			hwlib::cout << "Wave nummer " << i << " gehaald!\n";
			port1.write(0);
			kitt(5);
			port1.write(0);
			hwlib::wait_ms(500);
			lights(numbers, i + 1);
		}else if(uitkomst == 3){
			port1.write(1);
			hwlib::wait_ms(5000);
			return 3;
		}else if(uitkomst == 0){
			hwlib::cout << "Game over :(\n";
			hwlib::wait_ms(500);
			port1.write(7);
			hwlib::wait_ms(500);
			port1.write(0);
			hwlib::wait_ms(500);
			port1.write(7);
			hwlib::wait_ms(500);
			port1.write(0);
			hwlib::wait_ms(500);
			port1.write(7);
			hwlib::wait_ms(500);
			port1.write(0);
			hwlib::wait_ms(1000);
			return 0;
		}
	}
	hwlib::cout << "Je hebt het gedaan! Je won!\n";
	kitt(10);
	return 1;
}


int main( void ){
	// Dit volgende stuk, tot 'srand(hwlib::now_us());', is om de seed willekeurig te maken.
	// Zonder dit stuk code is elke aanroep van het programma hetzelfde, en krijg je elke keer
	// dezelfde string aan 10 getallen, wat er voor zorgt dat het spel zichzelf herhaalt,
	// en dan is het 'geheugen' gedeelte te makkelijk te bemeesteren. 
	// Hoe we de seed zetten is door een aanroep te maken naar hwlib::now_us(),
	// vervolgens te wachten op input van de speler, die op de zwarte knop moet drukken.
	// Als de zwarte knop is ingedrukt, kijken we nogmaals naar de tijd.
	// Waarom we het 2x doen is omdat hwlib::now_us() refereert naar de eerste keer dat die functie aangeroepen is,
	// en de eerste keer is het dus 0, als ik het goed begrijp. Doordat er nu wordt gewerkt met microseconden,
	// en de speler zelf input moet geven, is de kans op dezelfde seed ontzettend klein.
	auto startbutton = hwlib::target::pin_in( hwlib::target::pins::d30 );
	hwlib::now_us();
	bool started = 0;
	while(started == 0){
		if(startbutton.read() == 1){
			started = 1;
		}
	}
	srand(hwlib::now_us());
	
	// We wachten 1 seconde, om de speler tijd te geven om naar de lampjes te kijken, en starten vervolgens het spel op.
	// Als er een game-over is, wachten we 1.5 seconde en beginnen we opnieuw. De sequentie zal nu anders zijn.
	hwlib::wait_ms(1000);
	while(1){
		hetspel();
		hwlib::wait_ms(1500);
	}
	
	
}
