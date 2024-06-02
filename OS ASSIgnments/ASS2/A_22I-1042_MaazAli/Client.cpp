//Maaz Ali
//CS A
//22i-1042
//Q2_client
#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <cstring>

using namespace std;

const int max_groups = 10;
const int max_group_members = 20;

struct Server_Control
{
    int num_client;
    int id;
    int command;
    bool command_set = false;
    bool in_group = false;
    int in_group_id = -1;
    bool is_usr = true;
    bool execluded = false; // if true, the client will not be added  to group list or any other issue
    char chat_history[5000];
    bool is_admin = false;
    int admin_id;
    int current_g=0;
    char groupname[10][10]; // max groups=10 max characters per_group
    bool admin_grants_request = false;
};

struct Group_Join_Request
{
    int requester;
    bool request_entered = false;
    bool granted = false;
    char group_name[10];
    int G_id=-1;
};

struct Inbox
{
    char Inbox_info[4096];
    bool inbox_empty = true;
};

struct MSG
{
    bool written = false;
    bool recived = false;
    int to;
    int from;
    int mode;
    char group_name[10];
    char data[4096];
};

void MENU()
{
    cout << "\nUSEFUL COMMANDS TO MAKE CHAT REQUESTS (THROUGH SERVER).";
    cout << "\n1. TO MAKE A GROUP.";
    cout << "\n2. TO JOIN A GROUP.";
    cout << "\n3. TO LEAVE A GROUP.";
    cout << "\n4. TO VIEW CHAT HISTORY.";
    cout << "\n5. TO SEND MESSAGE TO A USR OR GROUP.";
    cout << "\n6. TO CHECK INBOX.";
    cout << "\n7. TO CHECK IF SERVER MADE REQUEST.";
    cout << "\n8. TO TERMINATE CHAT_VISTA.";
}

int main(int argc, char *argv[])
{
    int client_id = stoi(argv[1]) - 1;
    int num_usr = stoi(argv[2]);
    int shmid = stoi(argv[3]);
    int shmid_msg = stoi(argv[4]);
    int shmid_server = stoi(argv[5]);
    int shmid_join = stoi(argv[6]);
    cout << "\nTotal number of usrs:" << num_usr;
    cout << "\nClient_id:" << client_id + 1;
    
    fflush(stdout);
    Inbox *inbox;
    MSG *msg;
    Group_Join_Request *join;
    Server_Control *server;
    inbox = (struct Inbox *)shmat((key_t)shmid, NULL, 0);
    msg = (struct MSG *)shmat((key_t)shmid_msg, NULL, 0);
    server = (struct Server_Control *)shmat((key_t)shmid_server, NULL, 0);
    join = (struct Group_Join_Request *)shmat((key_t)shmid_join, NULL, 0);
    if (inbox == (void *)-1)
    {
        cout << "\nERROR";
        cin >> num_usr;
        perror("shmat inbox");
        // exit(EXIT_FAILURE);
    }
    if (msg == (void *)-1)
    {
        perror("shmat msg");
        // exit(EXIT_FAILURE);
    }
    if (server == (void *)-1)
    {
        perror("shmat server");
        // exit(EXIT_FAILURE);
    }
    if (join == (void *)-1)
    {
        perror("shmat join");
        // exit(EXIT_FAILURE);
    }
    int choice = 0;
    // handle command to server_control
    while (choice != 8)
    {
        if (server->admin_grants_request == true)
        {
            server->admin_grants_request = false;
            cout << "\nRequest routed from <SERVER> to <ADMIN> " ;
            cout << "\n<CLIENT:" << server->in_group_id+1 << "> is requesting to join group." << endl;
            cout << "Enter your Desision (0 for rejected / 1 for Granted):";
            cin >> choice;
            if (choice == 1)
            {
                server->in_group = true;
            }
            else
            {
                server->in_group = false;
            }
            choice = 0;
            /* code */
        }

        if (choice == 1)
        {
            if (server->is_admin == false)
            {
                cout << "\nENETR THE NAME OF GROUP (MAX 10 CHAR) YOU WANT TO CREATE:";
                cin.getline(join->group_name, 10);
                join->request_entered = true;
                join->requester = client_id;
                server->command = choice;
                server->command_set = true;
            }
            else
            {
                cout << "\nYOU ARE ALREADY AN ADMIN";
                join->request_entered = false;
                server->command = -1;
                server->command_set = false;
            }
        }
        else if (choice == 2)
        {
            cout << "\nENETR THE NAME OF GROUP (MAX 10 CHAR) YOU WANT TO JOIN:";
            cin.getline(join->group_name, 10);
            join->request_entered = true;
            join->requester = client_id;
            server->command = choice;
            server->command_set = true;
        }
        else if (choice == 3)
        {
            cout << "\nENETR THE NAME OF GROUP (MAX 10 CHAR) YOU WANT TO LEAVE:";
            cin.getline(join->group_name, 10);
            join->request_entered = true;
            join->requester = client_id;
            server->command = choice;
            server->command_set = true;
        }

        else if (choice == 4)
        {
            cout << "\nYour Chat History:\n";
            cout << server->chat_history;
        }
        else if (choice == 5)
        {
            cout << "\nEnter any string to send: ";
            cin.getline(msg->data, 4096);
            do
            {
                cout << "\nTO: ";
                cin >> msg->to;
            } while (msg->to < 0 || msg->to > num_usr);
            do
            {
                cout << "\nMODE OF MSG (0 for group & 1 for private):";
                cin >> msg->mode;
                cin.ignore();
            } while (msg->mode < 0 || msg->mode > 1);
            if (msg->mode == 0)
            {
                cout << "\nEnter Name of group: ";
                cin.getline(msg->group_name, 10);
            }

            msg->to--;
            msg->from = client_id;
            msg->written = true;
            if (msg->to == client_id)
            {
                cout << "\nMessage to yourself: " << msg->data << endl;
            }

            strcat(msg->data, "\n");
            
        }
        else if (choice == 6)
        {
            if (inbox->inbox_empty == false)
            {
                inbox->inbox_empty = true;
                cout << "\nInbox contents:\n"
                     << inbox->Inbox_info;
                inbox->Inbox_info[0] = '\0';
            }
            else
            {
                cout << "\nInbox contents: <EMPTY>\n";
            }
        }
        cout << "\nClient_id:" << client_id + 1;
        MENU();
        cout << "\nEnter choice: ";
        cin >> choice;
        cin.ignore();
    }
    server->command = choice;
    server->command_set = true;
    
    shmdt((void *)inbox);
    shmdt((void *)msg);
    shmdt((void *)server);
    shmdt((void *)join);

    return 0;
}
