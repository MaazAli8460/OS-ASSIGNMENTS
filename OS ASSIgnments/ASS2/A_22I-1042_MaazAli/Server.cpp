//Maaz Ali
//CS A
//22i-1042
//Q2_server

#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <cstring>
#include <cerrno>

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
    int current_g = 0;
    char groupname[10][10]; // max groups=10 max characters per_group
    bool admin_grants_request = false;
};

struct GC_info // group convo info
{
    int count = 0;
    int admin = -1;
    int id;
    char group_name[10];
    int current_members = 0;
    int members[20];
};

struct Group_Join_Request
{
    int requester;
    bool request_entered = false;
    bool granted = false;
    char group_name[10];
    int G_id = -1;
};

struct Inbox
{
    char Inbox_info[4096]; // 1 to 1 mapping
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

int main()
{
    int num_usr;
    cout << "\nEnter number of users: ";
    cin >> num_usr;

    int *shmid = new int[num_usr];
    int *shmid_msg = new int[num_usr];
    int *shmid_server = new int[num_usr];
    int *shmid_join = new int[num_usr];

    GC_info *GC = new GC_info[10]; // no need to make shared mem here
    Inbox **inbox = new Inbox *[num_usr];
    MSG **msg = new MSG *[num_usr];
    Server_Control **server = new Server_Control *[num_usr];
    Group_Join_Request **join = new Group_Join_Request *[num_usr];

    // Creating shared memory segments for each user
    for (int i = 0; i < num_usr; i++)
    {
        shmid[i] = shmget(IPC_PRIVATE, sizeof(struct Inbox), IPC_CREAT | 0666);
        shmid_msg[i] = shmget(IPC_PRIVATE, sizeof(struct MSG), IPC_CREAT | 0666);
        shmid_server[i] = shmget(IPC_PRIVATE, sizeof(struct Server_Control), IPC_CREAT | 0666);
        shmid_join[i] = shmget(IPC_PRIVATE, sizeof(struct Group_Join_Request), IPC_CREAT | 0666);

        if (shmid[i] == -1 || shmid_msg[i] == -1 || shmid_server[i] == -1 || shmid_join[i] == -1)
        {
            perror("shmget");
            exit(EXIT_FAILURE);
        }
        inbox[i] = (struct Inbox *)shmat(shmid[i], NULL, 0);
        msg[i] = (struct MSG *)shmat(shmid_msg[i], NULL, 0);
        server[i] = (struct Server_Control *)shmat(shmid_server[i], NULL, 0);
        join[i] = (struct Group_Join_Request *)shmat(shmid_join[i], NULL, 0);

        if (inbox[i] == (void *)-1 || msg[i] == (void *)-1 || server[i] == (void *)-1 || join[i] == (void *)-1)
        {
            perror("shmat");
            exit(EXIT_FAILURE);
        }
    }
    for (int i = 0; i < num_usr; i++)
    {
        server[i]->num_client = num_usr;
        server[i]->id = i;
    }
    for (int i = 0; i < 10; i++)
    {
        for (int j = 0; j < 20; j++)
        {
            GC[i].members[j] = -1;
        }
        GC[i].id = i;
    }
    cout << "\nEstablishing Server......";
    cout << "\nEstablishing Clients......";
    sleep(1);
    // Forking child processes for each user
    for (int i = 0; i < num_usr; i++)
    {
        int pid = fork();
        if (pid == 0)
        {
            char client_id[5];
            sprintf(client_id, "%d", i + 1);
            char number[5];
            char shmid_server1[40]; // for inbox
            char shmid_msg1[40];
            char shmid_se[40]; // for server control
            char shmid_je[40]; // for join request
            sprintf(number, "%d", num_usr);
            sprintf(shmid_server1, "%d", shmid[i]);
            sprintf(shmid_msg1, "%d", shmid_msg[i]);
            sprintf(shmid_se, "%d", shmid_server[i]);
            sprintf(shmid_je, "%d", shmid_join[i]);

            execlp("gnome-terminal", "gnome-terminal", "--", "./Client", client_id, number, shmid_server1, shmid_msg1, shmid_se, shmid_je, NULL);
            perror("execlp");
            exit(EXIT_FAILURE);
        }
        else if (pid < 0)
        {
            perror("fork");
            exit(EXIT_FAILURE);
        }
    }

    int current_usr = num_usr;
    int i = 0;

    while (true)
    {
        if (i >= num_usr)
        {
            i = 0;
        }
        if (server[i]->in_group == true)
        {
            server[i]->in_group = false;
            bool done = false;
            for (int k = 0; k < 10; k++)
            {
                if (GC[k].id == server[i]->admin_id && GC[k].current_members < 19)
                {
                    for (int f = 0; f < 20; f++)
                    {
                        if (GC[k].members[f] == -1)
                        {
                            GC[k].members[f] = server[i]->in_group_id;
                            strcpy(server[server[i]->in_group_id]->groupname[server[server[i]->in_group_id]->current_g], GC[k].group_name);
                            server[server[i]->in_group_id]->current_g++;
                            cout << "\nRequest for <CLIENT:" << server[i]->in_group_id << "> to be in <GROUP:" << GC[k].group_name << "> Granted.";
                            server[i]->in_group_id = -1;
                            GC[k].current_members++;
                            done = true;
                            // fflush(stdout);
                            break;
                        }
                    }
                }
                if (GC[k].id == server[i]->admin_id && GC[k].current_members > 19)
                {
                    cout << "\nRequest for <CLIENT:" << server[i]->in_group_id << "> to be in <GROUP:" << server[i]->groupname << "> Denied because group at its limit.";
                    break;
                }
                if (done)
                {
                    cout << "\nRequest for <CLIENT:" << server[i]->in_group_id << "> to be in <GROUP:" << server[i]->groupname << "> Granted.";
                    break;
                }
            }
            server[i]->in_group_id = -1;
        }

        if (msg[i]->written == true)
        {
            // TODO APPEND IN CHAT HISTORY ALSO
            if (msg[i]->mode == 1)
            {
                cout << "\nServer received message from  <CLIENT:" << msg[i]->from + 1 << "> directed to <CLIENT:" << msg[i]->to + 1 << "> in <PRIVATE>\n";
                string message;
                message += "\nRECIVED message from <CLIENT:" + to_string(msg[i]->from + 1) + ">:" + msg[i]->data;
                strcat(server[msg[i]->to]->chat_history, message.c_str());
                message = "\0";
                message += "\nSENT message TO <CLIENT:" + to_string(msg[i]->to + 1) + ">:" + msg[i]->data;
                strcat(server[i]->chat_history, message.c_str());
                // Update inbox
                strcat(inbox[msg[i]->to]->Inbox_info, "\nRECIVED message from <CLIENT:");
                strcat(inbox[msg[i]->to]->Inbox_info, to_string(msg[i]->from + 1).c_str());
                strcat(inbox[msg[i]->to]->Inbox_info, ">:");
                strcat(inbox[msg[i]->to]->Inbox_info, msg[i]->data);
                inbox[msg[i]->to]->inbox_empty = false;
                msg[i]->written = false;
                msg[i]->recived = false;
            }
            else if(msg[i]->mode == 0)// group msg
            {
                msg[i]->written = false;
                msg[i]->recived = false;
                bool fl_in_gp = false;
                for (int j = 0; j < 10; j++)
                {
                    if (strcmp(msg[i]->group_name, GC[j].group_name) == 0)
                    {
                        for (int k = 0; k < 20; k++)
                        {
                            
                            if (GC[j].members[k] == msg[i]->from)
                            {
                                fl_in_gp = true;
                                break;
                            }
                            // check if usr is in group or not;
                        }
                        if (fl_in_gp == false)
                        {
                            cout << "\n<CLIENT:" << msg[i]->from + 1 << "> Not in <GROUP:" << msg[i]->group_name << ">";
                            msg[i]->written = false;
                            msg[i]->recived = false;
                            break;
                        }
                        else
                        {
                            for (int k = 0; k < 20; k++)
                            {
                                if (GC[j].members[k] != -1)
                                {
                                    strcat(inbox[GC[j].members[k]]->Inbox_info, "\nRECIVED message from <CLIENT:");
                                    strcat(inbox[GC[j].members[k]]->Inbox_info, to_string(msg[i]->from + 1).c_str());
                                    strcat(inbox[GC[j].members[k]]->Inbox_info, ">:");
                                    strcat(inbox[GC[j].members[k]]->Inbox_info, msg[i]->data);
                                    strcat(inbox[GC[j].members[k]]->Inbox_info, " <Group :");
                                    strcat(inbox[GC[j].members[k]]->Inbox_info, msg[i]->group_name);
                                    strcat(inbox[GC[j].members[k]]->Inbox_info, ">");

                                    string ms;
                                    ms += "\nRECIVED message from <CLIENT:" + to_string(msg[i]->from + 1) + ">:" + msg[i]->data + " <Group :" + msg[i]->group_name + ">";
                                    strcat(server[GC[j].members[k]]->chat_history, ms.c_str());
                                }
                                /* code */
                            }
                            msg[i]->written = false;
                            msg[i]->recived = false;
                            break;
                        }
                        
                    }
                    
                    /* code */
                }

                // GROUP LOGIC
            }
        }
        if (server[i]->command_set == true)
        {
            // check for commands
            server[i]->command_set = false;
            if (server[i]->command == 1)
            {
                server[i]->command = -1;
                if (server[i]->is_admin == true)
                {
                    cout << "\n<CLIENT:" << server[i]->id + 1 << "> is Already admin of 1 group. <REQUEST DENIED>";
                }
                else
                {
                    for (int k = 0; k < 10; k++)
                    {
                        if (GC[k].admin == -1 && GC[k].count < 19)
                        {
                            /* code */
                            GC[k].admin = server[i]->id;
                            server[i]->admin_id = GC[k].id;
                            int l = 0;
                            while (GC[k].members[l] != -1 && l < 19)
                            {
                                l++;
                            }
                            GC[k].members[l] = server[i]->id;
                            server[i]->is_admin = true;
                            GC[k].count++;
                            strcpy(GC[k].group_name, join[i]->group_name);
                            strcpy(server[i]->groupname[server[i]->current_g], join[i]->group_name);
                            server[i]->current_g++;
                            GC[k].current_members++;
                            cout << "\n<GROUP:" << GC[k].group_name << "> Created Successfully.\n";
                            break;
                        }
                        else if (GC[k].admin == -1 && k + 1 == 10)
                        {
                            /* code */
                            cout << "CANNOT MAKE MORE GROUPS. <REQUEST DENIED>";
                        }
                        else if (GC[k].admin == -1 && GC[k].count + 1 == 20)
                        {
                            /* code */
                            cout << "GROUP FULL. <REQUEST DENIED>";
                        }
                        {
                            cout << "\n<GROUP:" << join[i]->group_name << "> Already has an admin. <REQUEST DENIED>";
                        }
                    }
                    fflush(stdout);
                }
            }
            else if (server[i]->command == 2)
            {
                server[i]->command = -1;
                for (int m = 0; m < 10; m++)
                {
                    if (strcmp(GC[m].group_name, join[i]->group_name) == 0 && GC[m].current_members < 19 && GC[m].admin != -1)
                    {
                        server[GC[m].admin]->admin_grants_request = true;
                        server[GC[m].admin]->in_group_id = join[i]->requester;
                        join[i]->G_id = GC[m].id;
                        cout << "\n<CLIENT:" << server[i]->id + 1 << "> REQUEST TO JOIN GROUP FRD TO <GROUP_ADMIN:" << GC[m].admin + 1 << ">";
                        fflush(stdout);
                        break;
                    }
                    else
                    {
                        continue;
                    }
                }
            }
            else if (server[i]->command == 3)
            {
                server[i]->command = -1;
                bool is_mem = false;

                for (int m = 0; m < 10; m++)
                {
                    if (strcmp(GC[m].group_name, join[i]->group_name) == 0)
                    {
                        int h;
                        for (int t = 0; t < 20; t++)
                        {
                            if (GC[m].members[t] == join[i]->requester)
                            {
                                h = t;
                                is_mem = true;
                                GC[m].members[t] = -1;
                                break;
                            }
                        }

                        // remove member from group
                        if (is_mem == true)
                        {
                            GC[m].members[h] = -1;
                            cout << "\n<CLIENT:" << join[i]->requester << "> has Left <GROUP: " << join[i]->group_name << ">" << endl;
                            break;
                        }
                    }
                }
                if (is_mem == false)
                {
                    cout << "\n<CLIENT:" << join[i]->requester << "> is not part of <GROUP: " << join[i]->group_name << endl;
                }
            }
            else if (server[i]->command == 8)
            {
                server[i]->execluded = true;
                cout << "\n<CLIENT:" << server[i]->id + 1 << "> disconnected" << endl;
                current_usr--;
                /* code */
            }
            server[i]->command = -1;
            /* code */
        }

        i++;
        if (current_usr <= 0)
        {
            cout << "\nTerminating Server......";
            break;
        }
    }

    // Detach and remove shared memory segments
    for (int j = 0; j < num_usr; j++)
    {
        shmdt((void *)inbox[j]);
        shmctl(shmid[j], IPC_RMID, NULL);
        shmdt((void *)msg[j]);
        shmctl(shmid_msg[j], IPC_RMID, NULL);
        shmdt((void *)server[j]);
        shmctl(shmid_server[j], IPC_RMID, NULL);
        shmdt((void *)join[j]);
        shmctl(shmid_join[j], IPC_RMID, NULL);
    }

    delete[] shmid;
    delete[] shmid_msg;
    delete[] shmid_server;
    delete[] shmid_join;

    delete[] server;
    delete[] join;
    delete[] inbox;
    delete[] msg;
    delete[] GC;

    return 0;
}
