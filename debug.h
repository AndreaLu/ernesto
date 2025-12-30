#ifndef NETWORK_H
#define NETWORK_H


/*
  debug.h: only included when compiling the project under windwos for testing
    It implements a simple TCP client that sends the arcs data to a python
    server that renders it to screen, just to test the game logic easily without
    compiling graphical code
*/
#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <chrono>
#include <thread>
#include "game.h"

unsigned long GetMillis() {
    auto now = std::chrono::steady_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    unsigned long retv = static_cast<unsigned long>(ms);
    return(retv);
}



int sock;
uint8_t* packet = new uint8_t[MAX_ARCS*sizeof(Arc)];


void InitSocket() {
    const char* ip = "127.0.0.1"; // vpn ip "100.72.23.62";  // IP di destinazione
    int port = 5007;    

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return;
    }
    // Setup indirizzo server
    sockaddr_in serverAddr;
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &serverAddr.sin_addr) <= 0) {
        perror("inet_pton");
        return;
    }
    // Connessione
    if (connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("connect");
        return;
    }
}

struct __attribute__((packed)) PACKET {
    int barX;
    float radius0;
    float radius1;
    float radius2;
    float angle0;
    float angle1;
    float angle2;
    int selection;
};
static_assert(sizeof(PACKET) == 32);

void SendArcsPacket() {
    Arc* arcs = GetArcs();
    PACKET pkt;
    pkt.barX = GetBarX();
    pkt.radius0 = arcs[0].radius;
    pkt.radius1 = arcs[1].radius;
    pkt.radius2 = arcs[2].radius;
    pkt.angle0 = arcs[0].angle;
    pkt.angle1 = arcs[1].angle;
    pkt.angle2 = arcs[2].angle;
    pkt.selection = GetSelection();
    // Invio dati
    if (send(sock, &pkt, sizeof(PACKET), 0) != (ssize_t)sizeof(PACKET)) {
        perror("send");
        return;
    }
}

float timer0 = 0;
float timer1 = 0;
float timer2 = 0;
bool dir = false;
int cnt = 0;
int main() {
    InitSocket();
    GameInit();

    while (true) {
        GameUpdate();
        SendArcsPacket();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / 60));
        timer0 += 1.0f/60.0f;
        
        if( timer0 >= 4 ) {
            GamePressButton();
            timer0 = 0;
        }

        timer1 += 1.0f/60.0f;
        if( timer1 >= 1.0f/4.0f ) {
            timer1 = 0.0f;
            GameRotateEncoder( dir );
            cnt += 1;
            if( cnt >= 10 ) {
                cnt = 0;
                dir = !dir;
            }
        }

        timer2 += 1.0f/60.0f;
        if( timer2 >= 5.0f ) {
            timer2 = 0.0f;
            GameLongPressButton();
        }


    }
    return 0;
}


#endif