/*
// Main.c - contains all relevent code for the Game of the Goose project (see documentation)
//
// The MIT License (MIT)
// 
// Copyright (c) 2021 Luke Andrews.  All Rights Reserved.
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy of this
// software and associated documentation files (the "Software"), to deal in the Software
// without restriction, including without limitation the rights to use, copy, modify, merge,
// publish, distribute, sub-license, and/or sell copies of the Software, and to permit persons
// to whom the Software is furnished to do so, subject to the following conditions:
// 
// * The above copyright notice and this permission notice shall be included in all copies or
// substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
// INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
// PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
// FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR 
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <time.h>

#ifndef _countof
#define _countof(a) sizeof(a)/sizeof(a[0])
#endif

#define NUM_BOARD_SPACES  24
#define NUM_GOOSE_SPACES  3
#define NUM_BRIDGE_SPACES 1
#define NUM_MAZE_SPACES   2
#define NUM_SKULL_SPACES  1
#define MAX_PLAYERS       2

const int gooseSpaces[NUM_GOOSE_SPACES]   = { 7, 11, 15 };
const int bridgeSpaces[NUM_BRIDGE_SPACES] = { 6 };
const int mazeSpaces[NUM_MAZE_SPACES]     = { 13, 30 };
const int skullSpaces[NUM_SKULL_SPACES]   = { 23 };

#define SYM_GOOSE    '+'
#define SYM_BRIDGE   '*'
#define SYM_MAZE     '-'
#define SYM_SKULL    '!'
#define SYM_NONE     '\0'
#define SYM_COMPUTER '%'
#define SYM_HUMAN    '$'

/*
 * A player of the game.
 */
struct Player
{
	const char* name;
	const char symbol;
	int position;
};

/*
 * A space of the game board.
 */
struct Space
{
	char special;
	bool isOccupied;
};

/*
 * Global variable declarations.
 */
struct Player players[MAX_PLAYERS] =
{
	{ "HUMAN",    SYM_HUMAN,    0 },
	{ "COMPUTER", SYM_COMPUTER, 0 }
};

struct Space board[NUM_BOARD_SPACES];

/*
 * Prints a line as page break. 
 */
void page_Break(void)
{
	printf("\n");
	printf("*********************************************************************************\n");
	printf("\n");
}

/*
 * Prompts for a string value and returns its length (or EOF on error).
 */
int prompt_For(char* buffer, int bufsize, const char* prompt, ...)
{
	int length = EOF;

	if (buffer != NULL && bufsize > 0)
	{
		*buffer = '\0';

		if (prompt != NULL)
		{
			va_list alist;
			va_start(alist, prompt);

			vprintf(prompt, alist);

			va_end(alist);
		}

		if (fgets(buffer, bufsize, stdin))
		{
			/*
			 * Trim the trailing newlines from the input string:
			 */
			int orglen = strlen(buffer);

			length = orglen;

			while (length > 0 && buffer[length - 1] == '\n')
			{
				buffer[--length] = '\0';
			}

			/*
			 * Clear the input buffer if we didn't see a newline:
			 */
			if (orglen == length)
			{
				while (!feof(stdin))
				{
					int ch = fgetc(stdin);

					if (ch <= EOF || ch == '\n')
					{
						break;
					}
				}
			}
		}
	}

	return length;
}

/*
 * Rolls the dice.
 */
int roll_Dice(void)
{
	int roll1     = (rand() % 6) + 1;
	int roll2     = (rand() % 6) + 1;
	int totalRoll = roll1 + roll2;
	   
	printf("\tPlayer rolled %d and %d for a total of %d.\n",roll1, roll2, totalRoll);

	return totalRoll;
}

/*
 * Determines which player's turn is next.
 */
int next_Player(int currentPlayer)
{
	int nextPlayer = 0;

	if (currentPlayer == _countof(players) - 1)
	{
		nextPlayer = 0;
	}
	else
	{
		nextPlayer = currentPlayer + 1;
	}

	return nextPlayer;
}

/*
 * Prints the winning message including which player won.
 */
void print_Winner(int currentPlayer)
{
	printf("*** Game Over! Player %s wins! ***\n", players[currentPlayer].name);
	page_Break();
}

/*
 * Updates a player position based on their dice roll.
 */
void update_Position(int currentPlayer, int roll)
{
	int  originalPosition = players[currentPlayer].position;
	bool moveAgain;

	do
	{
		moveAgain = false;

		players[currentPlayer].position += roll;

		if (players[currentPlayer].position > _countof(board) - 1)
		{
			players[currentPlayer].position = (_countof(board) - 1)
				- (players[currentPlayer].position - (_countof(board) - 1));
		}

		char special = board[players[currentPlayer].position].special;

		switch (special)
		{
			case SYM_GOOSE:
			{
				printf("Player landed on a goose! Moving the roll amount again!\n");
				moveAgain = true;
				break;
			}

			case SYM_BRIDGE:
			{
				printf("Player landed on a bridge! Moving to space 12!\n");
				players[currentPlayer].position = 11;
				break;
			}

			case SYM_MAZE:
			{
				printf("Player landed on a maze! No movement this round!\n");
				players[currentPlayer].position = originalPosition;
				break;
			}

			case SYM_SKULL:
			{
				printf("Player landed on a skull! Moving back to start!\n");
				players[currentPlayer].position = 0;
				break;
			}
		}
	}
	while (moveAgain);

	board[players[currentPlayer].position].isOccupied = true;

	bool stillOccupied = false;
	int  i;

	for (i = 0; i < _countof(players); ++i)
	{
		if (players[i].position == originalPosition)
		{
			stillOccupied = true;
			break;
		}
	}

	if (!stillOccupied)
	{
		board[originalPosition].isOccupied = false;
	}

	printf("New space is: %d\n", players[currentPlayer].position + 1);
}

/*
 * Handles a player's turn.
 */
void handle_Player(int currentPlayer)
{
	int roll = 0;
	char input[100] = {0};

	if (players[currentPlayer].name == "COMPUTER")
	{
		prompt_For(input, _countof(input), "Player %s turn. Press <Enter> to let them roll the dice...", players[currentPlayer].name);
	}
	else
	{	
		prompt_For(input, _countof(input), 
				   "Player %s turn. Press <Enter> to roll the dice...", players[currentPlayer].name);
	}

	roll = roll_Dice();
	update_Position(currentPlayer, roll);
	page_Break();
}

/*
 * Prints the game board to the standard out.
 */
bool print_Board()
{
	bool  isWinner   = false;
	char  preBox     = '[';
	char  postBox    = ']';
	int   i;

	for (i = 0; i < _countof(board); ++i)
	{
		if (i == _countof(board) - 1)
		{
			preBox  = '<';
			postBox = '>';
		}

		printf("%c%c", board[i].special, preBox);

		if (board[i].isOccupied)
		{
			int j;

			for (j = 0; j < _countof(players); ++j)
			{
				if (players[j].position == i)
				{
					printf("%c", players[j].symbol);

					if (i == _countof(board) - 1)
					{
						isWinner = true;
					}
				}
			}
		}
		else
		{
			printf("%d", i + 1);
		}

		printf("%c", postBox);

		if ((i + 1) % 12 == 0 || i == _countof(board) - 1)
		{
			printf("\n");
		}
		else
		{
			printf("\t");
		}
	}
	printf("\n");

	return isWinner;
}

/*
 * Determines which player will roll first.
 */
int determine_First_Player(void)
{
	int firstPlayer = 0;

	char input[100]                 = {0};
	int  rolls[_countof(players)]   = {0};
	bool rollers[_countof(players)] = {0};
	int  i;

	for (i = 0; i < _countof(players); ++i)
	{
		rolls[i]   = 0;
		rollers[i] = true;
	}

	bool tiesExist = false;

	printf("Everyone roll the dice. The highest roll plays first ...\n");
	printf("\n");

	do
	{
		int highestRoll = 0;

		for (i = 0; i < _countof(players); ++i)
		{
			if (rollers[i])
			{
				if (players[i].name == "COMPUTER")
				{
					prompt_For(input, _countof(input), "Press <Enter> to let player %s roll the dice...", players[i].name);
				}
				else
				{
					prompt_For(input, _countof(input), "Player %s, press <Enter> to roll the dice...", players[i].name);
				}

				rolls[i]  = roll_Dice();

				if (rolls[i] > highestRoll)
				{
					highestRoll = rolls[i];
					firstPlayer = i;
					rollers[i]  = false;
					tiesExist   = false;
				}
				else if (rolls[i] == highestRoll)
				{
					rollers[firstPlayer] = true;
					rollers[i]           = true;
					tiesExist            = true;
				}
				else
				{
					rollers[i] = false;
				}
			}
		}

		if (tiesExist)
		{
			printf("\n");
			printf("Rerolling...The following players all tied:\n");

			for (i = 0; i < _countof(rollers); ++i)
			{
				if (rollers[i])
				{
					printf("\tPlayer %s with roll of %d\n", players[i].name, rolls[i]);
				}
			}
			
			page_Break();
		}
	}
	while (tiesExist);
	
	printf("Player %s goes first!\n", players[firstPlayer].name);
	
	page_Break();

	return firstPlayer;
}

/*
 * Gets the special symbol at a given position on the board.
 */
char get_Special(int position)
{
	int i;

	for (i = 0; i < _countof(gooseSpaces); ++i)
	{
		if (gooseSpaces[i] == position + 1)
		{
			return SYM_GOOSE;
		}
	}

	for (i = 0; i < _countof(bridgeSpaces); ++i)
	{
		if (bridgeSpaces[i] == position + 1)
		{
			return SYM_BRIDGE;
		}
	}

	for (i = 0; i < _countof(mazeSpaces); ++i)
	{
		if (mazeSpaces[i] == position + 1)
		{
			return SYM_MAZE;
		}
	}

	for (i = 0; i < _countof(skullSpaces); ++i)
	{
		if (skullSpaces[i] == position + 1)
		{
			return SYM_SKULL;
		}
	}

	return SYM_NONE;
}

/*
 * Creates a new game board with the default state.
 */
void reset_Game()
{
	int position;

	for (position = 0; position < _countof(board); ++position)
	{
		board[position].special    = get_Special(position);

		if (position != 0)
		{
			board[position].isOccupied = false;
		}
		else
		{
			board[position].isOccupied = true;
		}
	}

	int player;

	for (player = 0; player < _countof(players); ++player)
	{
		players[player].position = 0;
	}
}

/*
 * The game simulation.
 */
void run_Game(void)
{
	reset_Game();

	int currentPlayer = determine_First_Player();

	print_Board();

	while (true)
	{
		handle_Player(currentPlayer);

		if (print_Board())
		{
			print_Winner(currentPlayer);
			break;
		}

		currentPlayer = next_Player(currentPlayer);
	}
}

/*
 * Prompts the user to either play or quit the game.
 */
bool prompt_For_Play(void)
{
	bool play = false;

	bool invalidInput = true;
	char buffer[100]  = {0};
	char ch           = 0;

	while (invalidInput)
	{
		prompt_For(buffer, _countof(buffer),
				   "*** Welcome to The Game of the Goose! ***\n"
				   "  1) To play, enter 'P' or 'p'\n"
				   "  2) To quit, enter 'Q' or 'q'\n"
				   "Please select an option: ");
		sscanf_s(buffer, "%c", &ch, _countof(buffer));

		switch (ch)
		{
			case 'P':
			case 'p':
			{
				invalidInput = false;
				play         = true;
				break;
			}

			case 'Q':
			case 'q':
			{
				invalidInput = false;
				break;
			}

			default:
			{
				printf("\n");
				printf("Selection was invalid. Try again.\n");
				printf("\n");
			}
		}
	}

	page_Break();

	return play;
}

/*
 * Prompts for a seed for the random number generator. A seed of 
 * zero of any invalid input is interpreted as time(NULL).
 */
void prompt_For_Seed(void)
{
	char buffer[100] = {0};
	char* remainder = buffer;

	prompt_For(buffer, _countof(buffer),
			   "Enter a seed for the random number generator\n(invalid input interpreted as time(NULL)): ");

	int seed = strtol(buffer, &remainder, 10);

	if (remainder == buffer || *remainder != '\0')
	{
		srand((int)time(NULL));
	}
	else
	{
		srand(seed);
	}

	srand(seed);

	page_Break();
}

/*
 * Program entry point.
 */
int main()
{
	prompt_For_Seed();
	
	while (prompt_For_Play())
	{
		run_Game();
	}
}

