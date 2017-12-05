#include "stdafx.h"
#include <iostream>
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <malloc.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "PerlinNoise.hpp"

#define SIZE 5			//5 - 10
//ako je 5 za 600*600 polje, onda increment value stavit na 0.04-0.05
#define PI 3.14159265359
#define COLOR_RANGE 768
#define END_GAME true
#define CONTINUE_GAME false
#define DOT -1
#define BACK -2

//Constants for text display
#define WINDOW_POS_CENTER	-1
#define WINDOW_POS_ZERO		-2
#define WINDOW_POS_END		-3

#define TEXT_OUTLINE_OFFSET 5

//Number of maximum entities
#define NO_ENTITIES 100000

//Number of initial entities that will be spawned
#define INITIAL_ENTITIES 10000
#define variousAccelerations false

typedef struct acc {
	double angle;
	double acce;
}acc;

typedef struct  color {
	uint8_t r;
	uint8_t g;
	uint8_t b;
}color;

bool arrowDisplay = false;
bool entityDisplay = true;
bool fpsPrint = false;
bool textOutlineEnabled = true;
bool clearScreenAfterOptions = false;

bool rainbowColours = true;

bool tracing = true;
bool fading = false;
//Some possible combinations:
//P=1:V=2.5:I=0.14		100000
//P=2:V=3:I=0.15		80000
//P=2:V=3.25:I=0.12		50000
int colourIncrement = 3;
int delay = 0;
SDL_Color colorFading = { 250,250,250,255 };
int PIX_SIZE = 2;			//1 -	2
double MAX_VEL = 3.25;		//2.25-	2.75-3.25
double MAX_ACC = 0.275;		//0.2-	.3	
double increment = 0.05;	//0.04- .12-.15
double tIncrement = 0.12;

const int WIDTH = 800;		//300-600
const int HEIGHT = 800;		//300-600
SDL_Texture	*arrow = NULL;

SDL_Texture	*targetTexure = NULL;
SDL_Texture	*targetTexureHelp = NULL;
SDL_Renderer *renderTarget = NULL;
SDL_Window *window = NULL;

SDL_Rect windowSize;
SDL_Rect sectorSize;

color white;
color black;
color background;
color use;

acc *acceleration = new acc[WIDTH*HEIGHT / SIZE / SIZE];

SDL_Texture *loadTexture(std::string filepath, SDL_Renderer *renderTarget) {
	SDL_Texture *texture = NULL;
	SDL_Surface *surface = SDL_LoadBMP(filepath.c_str());

	if (!surface)std::cout << "Error8:" << IMG_GetError() << std::endl;
	else {
		texture = SDL_CreateTextureFromSurface(renderTarget, surface);
		if (!texture)std::cout << "Error9:" << SDL_GetError() << std::endl;
	}
	SDL_FreeSurface(surface);
	surface = NULL;

	return texture;
}

class fieldPresentation {
public:
	//PODJELIT U PAR PODPROGRAMA (da bude organiziranije)
	void setVectorFieldPerlin() {
		int x = WIDTH / SIZE;
		int y = HEIGHT / SIZE;
		double cvX = currentValue;
		double cvY = currentValue + 100;
		double cvZ = z;
		for (int i = 0; i < y; i++) {
			for (int j = 0; j < x; j++) {

				if (notSet) {
					if (variousAccelerations)acceleration[i*x + j].acce = MAX_ACC * (pn.noise(cvX + distance, cvY + distance, cvZ) + 0.5);
					else acceleration[i*x + j].acce = MAX_ACC;
				}
				acceleration[i*x + j].angle = 360 * pn.noise(cvX, cvY, cvZ);

				if (arrowDisplay) {
					SDL_RenderCopyEx(renderTarget, arrow, &windowSize, &sectorSize, acceleration[i*x + j].angle, NULL, SDL_FLIP_NONE);
				}
				cvX += increment;
				sectorSize.x += SIZE;

			}
			cvX = currentValue;
			cvY += increment;
			sectorSize.x = 0;
			sectorSize.y += SIZE;
		}
		z += tIncrement / 50;
		if (notSet)notSet = false;
		sectorSize.x = 0;
		sectorSize.y = 0;
	}

private:
	bool notSet = true;

	double currentValue = rand();
	double z = 0;


	double distance = 500;

	int *arr = new int [WIDTH];
	siv::PerlinNoise pn;
};

class entity {
public:

	void spawn() {
		x = PIX_SIZE / 2 + (rand() % (WIDTH - PIX_SIZE));
		y = PIX_SIZE / 2 + (rand() % (HEIGHT - PIX_SIZE));
		pixelSize.x = (int)x - PIX_SIZE / 2;;
		pixelSize.y = (int)y - PIX_SIZE / 2;;
		pixelSize.h = PIX_SIZE;
		pixelSize.w = PIX_SIZE;
		SDL_SetRenderDrawColor(renderTarget, use.r, use.g, use.b, SDL_ALPHA_OPAQUE);
		SDL_RenderFillRect(renderTarget, &pixelSize);
		SDL_SetRenderDrawColor(renderTarget, background.r, background.g, background.b, SDL_ALPHA_OPAQUE);
	}

	void adjustPixelSize() {
		pixelSize.h = PIX_SIZE;
		pixelSize.w = PIX_SIZE;
	}

	void move() {
		changeXvel();
		changeYvel();

		x += X_vel;
		y += Y_vel;

		if (x < 0) x = WIDTH + x;
		if (x >= WIDTH) x = x - WIDTH;;
		if (y < 0) y = HEIGHT + y;
		if (y >= HEIGHT) y = y - HEIGHT;

		pixelSize.x = (int)x - PIX_SIZE / 2;
		pixelSize.y = (int)y - PIX_SIZE / 2;
		if (entityDisplay) {
			SDL_SetRenderDrawColor(renderTarget, use.r, use.g, use.b, SDL_ALPHA_OPAQUE);
			SDL_RenderFillRect(renderTarget, &pixelSize);
			SDL_SetRenderDrawColor(renderTarget, background.r, background.g, background.b, SDL_ALPHA_OPAQUE);
		}
	}

	void reset() {
		x = PIX_SIZE / 2 + (rand() % (WIDTH - PIX_SIZE));
		y = PIX_SIZE / 2 + (rand() % (HEIGHT - PIX_SIZE));
		X_vel = 0;
		Y_vel = 0;
	}

private:

	void changeXvel() {
		X_vel += getAcceleration()* cos(getAngle());
		if (X_vel > MAX_VEL)X_vel = MAX_VEL;
		if (X_vel < -MAX_VEL)X_vel = -MAX_VEL;
	}

	void changeYvel() {
		Y_vel += getAcceleration()* sin(getAngle());
		if (Y_vel > MAX_VEL)Y_vel = MAX_VEL;
		if (Y_vel < -MAX_VEL)Y_vel = -MAX_VEL;

	}

	double getAngle() {
		return (acceleration[(((int)y) / SIZE)*(WIDTH / SIZE) + (((int)x) / SIZE)].angle)*PI/180;
	}

	double getAcceleration() {
		return acceleration[(((int)y) / SIZE)*(WIDTH / SIZE) + (((int)x) / SIZE)].acce;
	}

	SDL_Rect pixelSize;
	double X_vel = 0;
	double Y_vel = 0;
	double x = 0;
	double y = 0;
};

class entityControl {

public:

	static void spawnXEntities(int *entitiesSpawned, entity *pixel, int numberToSpawn) {
		if (*entitiesSpawned + numberToSpawn > NO_ENTITIES) {
			std::cout << "Number exceeds maximal number of entities!\nEntities spawned:" << NO_ENTITIES - *entitiesSpawned << std::endl;
			numberToSpawn = NO_ENTITIES - *entitiesSpawned;
		}
		for (int i = 0; i < numberToSpawn; i++) {
			pixel[*entitiesSpawned].spawn();
			(*entitiesSpawned)++;
		}
	}

	static void killXEntities(int *entitiesSpawned, entity *pixel, int numberToKill) {
		if (*entitiesSpawned - numberToKill < 0) {
			std::cout << "Number is greater than the number of existing entities!\nEntities killed:" << *entitiesSpawned << std::endl;
			numberToKill = *entitiesSpawned;
		}
		for (int i = 0; i < numberToKill; i++) {
			pixel[--(*entitiesSpawned)].reset();
		}
	}

	static void colorSelect(color *use, int colour) {
		if (colour >= 0 && colour < COLOR_RANGE) {
			int cases = colour / (COLOR_RANGE / 6);
			switch (cases) {
			case 0:
				use->r = 255;
				use->b = 0;
				use->g = (int)((double)colour - COLOR_RANGE / 6 * cases) * 255 / (COLOR_RANGE / 6);
				break;
			case 1:
				use->g = 255;
				use->b = 0;
				use->r = 255 - (int)((double)colour - COLOR_RANGE / 6 * cases) * 255 / (COLOR_RANGE / 6);
				break;
			case 2:
				use->g = 255;
				use->r = 0;
				use->b = (int)((double)colour - COLOR_RANGE / 6 * cases) * 255 / (COLOR_RANGE / 6);
				break;
			case 3:
				use->b = 255;
				use->r = 0;
				use->g = 255 - (int)((double)colour - COLOR_RANGE / 6 * cases) * 255 / (COLOR_RANGE / 6);
				break;
			case 4:
				use->b = 255;
				use->g = 0;
				use->r = (int)((double)colour - COLOR_RANGE / 6 * cases) * 255 / (COLOR_RANGE / 6);
				break;
			case 5:
				use->r = 255;
				use->g = 0;
				use->b = 255 - (int)((double)colour - COLOR_RANGE / 6 * cases) * 255 / (COLOR_RANGE / 6);
			};
		}
		else std::cout << "Selected number is out of range" << std::endl;
	}
};

class textDisplay {
public:
	//Creates text texture from the input char array
	static SDL_Texture *createTextTexture(char *arr, TTF_Font *font, SDL_Color color, SDL_Rect *textSize, SDL_Texture *text) {
		//The function
		SDL_DestroyTexture(text);
		text = NULL;
		SDL_Surface *textSurface = NULL;
			
			textSurface = TTF_RenderText_Solid(font, arr, color);
				if (!textSurface)std::cout << "The text could not be rendered:" << TTF_GetError() << std::endl;
			
			text = SDL_CreateTextureFromSurface(renderTarget, textSurface);
				if (!text)std::cout << "The surface could not be converted:" << SDL_GetError() << std::endl;
			
				SDL_QueryTexture(text, NULL, NULL, &(textSize->w), &(textSize->h));
			
		SDL_FreeSurface(textSurface);
		
		return text;
	}

	//Does the same, just doesnt destroy the initial texture
	static SDL_Texture *createTextTexture_NoDestroy(const char *arr, TTF_Font *font, SDL_Color color, SDL_Rect *textSize) {
		//The function
		SDL_Texture *text = NULL;
		SDL_Surface *textSurface = NULL;

		textSurface = TTF_RenderText_Solid(font, arr, color);
		if (!textSurface)std::cout << "The text could not be rendered:" << TTF_GetError() << std::endl;

		text = SDL_CreateTextureFromSurface(renderTarget, textSurface);
		if (!text)std::cout << "The surface could not be converted:" << SDL_GetError() << std::endl;

		SDL_QueryTexture(text, NULL, NULL, &(textSize->w), &(textSize->h));

		SDL_FreeSurface(textSurface);

		return text;
	}

	//Makes a char array from integer
	static char *stringFromNumber(int number, char *string) {
		//Deletes the given string, allocates memory for the new string, and converts the number to a string whose pointer is returned as the outcome of the function
		if(string) delete[] string;

		int stringLength;		//cant use ceil because of the case that the number is 10(ceil would be 1, but the number has 2 digits)
		int lastDigit;
		bool negative=false;	//if the number is negative, the flag is set to true(in that case, in the end of the function, a '-' sign is put in the begining of the number)

		if (number == 0) stringLength = 1;
		else if (number > 0) stringLength = (int) floor(log10((double)number)) + 1;
		else {
			number = -number;
			stringLength =(int) floor(log10((double)number)) + 2;
			negative = true;
		}
			
			string = new char[stringLength+1];

			for (int i = stringLength-1; i >=0; i--) {
				lastDigit = number % 10;
				number /= 10;
				string[i] =(char) lastDigit + '0';
			}

				string[stringLength] = '\0';
				if (negative)string[0] = '-';

		return string;
	}

	//Please for your sake, do not look at this part of the code
	static char *stringFromNumber(double dNumber, char *string) {
		//Deletes the given string, allocates memory for the new string, and converts the number to a string whose pointer is returned as the outcome of the function
		if (string) delete[] string;

		int stringLength;		//cant use ceil because of the case that the number is 10(ceil would be 1, but the number has 2 digits)
		int lastDigit;
		bool negative = false;	//if the number is negative, the flag is set to true(in that case, in the end of the function, a '-' sign is put in the begining of the number)
		int number = (int)dNumber;
		int iniNumber = number;

		if (dNumber == 0) {
			string = new char[2];
			string[0] = '0';
			string[1] = '\0';
			return string;
		}
		else if (dNumber > 0) stringLength = (int)floor(log10(dNumber)) + 1 + 2 + 1;
		else {
			dNumber = -dNumber;
			stringLength = (int)floor(log10(dNumber)) + 2 + 2 + 1;
			negative = true;
		}

		if (dNumber < 1)stringLength = 4;

		string = new char[stringLength + 1];

		if (dNumber >= 1) {
			for (int i = stringLength - 1 - 2 - 1; i >= 0; i--) {
				lastDigit = number % 10;
				number /= 10;
				string[i] = (char)lastDigit + '0';
			}
		}
		else {
			if (negative)string[1] = '0';
			else string[0] = '0';
		}

		dNumber = dNumber - (double) iniNumber;
		dNumber *= 100;
		number = (int)dNumber;

		string[stringLength - 1 - 2] = '.';

		string[stringLength - 1 - 1] = number / 10 + '0';
		string[stringLength - 1 - 0] = number % 10 + '0';

		string[stringLength] = '\0';
		if (negative)string[0] = '-';

		return string;
	}

};

class options {
public:

	options(entity *pix, int *entitiesSpawnd) {
		gear = loadTexture("options.bmp", renderTarget);
		if (!gear) {
			std::cout << "Error 6:" << "Could not load texture" << std::endl;
		}
		SDL_QueryTexture(gear, NULL, NULL, &(gearSize.w), &(gearSize.h));
		gearSize.x = WIDTH - gearSize.w;
		gearSize.y = 0;

		pixel = pix;
		entitiesSpawned = entitiesSpawnd;

		setOptionsFont("OpenSans-Bold.ttf");
	}

	void displayGear() {
		SDL_RenderCopy(renderTarget, gear, &windowSize, &gearSize);
	}

	int getGearSize(char attribute) {
		if (attribute == 'x')return gearSize.x;
		else if (attribute == 'y')return gearSize.y;
		else if (attribute == 'h')return gearSize.h;
		else if (attribute == 'w')return gearSize.w;
		return NULL;
	}

	bool displayOptions(SDL_Texture *background) {
		char *numberString = new char[8];
		numberString = NULL;
		bool optionsOpen = true;
		SDL_Event optionsEvent;
		SDL_Rect optionsOutlines[15];
	
		//SETS THE RENDERTARGET AND BACKGROUND
		{
			SDL_SetRenderTarget(renderTarget, NULL);

			SDL_GetTextureColorMod(background, &(initialSettings.r), &(initialSettings.g), &(initialSettings.b));
			SDL_SetTextureColorMod(background, backgroundColorMod.r, backgroundColorMod.g, backgroundColorMod.b);
		}

		//The loop
		while (optionsOpen) {

			//Displays all the text
			{
				SDL_RenderCopy(renderTarget, background, &windowSize, &windowSize);

				displayText("OPTIONS", WINDOW_POS_CENTER, 60);
					
				displayText("Respawn entities:", WINDOW_POS_ZERO + TEXT_OUTLINE_OFFSET, 140);
					displayText("RESPAWN", WINDOW_POS_CENTER, 140, &(optionsOutlines[0]));
		
				displayText("Entity number:", WINDOW_POS_ZERO + TEXT_OUTLINE_OFFSET, 180);
					if (currentlyWriting != 1)displayText(textDisplay::stringFromNumber(*entitiesSpawned, numberString), WINDOW_POS_CENTER, 180, &(optionsOutlines[1]));
				
				displayText("Display vectors:", WINDOW_POS_ZERO + TEXT_OUTLINE_OFFSET, 220);
					displayToggle(arrowDisplay, WINDOW_POS_CENTER, 220, &(optionsOutlines[2]));
				
				displayText("Display entities:", WINDOW_POS_ZERO + TEXT_OUTLINE_OFFSET, 260);
					displayToggle(entityDisplay, WINDOW_POS_CENTER, 260, &(optionsOutlines[3]));
				
				displayText("Change pixel size:", WINDOW_POS_ZERO + TEXT_OUTLINE_OFFSET, 300);
					if (currentlyWriting != 4)displayText(textDisplay::stringFromNumber(PIX_SIZE, numberString), WINDOW_POS_CENTER, 300, &(optionsOutlines[4]));
				
				displayText("Colours change:", WINDOW_POS_ZERO + TEXT_OUTLINE_OFFSET, 340);
					displayToggle(rainbowColours, WINDOW_POS_CENTER, 340, &(optionsOutlines[5]));
				
				displayText("Colour change speed:", WINDOW_POS_ZERO + TEXT_OUTLINE_OFFSET, 380);
					if (currentlyWriting != 6)displayText(textDisplay::stringFromNumber(colourIncrement, numberString), WINDOW_POS_CENTER, 380, &(optionsOutlines[6]));
				
				displayText("Maximum velocity:", WINDOW_POS_ZERO + TEXT_OUTLINE_OFFSET, 420);
					if (currentlyWriting != 7)displayText(textDisplay::stringFromNumber(MAX_VEL, numberString), WINDOW_POS_CENTER, 420, &(optionsOutlines[7]));
				
				displayText("Perlin noise increment:", WINDOW_POS_ZERO + TEXT_OUTLINE_OFFSET, 460);
					if (currentlyWriting != 8)displayText(textDisplay::stringFromNumber(increment, numberString), WINDOW_POS_CENTER, 460, &(optionsOutlines[8]));
				
				displayText("Delay:", WINDOW_POS_ZERO + TEXT_OUTLINE_OFFSET, 500);
					if (currentlyWriting != 9)displayText(textDisplay::stringFromNumber(delay, numberString), WINDOW_POS_CENTER, 500, &(optionsOutlines[9]));
				
				displayText("Display FPS:", WINDOW_POS_ZERO + TEXT_OUTLINE_OFFSET, 540);
					displayToggle(fpsPrint, WINDOW_POS_CENTER, 540, &(optionsOutlines[10]));
				
				displayText("Clear the screen:", WINDOW_POS_ZERO + TEXT_OUTLINE_OFFSET, 580);
					displayText("CLEAR", WINDOW_POS_CENTER, 580, &(optionsOutlines[11]));
				
				displayText("Tracing enabled:", WINDOW_POS_ZERO + TEXT_OUTLINE_OFFSET, 620);
					displayToggle(tracing, WINDOW_POS_CENTER, 620, &(optionsOutlines[12]));
				
				displayText("Fading enabled:", WINDOW_POS_ZERO + TEXT_OUTLINE_OFFSET, 660);
					displayToggle(fading, WINDOW_POS_CENTER, 660, &(optionsOutlines[13]));
				
				displayText("Fading speed:", WINDOW_POS_ZERO + TEXT_OUTLINE_OFFSET, 700);
					if (currentlyWriting != 14)displayText(textDisplay::stringFromNumber(colorFading.r, numberString), WINDOW_POS_CENTER, 700, &(optionsOutlines[14]));
				
					if (currentlyWriting != -1)displayText(textDisplay::stringFromNumber(currentValue, numberString), WINDOW_POS_CENTER, 140 + 40 * currentlyWriting);

				SDL_RenderPresent(renderTarget);
			}

			//USED TO FETCH COMMANDS
			while (SDL_PollEvent(&optionsEvent)) {
				//Returns to the main program where it breaks the loop and exits
				if (optionsEvent.type == SDL_QUIT) {
					SDL_SetTextureColorMod(background, initialSettings.r, initialSettings.g, initialSettings.b);
					return END_GAME;
				}
				//Keyboard input
				else if (optionsEvent.type == SDL_KEYDOWN) {
					if (optionsEvent.key.keysym.sym == SDLK_0 && currentlyWriting != -1) {
						setCurrentValue(0);
					}
					else if (optionsEvent.key.keysym.sym == SDLK_1 && currentlyWriting != -1) {
						setCurrentValue(1);
					}
					else if (optionsEvent.key.keysym.sym == SDLK_2 && currentlyWriting != -1) {
						setCurrentValue(2);
					}
					else if (optionsEvent.key.keysym.sym == SDLK_3 && currentlyWriting != -1) {
						setCurrentValue(3);
					}
					else if (optionsEvent.key.keysym.sym == SDLK_4 && currentlyWriting != -1) {
						setCurrentValue(4);
					}
					else if (optionsEvent.key.keysym.sym == SDLK_5 && currentlyWriting != -1) {
						setCurrentValue(5);
					}
					else if (optionsEvent.key.keysym.sym == SDLK_6 && currentlyWriting != -1) {
						setCurrentValue(6);
					}
					else if (optionsEvent.key.keysym.sym == SDLK_7 && currentlyWriting != -1) {
						setCurrentValue(7);
					}
					else if (optionsEvent.key.keysym.sym == SDLK_8 && currentlyWriting != -1) {
						setCurrentValue(8);
					}
					else if (optionsEvent.key.keysym.sym == SDLK_9 && currentlyWriting != -1) {
						setCurrentValue(9);
					}
					else if (optionsEvent.key.keysym.sym == SDLK_RETURN && currentlyWriting != -1) {
						if (currentlyWriting == 1) {
							int delta =(int) currentValue - *entitiesSpawned;
							if (delta >= 0) {
								entityControl::spawnXEntities(entitiesSpawned, pixel, delta);
							}
							else {
								entityControl::killXEntities(entitiesSpawned, pixel, -delta);
							}
							*entitiesSpawned = (int) currentValue;
						}
						else if (currentlyWriting == 4) {
							PIX_SIZE = (int)currentValue;
							for (int i = 0; i < *entitiesSpawned; i++) {
								pixel[i].adjustPixelSize();
							}
						}
						else if (currentlyWriting == 6) {
							colourIncrement = (int)currentValue;
						}
						else if (currentlyWriting == 7) {
							MAX_VEL = currentValue;
						}
						else if (currentlyWriting == 8) {
							increment = currentValue;
						}
						else if (currentlyWriting == 9) {
							delay = (int)currentValue;
						}
						else if (currentlyWriting == 14) {
							colorFading.r = (int)currentValue;
							colorFading.g = (int)currentValue;
							colorFading.b = (int)currentValue;
						}
						currentlyWriting = -1;
					}
					else if (optionsEvent.key.keysym.sym == SDLK_COMMA && currentlyWriting != -1) {
						setCurrentValue(DOT);
					}
					else if (optionsEvent.key.keysym.sym == SDLK_PERIOD && currentlyWriting != -1) {
						setCurrentValue(DOT);
					}
					else if (optionsEvent.key.keysym.sym == SDLK_BACKSPACE && currentlyWriting != -1) {
						setCurrentValue(BACK);
					}
				}
				//Mouse input
				else if (optionsEvent.type == SDL_MOUSEBUTTONUP) {
					if (isInRect(optionsOutlines[0], optionsEvent)) {
						clearScreenAfterOptions = true;
						for (int i = 0; i < *(entitiesSpawned); i++) {
							pixel[i].reset();
						}
					}
					else if (isInRect(optionsOutlines[2], optionsEvent)) {
						arrowDisplay = !arrowDisplay;
						clearScreenAfterOptions = true;
					}
					else if (isInRect(optionsOutlines[3], optionsEvent)) {
						entityDisplay = !entityDisplay;
						clearScreenAfterOptions = true;
					}
					else if (isInRect(optionsOutlines[5], optionsEvent)) {
						rainbowColours = !rainbowColours;
					}
					else if (isInRect(optionsOutlines[10], optionsEvent)) {
						fpsPrint = !fpsPrint;
					}
					else if (isInRect(optionsOutlines[11], optionsEvent)) {
						clearScreenAfterOptions = true;
					}
					else if (isInRect(optionsOutlines[12], optionsEvent)) {
						tracing = !tracing;
					}
					else if (isInRect(optionsOutlines[13], optionsEvent)) {
						fading = !fading;
					}
					else if (isInRect(optionsOutlines[1], optionsEvent)) {
						currentValue = 0;
						currentlyWriting = 1;
					}
					else if (isInRect(optionsOutlines[4], optionsEvent)) {
						currentValue = 0;
						currentlyWriting = 4;
					}
					else if (isInRect(optionsOutlines[6], optionsEvent)) {
						currentValue = 0;
						currentlyWriting = 6;
					}
					else if (isInRect(optionsOutlines[7], optionsEvent)) {
						currentValue = 0;
						currentlyWriting = 7;
					}
					else if (isInRect(optionsOutlines[8], optionsEvent)) {
						currentValue = 0;
						currentlyWriting = 8;
					}
					else if (isInRect(optionsOutlines[9], optionsEvent)) {
						currentValue = 0;
						currentlyWriting = 9;
					}
					else if (isInRect(optionsOutlines[14], optionsEvent)) {
						currentValue = 0;
						currentlyWriting = 14;
					}
					else {
						currentlyWriting = -1;
						SDL_SetTextureColorMod(background, initialSettings.r, initialSettings.g, initialSettings.b);
						return CONTINUE_GAME;
					}
				}
			}

			SDL_RenderPresent(renderTarget);
		}

		return CONTINUE_GAME;
	}

private:

	//Adds a digit to the current value
	void setCurrentValue(int n) {
		double num = n;
		if (n != BACK) {
			if (n != DOT) {
				if (decimalPower == 0) {
					currentValue *= 10;
					currentValue += n;
				}
				else {
					if (decimalPower <= 2) {
						for (int i = 0; i < decimalPower; i++)num /= 10;
						decimalPower++;
						currentValue += num;
					}
				}
			}
			else {
				decimalPower = 1;
			}
		}
		else {
			if (decimalPower == 0) {
				currentValue /= 10;
				currentValue = floor(currentValue);
			}
			else {
				decimalPower--;
				for (int i = 0; i < decimalPower; i++) {
					currentValue *= 10;
				}
				currentValue /= 10;
				currentValue = floor(currentValue);
				for (int i = 0; i < decimalPower - 1; i++) {
					currentValue /= 10;
				}
			}
		}
	}

	//Returns a positive value if the event coordinates are inside the bounding box
	bool isInRect(SDL_Rect boundingBox, SDL_Event optionsEvent) {
		if (
			((optionsEvent.button.x) > (boundingBox.x)) &&
			((optionsEvent.button.y) > (boundingBox.y)) &&
			((optionsEvent.button.x) < (boundingBox.x + boundingBox.w)) &&
			((optionsEvent.button.y) < (boundingBox.y + boundingBox.h))
			)return true;
		else return false;
	}

	//DISPLAYS TEXT GIVEN BY THE CONSTANT CHAR ARRAY AT THE GIVEN X AND Y COORDINATES, ABILITY TO CENTER TEXT
	void displayText(const char *text, int x_pos = 0, int y_pos = 0, SDL_Rect *textSurface=NULL) {
		SDL_Texture *textTexture = NULL;
		SDL_Rect textOutline;
		textTexture = textDisplay::createTextTexture_NoDestroy(text, optionsFont, optionsColor, &textSize);
			
		//CHANGE THE FONT SIZE(IDK HOW BUT DO IT BUD)
		//Sets the values for some default positions
		{
			if (x_pos == WINDOW_POS_ZERO)x_pos = 0;
			else if (x_pos == WINDOW_POS_CENTER)x_pos = WIDTH / 2 - textSize.w / 2;
			else if (x_pos == WINDOW_POS_END)x_pos = WIDTH - textSize.w;

			if (y_pos == WINDOW_POS_ZERO)y_pos = 0;
			else if (y_pos == WINDOW_POS_CENTER)y_pos = HEIGHT / 2 - textSize.h / 2;
			else if (y_pos == WINDOW_POS_END)y_pos = HEIGHT - textSize.h;
		}

		//Set the x and y position
		textSize.x = x_pos;		
		textSize.y = y_pos;

		//Render a rect that will show the outline of the text
		if (textOutlineEnabled && textSurface) {
			
			//Set the SDL_Rect
			textOutline.x = x_pos - TEXT_OUTLINE_OFFSET;
			textOutline.y = y_pos;
			textOutline.w = textSize.w + 2 * TEXT_OUTLINE_OFFSET;
			textOutline.h = textSize.h;

			//Save and set colors
			SDL_GetRenderDrawColor(renderTarget, &(defaultColor.r), &(defaultColor.g), &(defaultColor.b), NULL);
			SDL_SetRenderDrawColor(renderTarget, textOutlineColor.r, textOutlineColor.g, textOutlineColor.b, SDL_ALPHA_OPAQUE);
			
			//Draw outline
			SDL_RenderDrawRect(renderTarget, &textOutline);
			
			//Restore the default colour
			SDL_SetRenderDrawColor(renderTarget, defaultColor.r, defaultColor.g, defaultColor.b, SDL_ALPHA_OPAQUE);
		}
		
		//Sets the rect where you can click on the text
		if (textSurface) {
			textSurface->x = x_pos;
			textSurface->y = y_pos;
			textSurface->w = textSize.w;
			textSurface->h = textSize.h;
		}

		SDL_RenderCopy(renderTarget, textTexture, &windowSize, &textSize);

		SDL_DestroyTexture(textTexture);
	}

	//Used for displaying bool values
	void displayToggle(bool value, int x = 0, int y = 0 , SDL_Rect *boundingBox = NULL) {
		if (value)displayText("true", x, y, boundingBox);
		else displayText("false", x, y, boundingBox);
	}

	//Sets the options font
	void setOptionsFont(const char *fontName) {
		optionsFont = TTF_OpenFont(fontName, 20);
	}

	double currentValue = 0;
	int currentlyWriting = -1;
	int decimalPower = 0;

	SDL_Texture *gear = NULL;
	SDL_Rect gearSize;
	SDL_Rect textSize;

	SDL_Color backgroundColorMod = { 75, 75, 75};
	SDL_Color optionsColor = { 255, 255, 255 };

	SDL_Color textOutlineColor = { 255, 255, 255 };
	SDL_Color defaultColor;

	SDL_Color initialSettings;

	TTF_Font *optionsFont = NULL;

	entity *pixel;
	int *entitiesSpawned;
};

int main(int argc, char *argv[]) {
	//#[Kookie]

	//Initialise the variables for the loop
	bool isRunning = true;
	int prevTime = 0;
	int ticks = 0;
	int currentColor = 0;

	int entitiesSpawned = 0;
	entity *pixel = new entity[NO_ENTITIES];
	
	SDL_Event	ev;
	fieldPresentation fp;

	TTF_Font *font = NULL;
	SDL_Surface *textSurface = NULL;
	SDL_Texture *text = NULL;
	SDL_Color textColor = { 255,255,255,255 };
	SDL_Rect textSize;

	SDL_Rect FPS_Size;

	srand((unsigned int)time(NULL));

	windowSize.x = windowSize.y = 0;
	windowSize.w = WIDTH;
	windowSize.h = HEIGHT;

	sectorSize.x = sectorSize.y = 0;
	sectorSize.w = sectorSize.h = SIZE;

	textSize.x = textSize.y = 0;

	use.r = 255;
	use.g = use.b = 0;

	background.r = background.g = background.b = 0;
	white.r = white.g = white.b = 255;
	black.r = black.g = black.b = 0;
	
	//Initialise all the things needed for SDL
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		std::cout << "Error 1:" << SDL_GetError() << std::endl;
		return EXIT_FAILURE;
	}
	else if (TTF_Init() < 0) {
		std::cout << "Error 6:" << TTF_GetError() << std::endl;
		return EXIT_FAILURE;
	}
	else {
		window = SDL_CreateWindow("FlowField", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_ALLOW_HIGHDPI);
		if (!window) {
			std::cout << "Error 2:" << SDL_GetError() << std::endl;
		}

		if (!(IMG_Init(IMG_INIT_JPG) & IMG_INIT_JPG))std::cout << "Error3:" << IMG_GetError() << std::endl;

		renderTarget = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
		
		if (!renderTarget) {
			std::cout << "Error 4:" << SDL_GetError() << std::endl;
		}

		arrow = loadTexture("arrow.bmp", renderTarget);
		if (!arrow) {
			std::cout << "Error 5:" << "Could not load texture" << std::endl;
		}

		targetTexure = SDL_CreateTexture(renderTarget, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, WIDTH, HEIGHT);
		targetTexureHelp = SDL_CreateTexture(renderTarget, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, WIDTH, HEIGHT);

		font = TTF_OpenFont("OpenSans-Bold.ttf", 20);

		SDL_SetRenderDrawColor(renderTarget, background.r, background.g, background.b, SDL_ALPHA_OPAQUE);
		SDL_RenderClear(renderTarget);
	}

	options op(pixel, &entitiesSpawned);

	//Spawn the initial entities
	for (int i = 0; i < INITIAL_ENTITIES; i++) {
		pixel[i].spawn();
		entitiesSpawned = INITIAL_ENTITIES;
	}
	
	//Game loop
	while (isRunning) {

		SDL_SetRenderTarget(renderTarget, targetTexure);

		//CLEARS THE window
		if (!tracing || clearScreenAfterOptions) {
			SDL_RenderClear(renderTarget);
			clearScreenAfterOptions = false;
		}
		//Used to display fading
		else if(fading){
			//Copies the faded version of the previous target texture to an additional texture
			SDL_SetRenderTarget(renderTarget, targetTexureHelp);
			SDL_SetTextureColorMod(targetTexure, (colorFading.r), (colorFading.g), (colorFading.b));
			SDL_RenderCopy(renderTarget, targetTexure, &windowSize, &windowSize);

			//Sets the target texture and copies the faded version on it, as a background
			SDL_SetTextureColorMod(targetTexure, (white.r), (white.g), (white.b));
			SDL_SetRenderTarget(renderTarget, targetTexure);
			SDL_RenderCopy(renderTarget, targetTexureHelp, &windowSize, &windowSize);
		}

		//USED TO FETCH COMMANDS
		while (SDL_PollEvent(&ev)) {
			if (ev.type == SDL_QUIT) {
				isRunning = false;
			}
			else if (ev.type == SDL_KEYDOWN) {
				if (ev.key.keysym.sym == SDLK_f) {
					std::cout << "Functions:" << std::endl;
					std::cout << "\t SPACE- Respawn entities" << std::endl;
					std::cout << "\t y-Spawn additional entities" << std::endl;
					std::cout << "\t x-Kill some entities" << std::endl;

					std::cout << "\t v-Vector Display" << std::endl;
					std::cout << "\t c-entityDisplay" << std::endl;

					std::cout << "\t a-Change pixel size" << std::endl;
					std::cout << "\t s-Enable colour change" << std::endl;
					std::cout << "\t d-Change the speed of the rainbow fade" << std::endl;

					std::cout << "\t w-Change maximum velocity" << std::endl;
					std::cout << "\t e-Change increment value" << std::endl;
					std::cout << "\t r-Change delay" << std::endl;

					std::cout << "\t q-Display FPS" << std::endl;
					std::cout << "\t TAB-Display number of entities" << std::endl;

					std::cout << "\t BACKSPACE-Clear the screen" << std::endl;
					std::cout << "\t 0-Enable trace" << std::endl;
					std::cout << "\t 9-Enable fading" << std::endl;
					std::cout << "\t 8-Change fading speed" << std::endl;
				}

				else if (ev.key.keysym.sym == SDLK_SPACE) {
					std::cout << "Entity positions reset!" << std::endl;
					for (int i = 0; i < entitiesSpawned; i++) {
						pixel[i].reset();
					}
				}
				else if (ev.key.keysym.sym == SDLK_y) {
					int n;
					std::cout << "Enter number of entities to be spawned:";
					std::cin >> n;
					entityControl::spawnXEntities(&entitiesSpawned, pixel, n);
				}
				else if (ev.key.keysym.sym == SDLK_x) {
					int n;
					std::cout << "Enter number of entities to be despawned:";
					std::cin >> n;
					entityControl::killXEntities(&entitiesSpawned, pixel, n);
				}

				else if (ev.key.keysym.sym == SDLK_v) {
					arrowDisplay = !arrowDisplay;
					if (arrowDisplay)std::cout << "Vector arrows enabled!" << std::endl;
					else std::cout << "Vector arrows disabled!" << std::endl;
					SDL_RenderClear(renderTarget);
				}
				else if (ev.key.keysym.sym == SDLK_c) {
					entityDisplay = !entityDisplay;
					if (entityDisplay)std::cout << "Entities visable!" << std::endl;
					else std::cout << "Entities invisible!" << std::endl;
					SDL_RenderClear(renderTarget);
				}

				else if (ev.key.keysym.sym == SDLK_a) {
					std::cout << "Enter new pixel size (Default=2):";
					std::cin >> PIX_SIZE;
					for (int i = 0; i < entitiesSpawned; i++) {
						pixel[i].adjustPixelSize();
					}
				}
				else if (ev.key.keysym.sym == SDLK_s) {
					rainbowColours = !rainbowColours;
					if (rainbowColours)std::cout << "Colour change enabled!" << std::endl;
					else std::cout << "Colour change disabled!" << std::endl;
				}
				else if (ev.key.keysym.sym == SDLK_d) {
					std::cout << "Enter colour increment value(Default=1)";
					int n;
					while (true) {
						std::cin >> n;
						if (n < 1 || n>20)std::cout << "Increment value not within boundaries(0<i<21)!\n Try again:";
						else break;
					}
					colourIncrement = n;
				}

				else if (ev.key.keysym.sym == SDLK_w) {
					std::cout << "Enter new maximal velocity (Default=2.75):";
					std::cin >> MAX_VEL;
				}
				else if (ev.key.keysym.sym == SDLK_e) {
					std::cout << "Enter new Perlin noise increment value (Default=0.15):";
					std::cin >> increment;
				}
				else if (ev.key.keysym.sym == SDLK_r) {
					std::cout << "Enter new delay (Default=0):";
					std::cin >> delay;
				}

				else if (ev.key.keysym.sym == SDLK_q) {
					fpsPrint = !fpsPrint;
					if (fpsPrint)std::cout << "FPS enabled!" << std::endl;
					else std::cout << "FPS disabled!" << std::endl;
				}
				else if (ev.key.keysym.sym == SDLK_TAB) {
					std::cout << "Number of entities:" << entitiesSpawned << std::endl;
				}

				else if (ev.key.keysym.sym == SDLK_0) {
					tracing = !tracing;
					if (tracing)std::cout << "Tracing enabled!" << std::endl;
					else {
						std::cout << "Tracing disabled!" << std::endl;
						SDL_RenderClear(renderTarget);
					}
				}
				else if (ev.key.keysym.sym == SDLK_9) {
					fading = !fading;
					if (fading)std::cout << "Fading enabled!" << std::endl;
					else {
						std::cout << "Fading disabled!" << std::endl;
						SDL_RenderClear(renderTarget);
					}
				}
				else if (ev.key.keysym.sym == SDLK_8) {
					int newValue;
					std::cout << "Enter new fading speed (Default=250):";
					std::cin >> newValue;
					colorFading.g = colorFading.b = colorFading.r = newValue;
				}
				else if (ev.key.keysym.sym == SDLK_BACKSPACE) {
					std::cout << "Screen cleared!"<< std::endl;
					SDL_RenderClear(renderTarget);
				}
			}
			else if (ev.type == SDL_MOUSEBUTTONUP) {
				if (ev.button.button == SDL_BUTTON_LEFT) {
					if (ev.button.x > WIDTH - op.getGearSize('w') && ev.button.y < op.getGearSize('h')) if (op.displayOptions(targetTexure))isRunning = false;
				}
			}
		}

			//UPDATES THE VECTORS (HERE SO IT GETS DISPLAYED IN THE BACK)
			fp.setVectorFieldPerlin();

			//USED TO CHANGE THE COLOR OF THE ENTITIES (ITS HERE SO THE COLOR CHANGES BEFORE THE ENTITIES ARE RENDERED)
			if (rainbowColours) {
				currentColor += colourIncrement;
				if (currentColor >= COLOR_RANGE)currentColor %=COLOR_RANGE;
				entityControl::colorSelect(&use, currentColor);
			}

			//MOVES THE ENTITIES
			for (int i = 0; i < entitiesSpawned; i++) {
				pixel[i].move();
			}

			//USED TO DISPLAY FPS (ITS HERE SO IT GETS DISPLAYED IN FRONT OF THE ENTITIES)
			if (prevTime != (int)time(NULL) && fpsPrint) {
				char *number = NULL;
				prevTime = (int)time(NULL);
				number = textDisplay::stringFromNumber(ticks, number);
				FPS_Size.x = 0;
				FPS_Size.y = 0;
				text = textDisplay::createTextTexture(number, font, textColor, &FPS_Size, text);
				ticks = 0;
			}
			if (fpsPrint) {
				ticks++;
				SDL_RenderCopy(renderTarget, text, &windowSize, &FPS_Size);
			}

		//DISPLAYS THE PRESENT RENDER TARGET, AND DELAYS THE OPERATION
		SDL_SetRenderTarget(renderTarget, NULL);
		SDL_RenderCopy(renderTarget, targetTexure, &windowSize, &windowSize);

		op.displayGear();

		SDL_RenderPresent(renderTarget);
		SDL_Delay(delay);
	}

	SDL_DestroyTexture(arrow);
	SDL_DestroyTexture(text);
	SDL_DestroyRenderer(renderTarget);
	SDL_DestroyWindow(window);
	
	text = NULL;
	textSurface = NULL;
	arrow = NULL;
	renderTarget = NULL;
	window = NULL;

	TTF_Quit();
	SDL_Quit();
	IMG_Quit();
	return EXIT_SUCCESS;
	//#[/Kookie]
}