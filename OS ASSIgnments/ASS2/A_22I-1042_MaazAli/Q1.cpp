//Maaz Ali
//CS A
//22i-1042
//Q1

#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstring>
#include <cerrno>
#include <ctime>
#include <termios.h>

using namespace std;



struct Game_Grid
{
    int n;
    char Grid[25][25];
    int sp_count1 = 0;
};

Game_Grid *GM_Grid;

int n; // grid size n x n
char sp1 = '*', sp2 = '$', sp3 = '#';
int sp1_score = 10, sp2_score = 20, sp3_score = 30;
int sp_count = 0;
int *score = NULL;
int num_player = 0;

struct coord
{
    int x = 0, y = 0;
};

coord *player;

struct Message
{
    bool colection = false;
    char collected_char = 'N';
    int p_id;
    bool terminated = false;
    char priv_collected_char = '0';

    coord Co;
};

Message *msg;
//Function from ncurses lib to input without echo on terminal and without pressing enter
char echo_func()
{
    char buf = 0;
    struct termios old = {0};
    fflush(stdout);
    if (tcgetattr(0, &old) < 0)
    {
        perror("tcsetattr()");
    }
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSANOW, &old) < 0)
    {
        perror("tcsetattr ICANON");
    }
    if (read(0, &buf, 1) < 0)
    {
        perror("read()");
    }
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if (tcsetattr(0, TCSADRAIN, &old) < 0)
    {
        perror("tcsetattr ~ICANON");
    }
    return (buf);
}

int calc_grid_size();
void display();
void create_start(int player1);
void set_grid(int player1);
string SetMessage(Message msg1);
Message DecodeMessage(char s[], int size);
void child_process(int num_player, int i);
void parent_process(int num_player);

int main()
{

    n = calc_grid_size();
    int shmid = shmget(IPC_PRIVATE, sizeof(struct Game_Grid), IPC_CREAT | 0666);
    if (shmid == -1)
    {
        perror("shmget");
        exit(EXIT_FAILURE);
    }
    GM_Grid = (struct Game_Grid *)shmat(shmid, NULL, 0);
    if (GM_Grid == (void *)-1)
    {
        perror("shmat");
        exit(EXIT_FAILURE);
    }
    GM_Grid->n = n;
    for (int i = 0; i < 25; i++)
    {
        for (int j = 0; j < 25; j++)
        {
            GM_Grid->Grid[i][j] = '0';
        }
    }

    cout << "\nn=" << n;
    cout << "\nEnter Number of players (min 2) ";
    do
    {
        cout << "choice:";
        cin >> num_player;
        cout << endl;
    } while (num_player < 2 || num_player > n || num_player == n);
    player = new coord[num_player];
    msg = new Message[num_player];
    create_start(num_player);
    set_grid(num_player);
    display();
    cout<<"\nWelcome to Adventure Quest: Galactic Pursuit Game.";
    cout<<"\nPress 'w' to move up.";
    cout<<"\nPress 's' to move down.";
    cout<<"\nPress 'a' to move left.";
    cout<<"\nPress 'd' to move right.";

    int pipe_fd = mkfifo("Hello", 0666 | O_CREAT);
    if (pipe_fd == -1)
    {
        perror("mkfifo");
        exit(EXIT_FAILURE);
    }
    int pid;
    for (int i = 0; i < num_player; i++)
    {
        pid = fork();
        if (pid == 0)
        {
            child_process(num_player, i);

            shmdt((void *)GM_Grid);
            // child_exiting_now

            int pipe_fd = open("Hello", O_WRONLY);
            string t = "TERMINATING." + to_string(i);
            t += '\0';
            write(pipe_fd, t.c_str(), t.length());
            close(pipe_fd);
            exit(0);
        }
    }

    if (pid != 0)
    {
        parent_process(num_player);
    }
    unlink("Hello");
    shmdt((void *)GM_Grid);
    shmctl(shmid, IPC_RMID, NULL);
    delete[] player;
    delete[] msg;
    delete[] score;
    cout<<"\nGame Over. Thanks for playing.\n";
    return 0;
}
void child_process(int num_player, int i)
{
    char choice = 'N';
    while (sp_count != 0 && choice != 'p' && GM_Grid->sp_count1 != 0)
    {
        switch (choice)
        {
            // case 'W': // for up
        case 'w':
            if (msg[i].Co.x > 0)
            {
                GM_Grid->Grid[msg[i].Co.x][msg[i].Co.y] = msg[i].priv_collected_char;
                int var = i + 49;
                msg[i].Co.x--;
                msg[i].priv_collected_char = GM_Grid->Grid[msg[i].Co.x][msg[i].Co.y];
                if (GM_Grid->Grid[msg[i].Co.x][msg[i].Co.y] == sp1)
                {
                    msg[i].colection = true;
                    msg[i].collected_char = sp1;
                    msg[i].priv_collected_char = (char)var;
                    GM_Grid->sp_count1--;
                    sp_count--;
                }
                else if (GM_Grid->Grid[msg[i].Co.x][msg[i].Co.y] == sp2)
                {
                    msg[i].colection = true;
                    msg[i].collected_char = sp2;
                    msg[i].priv_collected_char = (char)var;
                    GM_Grid->sp_count1--;
                    sp_count--;
                }
                else if (GM_Grid->Grid[msg[i].Co.x][msg[i].Co.y] == sp3)
                {
                    msg[i].colection = true;
                    msg[i].collected_char = sp3;
                    msg[i].priv_collected_char = (char)var;
                    sp_count--;
                    GM_Grid->sp_count1--;
                }
                else
                {
                    msg[i].colection = false;
                    msg[i].collected_char = 'N';
                }

                GM_Grid->Grid[msg[i].Co.x][msg[i].Co.y] = (char)var;
                int pipe_fd = open("Hello", O_WRONLY);
                string t = SetMessage(msg[i]);
                write(pipe_fd, t.c_str(), t.length());
                close(pipe_fd);
            }
            break;
            // case 'A':
        case 'a':
            if (msg[i].Co.y > 0)
            {
                GM_Grid->Grid[msg[i].Co.x][msg[i].Co.y] = msg[i].priv_collected_char;

                int var = i + 49;
                msg[i].Co.y--;
                msg[i].priv_collected_char = GM_Grid->Grid[msg[i].Co.x][msg[i].Co.y];
                if (GM_Grid->Grid[msg[i].Co.x][msg[i].Co.y] == sp1)
                {
                    msg[i].colection = true;
                    msg[i].collected_char = sp1;
                    msg[i].priv_collected_char = (char)var;

                    GM_Grid->sp_count1--;
                    sp_count--;
                }
                else if (GM_Grid->Grid[msg[i].Co.x][msg[i].Co.y] == sp2)
                {
                    msg[i].colection = true;
                    msg[i].collected_char = sp2;
                    msg[i].priv_collected_char = (char)var;
                    GM_Grid->sp_count1--;
                    sp_count--;
                }
                else if (GM_Grid->Grid[msg[i].Co.x][msg[i].Co.y] == sp3)
                {
                    msg[i].colection = true;
                    msg[i].collected_char = sp3;
                    msg[i].priv_collected_char = (char)var;
                    GM_Grid->sp_count1--;
                    sp_count--;
                }
                else
                {
                    msg[i].colection = false;
                    msg[i].collected_char = 'N';
                }
                GM_Grid->Grid[msg[i].Co.x][msg[i].Co.y] = (char)var;
                int pipe_fd = open("Hello", O_WRONLY);
                string t = SetMessage(msg[i]);
                write(pipe_fd, t.c_str(), t.length());
                close(pipe_fd);
            }
            break;
            // case 'S':
        case 's':
            if (msg[i].Co.x < n - 1)
            {
                GM_Grid->Grid[msg[i].Co.x][msg[i].Co.y] = msg[i].priv_collected_char;
                int var = i + 49;
                msg[i].Co.x++;
                msg[i].priv_collected_char = GM_Grid->Grid[msg[i].Co.x][msg[i].Co.y];

                if (GM_Grid->Grid[msg[i].Co.x][msg[i].Co.y] == sp1)
                {
                    msg[i].colection = true;
                    msg[i].collected_char = sp1;
                    msg[i].priv_collected_char = (char)var;
                    GM_Grid->sp_count1--;
                    sp_count--;
                }
                else if (GM_Grid->Grid[msg[i].Co.x][msg[i].Co.y] == sp2)
                {
                    msg[i].colection = true;
                    msg[i].collected_char = sp2;
                    msg[i].priv_collected_char = (char)var;
                    GM_Grid->sp_count1--;
                    sp_count--;
                }
                else if (GM_Grid->Grid[msg[i].Co.x][msg[i].Co.y] == sp3)
                {
                    msg[i].colection = true;
                    msg[i].collected_char = sp3;
                    msg[i].priv_collected_char = (char)var;
                    GM_Grid->sp_count1--;
                    sp_count--;
                }
                else
                {
                    msg[i].colection = false;
                    msg[i].collected_char = 'N';
                }
                GM_Grid->Grid[msg[i].Co.x][msg[i].Co.y] = (char)var;
                int pipe_fd = open("Hello", O_WRONLY);
                string t = SetMessage(msg[i]);
                write(pipe_fd, t.c_str(), t.length());
                close(pipe_fd);
            }
            break;
            // case 'D':
        case 'd':
            if (msg[i].Co.y < n - 1)
            {
                GM_Grid->Grid[msg[i].Co.x][msg[i].Co.y] = msg[i].priv_collected_char;
                int var = i + 49;
                msg[i].Co.y++;
                msg[i].priv_collected_char = GM_Grid->Grid[msg[i].Co.x][msg[i].Co.y];

                if (GM_Grid->Grid[msg[i].Co.x][msg[i].Co.y] == sp1)
                {
                    msg[i].colection = true;
                    msg[i].collected_char = sp1;
                    msg[i].priv_collected_char = (char)var;
                    GM_Grid->sp_count1--;
                    sp_count--;
                }
                else if (GM_Grid->Grid[msg[i].Co.x][msg[i].Co.y] == sp2)
                {
                    msg[i].colection = true;
                    msg[i].collected_char = sp2;
                    msg[i].priv_collected_char = (char)var;
                    GM_Grid->sp_count1--;
                    sp_count--;
                }
                else if (GM_Grid->Grid[msg[i].Co.x][msg[i].Co.y] == sp3)
                {
                    msg[i].colection = true;
                    msg[i].collected_char = sp3;
                    msg[i].priv_collected_char = (char)var;
                    GM_Grid->sp_count1--;
                    sp_count--;
                }
                else
                {
                    msg[i].colection = false;
                    msg[i].collected_char = 'N';
                }
                GM_Grid->Grid[msg[i].Co.x][msg[i].Co.y] = (char)var;
                int pipe_fd = open("Hello", O_WRONLY);
                string t = SetMessage(msg[i]);
                write(pipe_fd, t.c_str(), t.length());
                close(pipe_fd);
            }
            break;
        default:
            break;
        }
        choice = echo_func();
    }
}
void parent_process(int num_player)
{

    score = new int[num_player];
    for (int i = 0; i < num_player; i++)
    {
        score[i] = 0;
    }

    int var = num_player;
    while (sp_count != 0 && GM_Grid->sp_count1 != 0 && var != 0)
    {
        for (int i = 0; i < num_player; i++)
        {
            int pipe_fd = open("Hello", O_RDONLY);
            char t[14];
            system("clear");
            if (read(pipe_fd, t, sizeof(t)) == -1)
            {
                GM_Grid->sp_count1 = 0;
                break;
            }
            string Term = "TERMINATING." + to_string(i);
            if (!strcmp(Term.c_str(), t))
            {
                msg[i].terminated = true;
                cout << "\nPlayer: " << i + 1 << " has been terminated\n";
                var--;
            }
            Message temp = DecodeMessage(t, 14);
            if (temp.colection == true)
            {
                temp.colection = false;
                if (temp.collected_char == sp1)
                {
                    score[temp.p_id] += sp1_score;
                }
                else if (temp.collected_char = sp2)
                {
                    score[temp.p_id] += sp2_score;
                }
                else
                {
                    score[temp.p_id] += sp3_score;
                }
            }
            display();
        }
    }

    for (int i = 0; i < num_player; i++)
    {
        wait(NULL);
    }

}
string SetMessage(Message msg1)
{
    string message = "";
    message += to_string(msg1.p_id);
    message += to_string((int)msg1.colection);
    message += msg1.collected_char;
    message += to_string((int)msg1.Co.x) + to_string((int)msg1.Co.y);
    message += '\0';
    return message;
}
Message DecodeMessage(char s[], int size)
{
    Message temp;
    temp.p_id = ((int)s[0]) - 48;
    temp.colection = (bool)((int)s[1] - 48);
    temp.collected_char = s[2];

    temp.Co.x = ((int)s[3] - 48);
    temp.Co.y = ((int)s[4] - 48);

    return temp;
}
int calc_grid_size()
{

    int roll_no = 1042, last_digit = 2; // roll no =22i-1042
    srand(time(NULL));
    int rand_num = rand() % 90 + 10;
    rand_num *= last_digit;
    roll_no /= rand_num;
    roll_no %= 25;
    if (roll_no < 10)
    {
        roll_no += 15;
    }

    return roll_no;
}

void display()
{
    // cout << "\nDisplaying the grid:\n";
    if (score != NULL)
    {
        for (int j = 0; j < num_player; j++)
        {
            int var = j + 49;
            cout << "\nPlayer " << (char)var << " : Score=" << score[j];
        }
        cout << endl;
    }

    for (int j = 0; j < n; j++)
    {
        cout << "______";
    }
    cout << "_";
    for (int i = 0; i < n; i++)
    {
        cout << endl;
        cout << "|  ";
        for (int j = 0; j < n; j++)
        {
            if (GM_Grid->Grid[i][j] != '0' && GM_Grid->Grid[i][j] != sp1 && GM_Grid->Grid[i][j] != sp2 && GM_Grid->Grid[i][j] != sp3)
            {
                cout << GM_Grid->Grid[i][j] << "> |  ";
            }
            else
            {
                cout << "   |  ";
            }
        }
        cout << endl;
        for (int j = 0; j < n; j++)
        {
            cout << "|_____";
        }
        cout << "|";
    }
    cout << endl;
}
void create_start(int player1)
{
    srand(time(NULL));
    for (int i = 0; i < player1; i++)
    {
        int randx = rand() % n;
        int randy = rand() % n;
        if (GM_Grid->Grid[randx][randy] == '0')
        {
            player[i] = {randx, randy};
            int var = i + 49;
            cout << endl
                 << "\nPlayer " << i << ": ' " << (char)var << " ' ";
            GM_Grid->Grid[player[i].x][player[i].y] = (char)var;
            msg[i].Co.x = randx;
            msg[i].Co.y = randy;
            msg[i].p_id = i;
        }
        else
        {
            i--;
        }
    }
}

void set_grid(int player1)
{
    srand(time(NULL));
    int count_collect = rand() % n;
    int c1 = 0, c2 = 0;
    c1 = count_collect * 3;
    sp_count = c1;
    GM_Grid->sp_count1 = c1;
    for (int i = 0; i < count_collect; i++)
    {
        int randx = rand() % n;
        int randy = rand() % n;
        if (GM_Grid->Grid[randx][randy] == '0')
        {
            GM_Grid->Grid[randx][randy] = sp1;
            c2++;
        }
        else
        {
            i--;
        }
    }
    for (int i = 0; i < count_collect; i++)
    {
        int randx = rand() % n;
        int randy = rand() % n;
        if (GM_Grid->Grid[randx][randy] == '0')
        {
            GM_Grid->Grid[randx][randy] = sp2;
            c2++;
        }
        else
        {
            i--;
        }
    }
    for (int i = 0; i < count_collect; i++)
    {
        int randx = rand() % n;
        int randy = rand() % n;
        if (GM_Grid->Grid[randx][randy] == '0')
        {
            GM_Grid->Grid[randx][randy] = sp3;
            c2++;
        }
        else
        {
            i--;
        }
    }
}
