/*This source code copyrighted by Lazy Foo' Productions (2004-2015)
and may not be redistributed without written permission.*/

//Using SDL, SDL_image, standard IO, math, and strings
#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <string>
#include <cmath>
#include <stdlib.h>
#include <time.h>

using namespace std;

//Screen dimension constants
const int SCREEN_WIDTH = 400;
const int SCREEN_HEIGHT = 600;

//number of rows and columns of the brick wall
const int ROWS = 5;
const int COLS = 10;

// ball radius
const int RADIUS = 10;

//paddle
const int PADDLE_WIDTH = 70;
const int PADDLE_HEIGHT = 10;

//Starts up SDL and creates window
bool init();

//Loads media
bool loadMedia();

//Frees media and shuts down SDL
void close();

//Initialize the brick wall
void initBricks();

//Box collision detector
bool checkCollision( SDL_Rect a, SDL_Rect b );

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The window renderer
SDL_Renderer* gRenderer = NULL;

//The ball that will move around on the screen
class Ball
{
    public:
		//The dimensions of the dot
		static const int BALL_WIDTH = 20;
		static const int BALL_HEIGHT = 20;

		//Maximum axis velocity of the dot
		static const int DOT_VEL = 10;

		//Initializes the variables
		Ball();

		//Takes key presses and adjusts the ball's velocity
		void handleEvent( SDL_Event& e );

		//Moves the ball and checks collision
		void move( SDL_Rect& wall );

		//Shows the ball on the screen
		int render();

    private:
		//The X and Y offsets of the ball
		int mPosX, mPosY;

		//The velocity of the ball
		int mVelX, mVelY;

		//Ball's collision box
		SDL_Rect mCollider;
};

Ball::Ball()
{
    //Initialize the offsets
    mPosX = 0;
    mPosY = 0;

	//Set collision box dimension
	mCollider.w = BALL_WIDTH;
	mCollider.h = BALL_HEIGHT;

    //Initialize the velocity
    mVelX = 0;
    mVelY = 0;
}

void Ball::move( SDL_Rect& wall )
{
    //Move the dot left or right
    mPosX += mVelX;
	mCollider.x = mPosX;

    //If the dot collided or went too far to the left or right
    if( ( mPosX < 0 ) || ( mPosX + BALL_WIDTH > SCREEN_WIDTH ) || checkCollision( mCollider, wall ) )
    {
        //Move back
        mPosX -= mVelX;
		mCollider.x = mPosX;
    }

    //Move the dot up or down
    mPosY += mVelY;
	mCollider.y = mPosY;

    //If the dot collided or went too far up or down
    if( ( mPosY < 0 ) || ( mPosY + BALL_HEIGHT > SCREEN_HEIGHT ) || checkCollision( mCollider, wall ) )
    {
        //Move back
        mPosY -= mVelY;
		mCollider.y = mPosY;
    }
}

/**int Ball::render()
{
    int success = filledCircleColor(gRenderer, mPosX, mPosY, RADIUS, 0, 0, 0, 255);
    return success;
}*/

bool checkCollision( SDL_Rect a, SDL_Rect b )
{
    //The sides of the rectangles
    int leftA, leftB;
    int rightA, rightB;
    int topA, topB;
    int bottomA, bottomB;

    //Calculate the sides of rect A
    leftA = a.x;
    rightA = a.x + a.w;
    topA = a.y;
    bottomA = a.y + a.h;

    //Calculate the sides of rect B
    leftB = b.x;
    rightB = b.x + b.w;
    topB = b.y;
    bottomB = b.y + b.h;

    //If any of the sides from A are outside of B
    if( bottomA <= topB )
    {
        return false;
    }

    if( topA >= bottomB )
    {
        return false;
    }

    if( rightA <= leftB )
    {
        return false;
    }

    if( leftA >= rightB )
    {
        return false;
    }

    //If none of the sides from A are outside B
    return true;
}

bool init()
{
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
		printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
		success = false;
	}
	else
	{
		//Create window
		gWindow = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
		if( gWindow == NULL )
		{
			printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
			success = false;
		}
		else
		{
			//Create renderer for window
			gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED );
			if( gRenderer == NULL )
			{
				printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
				success = false;
			}
			else
			{
				//Initialize the brick wall
                initBricks();
            }
		}
	}

	return success;
}

void close()
{
	//Destroy window
	SDL_DestroyRenderer( gRenderer );
	SDL_DestroyWindow( gWindow );
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	IMG_Quit();
	SDL_Quit();
}

void initBricks()
{
    //Clear screen
            SDL_SetRenderDrawColor( gRenderer, 255, 255, 255, 255);
            SDL_RenderClear( gRenderer );

            int brick_x = 2;
            int brick_y = 40;

            int colors[5][3];
            //red
            colors[0][0] = 255;
            colors[0][1] = 0;
            colors[0][2] = 0;
            //orange
            colors[1][0] = 255;
            colors[1][1] = 165;
            colors[1][2] = 0;
            //yellow
            colors[2][0] = 255;
            colors[2][1] = 255;
            colors[2][2] = 0;
            //green
            colors[3][0] = 0;
            colors[3][1] = 128;
            colors[3][2] = 0;
            //blue
            colors[4][0] = 0;
            colors[4][1] = 0;
            colors[4][2] = 255;

            for (int row = 0; row < ROWS; row++)
            {
                for (int col = 0; col < COLS; col++, brick_x+=40)
                {
                    SDL_Rect fillRect = { brick_x, brick_y, SCREEN_WIDTH / 11, 10};
                    SDL_SetRenderDrawColor( gRenderer, colors[row][col], colors[row][col+1], colors[row][col+2], 255 );
                    SDL_RenderFillRect( gRenderer, &fillRect );
                }
                // reset x position for each row
                brick_y += 20;
                brick_x = 2;
            }

            //Update screen
            SDL_RenderPresent( gRenderer );
}

int main( int argc, char* args[] )
{
	//Start up SDL and create window
	if( !init() )
	{
		printf( "Failed to initialize!\n" );
	}
    else
    {
        //Main loop flag
        bool quit = false;

        //Event handler
        SDL_Event e;

        //While application is running
        while( !quit )
        {
            //Handle events on queue
            while( SDL_PollEvent( &e ) != 0 )
            {
                //User requests quit
                if( e.type == SDL_QUIT )
                {
                    quit = true;
                }
            }
        }
    }
    //Free resources and close SDL
	close();

	return 0;
}
