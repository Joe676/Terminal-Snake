/*
Gra Snake
Laboratorium Podstawy Programowania - Projekt
26 X 2019 - 28 X 2019
by J. Bossowski
*/
#define _WIN32_WINNT 0x0500
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

const int SCOREQ = 20;

int hx, hy, snakel, tailx, taily;//coords of the head, length, coords of the tail
int width, height, speed;//size of the board, refresh speed(frame rate)
bool walls;
short dir;//direction in which the snake moves
int *map;//board
int score;//current score
bool appleOnMap = false;//if not -> generate apple
int eaten = 0;//if > 0 -> don't delete tail
int appleTimer;//timer for changing fruit kind
int appleI;//random apple index
clock_t start;//time of game's start
clock_t curtime;//time now
int timesec;//time in seconds
bool go;//gameover
FILE *scores;//file with *SCOREQ* best scores
char ini[20];//initials

char names[SCOREQ][20];//array of names in the leaderboard
int tab[SCOREQ];//array of scores in the leaderboard

void menu();
void game();
void draw();
void getInput();
void color(int);
int index(int, int);
void findTail();
bool outOfBounds(int, int);
void gameover();
void randomApple();
int teleport(int, bool);
void showLeaderboard();

void initScores();
void getScores();
void setScores();
void addScore(char*, int);

int main()
{
    
    HWND console = GetConsoleWindow();
    RECT r;
    GetWindowRect(console, &r);
    MoveWindow(console, r.left, r.top, 800, 800, TRUE);
    
    scores = fopen("scores.txt", "r");
    if(scores != NULL) //checking if file exists
    {
	    fseek(scores, 0, SEEK_END); 
	    if(!ftell(scores))//if the end of the file is at 0th index -> file is empty
        {
            fclose(scores);
            initScores();
        }
    }
    else initScores();
    
    getScores();

    srand(time(NULL));
    menu();

    delete [] map;
    //Sleep(10000);

    return 0;
}

//main menu logic
void menu()
{
    system("CLS");
    printf("\tSNAKE\n\n1. %26s\n2. %26s\n3. %26s\n", "Start the game", "Leaderboard", "Quit");
    char choice;
    do
    {
        printf("Please, choose valid option: ");
        scanf("%d", &choice);
    } while (choice < 1 || choice > 3);

    switch (choice)
    {
    case 1:
        game();
        break;
    case 2:
        showLeaderboard();
        break;
    case 3:
        exit(0);
        break;
    default:
        break;
    }
    
}

//game logic & more
void game()
{
    system("CLS");
    printf("Choose board width 10 - 30:\n");
    scanf("%d", &width);
    //constrain width to valid options
    width = width>30 ? 30 : (width<10 ? 10 : width);
    
    system("CLS");
    printf("Choose board height 10 - 30:\n");
    scanf("%d", &height);
    //constrain height to valid options
    height = height>30 ? 30 : (height<10 ? 10 : height);

    system("CLS");
    printf("Choose speed 1 - 10:\n");
    scanf("%d", &speed);
    //consstrain speed to valid options
    speed = speed>10 ? 10: (speed<1 ? 1 : speed);

    system("CLS");
    printf("Choose the walls setting:\n1. OFF\n2. ON\n");
    int w;
    do{scanf("%d", &w);}while(w < 1 || w > 2);
    while (getchar() != '\n'); //clear the buffer
    walls = (w==2);

    system("CLS");
    printf("Enter your nick:\n");
    scanf("%s", &ini);

    map = new int[width*height];
    for(int i = 0; i<width*height; i++)//filling map with zeroes to get rid of some strange bug
    {
        map[i]=0;
    }

    appleOnMap = false;
    go = false;
    start = clock();

    hx = 0;
    hy = 0;

    score = 0;
    snakel = 1;
    tailx = hx;
    taily = hy;
    dir = 1; //1 - right, 2 - up, 3 - left, 4 - down

    map[index(hx, hy)] = 0;     //map legend: 1..4 - direction of the tail, 5 - apple, 6 - berry, 7 - banana
    //int run = 30;               //debug
    bool run = true;
    while(run)
    {
        if(go)run=false;
        else{
        //if(run==4)dir=4;      //debug

        getInput();
        //move head delete tail
        switch (dir)//moving the head
        {
        case 1:
            hx ++;
            break;
        case 2:
            hy --;
            break;
        case 3:
            hx --;
            break;
        case 4:
            hy ++;
            break;
        
        default:
            break;
        }
        //printf(outOfBounds(hx, hy) ? "true" : "false");   //debug

        //HITTING STUFF
        if(map[index(hx, hy)]<=4 && map[index(hx, hy)]>=1)//hitting self
        {
            gameover();
            break;
        }

        if(walls && outOfBounds(hx, hy))//wall hit
        {
            gameover();
            break;
        }
        else if(!walls)//no wall hit -> teleportation
        {
            hx = teleport(hx, 1);
            hy = teleport(hy, 0);
        }
        //hitting apple after teleportation, for correct checks
        if (map[index(hx, hy)] == 5)//hitting apple
        {
            snakel++;
            score += 100;
            eaten += 1;
            appleOnMap = false;
            appleTimer = 0;
        }
        else if (map[index(hx, hy)] == 6)//hitting berry
        {
            snakel+=2;
            score += 300;
            eaten += 2;
            appleOnMap = false;
            appleTimer = 0;
        }
        else if (map[index(hx, hy)] == 7)//hitting banana
        {
            snakel+=3;
            score += 500;
            eaten += 3;
            appleOnMap = false;
            appleTimer = 0;
        }
        //END HITTING

        //changing fruit
        if(appleTimer)appleTimer--;
        if(appleTimer==16 && map[appleI]==7)
        {
            map[appleI] = 6;
        }else if(appleTimer == 1 && map[appleI]==6)
        {
            map[appleI] = 5;
        }

        map[index(hx, hy)] = (dir + 1) % 4 + 1; //saving the direction to the previous segment
        if(!eaten)
        {
            map[index(tailx, taily)] = 0;//deleting the tail
        }
        else
        {
            eaten--;
        }

        findTail();

        if(!appleOnMap)//apple generation
        {
            randomApple();
            appleOnMap = true;
        }
        curtime = clock() - start;//clock update
        timesec = int(curtime)/CLOCKS_PER_SEC;
        draw();
        
        printf("\n");
        //run = run - 1;                      //debug
        Sleep(2000/speed);
    }
    }
}

//shows the game
void draw()
{
    system("CLS");
    SetConsoleTextAttribute( hConsole, 240);//HUD text - black on white
    printf("SCORE: %-7d%*s%02d:%02d\n", score, 2*width-18, "TOTAL: ", timesec/60, timesec%60);
    SetConsoleTextAttribute( hConsole, 15);//normal text - white on black
    
    for(int i = 0; i < height; i++)
    {

        for (int j = 0; j < width; j++)
        {

            color(map[index(j, i)]);                      //Right printing
            printf(" #");
            //printf("%d", map[index(j, i)]);                 //VSCode terminal compatible debug
        }
        
        printf("\n");
    }
    
    SetConsoleTextAttribute( hConsole, 240);
    printf("Control with arrows%*s\n", 2*width - 18, "Quit - Q");
    SetConsoleTextAttribute( hConsole, 15);
}

//returns linear index of the map
int index(int x, int y)
{
    return x + y * width;
}

//find new tail coords
void findTail()
{
    int x = hx, y = hy; //temporary coords
    int nextx = x, nexty = y;   //help coords

    if(walls)//one check per loop
    {
        do
        {
    	    x = nextx;
    	    y = nexty;
            int temp = map[index(x, y)];
            switch (temp)
            {
            case 1:
                nextx++;
                break;
            case 2:
                nexty--;
                break;
            case 3:
                nextx--;
                break;
            case 4:
                nexty++;
                break;
            default:
                break;
            }
        } while (map[index(nextx, nexty)]>=1 && map[index(nextx, nexty)]<=4);
        tailx = x;
        taily = y;
        return;
    }
    else    //no walls -> different rules
    {
        do
        {
    	    x = nextx;
    	    y = nexty;
            int temp = map[index(x, y)];
            switch (temp)
            {
            case 1:
                nextx++;
                nextx=teleport(nextx, 1);
                break;
            case 2:
                nexty--;
                nexty=teleport(nexty, 0);
                break;
            case 3:
                nextx--;
                nextx=teleport(nextx, 1);
                break;
            case 4:
                nexty++;
                nexty=teleport(nexty, 0);
                break;
            default:
                break;
            }
        } while (map[index(nextx, nexty)]>=1 && map[index(nextx, nexty)]<=4);
        tailx = x;
        taily = y;
        return;   
    }
    
    
}

//setting character color
void color(int a)
{
    /*
    snake - green - 10
    apple - red - 4
    berries - blue - 1
    banana/orange - yellow/orange - 6
    GUI/HUD - white - 7
    */
    
    switch (a)
    {
    case 1 ... 4://snake segment
        SetConsoleTextAttribute( hConsole, 10);
        break;
    
    case 5://apple
        SetConsoleTextAttribute( hConsole, 4);
        break;
        
    case 6://berry
        SetConsoleTextAttribute( hConsole, 1);
        break;
        
    case 7://banana
        SetConsoleTextAttribute( hConsole, 6);
        break;

    default:
        SetConsoleTextAttribute( hConsole, 7);
        break;
    }
}

//getting user input during game
void getInput()
{
    //source: http://www.cplusplus.com/articles/3Cpk4iN6/; https://docs.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes

        if (GetAsyncKeyState(VK_LEFT))
        {
            if(map[index(hx, hy)]!=3)dir = 3;
        }
        else if (GetAsyncKeyState(VK_RIGHT))
        {
            if(map[index(hx, hy)]!=1)dir = 1;
        }
        else if (GetAsyncKeyState(VK_UP))
        {
            if(map[index(hx, hy)]!=2)dir = 2;
        }
        else if (GetAsyncKeyState(VK_DOWN))
        {
            if(map[index(hx, hy)]!=4)dir = 4;
        }
        else if (GetAsyncKeyState(0x51))          //quitting
        {
            delete [] map;
            exit(0);
        }
        /*
        if (GetAsyncKeyState(0x41))//A
        {
            if(map[index(hx, hy)]!=3)dir = 3;
        }
        else if (GetAsyncKeyState(0x44))//D
        {
            if(map[index(hx, hy)]!=1)dir = 1;
        }
        else if (GetAsyncKeyState(0x57))//W
        {
            if(map[index(hx, hy)]!=2)dir = 2;
        }
        else if (GetAsyncKeyState(0x53))//S
        {
            if(map[index(hx, hy)]!=4)dir = 4;
        }
        else if (GetAsyncKeyState(0x51))          //quitting
        {
            delete [] map;
            exit(0);
        }*/

}

//checks whether given point is on the board
bool outOfBounds(int x, int y)
{
    return (x>=width || x<0 || y>=height || y<0);
}

//pretty selfexplanatory
void gameover()
{
    delete [] map;//clear the memory
    //fflush(stdin);

    go = true;
    system("CLS");
    int total = score + timesec;//points = score + seconds played
    printf("\n\t\tGAME OVER\n\nYour score:\t\t\t%5d\nYour time:\t\t\t%02d:%02d\nTOTAL:\t\t\t\t%5d\n", score, timesec/60, timesec%60, total);
    //score saving
    addScore(ini, total);

    printf("1.\tGo back to main menu\n2.\tQuit\n");
    //get input
    
    char choice;
    do
    {
        scanf("%c", &choice);
    } while (choice < '1' || choice > '2');
    
    switch (choice)
    {
    case '1':
        menu();
        break;
    case '2':
        exit(0);
        break;
    
    default:
        break;
    }
}

//generating apple's position
void randomApple()
{
    do
    {
        appleI = rand() % (width*height);
    } while (map[appleI]);//try until empty spot found
    int kind = rand()%100;
    if(kind<80)map[appleI] = 5;//apple
    else if(kind<95)
    {
        map[appleI] = 6;//berry
        appleTimer = 16;
    }
    else 
    {
        map[appleI] = 7;//banana
        appleTimer = 26;
    }
}

//walking through walls
int teleport(int x, bool w)
{
    if(x>=(w==1?width:height)) //teleporting
        {
            return 0;
        }
    else if(x<0)
        {
            return (w==1?width:height) - 1;
        }
    else return x;
}

//initialize the scoreboard
void initScores()
{
    scores = fopen("scores.txt", "w");
    for(int i = 1; i <= SCOREQ; i++)
    {
        fprintf(scores, "-\t0\n");
    }
    fclose(scores);
}

//move scores from file to arrays
void getScores()
{
    scores = fopen("scores.txt", "r");
    int i = 0;
    while (!feof(scores))//if not at the end of file
    {
        fscanf(scores, "%s%d", &names[i], &tab[i]);
        i++;
    }
    fclose(scores);
}

//save new highscores
void setScores()
{
    scores = fopen("scores.txt", "w");
    for(int i = 0; i < SCOREQ; i++)
    {
        fprintf(scores, "%s\t%d\n", names[i], tab[i]);
    }
    fclose(scores);
}

//add a score to the leaderboard
void addScore(char* name, int score)
{
    int i = 0;
    while(score < tab[i])i++;
    if(i>=SCOREQ)return;
    char *help;
    for (int j = SCOREQ-1; j > i; j--)//moving all smaller scores 1 down
    {
        tab[j] = tab[j-1];
        help = names[j-1];
        
        for (int k = 0; k < strlen(help); k++)
        {
            names[j][k] = help[k];
        }
    }
    for (int k = 0; k < strlen(name); k++)
    {
        names[i][k] = name[k];
    }
    names[i][strlen(name)]='\0';
    tab[i] = score;
    setScores();
    
}

//shows *SCOREQ* highscores
void showLeaderboard()
{
    system("CLS");
    printf("\t\tLEADERBOARD\n\n");
    for(int i = 0; i < SCOREQ; i++)
    {
        printf("%02d. %-20s\t%7d\n", i+1, names[i], tab[i]);
    }
    printf("\nM - MENU%31s\n", "Q - Quit");
    
    char choice;
    do
    {
        scanf("%c", &choice);
    } while (choice!='M' && choice!='Q' && choice!='m' && choice!='q');
    
    switch (choice)
    {
    case 'M':
    case 'm':
        menu();
        break;
    case 'Q':
    case 'q':
        exit(0);
        break;
    
    default:
        break;
    }
}

//flushing GetAsyncKeyState buffer
//void flushBufer()
//{
//    GetAsyncKeyState(VK_LEFT);
//    GetAsyncKeyState(VK_UP);
 //   GetAsyncKeyState(VK_DOWN);
//    GetAsyncKeyState(VK_RIGHT);
//}