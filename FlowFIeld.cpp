#include "stdafx.h"
#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
#include <malloc.h>
#include <stdlib.h>
#include <time.h>
#include "PerlinNoise.hpp"

#define SIZE 5			//5 - 10
//ako je 5 za 600*600 polje, onda increment value stavit na 0.04-0.05
#define PI 3.14159265359
#define COLOR_RANGE 768


#define NO_ENTITIES 100000

#define INITIAL_ENTITIES 50000
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
bool rainbowColours = false;
//Some possible combinations:
//P=1:V=2.5:I=0.14		100000
//P=2:V=3:I=0.15		80000
//P=2:V=3.25:I=0.12		50000
int PIX_SIZE = 1;			//1 -	2
double MAX_VEL = 3.25;		//2.25-	2.75-3.25
double MAX_ACC = 0.275;		//0.2-	.3	
double increment = 0.05;	//0.04- .12-.15
double tIncrement = 0.12;

const int WIDTH = 600;		//300-600
const int HEIGHT = 600;		//300-600
SDL_Texture	*arrow = NULL;
SDL_Renderer *renderTarget = NULL;
SDL_Window *window = NULL;

SDL_Rect windowSize;
SDL_Rect sectorSize;

color white;
color black;
color background;
color use;

acc *acceleration = new acc[WIDTH*HEIGHT / SIZE / SIZE];

class fieldPresentation {
public:

	void setVectorFieldRand() {
		int x = WIDTH / SIZE;
		int y = HEIGHT / SIZE;
		double randAng;
		for (int i = 0; i < y; i++) {
			for (int j = 0; j < x; j++) {
				randAng = rand() * 1.0 / RAND_MAX * 360;
				SDL_RenderCopyEx(renderTarget, arrow, &windowSize, &sectorSize, randAng, NULL, SDL_FLIP_NONE);
				sectorSize.x += SIZE;
			}
			sectorSize.x = 0;
			sectorSize.y += SIZE;
		}
		sectorSize.x = 0;
		sectorSize.y = 0;
	}

	void setVectorFieldPerlin() {
		int x = WIDTH / SIZE;
		int y = HEIGHT / SIZE;
		double cvX=currentValue;
		double cvY=currentValue+100;
		double cvZ = z;
		for (int i = 0; i < y; i++) {
			for (int j = 0; j < x; j++) {
				
				if (notSet) {
					if (variousAccelerations)acceleration[i*x + j].acce = MAX_ACC * (pn.noise(cvX + distance, cvY + distance, cvZ) + 0.5);
					else acceleration[i*x + j].acce = MAX_ACC;
				}
				acceleration[i*x + j].angle = 360 * pn.noise(cvX, cvY, cvZ);

				if(arrowDisplay)SDL_RenderCopyEx(renderTarget, arrow, &windowSize, &sectorSize, acceleration[i*x + j].angle, NULL, SDL_FLIP_NONE);
				cvX += increment;
				sectorSize.x += SIZE;
				
			}
			cvX = currentValue;
			cvY += increment;
			sectorSize.x = 0;
			sectorSize.y += SIZE;
		}
		z += tIncrement/50;
		if (notSet)notSet = false;
		sectorSize.x = 0;
		sectorSize.y = 0;
	}

	void setDrawFlagRand(bool set) {
		drawFlagRand = set;
	}

	void movingRandField() {
		if (drawFlagRand) {
			for (int i = 0; i < WIDTH; i++) {
				arr[i] = rand() % HEIGHT;
			}
			drawFlagRand = false;
		}
		else {
			for (int i = 0; i < WIDTH - 1; i++) {
				arr[i] = arr[i + 1];
			}
			arr[WIDTH - 1] = rand() % HEIGHT;
		}

		SDL_RenderClear(renderTarget);
		SDL_SetRenderDrawColor(renderTarget, black.r, black.g, black.b, SDL_ALPHA_OPAQUE);

		for (int i = 0; i < WIDTH - 1; i++) {
			SDL_RenderDrawLine(renderTarget, i, arr[i], i + 1, arr[i + 1]);
		}

		SDL_SetRenderDrawColor(renderTarget, white.r, white.g, white.b, SDL_ALPHA_OPAQUE);
	}

	void movingPerlinField() {
		if (drawFlagPerlin) {
			for (int i = 0; i < WIDTH; i++) {
				arr[i] = (int) floor(HEIGHT*pn.noise(currentValue)+HEIGHT/2);
				currentValue += increment;
			}
			drawFlagPerlin = false;
		}
		else {
			for (int i = 0; i < WIDTH - 1; i++) {
				arr[i] = arr[i + 1];
			}
			arr[WIDTH - 1] = (int) floor(HEIGHT*pn.noise(currentValue) + HEIGHT / 2);
			currentValue += increment;
		}

		SDL_RenderClear(renderTarget);
		SDL_SetRenderDrawColor(renderTarget, black.r, black.g, black.b, SDL_ALPHA_OPAQUE);

		for (int i = 0; i < WIDTH - 1; i++) {
			SDL_RenderDrawLine(renderTarget, i, arr[i], i + 1, arr[i + 1]);
		}

		SDL_SetRenderDrawColor(renderTarget, white.r, white.g, white.b, SDL_ALPHA_OPAQUE);
	}

private:
	bool notSet = true;

	double currentValue = rand();
	double z = 0;

	bool drawFlagRand = true;
	bool drawFlagPerlin = true;


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

void setRainbowField() {
	color current;
	for (int i = 0; i < WIDTH; i++) {
		entityControl::colorSelect(&current, (int)(((double)i) / WIDTH*COLOR_RANGE));

		std::cout << (int)(((double)i) / WIDTH*COLOR_RANGE) << std::endl;

		SDL_SetRenderDrawColor(renderTarget, current.r, current.g, current.b, SDL_ALPHA_OPAQUE);
		SDL_RenderDrawLine(renderTarget, i, 0, i, HEIGHT - 1);
	}
	SDL_SetRenderDrawColor(renderTarget, background.r, background.g, background.b, SDL_ALPHA_OPAQUE);
}

int main(int argc, char *argv[]) {
	
	//Initialise the variables for the loop
	bool isRunning = true;
	int prevTime = 0;
	int ticks = 0;
	int currentColor = 0;
	int colourIncrement = 1;
	int delay = 0;

	int entitiesSpawned = 0;
	entity *pixel = new entity[NO_ENTITIES];
	
	SDL_Event	ev;

	fieldPresentation fp;

	srand((unsigned int)time(NULL));

	windowSize.x = windowSize.y = 0;
	windowSize.w = WIDTH;
	windowSize.h = HEIGHT;

	sectorSize.x = sectorSize.y = 0;
	sectorSize.w = sectorSize.h = SIZE;

	use.r = 255;
	use.g = use.b = 0;

	white.r = white.g = white.b = 255;
	black.r = black.g = black.b = 0;
	background.r = background.g = background.b = 0;
	
	//Initialise all the things needed for SDL
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		std::cout << "Error 1:" << SDL_GetError() << std::endl;
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

		SDL_SetRenderDrawColor(renderTarget, background.r, background.g, background.b, SDL_ALPHA_OPAQUE);
		SDL_RenderClear(renderTarget);
	}

	//Spawn the initial entities
	for (int i = 0; i < INITIAL_ENTITIES; i++) {
		pixel[i].spawn();
		entitiesSpawned = INITIAL_ENTITIES;
	}
	
	//Game loop
	while (isRunning) {

		//USED TO DISPLAY FPS
		
		if (prevTime != (int)time(NULL) && fpsPrint) {
			prevTime = (int)time(NULL);
			std::cout << ticks << std::endl;
			ticks = 0;
		}
		if (fpsPrint)ticks++;
		
		//USED TO DISPLAY RAINBOW COLORS
		if (rainbowColours) {
			currentColor+=colourIncrement;
			if (currentColor >= COLOR_RANGE)currentColor = 0;
			entityControl::colorSelect(&use, currentColor);
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
					if(arrowDisplay)std::cout << "Vector arrows enabled!" << std::endl;
					else std::cout << "Vector arrows disabled!" << std::endl;
				}
				else if (ev.key.keysym.sym == SDLK_c) {
					entityDisplay = !entityDisplay;
					if (entityDisplay)std::cout << "Entities visable!" << std::endl;
					else std::cout << "Entities invisible!" << std::endl;
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
			}
		}

		//CLEARS THE FIELD
		SDL_RenderClear(renderTarget);

		//UPDATES THE VECTORS
		fp.setVectorFieldPerlin();

		//MOVES THE ENTITIES
		for (int i = 0; i < entitiesSpawned; i++) {
			pixel[i].move();
		}

		//DISPLAYS THE PRESENT RENDER TARGET, AND DELAYS THE OPERATION
		SDL_RenderPresent(renderTarget);
		SDL_Delay(delay);
	}

	SDL_DestroyTexture(arrow);
	SDL_DestroyRenderer(renderTarget);
	SDL_DestroyWindow(window);					
	arrow = NULL;
	renderTarget = NULL;
	window = NULL;

	SDL_Quit();
	IMG_Quit();
	return EXIT_SUCCESS;
}