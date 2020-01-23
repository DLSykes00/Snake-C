//C Programming Assignment ACS130-006
//Author: Daniel Sykes
//Date: 05/2019

/* Snake game written in C (C99)
- Only compatible with windows
- May have issues with older compilers
*/

#include <stdio.h>
#include <stdlib.h>

#include <unistd.h> //For Sleep Function
#include <conio.h> //More input / output options
#include <windows.h> //More control over console window
#include <string.h>

#define WIDTH 40 //Level Width
#define HEIGHT 40 //Level Height

struct snake{
    int length;
    int score;
    int snakeCoords[2][500]; //Allocate array to hold snake piece coordinate pairs. Access via [0][index of x] / [1][index of y].
};                           //First coordinate pair = head of snake.

struct food{
    int foodX;
    int foodY;
    int foodAvaialable;
};

struct scores{
    char name[50];
    int score;
};

void runGame(); //Contains main game loop
void menu(); //Show and handle menu interaction
int parseScores(FILE *f, struct scores highScores[]); //Interpret and sort scores from file. Returns number of scores.
void drawLevel(HANDLE output); //Sets up console and draws the level borders
void drawSnake(HANDLE output, struct snake s); //Draw snake pieces to the screen
void drawFood(HANDLE output, struct food *f, struct snake s); //Draw food to screen
void checkForKeyPress(char *playerDirection); //Check for WASD input
void checkIfDead(HANDLE output, struct snake s); //Check if snake is dead (Hit border or its own tail)
void death(int score); //Handle death event
void showLeaderboard(); //Display leaderboard scores
void setConsoleMenuSettings(); //Adjust console window to make menu interaction font more pleasing


int main()
{
    //Start application by launching menu
    menu();

    return 0;
}


void runGame(){
    HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);

    //Make character sizes equal in terms of pixels (so coordinates are 1:1 ratio)
    CONSOLE_FONT_INFOEX font_info;
    COORD fontSize = { 15, 15 };
    font_info.cbSize = sizeof(CONSOLE_FONT_INFOEX);
    font_info.dwFontSize = fontSize;
    SetCurrentConsoleFontEx(output, 0, &font_info);

    char playerDirection = 'N'; //N E S W - North East South West

    //Initialise players snake
    struct snake playerSnake;
    playerSnake.score = 0;
    playerSnake.length = 3;

    //Set starting positions
    playerSnake.snakeCoords[0][0] = WIDTH / 2;
    playerSnake.snakeCoords[1][0] = HEIGHT / 2;

    playerSnake.snakeCoords[0][1] = (WIDTH / 2);
    playerSnake.snakeCoords[1][1] = (HEIGHT / 2)+1;

    playerSnake.snakeCoords[0][2] = WIDTH / 2;
    playerSnake.snakeCoords[1][2] = (HEIGHT / 2) + 2;

    //Initialise food
    struct food snakeFood;
    snakeFood.foodX = 0;
    snakeFood.foodY = 0;
    snakeFood.foodAvaialable = 0;

    char gameRunning = 1;
    //Draw Level Border
    drawLevel(output);

    while(gameRunning){

        checkIfDead(output, playerSnake);

        checkForKeyPress(&playerDirection);

        //Check if snake ate food
        if( (playerSnake.snakeCoords[0][0] == snakeFood.foodX) && (playerSnake.snakeCoords[1][0] == snakeFood.foodY) ){
            playerSnake.length++;
            playerSnake.score += 10;
            snakeFood.foodAvaialable = 0;
        }

        //Remove Tail Piece from snake if the snake hasn't just eaten a piece of food (i.e. food is available)
        if(snakeFood.foodAvaialable == 1){
            COORD clearBitPos;
            clearBitPos.X = playerSnake.snakeCoords[0][playerSnake.length-1];
            clearBitPos.Y = playerSnake.snakeCoords[1][playerSnake.length-1];
            SetConsoleCursorPosition(output, clearBitPos);
            printf(" ");
        }

        //Shift other snake bits 1 place to the right each
        for(int i = playerSnake.length - 2; i >= 0; i--){ // i = element before previous snake tail.
            playerSnake.snakeCoords[0][i+1] = playerSnake.snakeCoords[0][i]; //Shift it to the right 1 place
            playerSnake.snakeCoords[1][i+1] = playerSnake.snakeCoords[1][i];
        }

        //Insert new head 1 place in front
        switch(playerDirection){
            case 'N':
                playerSnake.snakeCoords[0][0] = (playerSnake.snakeCoords[0][1]);
                playerSnake.snakeCoords[1][0] = (playerSnake.snakeCoords[1][1]) - 1;
                break;
            case 'E':
                playerSnake.snakeCoords[0][0] = (playerSnake.snakeCoords[0][1]) + 1;
                playerSnake.snakeCoords[1][0] = (playerSnake.snakeCoords[1][1]);
                break;
            case 'S':
                playerSnake.snakeCoords[0][0] = (playerSnake.snakeCoords[0][1]);
                playerSnake.snakeCoords[1][0] = (playerSnake.snakeCoords[1][1]) + 1;
                break;
            case 'W':
                playerSnake.snakeCoords[0][0] = (playerSnake.snakeCoords[0][1]) - 1;
                playerSnake.snakeCoords[1][0] = (playerSnake.snakeCoords[1][1]);
                break;
        }

        drawSnake(output, playerSnake);

        //If no food available then spawn another piece.
        if(snakeFood.foodAvaialable == 0){
            drawFood(output, &snakeFood, playerSnake);
            snakeFood.foodAvaialable = 1;
        }

        Sleep(100);
    }
}

void menu(){
    system("cls");
    setConsoleMenuSettings();

    printf("S: Start Game\nL: Leaderboard\n\nQ: Quit Game\n");

    //Get user selection choice
    char userChoice;
    while( (userChoice != 's' && userChoice != 'S') && (userChoice != 'q' && userChoice != 'Q') && (userChoice != 'l' && userChoice != 'L') ){
        fflush(stdin);
        userChoice = getchar();

        if( (userChoice != 's' && userChoice != 'S') && (userChoice != 'q' && userChoice != 'Q') && (userChoice != 'l' && userChoice != 'L') ){
            printf("\nInvalid Choice\n\n");
        }
    }

    //Start Game
    if(userChoice == 's' || userChoice == 'S'){
        system("cls");
        runGame();
    }
    //Show Leaderboard
    if(userChoice == 'l' || userChoice == 'L'){
        showLeaderboard();
    }
    //Quit Game
    if(userChoice == 'q' || userChoice == 'Q'){
        exit(1);
    }
}

int parseScores(FILE *f, struct scores highScores[]){

    //Get 5 top scores from file
    int numberOfScores = 0;
    int tempScore;
    char tempName[20];

    //Using %[^;] instead of %s means string stops once ';' is found which is used as a delimiter.
    while(fscanf(f, "%[^;];%d\n", tempName, &tempScore) != EOF){

        strcpy(highScores[numberOfScores].name, tempName);
        highScores[numberOfScores].score = tempScore;
        numberOfScores++;
    }

    struct scores tempHighScore;
    char swapOccured;

    //Sort into order (bubble sort)
    for(int i = 0; i < numberOfScores; i++){
        swapOccured = 0;
        for(int j = 0; j < numberOfScores-1; j++){
            if(highScores[j].score < highScores[j+1].score){
                tempHighScore = highScores[j+1];
                highScores[j+1] = highScores[j];
                highScores[j] = tempHighScore;

                swapOccured = 1;
            }
        }
        if(swapOccured == 0){
            break;
        }
    }
    return numberOfScores;
}

void drawLevel(HANDLE output){
    SMALL_RECT windowSize = {0 , 0 , WIDTH , HEIGHT}; //Set console windows dimensions
    SetConsoleWindowInfo(output, TRUE, &windowSize);

    COORD top_left = {0, 0};
    //Draw  top horizontal level wall
    SetConsoleCursorPosition(output, top_left);
    printf(" ");
    for(int w = 1; w < WIDTH; w++){
        printf("-");
    }
    printf("\n");

    COORD nextPos = {0, 0};
    //Draw vertical level walls
    for(int i = 1; i < HEIGHT; i++){
        nextPos.X = 0; nextPos.Y = i;
        printf("|");
        nextPos.X = WIDTH; nextPos.Y = i;
        SetConsoleCursorPosition(output, nextPos);
        printf("|\n");
    }
    //Draw bottom horizontal level wall
    printf(" ");
    for(int w = 1; w < WIDTH; w++){
        printf("-");
    }
}

void drawSnake(HANDLE output, struct snake s){

    COORD snakeBitPos;

    snakeBitPos.X = s.snakeCoords[0][0];
    snakeBitPos.Y = s.snakeCoords[1][0];

    SetConsoleCursorPosition(output, snakeBitPos);

    printf("O");

    for(int i = 1; i < s.length; i++){ //Draw snake tail

        snakeBitPos.X = s.snakeCoords[0][i];
        snakeBitPos.Y = s.snakeCoords[1][i];

        SetConsoleCursorPosition(output, snakeBitPos);

        printf("o");
    }
}

void drawFood(HANDLE output, struct food *f, struct snake s){

    char validFoodPos = 0;
    COORD foodPos;

    while(validFoodPos == 0){
        //srand() needed to avoid pattern in rng
        foodPos.X = (rand() % (WIDTH-1))+1;
        foodPos.Y = (rand() % (HEIGHT-1))+1;
        validFoodPos = 1;

        //Check food isn't inside snake
        for(int i = 0; i<s.length; i++){
            if( (s.snakeCoords[0][i] == foodPos.X) && (s.snakeCoords[1][i] == foodPos.Y)){
                validFoodPos = 0;
            }
        }
    }

    f->foodY = foodPos.Y;
    f->foodX = foodPos.X;

    //Print '+' in food location for player to see
    SetConsoleCursorPosition(output, foodPos);
    printf("+");
}

void checkForKeyPress(char *playerDirection){
    if(kbhit()){
        char c = getch();
        if(c=='w' || c=='W'){
            if(*playerDirection != 'S'){
                *playerDirection = 'N';
            }
        }else if(c=='a' || c=='A'){
            if(*playerDirection != 'E'){
                *playerDirection = 'W';
            }
        }else if(c=='s' || c=='S'){
            if(*playerDirection != 'N'){
                *playerDirection = 'S';
            }
        }else if(c=='d' || c=='D'){
            if(*playerDirection != 'W'){
                *playerDirection = 'E';
            }
        }
    }
}

void checkIfDead(HANDLE output, struct snake s){
    char isDead = 0;

    //Check if snake has hit itself
    for(int i = 1; i < s.length; i++){
        if( (s.snakeCoords[0][0] == s.snakeCoords[0][i]) && (s.snakeCoords[1][0] == s.snakeCoords[1][i]) ){
            isDead = 1;
        }
    }
    //Check if snake hit walls
    if( (s.snakeCoords[0][0] <= 0 || s.snakeCoords[0][0] >= WIDTH) || (s.snakeCoords[1][0] <= 0 || s.snakeCoords[1][0] >= HEIGHT) ){
        isDead = 1;
    }

    if(isDead){
        COORD headPos;
        headPos.X = s.snakeCoords[0][0];
        headPos.Y = s.snakeCoords[1][0];

        SetConsoleCursorPosition(output, headPos);
        printf("X");
        Sleep(500);
        death(s.score);
    }
}

void death(int score){
    system("cls");

    setConsoleMenuSettings();

    printf("Score: %d\n", score);

    //Load in high scores for comparison to check if user got a top 5 score.
    struct scores allHighScores[5];
    FILE *f;
    f = fopen("scores.txt", "r");

    if(f == 0){ //If the file doesn't exist, create one
        fclose(f);
        printf("\nNo scores file found,\nsaving your score to 'scores.txt'.\n");
        f = fopen("scores.txt", "w+");
    }

    int numberOfScores = parseScores(f, allHighScores);

    fclose(f);

    //Add high score if score greater than lowest top 5 score, or there is less than 5 scores in total.
    if((score > allHighScores[numberOfScores-1].score) || (numberOfScores < 5 )){
            if(numberOfScores < 5){
                numberOfScores++;
            }

            char username[20];

            printf("\nTop 5 Score!\n\n");
            printf("Enter Name: ");

            fflush(stdin);
            gets(username);
            //Check for valid input
            while( (strchr(username, ';')!= NULL) || (strlen(username) >  15)){
                printf("\nInvalid Name\n\n");
                printf("Enter Name: ");

                fflush(stdin);
                gets(username);
            }

            //If user didn't enter name assign some place-holder name
            if(username[0] == '\0'){
                strcpy(username, "noName");
            }

            //Replace lowest top 5 score with new top 5 score just made
            strcpy(allHighScores[numberOfScores-1].name, username);
            allHighScores[numberOfScores-1].score = score;

            //Write new top 5 scores list to file
            f = fopen("scores.txt", "w");
            for(int i = 0; i < numberOfScores; i++){
                fprintf(f, "%s;%d\n", allHighScores[i].name, allHighScores[i].score);
            }
            fclose(f);
        }

    printf("\n\nPress Enter To Return To The Menu\n");
    fflush(stdin);
    getchar();
    menu();
}

void showLeaderboard(){
    system("cls");

    //Get top 5 sorted scores from file
    struct scores highScores[5];
    FILE *f;
    f = fopen("scores.txt", "r");
    int numberOfScores = parseScores(f, highScores);
    fclose(f);

    //Output top scores in a list
    printf("Leaderboard (Top 5 Scores)\n\n");
    for(int i = 0; i < numberOfScores; i++){
        printf("%d- %s: %d\n", i+1, highScores[i].name, highScores[i].score);
    }

    printf("\n\nPress Enter To Return To The Menu\n");
    fflush(stdin);
    getchar();
    menu();
}

void setConsoleMenuSettings(){
    HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);

    //Make console cursor not visible to the user
    CONSOLE_CURSOR_INFO cursor_info;
    cursor_info.dwSize = 100;
    cursor_info.bVisible = FALSE;
    SetConsoleCursorInfo(output, &cursor_info);

    //Set console window size
    SMALL_RECT windowSize = {0 , 0 , WIDTH , HEIGHT/2};
    SetConsoleWindowInfo(output, TRUE, &windowSize);

    //Set font size to something more readable in the menu
    CONSOLE_FONT_INFOEX font_info;
    COORD fontSize = { 15, 30 };

    font_info.cbSize = sizeof(CONSOLE_FONT_INFOEX);
    font_info.dwFontSize = fontSize;

    SetCurrentConsoleFontEx(output, 0, &font_info);

}
