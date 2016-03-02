// main.cpp
//
// Author: J. Brown (1201717)
// Date: 01/03/2016
// Purpose: This is the application entry point for the AG1070A AI Coursework
// Detail: This application uses a Fuzzy Inference System to control the steering of a car in a racing game type environment, based on its
// position relative to a racing line, and its linear velocity in relation to that line.

// The Fuzzy Inference System used here was designed using MATLAB.
// The original file is available in "Fuzzy.fis".

// This project makes use of the FuzzyLite library for implementing and controlling Fuzzy Inference Systems.
// The library is available online at http://www.fuzzylite.com/cpp/ .
// The library was compiled using VS2015 with CMake. 
// This application was written in VS2015.

// This project makes use of the SFML framework for displaying basic graphics.
// SFML is available online at http://www.sfml-dev.org/

#include <iostream>
#include "fl/Headers.h"
#include "SFML/System.hpp"
#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"


// Variables for the simulation

// FuzzyLite Library Engine for Fuzzy Inference System
fl::Engine* fuzzyLiteEngine;

// FIS Inputs

// Input 1: Car Position Relative to Line
fl::InputVariable* carPosition;
// Input 2: Car Velocity Relative to Line
fl::InputVariable* carVelocity;

// FIS Outputs
// Output 1: Car Steering Factor
fl::OutputVariable* carSteering;

// FIS Rules
fl::RuleBlock* fuzzyRules;

// Application mode:
// 0 : "Game" Mode, which involves an interactively-movable racing line for the car to follow.
// This happens in the SFML window, with simultaneous console output.
// 1 : "Analysis" Mode, which allows the user to input the starting conditions of the fuzzy system and retrieve output.
// This happens solely within the console window.
int ApplicationMode = 0;

// Variables for the game's logic.
float GameCarPositionRelativeToLine = 0.0f;
float GameCarVelocityRelativeToLine = 0.0f;
float GameCarSteering = 0.0f;
float GameLinePosition = 0.0f;
bool MousePressed = false;

// Variables for SFML
sf::RenderWindow* appWindow;

// Variables for rendering shapes
sf::RectangleShape* carRect;
sf::RectangleShape* racingLine;

// Functions
void SetupSFMLWindow();
void SetupFuzzyInferenceSystem();
void SetupGFX();
void DoGameLogic();

// Entry Point
int main()
{
	// Set up application window
	SetupSFMLWindow();

	// Set up FIS
	SetupFuzzyInferenceSystem();

	// Set up graphics
	SetupGFX();

	// Main SFML processing loop
	while (appWindow->isOpen())
	{
		// Fetch input for SFML
		sf::Event windowEvent;
		while (appWindow->pollEvent(windowEvent))
		{
			// Close the window if it receives a close command.
			if (windowEvent.type == sf::Event::Closed)
			{
				appWindow->close();
			}
			if (windowEvent.type == sf::Event::KeyPressed)
			{
				if (windowEvent.key.code == sf::Keyboard::Space)
				{
					ApplicationMode = 1;
				}
			}

			// Mouse Input: Allow user to drag racing line around
			if (windowEvent.type == sf::Event::MouseButtonPressed)
			{
				MousePressed = true;
				racingLine->setPosition(sf::Vector2f(windowEvent.mouseButton.x, 0));
				
			}
			if (windowEvent.type == sf::Event::MouseButtonReleased)
			{
				MousePressed = false;
			}
			if (windowEvent.type == sf::Event::MouseMoved && MousePressed)
			{
				racingLine->setPosition(sf::Vector2f(windowEvent.mouseMove.x, 0));
			}

		}

		// Game logic; defuzzification is handled here.
		DoGameLogic();

		// SFML Window Rendering
		appWindow->clear();

		// Draw racing line & car
		appWindow->draw(*racingLine);
		appWindow->draw(*carRect);

		// Draw fuzzy system logic

		// Send to graphics card
		appWindow->display();




	}

	// Program ends
	return 0;
	
}


// This is where SFML is set up.
void SetupSFMLWindow()
{
	// Set up a 640x480 window.
	appWindow = new sf::RenderWindow(sf::VideoMode(640, 480), "AI Coursework (J. Brown, 1201717)");
	
	// Set framerate limit to 30FPS.
	appWindow->setFramerateLimit(30);
}

// This is where the FIS is set up.
void SetupFuzzyInferenceSystem()
{

	// First, setting up the FIS.
	fuzzyLiteEngine = new fl::Engine("FuzzyCar");
	fuzzyLiteEngine->setName("FuzzyCar");

	// Set up inputs. 
	// There will be 5 positions on each of the inputs.
	carPosition = new fl::InputVariable();
	carPosition->setEnabled(true);
	carPosition->setName("CarPosition");
	// Range is from -1.0 to 1.0; given that a car AI in racing game may be many units away, it is inferred that the value is normalised (probably by the width of the track) before being passed here.
	carPosition->setRange(-1.0, 1.0);
	// The MF's are triangular; this is a fairly simple shape to both conceptualize and code.
	// The MF's at the far left and far right edges have significant overlap. This allows the shape to fully encompass the maxima and minima of the range.
	carPosition->addTerm(new fl::Triangle("FarLeft", -2.000, -1.000, -0.400));
	carPosition->addTerm(new fl::Triangle("NearLeft", -0.800, -0.400, 0.000));
	carPosition->addTerm(new fl::Triangle("Neutral", -0.400, 0.000, 0.400));
	carPosition->addTerm(new fl::Triangle("NearRight", 0.000, 0.400, 0.800));
	carPosition->addTerm(new fl::Triangle("FarRight", 0.400, 1.000, 2.00));

	// The velocity input is much the same as the position input.
	carVelocity = new fl::InputVariable();
	carVelocity->setEnabled(true);
	carVelocity->setName("CarVelocity");
	carVelocity->setRange(-1.0, 1.0);
	carVelocity->addTerm(new fl::Triangle("MovingFastLeft", -2.000, -1.000, -0.400));
	carVelocity->addTerm(new fl::Triangle("MovingSlowLeft", -0.800, -0.400, 0.000));
	carVelocity->addTerm(new fl::Triangle("Neutral", -0.400, 0.000, 0.400));
	carVelocity->addTerm(new fl::Triangle("MovingSlowRight", 0.000, 0.400, 0.800));
	carVelocity->addTerm(new fl::Triangle("MovingFastRight", 0.400, 1.000, 2.00));

	// The output is a little more complex.
	carSteering = new fl::OutputVariable();
	carSteering->setEnabled(true);
	carSteering->setName("CarSteering");
	// The range of output is -1.0 to 1.0, like the inputs.
	// They signify the left/right turning angle of the steering wheel.
	carSteering->setRange(-1.0, 1.0);
	// The default value is 0; no alterating in steering at all.
	carSteering->setDefaultValue(0);
	// This prevents the output range going above or below the car's maxiumum turning angle.
	carSteering->setLockOutputValueInRange(true);

	// There are 7 outputs; this allows a more granular approach to the car's responses without increasing the number of inputs too much.
	carSteering->addTerm(new fl::Triangle("SteerFarLeft", -2.000, -1.000, -0.600));
	carSteering->addTerm(new fl::Triangle("SteerMediumLeft", -0.750, -0.500, -0.250));
	carSteering->addTerm(new fl::Triangle("SteerNearLeft", -0.400, -0.200, 0.000));
	carSteering->addTerm(new fl::Triangle("Neutral", -0.200, 0.000, 0.200));
	carSteering->addTerm(new fl::Triangle("SteerNearRight", 0.000, 0.200, 0.400));
	carSteering->addTerm(new fl::Triangle("SteerMediumRight", 0.250, 0.500, 0.750));
	carSteering->addTerm(new fl::Triangle("SteerFarRight", 0.600, 1.000, 1.200));


	// Add the inputs/outputs to the Engine.
	fuzzyLiteEngine->addInputVariable(carPosition);
	fuzzyLiteEngine->addInputVariable(carVelocity);
	fuzzyLiteEngine->addOutputVariable(carSteering);

	// Now, we add the rules to the engine.
	// FuzzyLite allows near-English grammatical parsing of rules.
	// The rules were designed on a Fuzzy Associative Map, accessible in the file Fuzzy_Grid.pdf
	// Because we have 5 positions on each input, we have 25 rules to implement.
	// Each rule corresponds to a square in Fuzzy_Grid, and they are ordered going from the top-left to the bottom-right, one row at a time.
	fuzzyRules = new fl::RuleBlock();

	try
	{
		//// Row 1: Moving Fast Left
		fuzzyRules->addRule(fl::Rule::parse("if CarPosition is FarLeft and CarVelocity is MovingFastLeft then CarSteering is SteerFarRight", fuzzyLiteEngine));
		fuzzyRules->addRule(fl::Rule::parse("if CarPosition is NearLeft and CarVelocity is MovingFastLeft then CarSteering is SteerMediumRight", fuzzyLiteEngine));
		fuzzyRules->addRule(fl::Rule::parse("if CarPosition is Neutral and CarVelocity is MovingFastLeft then CarSteering is SteerMediumRight", fuzzyLiteEngine));
		fuzzyRules->addRule(fl::Rule::parse("if CarPosition is NearRight and CarVelocity is MovingFastLeft then CarSteering is SteerNearRight", fuzzyLiteEngine));
		fuzzyRules->addRule(fl::Rule::parse("if CarPosition is FarRight and CarVelocity is MovingFastLeft then CarSteering is Neutral", fuzzyLiteEngine));
		// Row 2: Moving Slow Left
		fuzzyRules->addRule(fl::Rule::parse("if CarPosition is FarLeft and CarVelocity is MovingSlowLeft then CarSteering is SteerMediumRight", fuzzyLiteEngine));
		fuzzyRules->addRule(fl::Rule::parse("if CarPosition is NearLeft and CarVelocity is MovingSlowLeft then CarSteering is SteerNearRight", fuzzyLiteEngine));
		fuzzyRules->addRule(fl::Rule::parse("if CarPosition is Neutral and CarVelocity is MovingSlowLeft then CarSteering is SteerNearRight", fuzzyLiteEngine));
		fuzzyRules->addRule(fl::Rule::parse("if CarPosition is NearRight and CarVelocity is MovingSlowLeft then CarSteering is Neutral", fuzzyLiteEngine));
		fuzzyRules->addRule(fl::Rule::parse("if CarPosition is FarRight and CarVelocity is MovingSlowLeft then CarSteering is SteerNearLeft", fuzzyLiteEngine));
		// Row 3: Neutral
		fuzzyRules->addRule(fl::Rule::parse("if CarPosition is FarLeft and CarVelocity is Neutral then CarSteering is SteerMediumRight", fuzzyLiteEngine));
		fuzzyRules->addRule(fl::Rule::parse("if CarPosition is NearLeft and CarVelocity is Neutral then CarSteering is SteerNearRight", fuzzyLiteEngine));
		fuzzyRules->addRule(fl::Rule::parse("if CarPosition is Neutral and CarVelocity is Neutral then CarSteering is Neutral", fuzzyLiteEngine));
		fuzzyRules->addRule(fl::Rule::parse("if CarPosition is NearRight and CarVelocity is Neutral then CarSteering is SteerNearLeft", fuzzyLiteEngine));
		fuzzyRules->addRule(fl::Rule::parse("if CarPosition is FarRight and CarVelocity is Neutral then CarSteering is SteerMediumLeft", fuzzyLiteEngine));
		// Row 4: Moving Slow Right
		fuzzyRules->addRule(fl::Rule::parse("if CarPosition is FarLeft and CarVelocity is MovingSlowRight then CarSteering is SteerNearRight", fuzzyLiteEngine));
		fuzzyRules->addRule(fl::Rule::parse("if CarPosition is NearLeft and CarVelocity is MovingSlowRight then CarSteering is Neutral", fuzzyLiteEngine));
		fuzzyRules->addRule(fl::Rule::parse("if CarPosition is Neutral and CarVelocity is MovingSlowRight then CarSteering is SteerNearLeft", fuzzyLiteEngine));
		fuzzyRules->addRule(fl::Rule::parse("if CarPosition is NearRight and CarVelocity is MovingSlowRight then CarSteering is SteerNearLeft", fuzzyLiteEngine));
		fuzzyRules->addRule(fl::Rule::parse("if CarPosition is FarRight and CarVelocity is MovingSlowRight then CarSteering is SteerMediumLeft", fuzzyLiteEngine));
		// Row 5: Moving Fast Right
		fuzzyRules->addRule(fl::Rule::parse("if CarPosition is FarLeft and CarVelocity is MovingFastRight then CarSteering is Neutral", fuzzyLiteEngine));
		fuzzyRules->addRule(fl::Rule::parse("if CarPosition is NearLeft and CarVelocity is MovingFastRight then CarSteering is SteerNearLeft", fuzzyLiteEngine));
		fuzzyRules->addRule(fl::Rule::parse("if CarPosition is Neutral and CarVelocity is MovingFastRight then CarSteering is SteerMediumLeft", fuzzyLiteEngine));
		fuzzyRules->addRule(fl::Rule::parse("if CarPosition is NearRight and CarVelocity is MovingFastRight then CarSteering is SteerMediumLeft", fuzzyLiteEngine));
		fuzzyRules->addRule(fl::Rule::parse("if CarPosition is FarRight and CarVelocity is MovingFastRight then CarSteering is SteerFarLeft", fuzzyLiteEngine));
	}
	catch (fl::Exception& e)
	{
		// Exception handling for FuzzyLite; if the parser fails, it raises an exception.
		std::cout << "Couldn't add rules to Fuzzy Engine. Reason: " << e.getWhat() << std::endl;
	}



	// Apply rules to engine.
	fuzzyLiteEngine->addRuleBlock(fuzzyRules);

	// Configure Defuzzification methods.
	// Centroid defuzzification is appropriate for this FIS; generally, if the car is moving left and is to the left of the line, we'll want to steer rightwards by the combined amount.
	// Conjunction, Disjunction, Activation and Accumulation/Aggregation functions are simple Min/Max functions.
	fuzzyLiteEngine->configure("Minimum", "Maximum", "Minimum", "Maximum", "Centroid");


}

// This function sets up the game's graphics
void SetupGFX()
{
	carRect = new sf::RectangleShape(sf::Vector2f(16, 32));
	carRect->setFillColor(sf::Color::Red);
	carRect->setPosition(sf::Vector2f(640.0f / 2.0f, 480.0f / 2.0f));

	racingLine = new sf::RectangleShape(sf::Vector2f(4, 480));
	racingLine->setFillColor(sf::Color::White);
	racingLine->setPosition(sf::Vector2f(640.0f / 2.0f, 0.0f));
}

// This function handles the game's logic.

void DoGameLogic()
{

	if (ApplicationMode == 0) // "Game" Mode.
	{
		// If the line has moved, we need to update the car's relative position.
		GameCarPositionRelativeToLine = ((carRect->getPosition().x) - racingLine->getPosition().x) / 640.0f;

		// Now evaluate current fuzzy state.
		// Send current car input values to FIS.
		carVelocity->setInputValue(GameCarVelocityRelativeToLine);
		carPosition->setInputValue(GameCarPositionRelativeToLine);

		// Resolve the FIS
		fuzzyLiteEngine->process();

		// Retrieve output via defuzzification
		carSteering->defuzzify();

		// Set game object to use new value
		GameCarSteering = carSteering->getOutputValue();

		// Write values to console
		std::cout << "================================================================" << std::endl;
		std::cout << "Car Position [VALUE]: " << GameCarPositionRelativeToLine << std::endl;
		std::cout << "Car Position [FUZZY]: " << carPosition->fuzzify(GameCarPositionRelativeToLine) << std::endl;
		std::cout << std::endl;

		std::cout << "Car Velocity [VALUE]: " << GameCarVelocityRelativeToLine << std::endl;
		std::cout << "Car Velocity [FUZZY]: " << carVelocity->fuzzify(GameCarVelocityRelativeToLine) << std::endl;
		std::cout << std::endl;

		std::cout << "Car Steering [VALUE]: " << GameCarSteering << std::endl;
		std::cout << "Car Steering [FUZZY]: " << carSteering->fuzzify(GameCarSteering) << std::endl;
		std::cout << std::endl;

		// Car movement calculations:
		// Steering is a horizontal acceleration in this simplified case.
		// Velocity adds to position as with ordinary physics.
		GameCarVelocityRelativeToLine += GameCarSteering;
		GameCarPositionRelativeToLine += GameCarVelocityRelativeToLine;

		// Move graphics to new positions
		carRect->setPosition(sf::Vector2f((GameCarPositionRelativeToLine * 640.0f) + racingLine->getPosition().x, 480.0f / 2.0f));
	}
	else if(ApplicationMode == 1) // "Analysis" Mode.
	{
		float carSpeedValue, carPositionValue, carSteeringValue;
		int continuetype;
		std::cout << "------------------" << std::endl;
		std::cout << "Analysis Mode." << std::endl;
		std::cout << "Input Car's Position Relative to Line (-1.0 to 1.0): " << std::endl;
		std::cin >> carPositionValue;
		std::cout << "Input Car's Velocity Relative to Line (-1.0 to 1.0): " << std::endl;
		std::cin >> carSpeedValue;
		std::cout << std::endl << std::endl;
		
		// Solve fuzzy system.
		carVelocity->setInputValue(carSpeedValue);
		carPosition->setInputValue(carPositionValue);

		// Resolve the FIS
		fuzzyLiteEngine->process();

		// Retrieve output via defuzzification
		carSteering->defuzzify();
		carSteeringValue = carSteering->getOutputValue();

		// Display output
		std::cout << "Fuzzy Membership Values: " << std::endl;
		std::cout << "Position: " << carPosition->fuzzify(GameCarPositionRelativeToLine) << std::endl;
		std::cout << "Velocity: " << carVelocity->fuzzify(GameCarVelocityRelativeToLine) << std::endl << std::endl;
		std::cout << "Fuzzy Steering Output: " << carSteering->fuzzify(carSteeringValue) << std::endl << std::endl;
		
		std::cout << "Defuzzified Steering Output: " << carSteeringValue << std::endl;
		std::cout << std::endl;
		std::cout << "Type 1 to continue analysis, or type 0 to return to active mode." << std::endl;
		std::cin >> continuetype;
		if (continuetype == 1)
		{
			ApplicationMode = 1;
		}
		else
		{
			ApplicationMode = 0;
		}
	}
}