//Using SDL, SDL_image, standard IO, and strings
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <string>
#include <sstream>

using namespace std;

//Screen dimension constants
const int SCREEN_WIDTH = 400;
const int SCREEN_HEIGHT = 600;
const int ROWS = 5;
const int COLS = 10;
const int BRICK_NUMBER = ROWS * COLS;

int lives = 3, score = 0, brick = BRICK_NUMBER;

string scoreText = "Score: ";
string lifeText = "Lives: ";
string msgText = "Hit UP to start/pause/resume/quit";

//Texture wrapper class
class LTexture
{
	public:
		//Initializes variables
		LTexture();

		//Deallocates memory
		~LTexture();

		//Loads image at specified path
		bool loadFromFile( std::string path );

		//Creates image from font string
        bool loadFromRenderedText( std::string textureText, SDL_Color textColor );

		//Deallocates texture
		void free();

		//Set color modulation
		void setColor(Uint8 red, Uint8 green, Uint8 blue);

		//Set blending
		void setBlendMode(SDL_BlendMode blending);

		//Set alpha modulation
		void setAlpha(Uint8 alpha);

		//Renders texture at given point
		void render(int x, int y, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE);

		//Gets image dimensions
		int getWidth();
		int getHeight();

	private:
		//The actual hardware texture
		SDL_Texture* mTexture;

		//Image dimensions
		int mWidth;
		int mHeight;
};

//The user controlled paddle
class Paddle
{
    public:
		//The dimensions of the paddle
		static const int PADDLE_WIDTH = 60;
		static const int PADDLE_HEIGHT = 10;

		//Maximum axis velocity of the paddle
		static const int PADDLE_VEL = 10;

        //paddle's collision box
        SDL_Rect pCollider;

		//Initializes the variables
		Paddle();

		//Takes key presses and adjusts the paddle's velocity
		void handleEvent(SDL_Event& e);

		//Moves the paddle and checks collision
		void move();

		//Shows the paddle on the screen
		void render();

    private:
		//The X and Y offsets of the paddle
		int pPosX, pPosY;

		//The velocity of the paddle
		int pVelX, pVelY;
};

//The dot that will move around on the screen
class Dot
{
    public:
		//The dimensions of the dot
		static const int DOT_WIDTH = 20;
		static const int DOT_HEIGHT = 20;

		//Maximum axis velocity of the dot
		static const int DOT_VEL = 10;

		//Initializes the variables
		Dot();

		//Moves the dot and checks collision
		void move(Paddle p);

		//Shows the dot on the screen
		void render();

    private:
		//The X and Y offsets of the dot
		int dPosX, dPosY;

		//The velocity of the dot
		int dVelX, dVelY;

		//Dot's collision box
		SDL_Rect dCollider;
};

//Starts up SDL and creates window
bool init();

//Loads media
bool loadMedia();

//Frees media and shuts down SDL
void close();

//Box collision detector
bool checkCollision(SDL_Rect a, SDL_Rect b);

bool handleCollision(SDL_Rect c, Paddle p);

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The window renderer
SDL_Renderer* gRenderer = NULL;

//Array to hold brick wall
SDL_Rect bricks[ROWS][COLS];

//paddle bounce sound effect
Mix_Chunk *bounce = NULL;

//brick breaking sound effect
Mix_Chunk *breaking = NULL;

//Dot texture
LTexture gDotTexture;

//Globally used font
TTF_Font *gFont = NULL;

//Score texture
LTexture gScoreTexture;
//Life texture
LTexture gLifeTexture;
//Message texture
LTexture gMsgTexture;

LTexture::LTexture()
{
	//Initialize
	mTexture = NULL;
	mWidth = 0;
	mHeight = 0;
}

LTexture::~LTexture()
{
	//Deallocate
	free();
}

bool LTexture::loadFromFile( std::string path )
{
	//Get rid of preexisting texture
	free();

	//The final texture
	SDL_Texture* newTexture = NULL;

	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load( path.c_str() );
	if( loadedSurface == NULL )
	{
		printf( "Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError() );
	}
	else
	{
		//Color key image
		SDL_SetColorKey( loadedSurface, SDL_TRUE, SDL_MapRGB( loadedSurface->format, 0, 0xFF, 0xFF ) );

		//Create texture from surface pixels
        newTexture = SDL_CreateTextureFromSurface( gRenderer, loadedSurface );
		if( newTexture == NULL )
		{
			printf( "Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError() );
		}
		else
		{
			//Get image dimensions
			mWidth = loadedSurface->w;
			mHeight = loadedSurface->h;
		}

		//Get rid of old loaded surface
		SDL_FreeSurface( loadedSurface );
	}

	//Return success
	mTexture = newTexture;
	return mTexture != NULL;
}

bool LTexture::loadFromRenderedText( std::string textureText, SDL_Color textColor )
{
    //Get rid of preexisting texture
    free();

    //Render text surface
    SDL_Surface* textSurface = TTF_RenderText_Solid( gFont, textureText.c_str(), textColor );
    if( textSurface == NULL )
    {
        printf( "Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError() );
    }
    else
    {
        //Create texture from surface pixels
        mTexture = SDL_CreateTextureFromSurface( gRenderer, textSurface );
        if( mTexture == NULL )
        {
            printf( "Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError() );
        }
        else
        {
            //Get image dimensions
            mWidth = textSurface->w;
            mHeight = textSurface->h;
        }

        //Get rid of old surface
        SDL_FreeSurface( textSurface );
    }

    //Return success
    return mTexture != NULL;
}

void LTexture::free()
{
	//Free texture if it exists
	if( mTexture != NULL )
	{
		SDL_DestroyTexture( mTexture );
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
	}
}

void LTexture::setColor( Uint8 red, Uint8 green, Uint8 blue )
{
	//Modulate texture rgb
	SDL_SetTextureColorMod( mTexture, red, green, blue );
}

void LTexture::render( int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip )
{
	//Set rendering space and render to screen
	SDL_Rect renderQuad = { x, y, mWidth, mHeight };

	//Set clip rendering dimensions
	if( clip != NULL )
	{
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	//Render to screen
	SDL_RenderCopyEx( gRenderer, mTexture, clip, &renderQuad, angle, center, flip );
}

Dot::Dot()
{
    //Initialize the offsets
    dPosX = (SCREEN_WIDTH - DOT_WIDTH) / 2;
    dPosY = (SCREEN_HEIGHT - DOT_HEIGHT) / 2;

	//Set collision box dimension
	dCollider.w = DOT_WIDTH;
	dCollider.h = DOT_HEIGHT;

    //Initialize the velocity
    dVelX = 3;
    dVelY = 3;
}

Paddle::Paddle()
{
    //Initialize the offsets
    pPosX = (SCREEN_WIDTH - PADDLE_WIDTH) / 2;
    pPosY = 580;

	//Set collision box dimension
	pCollider.w = PADDLE_WIDTH;
	pCollider.h = PADDLE_HEIGHT;
    pCollider.x = pPosX;
    pCollider.y = pPosY;
    //Initialize the velocity
    pVelX = 0;
}

void Paddle::handleEvent( SDL_Event& e )
{
    //If a key was pressed
	if( e.type == SDL_KEYDOWN && e.key.repeat == 0 )
    {
        //Adjust the velocity
        switch( e.key.keysym.sym )
        {
            case SDLK_LEFT: pVelX = -PADDLE_VEL; break;
            case SDLK_RIGHT: pVelX = PADDLE_VEL; break;
        }
    }
}

void Dot::move(Paddle p)
{
    //Move the dot left or right
    dPosX += dVelX;
	dCollider.x = dPosX;

    //If the dot collided or went too far to the left or right
    if((dPosX < 0) || (dPosX + DOT_WIDTH > SCREEN_WIDTH) || handleCollision(dCollider, p))
    {
        //Move back
        dPosX -= dVelX;
		dCollider.x = dPosX;
		dVelX = -dVelX; //bounce to the opposite direction
    }

    //Move the dot up or down
    dPosY += dVelY;
	dCollider.y = dPosY;

	//If the dot went too far down
	if(dPosY + DOT_HEIGHT > SCREEN_HEIGHT)
    {
        //reset to original position
        dPosX = (SCREEN_WIDTH - DOT_WIDTH) / 2;
        dPosY = (SCREEN_HEIGHT - DOT_HEIGHT) / 2;

        //update the life label
        lives--;
        SDL_Color textColor = { 0, 0, 0 };
        stringstream sstm;
        lifeText = "Lives:";
        sstm << lifeText << lives;
        lifeText = sstm.str();
        gLifeTexture.loadFromRenderedText(lifeText, textColor);
        gLifeTexture.render(320, 5);
    }

    //If the dot collided or went too far up
    if((dPosY < 0) || handleCollision(dCollider, p))
    {
        //Move back
        dPosY -= dVelY;
		dCollider.y = dPosY;
		dVelY = -dVelY;        // bounce to the opposite direction
    }
}

void Paddle::move()
{
    //Move the paddle left or right
    pPosX += pVelX;
	pCollider.x = pPosX;

    //If the paddle collided or went too far to the left or right
    if((pPosX < 0) || (pPosX + PADDLE_WIDTH > SCREEN_WIDTH))
    {
        //Move back
        pPosX -= pVelX;
		pCollider.x = pPosX;
    }
}

void Dot::render()
{
    //Show the dot
	gDotTexture.render( dPosX, dPosY );
}

void Paddle::render()
{
    SDL_Rect paddle = {pPosX, pPosY, PADDLE_WIDTH, PADDLE_HEIGHT};
    SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255 );
    SDL_RenderFillRect(gRenderer, &paddle);
}

bool init()
{
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0 )
	{
		printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
		success = false;
	}
	else
	{
		//Create window
		gWindow = SDL_CreateWindow( "Breakout", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
		if( gWindow == NULL )
		{
			printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
			success = false;
		}
		else
		{
			//Create vsynced renderer for window
			gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
			if( gRenderer == NULL )
			{
				printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
				success = false;
			}
			else
			{
				//Initialize renderer color
				SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );

				//Initialize PNG loading
				int imgFlags = IMG_INIT_PNG;
				if( !( IMG_Init( imgFlags ) & imgFlags ) )
				{
					printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
					success = false;
				}

                //Initialize SDL_mixer
                if( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048 ) < 0 )
                {
                    printf( "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError() );
                    success = false;
                }

                 //Initialize SDL_ttf
                if( TTF_Init() == -1 )
                {
                    printf( "SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError() );
                    success = false;
                }
			}
		}
	}

	return success;
}

bool loadMedia()
{
	//Loading success flag
	bool success = true;

	//Load press texture
	if( !gDotTexture.loadFromFile("dot.bmp"))
	{
		printf( "Failed to load dot texture!\n" );
		success = false;
	}

    //Load sound effect
	bounce = Mix_LoadWAV("bounce.wav");
	if( bounce == NULL )
	{
		printf( "Failed to load low sound effect! SDL_mixer Error: %s\n", Mix_GetError() );
		success = false;
	}

    //Load sound effect
	breaking = Mix_LoadWAV("break.wav");
	if( breaking == NULL )
	{
		printf("Failed to load high sound effect! SDL_mixer Error: %s\n", Mix_GetError());
		success = false;
	}

	 //Open the font
    gFont = TTF_OpenFont("Rabbit On The Moon.ttf", 28);
    if(gFont == NULL)
    {
        printf("Failed to load lazy font! SDL_ttf Error: %s\n", TTF_GetError());
        success = false;
    }
    else
    {
        //Render score label
        SDL_Color textColor = { 0, 0, 0 };

        if(!gScoreTexture.loadFromRenderedText(scoreText, textColor))
        {
            printf("Failed to render score texture!\n");
            success = false;
        }
        else
        {
            scoreText = "Score:0";
            gScoreTexture.loadFromRenderedText(scoreText, textColor);
        }

        //Render life label
        if(!gLifeTexture.loadFromRenderedText(lifeText, textColor))
        {
            printf("Failed to render life texture!\n");
            success = false;
        }
        else
        {
            lifeText = "Lives:3";
            gLifeTexture.loadFromRenderedText(lifeText, textColor);
        }


        //Render message label
        if(!gMsgTexture.loadFromRenderedText(msgText, textColor))
        {
            printf("Failed to render message texture!\n");
            success = false;
        }
        else
        {
            msgText = "Hit UP to start/pause/resume/quit";
            gMsgTexture.loadFromRenderedText(msgText, textColor);
        }
    }

	return success;
}

void close()
{
    //Free the sound effects
	Mix_FreeChunk(bounce);
	Mix_FreeChunk(breaking);

	bounce = NULL;
	breaking = NULL;

	//Free loaded images
	gDotTexture.free();
    gScoreTexture.free();
    gLifeTexture.free();

    //Free global font
    TTF_CloseFont( gFont );
    gFont = NULL;

	//Destroy window
	SDL_DestroyRenderer( gRenderer );
	SDL_DestroyWindow( gWindow );
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}

bool checkCollision(SDL_Rect a, SDL_Rect b)
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
    if(bottomA <= topB)
    {
        return false;
    }

    if(topA >= bottomB)
    {
        return false;
    }

    if(rightA <= leftB)
    {
        return false;
    }

    if(leftA >= rightB)
    {
        return false;
    }

    //If none of the sides from A are outside B
    return true;
}

//initialize the brick wall
void initWall()
{
    int brick_x = 2;
    int brick_y = 40;

    int colors[5][3];
    //red
    colors[0][0] = 255;
    colors[0][1] = 0;
    colors[0][2] = 0;
    //orange
    colors[1][0] = 255;
    colors[1][1] = 144;
    colors[1][2] = 0;
    //green
    colors[2][0] = 0;
    colors[2][1] = 128;
    colors[2][2] = 0;
    //yellow
    colors[3][0] = 255;
    colors[3][1] = 255;
    colors[3][2] = 0;
    //blue
    colors[4][0] = 0;
    colors[4][1] = 0;
    colors[4][2] = 255;

    for (int row = 0; row < ROWS; row++)
    {
        for (int col = 0; col < COLS; col++, brick_x += 40)
        {
            SDL_Rect fillRect = { brick_x, brick_y, SCREEN_WIDTH / 11, 10};
            SDL_SetRenderDrawColor( gRenderer, colors[row][0], colors[row][1], colors[row][2], 255 );
            SDL_RenderFillRect( gRenderer, &fillRect );
            bricks[row][col] = fillRect;
        }
        // reset x position for each row
        brick_y += 20;
        brick_x = 2;
    }
}

//check if the dot collided with any of the bricks of the wall
bool handleCollision(SDL_Rect c, Paddle p)
{
    bool collided = false;
    //checking dot and paddle collision
    collided = checkCollision(c, p.pCollider);
    //return if collided
    if(collided)
    {
        //play bounce sound
        Mix_PlayChannel(-1, bounce, 0);
        return collided;
    }

    //checking dot and wall for each brick
    for(int row = 0; row < ROWS; row++)
    {
        for(int col = 0; col < COLS; col++)
        {
            //check collision only if the current brick position has dimensions more than zero
            if(bricks[row][col].w > 0 && bricks[row][col].h > 0)
                collided = checkCollision(c, bricks[row][col]);

            //If collided, set the collided brick's dimensions to zero
            if(collided)
            {
                //play break sound
                Mix_PlayChannel(-1, breaking, 0);

                //increase the score depending on the row number
                switch(row)
                {
                    case 4 :
                        score += 1;
                        break;
                    case 3 :
                        score += 2;
                        break;
                    case 2 :
                        score += 3;
                        break;
                    case 1 :
                        score += 4;
                        break;
                    case 0 :
                        score += 5;
                        break;
                }

                //concatenate score text and score, updating the score label
                stringstream sstm;
                scoreText = "Score:";
                sstm << scoreText << score;
                scoreText = sstm.str();
                SDL_Color textColor = {0, 0, 0};
                gScoreTexture.loadFromRenderedText(scoreText, textColor);

                //destroy the collided brick
                bricks[row][col].w = 0;
                bricks[row][col].h = 0;
                brick--;
                return collided;
            }
        }
    }
    return collided;
}

void updateWall()
{
    int brick_x = 2;
    int brick_y = 40;

    int colors[5][3];
    //red
    colors[0][0] = 255;
    colors[0][1] = 0;
    colors[0][2] = 0;
    //orange
    colors[1][0] = 255;
    colors[1][1] = 144;
    colors[1][2] = 0;
    //yellow
    colors[2][0] = 0;
    colors[2][1] = 128;
    colors[2][2] = 0;
    //green
    colors[3][0] = 255;
    colors[3][1] = 255;
    colors[3][2] = 0;
    //blue
    colors[4][0] = 0;
    colors[4][1] = 0;
    colors[4][2] = 255;

    for (int row = 0; row < ROWS; row++)
    {
        for (int col = 0; col < COLS; col++, brick_x += 40)
        {
            if(bricks[row][col].w > 0 && bricks[row][col].h > 0)
            {
                SDL_Rect fillRect = { brick_x, brick_y, SCREEN_WIDTH / 11, 10};
                SDL_SetRenderDrawColor( gRenderer, colors[row][0], colors[row][1], colors[row][2], 255 );
                SDL_RenderFillRect( gRenderer, &fillRect );
            }
        }
        // reset x position for each row
        brick_y += 20;
        brick_x = 2;
    }
}

// main
int main(int argc, char* args[])
{
	//Start up SDL and create window
	if( !init() )
	{
		printf( "Failed to initialize!\n" );
	}
	else
	{
		//Load media
		if(!loadMedia())
		{
			printf("Failed to load media!\n");
		}
		else
		{
			//Main loop flags
			bool quit = false;
            bool pause = true;

			//Event handler
			SDL_Event e1, e2;

			//Initialize dot and paddle
			Dot dot;
            Paddle paddle;

            //Clear screen
            SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
            SDL_RenderClear(gRenderer);

            initWall();

            //Render the paddle
            paddle.render();

            //Render dot
            dot.render();

            //Render text labels
            gScoreTexture.render(5, 5);
            gLifeTexture.render(320, 5);
            gMsgTexture.render(5, (SCREEN_HEIGHT/2) + 20);

            //Update screen
            SDL_RenderPresent(gRenderer);

			//keep checking for key press while pause is true
            while(pause && !quit)
            {
                while(SDL_PollEvent(&e1) !=0)
                {
                    //Start the game if UP key is pressed
                    if(e1.type == SDL_KEYDOWN && e1.key.keysym.sym == SDLK_UP)
                    {
                        pause = false;
                    }

                    //quit the game from paused state
                    if(e1.type == SDL_QUIT)
                    {
                        quit = true;
                    }
                }
            }

			//Main game loop
			while(!quit && brick > 0 && lives > 0)
			{
                //Handle events on queue
				while(SDL_PollEvent(&e1 ) != 0)
				{
                    //User requests quit
					if(e1.type == SDL_QUIT)
					{
						quit = true;
					}

                    //Pause if the player press UP key
					if(e1.type == SDL_KEYDOWN && e1.key.keysym.sym == SDLK_UP)
                    {
                        pause = true;
                    }

                    //keep checking for key press while pause is true
                    while(pause && !quit)
                    {
                        while(SDL_PollEvent(&e2) !=0)
                        {
                            //resume the game if UP key is pressed again
                            if(e2.type == SDL_KEYDOWN && e2.key.keysym.sym == SDLK_UP)
                            {
                                pause = false;
                            }

                            //quit the game from paused state
                             if(e2.type == SDL_QUIT)
                            {
                                quit = true;
                            }
                        }
                    }

                    //move the paddle if only LEFT or RIGHT keys are pressed
                    if(e1.key.keysym.sym == SDLK_LEFT || e1.key.keysym.sym == SDLK_RIGHT)
					{
					    //Handle input for the dot if LEFT or RIGHT key is pressed
                        paddle.handleEvent(e1);
                        paddle.move();
					}
                }

				//Move the dot and check collision
				dot.move(paddle);

				//Clear screen
				SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
				SDL_RenderClear(gRenderer);

                //Render wall
                updateWall();
                //Render paddle
                paddle.render();
				//Render dot
				dot.render();

                //Render text labels
                gScoreTexture.render(5, 5);
                gLifeTexture.render(320, 5);
                gMsgTexture.render(5, (SCREEN_HEIGHT / 2) + 20);

				//Update screen
				SDL_RenderPresent(gRenderer);

				//Wait until player presses UP key to quit the game when lives == 0
				if(lives == 0)
                {
                    SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
                    SDL_RenderClear(gRenderer);

                    SDL_Color textColor = { 0, 0, 0 };
                    msgText = "Game Over!, press UP key to quit";
                    gMsgTexture.loadFromRenderedText(msgText, textColor);
                    gMsgTexture.render(5, (SCREEN_HEIGHT / 2) + 20);

                    SDL_RenderPresent(gRenderer);

                    //keep checking for key press while quit is false
                    while(!quit)
                    {
                        while(SDL_PollEvent(&e1) != 0)
                        {
                            //Quit the game if UP key is pressed
                            if(e1.type == SDL_KEYDOWN && e1.key.keysym.sym == SDLK_UP)
                            {
                                quit = true;
                            }
                        }
                    }
                }

                if(score == 150)
                {
                    SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
                    SDL_RenderClear(gRenderer);

                    SDL_Color textColor = { 0, 0, 0 };
                    msgText = "Congratulations!,hit UP to quit";
                    gMsgTexture.loadFromRenderedText(msgText, textColor);
                    gMsgTexture.render(5, (SCREEN_HEIGHT / 2) + 20);

                    SDL_RenderPresent(gRenderer);

                    //keep checking for key press while quit is false
                    while(!quit)
                    {
                        while(SDL_PollEvent(&e1) != 0)
                        {
                            //Quit the game if UP key is pressed
                            if(e1.type == SDL_KEYDOWN && e1.key.keysym.sym == SDLK_UP)
                            {
                                quit = true;
                            }
                        }
                    }
                }
			}
		}
	}

	//Free resources and close SDL
	close();

	return 0;
}
