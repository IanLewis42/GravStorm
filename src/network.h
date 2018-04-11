#include "enet/enet.h"


typedef enum
{
    NO_MESSAGE = 0,
    //messages from host
    HOST_ID,            //tell client its ID
    HOST_LEVEL,         //
    HOST_NOSPACE,       //No space for more players
    HOST_GO,            //start game
    HOST_GAMESTATE,     //normal update
    HOST_GAMEOVER,
    HOST_ABORT,
    //messages from client
    CLIENT_READY,
    CLIENT_KEYS,
    CLIENT_SHIPSTATE,
    CLIENT_KILLED,
    CLIENT_OUTOFLIVES
} NetMessageType;

typedef enum
{
    IDLE = 0,
    SEARCHING,      //sending pings
    FOUND,          //got ping reply
    CONNECTED,      //connected to host
    NO_SPACE,       //disconnected, no space on host
    NO_MAP,          //disconnected, we don't have the right map
    RUNNING,         //Game started
    ABORTING,       //received abort from host
    ABORTED         //Stopped by host
} NetClientStateType;

typedef struct
{
    //int net;                //are we networked?
    int server;             //are we the server?
    int client;             //are we a client?
    //int clients;            //number of clients connected (only valid if we're a host)
    //int clients_ready;
    //int connected;          //are connected to a host?
    NetClientStateType  client_state;
    int aborted;
    int updated;            //set by client when it receives a GAMESTATE packet

    int started;            //has the game started?

    int id;                 //to identify clients; assigned by host
    char menuaddress[16];   //IP address - own address if host, host address if client.
    //char temp_address[16];  //used for typing in.
    char myaddress[16];
    char sounds;
    //need addresses for all clients?

    char mapfile[MAP_NAME_LENGTH];

    ENetAddress address;
    int ping_port;
    int game_port;
    //ENetSocket listen;
    ENetSocket ping;
    int pingtimer;

    //ENetHost *server;
    //ENetHost *client;

    ENetHost *host;
    ENetPeer *peer;     //this is the host, from the point of view of the client.
    ENetEvent event;
    int eventStatus;

    float quality;
    int msgtimer;
} NetworkType;

typedef struct
{
	char hostname[1024];
	enet_uint16 port;
} ServerInfo;

//bit defines for sounds to be played by client
#define PARTICLE 0x01
#define SHOOTA   0x02
#define SHOOTB   0x04
#define DEAD     0x08

extern NetworkType Net;

extern ALLEGRO_FILE *hostfile,*clientfile;

int NetStartNetwork(void);
void NetStopNetwork(void);

int NetStartHost(int);      //server
void NetStopServer(void);
void NetStopListen(void);

int NetStartClient(void);
void NetStopClient(void);
void NetDisconnectClient(void);

void NetSendReady(void);
void NetSendGameState(void);
void NetSendGameOver(void);
void NetSendKeys(void);
void NetSendShipState(void);
void NetStartGame(void);
void NetSendAbort(void);
void NetSendKilled(int killer);
void NetSendOutOfLives(void);

void AddressToString(int address, char* string);
NetMessageType ServiceNetwork(void);
