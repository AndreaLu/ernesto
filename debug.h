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
#include <windows.h>

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
    std::cout << "Connected!" << std::endl;

}

struct __attribute__((packed)) PACKET {
    int barX;
    float radius0;
    float radius1;
    float angle0;
    float angle1;
    int selection;
};
static_assert(sizeof(PACKET) == 24);

void SendArcsPacket() {
    Arc* arcs = GetArcs();
    PACKET pkt;
    pkt.barX = GetBarX();
    pkt.radius0 = arcs[0].radius;
    pkt.radius1 = arcs[1].radius;
    pkt.angle0 = arcs[0].angle;
    pkt.angle1 = arcs[1].angle;
    pkt.selection = GetSelection();
    // Invio dati
    if (send(sock, &pkt, sizeof(PACKET), 0) != (ssize_t)sizeof(PACKET)) {
        perror("send");
        return;
    }
}


int main() {
    bool a_pressed = false;
    bool a_prev_pressed = false;

    bool b_pressed = false;
    bool b_prev_pressed = false;
    GameInit();
    
    while (true) {
        a_pressed = (GetAsyncKeyState('A') & 0x8000);
        if( a_pressed && !a_prev_pressed)
            GamePressButton();
        a_prev_pressed = a_pressed;

        b_pressed = (GetAsyncKeyState('B') & 0x8000);
        if( b_pressed && !b_prev_pressed)
            GameLongPressButton();
        b_prev_pressed = b_pressed;
        
        GameUpdate();
    }
    return 0;
}


#endif