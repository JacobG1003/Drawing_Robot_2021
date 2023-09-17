#include "PC_FileIO.c"

bool openFile(int index, TFileHandle & fin)
{
  string fileName = "drawing" + char('0' + index) + ".txt";
  openReadPC(fin, fileName);

  return fin != -1;
}

int navigateMenu()
{
	int returnvalue = 0;
	while(!getButtonPress(buttonAny)){}
	if(getButtonPress(buttonEnter))
		returnvalue = 0;
	else if(getButtonPress(buttonUp))
		returnvalue = -1;
	else if(getButtonPress(buttonDown))
		returnvalue = 1;
	else if(getButtonPress(buttonLeft))
		returnvalue = 10;
	else if (getButtonPress(buttonRight))
		returnvalue = -10;
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
	openReadPC(fin, fileName);
	bool okay = true;
	if(fin == -1)
		okay = false;
	if(okay)
		readTextPC(fin, name);
	closeFilePC(fin);

	return okay;
}

int selectMenu()
{
	displayTextLine(3, "Select your drawing");

	const int STARTLINE = 5;

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
			if(entered >= -1 && entered <= 1)
				location += entered;
			if(location < STARTLINE)
					location = index + STARTLINE;
			if (location > index + STARTLINE)
				location = STARTLINE;
		displayString(location, " >");
	}while (entered != 0);
	eraseDisplay();
	return location - STARTLINE - 1;
}

float scaleMenu(string name, float sizeX, float sizeY)
{
	float scale = 1;
	displayTextLine(3, "Drawing: %s", name);
	displayTextLine(4, "Press up/dn to inc/dec by 1%%");
	displayTextLine(5, "Press r/l to inc/dec by 10%%");
	int entered = 0;
	do
	{
		displayTextLine(7, "Current scale: %0.2f", scale);
		displayTextLine(8, "Current size: %0.2f x %0.2f", sizeX*scale, sizeY*scale);
		entered = navigateMenu();
		scale -= entered/100.0;
		if(scale < 0)
			scale = 0;
	} while(entered != 0);
	displayTextLine(5, "Select your drawing");
	eraseDisplay();
	return scale;
}

task main()
{
	int fileNum = selectMenu();
	if(fileNum != -1)
	{
		TFileHandle fin;

		char num = '0' + fileNum;
		string fileName = "drawing";
		fileName += num;
		fileName += ".txt";
		TFileHandle fin;
		openReadPC(fin, fileName);
		string drawingName;
		float sizeX = 0, sizeY = 0;
		readTextPC(fin, drawingName);
		readFloatPC(fin, sizeX);
		readFloatPC(fin, sizeY);

		scaleMenu(drawingName, sizeX, sizeY);
	}

}
