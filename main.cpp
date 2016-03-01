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

#include <iostream>
#include "fl/Headers.h"


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


// Entry Point
int main()
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
	
	float CarX = 0.9f;
	float CarVX = -0.8f;
	float CarS = 0.0f;

	while (true)
	{
		// Evaluate FIS for given inputs
		carVelocity->setInputValue(CarVX);
		carPosition->setInputValue(CarX);
		fuzzyLiteEngine->process();
		carSteering->defuzzify();
		CarS = carSteering->getOutputValue();

		std::cout << "Car Position [VALUE]: " << CarX << std::endl;
		std::cout << "Car Position [FUZZY]: " << carPosition->fuzzify(CarX) << std::endl;

		std::cout << "Car Velocity [VALUE]: " << CarVX << std::endl;
		std::cout << "Car Velocity [FUZZY]: " << carVelocity->fuzzify(CarVX) << std::endl;

		std::cout << "Car Steering [VALUE]: " << CarS << std::endl;
		std::cout << "Car Steering [FUZZY]: " << carSteering->fuzzify(CarS) << std::endl;

		// Car movement calcs
		CarVX += CarS;
		CarX += CarVX;


	}
	
}