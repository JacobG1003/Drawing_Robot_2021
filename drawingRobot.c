#include "PC_FileIO.c"
const float ANGLE_TOLERANCE = 1;
const tSensors GYRO_SENSOR_PORT = S2;
const int ENTER =0;
const int UP = -1;
const int DOWN = 1;
const int LEFT = 10;
const int RIGHT = -10;
const float ENCODER_CONVERSION = (PI * 2.8) / 180.0;

bool openFile(int index, TFileHandle & fin)
{
	string fileName = "drawing";
	char num = '0' + index;
	fileName += num;
	fileName += ".txt";
	openReadPC(fin, fileName);

	return fin != -1;
}

void readXY(TFileHandle & fin, float & x, float & y)
{
	readFloatPC(fin, x);
	readFloatPC(fin, y);
}

void readXYM(TFileHandle & fin, float & x, float & y, bool & marker)
{
	readXY(fin, x, y);

	int markerNum = 0;
	readIntPC(fin, markerNum);
	marker = (bool)markerNum;
}



//Navigate menu is used to interpret button presses
//Returns value assosiated with button press
int navigateMenu()
{
	int returnvalue = 0;
	while(!getButtonPress(buttonAny)){}
	if(getButtonPress(buttonEnter))
		returnvalue = ENTER;
	else if(getButtonPress(buttonUp))
		returnvalue = UP;
	else if(getButtonPress(buttonDown))
		returnvalue = DOWN;
	else if(getButtonPress(buttonLeft))
		returnvalue = LEFT;
	else if (getButtonPress(buttonRight))
		returnvalue = RIGHT;
	while(getButtonPress(buttonAny)){}
	return returnvalue;
}

bool readTitle(int index, string & name)
{
	char num = '0' + index;
	string fileName = "drawing";
	fileName += num;
	fileName += ".txt";
	TFileHandle fin;
	bool success = openFile(index, fin);

	if(success)
		readTextPC(fin, name);

	closeFilePC(fin);
	return success;
}

//select menu displays all of the files on the robot
//uses navigateMenu to select on one them then returns the number with it
int selectMenu()
{
	const int STARTLINE = 5;
	displayTextLine(STARTLINE-2, "Select your drawing");

	displayTextLine(STARTLINE, "  EXIT");

	int index = 0;
	string title;
	while(readTitle(index, title))
	{
		displayTextLine(index + STARTLINE + 1, "  %s", title);
		index++;
	}
	int entered = 0, location = STARTLINE;
	displayString(STARTLINE, " >");
	do
	{
		entered = navigateMenu();
		displayString(location, "  ");
		if(entered == UP || entered == DOWN)
			location += entered;
		if(location < STARTLINE)
			location = index + STARTLINE;
		if (location > index + STARTLINE)
			location = STARTLINE;
		displayString(location, " >");
	}while (entered != ENTER);
	eraseDisplay();
	return location - STARTLINE - 1;
}

//Takes in the name and the sizes of the selected file
//Uses navigateMenu to allow the user to change how large the menu will be
//Returns the value that the user chose to scale the menu by
float scaleMenu(string name, float sizeX, float sizeY)
{
	const int STARTLINE = 3;
	const float SCALEENTER = 10.0;
	float scale = 1;
	displayTextLine(STARTLINE, "Drawing: %s", name);
	displayTextLine(STARTLINE+1, "Press up/dn to inc/dec by 10%%");
	displayTextLine(STARTLINE+2, "Press r/l to inc/dec by 100%%");
	int entered = 0;
	do
	{
		displayTextLine(STARTLINE+4, "Current scale: %0.1f", scale);
		displayTextLine(STARTLINE+5, "Current size: %0.2f x %0.2f", sizeX*scale, sizeY*scale);
		entered = navigateMenu();
		scale -= entered/SCALEENTER;
		if(scale < 0)
			scale = 0;
	}
	while(entered != ENTER);
	eraseDisplay();
	return scale;
}

void setupSensor(tSensors gyroPort)
{
	SensorType[gyroPort] = sensorEV3_Gyro;
	SensorMode[gyroPort] = modeEV3Gyro_Calibration;
	SensorMode[gyroPort] = modeEV3Gyro_RateAndAngle;
	wait1Msec(50);
}

void drive(int leftMotorPower, int rightMotorPower)
{
	motor[motorA] = leftMotorPower;
	motor[motorD] = rightMotorPower;
}

float getAngleEncoder()
{
	const float WHEEL_DISTANCE = 12.05;
	const float ERROR_MULTIPLIER = 1.185;
	const float CONVERT = 360.0 / PI;
	float arc = (nMotorEncoder[motorD] - nMotorEncoder[motorA]) * ENCODER_CONVERSION / 2.0;
	return (arc / WHEEL_DISTANCE) * (CONVERT) * ERROR_MULTIPLIER;
}

void turn(float angle)
{
	drive(0, 0);

	if(angle > 180)
		angle = angle - 360;

	if(angle < -180)
		angle = angle + 360;

	nMotorEncoder[motorA] = 0;
	nMotorEncoder[motorD] = 0;

	float error = angle;
	while (fabs(error) > ANGLE_TOLERANCE)
	{
		error = angle - getAngleEncoder();
		if (error > 0)
			drive(-10, 10);
		else
			drive(10, -10);
	}
	drive(0,0);
}

void driveDist(float distanceCM, int motorPower, bool useAngle)
{
	float encoderLimit = distanceCM / ENCODER_CONVERSION;
	nMotorEncoder[motorA] = 0;

	resetGyro(GYRO_SENSOR_PORT);

	while (fabs(nMotorEncoder[motorA]) < fabs(encoderLimit))
	{
		float offset = 0;
		float angle = getGyroDegrees(GYRO_SENSOR_PORT);
		if (useAngle && fabs(angle) > 10.0)
			offset = angle;

		drive(motorPower + offset, motorPower - offset);
	}

	drive(0,0);
}

void toggleMarker(bool & currentState)
{
	nMotorEncoder[motorC] = 0;
	if(!currentState)
	{
		float prevValue = 0;
		motor[motorC] = -10;
		do
		{
			prevValue = nMotorEncoder[motorC];
			wait1Msec(500);
		}
		while (fabs(prevValue - nMotorEncoder[motorC]) < 1);
	}
	else
	{
		motor[motorC] = 40;
		wait1Msec(100);
	}
	currentState = !currentState;

	motor[motorC] = 0;
}

void updatePosition(float x, float y, float &prevX, float &prevY, float &angle, bool useAngle)
{
	float expectedAngle = atan2((float)(x - prevX), y - prevY) * (180.0 / PI);
	float distance = sqrt(pow((float)(x - prevX), 2) + pow(y - prevY, 2));

	turn(expectedAngle - angle);

	driveDist(distance, 20, useAngle);

	prevX = x;
	prevY = y;

	angle = expectedAngle;
}

task main()
{
	bool markerState = false;
	int fileNum = selectMenu();
	while(fileNum != -1)
	{
		setupSensor(GYRO_SENSOR_PORT);

		bool correctAngle = fileNum == 0 || fileNum == 6;

		float prevX = 0, prevY = 0, angle = 0;
		TFileHandle fin;
		openFile(fileNum, fin);

		string drawingName;
		readTextPC(fin, drawingName);

		float sizeX = 0, sizeY = 0;
		readXY(fin, sizeX, sizeY);

		int size = 0;
		readIntPC(fin, size);

		float scaleFactor = scaleMenu(drawingName, sizeX,sizeY);
		int startTime = time1[T1];
		for (int count = 0; count < size; count++)
		{
			float curX = 0, curY = 0;
			bool markerDown = markerState;

			readXYM(fin, curX, curY, markerDown);

			if (count != 0 && markerDown != markerState)
				toggleMarker(markerState);

			curX *= scaleFactor;
			curY *= scaleFactor;

			updatePosition(curX, curY, prevX, prevY, angle, correctAngle);
			wait1Msec(500);
		}
		toggleMarker(markerState);

		displayTextLine(3, "The drawing took %d seconds", (time1[T1]-startTime)/1000);
		displayTextLine(4, "Press any button to continue");
		while(!getButtonPress(buttonAny)){}
		while(getButtonPress(buttonAny)){}
		eraseDisplay();
		fileNum = selectMenu();
	}
}
