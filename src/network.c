
#include <stdio.h>
#include <stdlib.h>

#include "allegro5/allegro.h"
#include "allegro5/allegro_image.h"
#include "allegro5/allegro_primitives.h"
#include "allegro5/allegro_font.h"
#include "allegro5/allegro_ttf.h"
#include "allegro5/allegro_audio.h"
#include "allegro5/allegro_acodec.h"

#include "game.h"
#include "init.h"
#include "objects.h"
#include "network.h"
#include "gameover.h"

NetworkType Net;

void PingServer(void);
void PingReply(void);
void CheckPingReply(void);

int NetStartNetwork(void)
{
    FILE *portfile;
    char line[6];

    Net.ping_port = 1803;

    if ((portfile = fopen("ports.txt","r")) == NULL)
        fprintf(logfile,"FAILED TO OPEN ports.txt , USING DEFAULT PORT\n");
    else
    {
        fgets(line,6,portfile);
        Net.ping_port = atoi(line);
    }
    fprintf(logfile,"Broadcast port:%d\n",Net.ping_port);
    fclose (portfile);

    if (enet_initialize () != 0)
    {
        if (hostfile) fprintf (hostfile, "An error occurred while initializing ENet.\n");
        if (clientfile) fprintf (clientfile, "An error occurred while initializing ENet.\n");
        return EXIT_FAILURE;
    }

    else
    {
        if (hostfile) fprintf (hostfile, "Initialized ENet.\n");
        if (clientfile) fprintf (clientfile, "Initialized ENet.\n");
    }

    atexit(enet_deinitialize);

    return EXIT_SUCCESS;
}

void NetStopNetwork(void)
{


    //enet_deinitialize();
}

int NetStartHost(int players)
{
    //set up ping port for client and server to find each other
    Net.ping = enet_socket_create(ENET_SOCKET_TYPE_DATAGRAM);
    // Allow the port to be reused by other applications - this means we can run several servers at once
    enet_socket_set_option(Net.ping, ENET_SOCKOPT_REUSEADDR, 1);
    ENetAddress listenaddr;
    listenaddr.host = ENET_HOST_ANY;
    listenaddr.port = Net.ping_port;
    enet_socket_bind(Net.ping, &listenaddr);
    enet_socket_get_address(Net.ping, &listenaddr);
    fprintf(hostfile,"Listening for scans on port %d\n", listenaddr.port);

    //Create a host using enet_host_create
    Net.address.host = ENET_HOST_ANY;
    Net.address.port = ENET_PORT_ANY;//Net.game_port;

    Net.host = enet_host_create(&Net.address, players, 2, 0, 0);

    if (Net.host == NULL) {
        fprintf(hostfile, "An error occured while trying to create an ENet server host\n");
        exit(EXIT_FAILURE);
    }

	//get host address
	char buffer[128];
	if (enet_address_get_host( &Net.address, buffer, 128 ) < 0)
        fprintf(hostfile,"enet_address_get_host failed.\n");
	if (enet_address_set_host( &Net.address, buffer ) < 0)      	//address.host field now contains the ip address as hex number
        fprintf(hostfile,"enet_address_set_host failed.\n");

    fprintf(hostfile,"Server address = 0x%08X\n", Net.address.host);

    AddressToString(Net.address.host, Net.myaddress);       //convert to string
    fprintf(hostfile,"Server address = %s\n", Net.myaddress);

    strcpy(buffer,NAME);                                    //display in title bar
    sprintf(buffer+strlen(NAME)," (Host: %s ; %d players)",Net.myaddress,num_ships);

    al_set_window_title(display, buffer);

    if (enet_address_get_host_ip (&Net.address, buffer, 128) < 0)
        fprintf(hostfile,"enet_address_get_host_ip failed.\n");

    fprintf(hostfile,"Server address(2) = %s\n", buffer);

    return EXIT_SUCCESS;
}

void NetStopListen(void)
{
	fprintf(hostfile,"Stop listening\n");
	if (enet_socket_shutdown(Net.ping, ENET_SOCKET_SHUTDOWN_READ_WRITE) != 0)
	{
		fprintf(hostfile,"Failed to shutdown listen socket\n");
	}
	enet_socket_destroy(Net.ping);
}

void NetStopServer(void)
{
    enet_socket_shutdown(Net.ping, ENET_SOCKET_SHUTDOWN_READ_WRITE);
    enet_socket_destroy(Net.ping);

    enet_host_destroy(Net.host);
    Net.host = NULL;
    num_ships = 1;
    al_set_window_title(display, NAME);
    Net.server = false;
}

void AddressToString(int address, char* string)
{
    char temp[4];
    char dot = '.';
    int i;

    for (i=0 ; i<16 ; i++)  //4x3 digits, + 3 dots, + null terminator
        string[i]=0;

    //itoa((address>>0)&0xff,temp,10);
    sprintf(temp,"%d",(address>>0)&0xff);
    strncpy(string,temp,3);
    strncat(string,&dot,1);

    //itoa((address>>8)&0xff,temp,10);
    sprintf(temp,"%d",(address>>8)&0xff);
    strncat(string,temp,3);
    strncat(string,&dot,1);

    //itoa((address>>16)&0xff,temp,10);
    sprintf(temp,"%d",(address>>16)&0xff);
    strncat(string,temp,3);
    strncat(string,&dot,1);

    //itoa((address>>24)&0xff,temp,10);
    sprintf(temp,"%d",(address>>24)&0xff);
    strncat(string,temp,3);

    return;
}

int NetStartClient(void)
{
    // b. Create a host using enet_host_create
    if (Net.host == NULL)
        Net.host = enet_host_create(NULL, 1, 2, 0, 0);

    if (Net.host == NULL) {
        fprintf(clientfile, "An error occured while trying to create an ENet server host\n");
        exit(EXIT_FAILURE);
    }

    // c. Connect and user service
    Net.peer = enet_host_connect(Net.host, &Net.address, 2, 0);

    if (Net.peer == NULL)
    {
        fprintf(clientfile, "No available peers for initializing an ENet connection");
        exit(EXIT_FAILURE);
    }

    return 0;
}

void NetDisconnectClient(void)
{
    enet_peer_disconnect(Net.peer,0);

}

void NetStopClient(void)
{
    enet_socket_shutdown(Net.ping, ENET_SOCKET_SHUTDOWN_READ_WRITE);
    enet_socket_destroy(Net.ping);

    enet_host_destroy(Net.host);
    Net.host = NULL;
    Net.id = 0;
    Net.client = false;
    Net.client_state = IDLE;
    al_set_window_title(display, NAME);
}

void NetStartGame(void)
{
    char temp_pkt[100];

    temp_pkt[0] = HOST_GO;

    ENetPacket * packet = enet_packet_create (&temp_pkt,1,ENET_PACKET_FLAG_RELIABLE);
    enet_host_broadcast (Net.host, 0, packet);
    Net.started = true;
    fprintf(hostfile,"Sent GO (broadcast)\n");
}

void NetSendReady(void)
{
    char temp_pkt[100];

    temp_pkt[0] = CLIENT_READY;
    temp_pkt[1] = Net.id;
    temp_pkt[2] = Ship[0].image;

    Ship[Net.id].image = Ship[0].image;
    Ship[Net.id].colour = Ship[0].colour;

    ENetPacket * packet = enet_packet_create (&temp_pkt,3,ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send (Net.peer, 0, packet);
    fprintf(clientfile,"Sent Ready. ID:%d Image:%d\n",Net.id,Ship[0].image);
    Net.quality = 100;
}

void NetSendShipState(void)
{
    char temp_pkt[100];
    int i=0;
    int j = Net.id;

    temp_pkt[i++] = CLIENT_SHIPSTATE;
    temp_pkt[i++] = Net.id;

    temp_pkt[i++] = ((unsigned short int)Ship[j].xpos)>>8;
    temp_pkt[i++] = ((unsigned short int)Ship[j].xpos)&0xff;
    temp_pkt[i++] = ((unsigned short int)Ship[j].ypos)>>8;
    temp_pkt[i++] = ((unsigned short int)Ship[j].ypos)&0xff;
    temp_pkt[i++] = (signed char)Ship[j].xv;
    temp_pkt[i++] = (signed char)Ship[j].yv;
    temp_pkt[i++] = (char)Ship[j].angle;
    temp_pkt[i++] = (char)Ship[j].thrust;
    //temp_pkt[i++] = (char)Ship[j].landed;

    temp_pkt[i++] = (char)Ship[j].lives;
    //temp_pkt[i++] = (char)Ship[j].shield;
    //temp_pkt[i++] = (char)(Ship[j].fuel>>8);
    //temp_pkt[i++] = (char)(Ship[j].fuel&0xff);
    //temp_pkt[i++] = (char)Ship[j].ammo1;
    //temp_pkt[i++] = (char)Ship[j].ammo2;
    temp_pkt[i++] = (char)Ship[j].image;
    temp_pkt[i++] = (char)Ship[j].reincarnate_timer;
    //temp_pkt[i++] = (char)Ship[j].menu;
    //if (Ship[j].menu)
    {
        //temp_pkt[i++] = (char)Ship[j].menu_state;
        temp_pkt[i++] = (char)Ship[j].ammo1_type;
        temp_pkt[i++] = (char)Ship[j].ammo2_type;
        //temp_pkt[i++] = (char)Ship[j].user_ammo1;
        //temp_pkt[i++] = (char)Ship[j].user_ammo2;
        //temp_pkt[i++] = (char)Ship[j].user_fuel;
    }
    temp_pkt[i++] = (char)Ship[j].actions;
    Ship[j].actions = 0;
    temp_pkt[i++] = (char)Net.sounds;
    Net.sounds = 0;

    ENetPacket * packet = enet_packet_create (&temp_pkt,i,ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send (Net.peer, 0, packet);

    /*
    Ship[0].fire1_down = false;
    Ship[0].fire2_down = false;
    Ship[0].left_down = false;
    Ship[0].right_down = false;
    Ship[0].thrust_down = false;
    */
}

void NetSendKilled(int killer)
{
    char temp_pkt[100];

    temp_pkt[0] = CLIENT_KILLED;
    temp_pkt[1] = killer;
    temp_pkt[2] = Net.id;

    ENetPacket * packet = enet_packet_create (&temp_pkt,4,ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send (Net.peer, 0, packet);

    fprintf(clientfile,"Sent CLIENT KILLED\n");

}

void NetSendOutOfLives(void)
{
    char temp_pkt[100];

    temp_pkt[0] = CLIENT_OUTOFLIVES;
    temp_pkt[1] = Net.id;

    ENetPacket * packet = enet_packet_create (&temp_pkt,3,ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send (Net.peer, 0, packet);

    fprintf(clientfile,"Sent CLIENT OUTOFLIVES\n");
}

void NetSendKeys(void)
{
    char temp_pkt[100];

    temp_pkt[0]  = CLIENT_KEYS;
    temp_pkt[1]  = Net.id;
    temp_pkt[2]  = Ship[0].fire1_down;
    temp_pkt[3]  = Ship[0].fire1_held;
    temp_pkt[4]  = Ship[0].fire2_down;
    temp_pkt[5]  = Ship[0].fire2_held;
    temp_pkt[6]  = Ship[0].left_down;
    temp_pkt[7]  = Ship[0].left_held;
    temp_pkt[8]  = Ship[0].right_down;
    temp_pkt[9]  = Ship[0].right_held;
    temp_pkt[10] = Ship[0].thrust_down;
    temp_pkt[11] = Ship[0].thrust_held;

    ENetPacket * packet = enet_packet_create (&temp_pkt,13,0);//ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send (Net.peer, 0, packet);

    Ship[0].fire1_down = false;
    Ship[0].fire2_down = false;
    Ship[0].left_down = false;
    Ship[0].right_down = false;
    Ship[0].thrust_down = false;
}

void NetSendGameState()
{
    char temp_pkt[1000];
    int i=0,j;
    static int max_packet_size = 0;

#if 0
    static int skip_count = 0;
    if (skip_count == 0)            //DEBUG!! simulate packet loss
    {
        if ((rand() % 10) == 0)
        {
            skip_count = rand() % 4;
        }
    }
    else
    {
        skip_count--;
        return;
    }
#endif
    temp_pkt[i++]  = HOST_GAMESTATE;
    temp_pkt[i++]  = num_ships;
    for (j=0 ; j<num_ships ; j++)
    {
        temp_pkt[i++] = ((unsigned short int)Ship[j].xpos)>>8;
        temp_pkt[i++] = ((unsigned short int)Ship[j].xpos)&0xff;
        temp_pkt[i++] = ((unsigned short int)Ship[j].ypos)>>8;
        temp_pkt[i++] = ((unsigned short int)Ship[j].ypos)&0xff;
        temp_pkt[i++] = (signed char)Ship[j].xv;
        temp_pkt[i++] = (signed char)Ship[j].yv;
        temp_pkt[i++] = (char)Ship[j].angle;
        temp_pkt[i++] = (char)Ship[j].thrust;
        temp_pkt[i++] = (char)Ship[j].left_held;
        temp_pkt[i++] = (char)Ship[j].right_held;
        temp_pkt[i++] = (char)Ship[j].mass;


        //temp_pkt[i++] = (char)Ship[j].landed;

        temp_pkt[i++] = (char)Ship[j].lives;
        //temp_pkt[i++] = (char)Ship[j].shield;
        //temp_pkt[i++] = (char)(Ship[j].fuel>>8);
        //temp_pkt[i++] = (char)(Ship[j].fuel&0xff);
        //temp_pkt[i++] = (char)Ship[j].ammo1;
        //temp_pkt[i++] = (char)Ship[j].ammo2;
        temp_pkt[i++] = (char)Ship[j].image;
        temp_pkt[i++] = (char)Ship[j].reincarnate_timer;
        //temp_pkt[i++] = (char)Ship[j].shield;
        //temp_pkt[i++] = (char)Ship[j].landed;

        //temp_pkt[i++] = (char)Ship[j].menu;
        //if (Ship[j].menu)
        {
        //    temp_pkt[i++] = (char)Ship[j].menu_state;
        //    temp_pkt[i++] = (char)Ship[j].ammo1_type;
        //    temp_pkt[i++] = (char)Ship[j].ammo2_type;
        //    temp_pkt[i++] = (char)Ship[j].user_ammo1;
        //    temp_pkt[i++] = (char)Ship[j].user_ammo2;
        //    temp_pkt[i++] = (char)Ship[j].user_fuel;
        }
        temp_pkt[i++] = (char)Ship[j].bullet.damage;
        temp_pkt[i++] = (char)Ship[j].bullet.owner;
        temp_pkt[i++] = (char)Ship[j].bullet.type;
        temp_pkt[i++] = (signed char)Ship[j].bullet.xv;
        temp_pkt[i++] = (signed char)Ship[j].bullet.yv;

        Ship[j].bullet.damage = 0;
        Ship[j].bullet.owner = NO_OWNER;
        Ship[j].bullet.xv = 0;
        Ship[j].bullet.yv = 0;
    }

    //bullets
    int current_bullet = first_bullet;
    //previous_bullet = END_OF_LIST;

    int num_bullets_idx = i++;
    char num_bullets = 0;

    while(current_bullet != END_OF_LIST)		//handle all 'live' bullets.
    {
        temp_pkt[i++] = ((unsigned short int)Bullet[current_bullet].xpos) >> 8;
        temp_pkt[i++] = ((unsigned short int)Bullet[current_bullet].xpos) & 0xff;
        temp_pkt[i++] = ((unsigned short int)Bullet[current_bullet].ypos) >> 8;
        temp_pkt[i++] = ((unsigned short int)Bullet[current_bullet].ypos) & 0xff;
        temp_pkt[i++] = Bullet[current_bullet].ttl >> 8;
        temp_pkt[i++] = Bullet[current_bullet].ttl & 0xff;
        temp_pkt[i++] = (signed char)Bullet[current_bullet].xv;
        temp_pkt[i++] = (signed char)Bullet[current_bullet].yv;
        temp_pkt[i++] = Bullet[current_bullet].type;
        //temp_pkt[i++] = Bullet[current_bullet].owner;

        num_bullets++;
        current_bullet = Bullet[current_bullet].next_bullet;
    }
    temp_pkt[num_bullets_idx] = num_bullets;

    temp_pkt[i++] = Map.num_sentries;               //arguably redundant, but it feels a bit safer.....
    for (j=0 ; j< Map.num_sentries ; j++)
        temp_pkt[i++] = Map.sentry[j].alive;

    temp_pkt[i++] = Map.num_switches;
    for (j=0 ; j< Map.num_switches ; j++)
        temp_pkt[i++] = Map.switches[j].open;

    temp_pkt[i++] = Map.num_forcefields;
    for (j=0 ; j< Map.num_forcefields ; j++)
        temp_pkt[i++] = Map.forcefield[j].alpha;

    temp_pkt[i++] = Net.sounds;
    Net.sounds = 0;

    if (i > max_packet_size)
    {
        max_packet_size = i;
        fprintf(hostfile,"Max Packet:%d bytes (%d ships, %d bullets)\n",i,num_ships,num_bullets);
    }

    ENetPacket * packet = enet_packet_create (&temp_pkt,i,ENET_PACKET_FLAG_RELIABLE);
    enet_host_broadcast (Net.host, 0, packet);
}

void NetSendGameOver(void)
{
    char temp_pkt[100];
    int i,j;

    temp_pkt[0] = HOST_GAMEOVER;

    i=1;
    for (j=0 ; j<num_ships ; j++)
    {
        temp_pkt[i++] = Map.score[j].player;
        temp_pkt[i++] = Map.score[j].kills;
        temp_pkt[i++] = Map.score[j].lives;
    }

    ENetPacket * packet = enet_packet_create (&temp_pkt,num_ships*3+1,ENET_PACKET_FLAG_RELIABLE);
    enet_host_broadcast (Net.host, 0, packet);
    fprintf(hostfile,"Sent GAMEOVER (broadcast)\n");
    NetStopListen();
}

void NetSendAbort(void)
{
    char temp_pkt[100];

    temp_pkt[0] = HOST_ABORT;

    ENetPacket * packet = enet_packet_create (&temp_pkt,1,ENET_PACKET_FLAG_RELIABLE);
    enet_host_broadcast (Net.host, 0, packet);
    fprintf(hostfile,"Sent ABORT (broadcast)\n");
    NetStopListen();
}

NetMessageType ServiceNetwork(void)
{
    if (Net.client)
    {
        int i=0, j=0;
        char SoundsToPlay;
        char buf[100];

        if (Net.client_state == SEARCHING)
        {
            if (Net.host == NULL)
            {
                if (Net.pingtimer >= 30)
                {
                    Net.pingtimer = 0;
                    PingServer();
                }
                CheckPingReply();
            }
        }
        else
        {
            Net.updated = false;
            Net.quality *= 0.99;

            while (enet_host_service(Net.host, &Net.event, 0))
            {
                switch(Net.event.type)
                {
                    case ENET_EVENT_TYPE_CONNECT:
                        fprintf(clientfile,"Connection to %s:%d succeeded.\n",Net.menuaddress,Net.address.port);
                    break;
                    case ENET_EVENT_TYPE_RECEIVE:
                        switch(Net.event.packet->data[0])
                        {
                        case HOST_ID:
                            Net.id = Net.event.packet->data[1];
                            Ship[0].image = Net.id;                         //for menu display
                            Ship[0].colour = ShipColour[Net.id];                         //for menu display
                            fprintf(clientfile,"Received ID:%d\n",Net.id);
                            enet_packet_destroy (Net.event.packet);

                            reinit_ship(Net.id);

                            strcpy(buf,NAME);                                    //display in title bar
                            sprintf(buf+strlen(NAME)," (Client: P%d)",Net.id+1);
                            al_set_window_title(display, buf);
                        break;
                        case HOST_LEVEL:
                            strncpy(Net.mapfile, (char *)&Net.event.packet->data[1], MAP_NAME_LENGTH);
                            fprintf(clientfile,"Received Level:%s\n",Net.mapfile);
                            if (init_map(0,0) != 0)
                            {
                                fprintf(clientfile,"Failed to open level:%s\n",Net.mapfile);
                                Net.client_state = NO_MAP;
                                enet_peer_disconnect(Net.peer,0);
                            }
                            else
                                Net.client_state = CONNECTED;

                            enet_packet_destroy (Net.event.packet);
                        break;
                        case HOST_NOSPACE:
                            fprintf(clientfile,"Received FULL message\n");
                            Net.client_state = NO_SPACE;
                            enet_peer_disconnect(Net.peer,0);
                            enet_packet_destroy (Net.event.packet);
                        break;
                        case HOST_GO:
                            fprintf(clientfile,"Received GO message\n");
                            Net.started = true;
                            Net.quality = 100;
                            enet_packet_destroy (Net.event.packet);
                        break;
                        case HOST_GAMESTATE:
                            //break;
                            if (Net.client_state != RUNNING) break;
                            fpsnet_acc++;
                            i=1;
                            num_ships = Net.event.packet->data[i++];
                            for (j=0 ; j<num_ships ; j++)
                            {
                                if (j==Net.id)
                                {
                                    i+=14;                              //skip a few...

                                    Ship[j].bullet.damage = Net.event.packet->data[i++];    //read damage
                                    if (Ship[j].bullet.damage)
                                    {
                                        Ship[j].shield -=  Ship[j].bullet.damage; //subtract damage
                                        Ship[j].bullet.owner = Net.event.packet->data[i++];
                                        if (Ship[j].shield <= 0)
                                        {
                                            Ship[j].killed++;
                                            if (Ship[j].bullet.owner != NO_OWNER)
                                                NetSendKilled(Ship[j].bullet.owner);
                                        }
                                        //if (Ship[j].bullet.owner != NO_OWNER)
                                        {
                                            Ship[j].bullet.type = Net.event.packet->data[i++];                                      //read type so we can look up mass
                                            Ship[j].xv     += Mass[Ship[j].bullet.type]*(signed char)Net.event.packet->data[i++];	//momentum from bullet to ship
                                            Ship[j].yv     += Mass[Ship[j].bullet.type]*(signed char)Net.event.packet->data[i++];
                                        }
                                    }
                                    else
                                        i+=4;
                                }
                                else
                                {
                                    Ship[j].xpos   = Net.event.packet->data[i++] << 8;
                                    Ship[j].xpos  += Net.event.packet->data[i++];
                                    Ship[j].ypos   = Net.event.packet->data[i++] << 8;
                                    Ship[j].ypos  += Net.event.packet->data[i++];
                                    Ship[j].xv     = (signed char)Net.event.packet->data[i++];
                                    Ship[j].yv     = (signed char)Net.event.packet->data[i++];
                                    Ship[j].angle  = Net.event.packet->data[i++];
                                    Ship[j].thrust = Net.event.packet->data[i++];

                                    Ship[j].left_held  = Net.event.packet->data[i++];
                                    Ship[j].right_held = Net.event.packet->data[i++];
                                    Ship[j].mass       = Net.event.packet->data[i++];

                                    //Ship[j].landed = Net.event.packet->data[i++];

                                    Ship[j].lives  = Net.event.packet->data[i++];
                                    //Ship[j].shield = Net.event.packet->data[i++];
                                    //Ship[j].fuel   = Net.event.packet->data[i++] << 8;
                                    //Ship[j].fuel  += Net.event.packet->data[i++];
                                    //Ship[j].ammo1  = Net.event.packet->data[i++];
                                    //Ship[j].ammo2  = Net.event.packet->data[i++];

                                    Ship[j].image  = Net.event.packet->data[i++];
                                    Ship[j].colour = ShipColour[Ship[j].image];
                                    Ship[j].statuscolour = StatusColour[Ship[j].image];

                                    Ship[j].reincarnate_timer  = Net.event.packet->data[i++];
                                    //Ship[j].shield = Net.event.packet->data[i++];
                                    //Ship[j].landed = Net.event.packet->data[i++];

                                    //Ship[j].menu = Net.event.packet->data[i++];
                                    //if (Ship[j].menu)
                                    //{
                                    //    Ship[j].menu_state = Net.event.packet->data[i++];
                                    //    Ship[j].ammo1_type = Net.event.packet->data[i++];
                                    //    Ship[j].ammo2_type = Net.event.packet->data[i++];
                                    //    Ship[j].user_ammo1 = Net.event.packet->data[i++];
                                    //    Ship[j].user_ammo2 = Net.event.packet->data[i++];
                                    //    Ship[j].user_fuel  = Net.event.packet->data[i++];
                                   // }
                                   i+=5; //skip bullet params
                                }
                            }
                            //create linked list.
                            char num_bullets = Net.event.packet->data[i++];
                            if (num_bullets == 0)
                                first_bullet = END_OF_LIST;
                            else
                                first_bullet = 0;

                            for (j=0 ; j<num_bullets ; j++)
                            {
                                Bullet[j].xpos  = Net.event.packet->data[i++] << 8;
                                Bullet[j].xpos += Net.event.packet->data[i++];
                                Bullet[j].ypos  = Net.event.packet->data[i++] << 8;
                                Bullet[j].ypos += Net.event.packet->data[i++];
                                Bullet[j].ttl   = Net.event.packet->data[i++] << 8;
                                Bullet[j].ttl  += Net.event.packet->data[i++];
                                Bullet[j].xv    = (signed char)Net.event.packet->data[i++];
                                Bullet[j].yv    = (signed char)Net.event.packet->data[i++];
                                Bullet[j].type  = Net.event.packet->data[i++];
                                Bullet[j].next_bullet = j+1;
                            }
                            Bullet[j-1].next_bullet = END_OF_LIST;

                            Map.num_sentries = Net.event.packet->data[i++];
                            for (j=0 ; j< Map.num_sentries ; j++)
                                Map.sentry[j].alive = Net.event.packet->data[i++];

                            Map.num_switches = Net.event.packet->data[i++];
                            for (j=0 ; j< Map.num_switches ; j++)
                                Map.switches[j].open = Net.event.packet->data[i++];

                            Map.num_forcefields = Net.event.packet->data[i++];
                            for (j=0 ; j< Map.num_forcefields ; j++)
                                Map.forcefield[j].alpha = Net.event.packet->data[i++];

                            SoundsToPlay = Net.event.packet->data[i++];
                            if (SoundsToPlay & PARTICLE)
                            {
                                if (al_get_sample_instance_playing(particle_inst[0]))
                                    al_stop_sample_instance(particle_inst[0]);

                                al_play_sample_instance(particle_inst[0]);
                            }
                            if (SoundsToPlay & SHOOTA)
                            {
                                if (al_get_sample_instance_playing(shoota_inst[0]))
                                    al_stop_sample_instance(shoota_inst[0]);

                                al_play_sample_instance(shoota_inst[0]);
                            }
                            if (SoundsToPlay & SHOOTB)
                            {
                                if (al_get_sample_instance_playing(shootb_inst[0]))
                                    al_stop_sample_instance(shootb_inst[0]);

                                al_play_sample_instance(shootb_inst[0]);
                            }
                            if (SoundsToPlay & DEAD)
                            {
                                if (al_get_sample_instance_playing(dead_inst[0]))
                                    al_stop_sample_instance(dead_inst[0]);

                                al_play_sample_instance(dead_inst[0]);
                            }

                            Net.updated = true;
                            enet_packet_destroy (Net.event.packet);
                        break;

                        case HOST_GAMEOVER:
                            fprintf(clientfile,"Received GAMEOVER message\n");
                            game_over = GO_TIMER;
                            //read scores.
                            i=1;
                            for (j=0 ; j<num_ships ; j++)
                            {
                                Map.score[j].player = Net.event.packet->data[i++];
                                Map.score[j].kills = Net.event.packet->data[i++];
                                Map.score[j].lives = Net.event.packet->data[i++];
                            }

                            enet_peer_disconnect(Net.peer, 0);
                            enet_packet_destroy (Net.event.packet);
                        break;

                        case HOST_ABORT:
                            fprintf(clientfile,"Received ABORT message\n");
                            enet_peer_disconnect(Net.peer,0);
                            //Net.client_state = ABORTING;
                            Net.aborted = true;
                            enet_packet_destroy (Net.event.packet);
                        break;

                        default:
                            enet_packet_destroy (Net.event.packet);
                        break;
                        }
                    break;
                    case ENET_EVENT_TYPE_DISCONNECT:
                        fprintf(clientfile,"Disconnected from host.\n");

                        //if (Net.client_state == ABORTING)        //we were in game, so
                        //    Net.client_state = ABORTED;            //flag to foreground
                        //else
                            Net.client_state = IDLE;

                    break;

                    case ENET_EVENT_TYPE_NONE:
                    default:
                    break;
                }
            }
            if (Net.updated)
                Net.quality++;
        }
    }

    else if (Net.server)
    {
        int i,j;

        if (num_ships < 2)
            PingReply();    //check if we've been pinged by a client. If so, reply.

        while(enet_host_service(Net.host, &Net.event, 0))
        {
            switch(Net.event.type)
            {
                char temp_pkt[100];

                case ENET_EVENT_TYPE_CONNECT:
                    AddressToString(Net.event.peer->address.host, temp_pkt);
                    fprintf(hostfile,"Connection from %s\n",temp_pkt);

                    if (num_ships >= Map.max_players)
                    {
                        //send full message to client
                        temp_pkt[0] = HOST_NOSPACE;
                        ENetPacket * packet = enet_packet_create ((char*)&temp_pkt,2,ENET_PACKET_FLAG_RELIABLE);
                        enet_peer_send (Net.event.peer, 0, packet);
                        fprintf(hostfile,"Sent FULL message\n");
                        num_ships++;    //will be decremented again when client disconnects.
                    }
                    else
                    {
                        //send id to client
                        temp_pkt[0] = HOST_ID;
                        temp_pkt[1] = num_ships;//Net.clients;
                        ENetPacket * packet = enet_packet_create ((char*)&temp_pkt,3,ENET_PACKET_FLAG_RELIABLE);
                        enet_peer_send (Net.event.peer, 0, packet);
                        fprintf(hostfile,"Issued ID: %d\n",num_ships++);//Net.clients++);

                        //send level to client
                        temp_pkt[0] = HOST_LEVEL;
                        strncpy(&temp_pkt[1],(char*)&MapNames[Menu.group].Map[Menu.map],100);
                        packet = enet_packet_create ((char*)&temp_pkt,strlen((char*)&MapNames[Menu.group].Map[Menu.map])+2,ENET_PACKET_FLAG_RELIABLE);
                        enet_peer_send (Net.event.peer, 0, packet);
                        fprintf(hostfile,"Sent Level: %s\n",&temp_pkt[1]);

                        if (Net.started)
                        {
                            temp_pkt[0] = HOST_GO;
                            packet = enet_packet_create ((char*)&temp_pkt,2,ENET_PACKET_FLAG_RELIABLE);
                            enet_peer_send (Net.event.peer, 0, packet);
                            fprintf(hostfile,"Sent GO (late joiner)\n");
                        }
                    }
                    char buffer[256];
                    strcpy(buffer,NAME);
                    sprintf(buffer+strlen(NAME)," (Host: %s ; %d players)",Net.myaddress,num_ships);
                    al_set_window_title(display, buffer);
                break;

                case ENET_EVENT_TYPE_RECEIVE:
                    //fprintf(hostfile,"(Server) Message from client : %s\n", Net.event.packet->data);
                    switch(Net.event.packet->data[0])
                    {
                        case CLIENT_READY:
                            i = Net.event.packet->data[1];
                            Ship[i].image = Net.event.packet->data[2];
                            Ship[i].colour = ShipColour[Ship[i].image];
                            fprintf(hostfile,"Received 'Ready'. Client:%d Image:%d\n",i,Ship[i].image);
                        break;
                        case CLIENT_KEYS:
                            i = Net.event.packet->data[1];
                            Ship[i].fire1_down  = Net.event.packet->data[2];
                            Ship[i].fire1_held  = Net.event.packet->data[3];
                            Ship[i].fire2_down  = Net.event.packet->data[4];
                            Ship[i].fire2_held  = Net.event.packet->data[5];
                            Ship[i].left_down   = Net.event.packet->data[6];
                            Ship[i].left_held   = Net.event.packet->data[7];
                            Ship[i].right_down  = Net.event.packet->data[8];
                            Ship[i].right_held  = Net.event.packet->data[9];
                            Ship[i].thrust_down = Net.event.packet->data[10];
                            Ship[i].thrust_held = Net.event.packet->data[11];
                        break;
                        case CLIENT_SHIPSTATE:
                            i=1;
                            j = Net.event.packet->data[i++];
                            Ship[j].xpos   = Net.event.packet->data[i++] << 8;
                            Ship[j].xpos  += Net.event.packet->data[i++];
                            Ship[j].ypos   = Net.event.packet->data[i++] << 8;
                            Ship[j].ypos  += Net.event.packet->data[i++];
                            Ship[j].xv     = (signed char)Net.event.packet->data[i++];
                            Ship[j].yv     = (signed char)Net.event.packet->data[i++];
                            Ship[j].angle  = Net.event.packet->data[i++];
                            Ship[j].thrust = Net.event.packet->data[i++];
                            //Ship[j].landed = Net.event.packet->data[i++];

                            Ship[j].lives  = Net.event.packet->data[i++];
                            //Ship[j].shield = Net.event.packet->data[i++];
                            //Ship[j].fuel   = Net.event.packet->data[i++] << 8;
                            //Ship[j].fuel  += Net.event.packet->data[i++];
                            //Ship[j].ammo1  = Net.event.packet->data[i++];
                            //Ship[j].ammo2  = Net.event.packet->data[i++];

                            Ship[j].image  = Net.event.packet->data[i++];
                            Ship[j].colour = ShipColour[Ship[j].image];
                            Ship[j].statuscolour = StatusColour[Ship[j].image];

                            Ship[j].reincarnate_timer  = Net.event.packet->data[i++];

                            //Ship[j].menu = Net.event.packet->data[i++];
                            //if (Ship[j].menu)
                            {
                                //Ship[j].menu_state = Net.event.packet->data[i++];
                                Ship[j].ammo1_type = Net.event.packet->data[i++];
                                Ship[j].ammo2_type = Net.event.packet->data[i++];
                                //Ship[j].user_ammo1 = Net.event.packet->data[i++];
                                //Ship[j].user_ammo2 = Net.event.packet->data[i++];
                                //Ship[j].user_fuel  = Net.event.packet->data[i++];
                            }
                            if (Net.event.packet->data[i]   & FIRE_NORMAL)  FireNormal(j);
                            if (Net.event.packet->data[i]   & FIRE_SPECIAL) FireSpecial(j);
                            if (Net.event.packet->data[i++] & EXPLODE)
                                CreateExplosion(Ship[j].xpos, Ship[j].ypos, 2, 8, Ship[j].xv, Ship[j].yv);//float outward_v);
                            Net.sounds |= Net.event.packet->data[i++];    //take sounds from client
                        break;
                        case CLIENT_KILLED:
                            fprintf(hostfile,"Received CLIENT KILLED\n");
                            Ship[Net.event.packet->data[1]].kills++;
                            Ship[Net.event.packet->data[2]].killed++;
                        break;
                        case CLIENT_OUTOFLIVES:
                            fprintf(hostfile,"Received CLIENT OUTOFLIVES\n");
                            game_over = GO_TIMER;
                        break;
                    }
                break;

                case ENET_EVENT_TYPE_DISCONNECT:
                    num_ships--;
                    Net.event.peer->data = NULL;
                    strcpy(buffer,NAME);
                    sprintf(buffer+strlen(NAME)," (Host: %s ; %d players)",Net.myaddress,num_ships);
                    al_set_window_title(display, buffer);
                break;
                case ENET_EVENT_TYPE_NONE:
                default:
                break;
            }
        }
    }
    fflush(hostfile);
    fflush(clientfile);

    return NO_MESSAGE;
}

//client pings server via broadcast packet
void PingServer(void)
{
    Net.ping = enet_socket_create(ENET_SOCKET_TYPE_DATAGRAM);
    // We need to set a socket option in order to send to the broadcast address
    enet_socket_set_option(Net.ping, ENET_SOCKOPT_BROADCAST, 1);
    ENetAddress addr;
    addr.host = ENET_HOST_BROADCAST;
    addr.port = Net.ping_port;
    char data = 42;
    ENetBuffer sendbuf;
    sendbuf.data = &data;
    sendbuf.dataLength = 1;
    enet_socket_send(Net.ping, &addr, &sendbuf, 1);
    fprintf(clientfile,"Pinged server on port:%d\n",Net.ping_port);
    return;
}

//host checks for a ping, if there is one, it replies
void PingReply(void)
{
        char pkt[10];

        ENetSocketSet set;                       //Listen for connects on broadcast socket
        ENET_SOCKETSET_EMPTY(set);              // Use select to see if there is data to read
        ENET_SOCKETSET_ADD(set, Net.ping);
        if (enet_socketset_select(Net.ping, &set, NULL, 0) <= 0) return;

        ENetAddress addr;
        ENetBuffer recvbuf;
        recvbuf.data = &pkt;
        recvbuf.dataLength = 1;
        if (enet_socket_receive(Net.ping, &addr, &recvbuf, 1) <= 0) return;

        if (pkt[0] != 42) return;

        char addrbuf[256];
        enet_address_get_host_ip(&addr, addrbuf, sizeof addrbuf);
        fprintf(hostfile,"Listen port: received (%d) from %s:%d\n", *(char *)recvbuf.data, addrbuf, addr.port);

        ServerInfo sinfo;

        if (enet_address_get_host(&Net.host->address, sinfo.hostname, sizeof sinfo.hostname) != 0)
        {
            fprintf(stderr, "Failed to get hostname\n");
            return;
        }

        sinfo.port = htons(Net.host->address.port);
        recvbuf.data = &sinfo;
        recvbuf.dataLength = sizeof sinfo;
        enet_socket_send(Net.ping, &addr, &recvbuf, 1);
        fprintf(hostfile,"Sent server address to: %s:%d\n", addrbuf, addr.port);

        return;
}

//client checks to see if server has replied to ping, extracts address and sets up
void CheckPingReply(void)
{
    ServerInfo sinfo;
    ENetSocketSet set;
    ENET_SOCKETSET_EMPTY(set);
    ENET_SOCKETSET_ADD(set, Net.ping);

    while (enet_socketset_select(Net.ping, &set, NULL, 0) > 0)
    {
        ENetBuffer recvbuf;
        recvbuf.data = &sinfo;
        recvbuf.dataLength = sizeof sinfo;
        const int recvlen = enet_socket_receive(Net.ping, &Net.address, &recvbuf, 1);
        if (recvlen > 0)
        {
            if (recvlen != sizeof(ServerInfo))
            {
                fprintf(stderr, "Unexpected reply from scan\n");
                return;
            }

            // The server itself runs on a different port, so take it from the message
            Net.address.port = ntohs(sinfo.port);
            char buf[256];
            enet_address_get_host_ip(&Net.address, buf, sizeof buf);
            fprintf(clientfile,"Found server '%s' at %s:%d\n", sinfo.hostname, buf, Net.address.port);
            strncpy(Net.menuaddress,buf,16);

            NetStartClient();
            Net.client_state = FOUND;
        }
    }
    return;
}
