import socket
import struct



import pygame
import math
import time

pygame.init()

def intToCol(icol):
    return (icol & 0xFF, (icol & 0xFF00) >> 8, (icol & 0xFF0000) >> 16)

# --- DPI & scaling ---
def get_scale_for_physical_size(px_w, px_h, diagonal_inch, assumed_dpi=95):
    cmpp_big = 53/1980 # 
    cmpp_small = 4.8 /  math.sqrt(320*320 + 170*170) 
    scale = cmpp_small / cmpp_big
    return scale

VIRTUAL_W, VIRTUAL_H = 320, 170
DIAGONAL_INCH = 1.9

scale = get_scale_for_physical_size(VIRTUAL_W, VIRTUAL_H, DIAGONAL_INCH)
print( scale) 

WINDOW_W = 320 # int(VIRTUAL_W * scale)
WINDOW_H = 170 # int(VIRTUAL_H * scale)

screen = pygame.display.set_mode((WINDOW_W, WINDOW_H))
pygame.display.set_caption("Concentric arcs timing game")

clock = pygame.time.Clock()

# --- game params ---
CENTER = (VIRTUAL_W // 2, VIRTUAL_H // 2)
RADII = [30, 45, 60]
GAP_ANGLE = math.radians(20)

base_speed = math.radians(30)  # deg/sec
speeds = [base_speed * (i + 1) for i in range(len(RADII))]
angles = [0.0 for _ in RADII]

font = pygame.font.SysFont(None, 20)

def draw_arc(surface, color, center, radius, angle, gap):


    start = angle + gap / 2
    end = angle + 2 * math.pi - gap / 2
    rect = pygame.Rect(0, 0, radius * 2, radius * 2)
    rect.center = center
    pygame.draw.arc(surface, color, rect, start, end, 3)

def is_aligned(angles, gap, tol=math.radians(5)):
    for a in angles:
        a = (a + math.pi * 2) % (math.pi * 2)
        if not (a < tol or abs(a - 2 * math.pi) < tol):
            return False
    return True

running = True
success = False
last_press = 0


class PACKET:
    barX,radius0,radius1,radius2,angle0,angle1,angle2,color0,color1,color2,posx0,posx1,posx2,posy0,posy1,posy2,selection = 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    def __init__(self):
        self.radius0 = 30
        self.radius1 = 42
        self.angle0 = 0
        self.angle1 = 1
        self.radius2 = 54
        self.angle2 = 2
        self.color0 = 0xFFFFFF
        self.color1 = 0xFFFFFF
        self.color2 = 0xFFFFFF


pkt = PACKET()

import threading

def server():
    global pkt
    
    TCP_IP = "0.0.0.0"
    TCP_PORT = 5007
    print(f"Ascoltando sulla porta {TCP_PORT}...")
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.bind((TCP_IP, TCP_PORT))
    sock.listen(1)  # accetta una connessione alla volta

    conn, addr = sock.accept()
    print(f"Connessione da {addr}")
    
    while running:
        data = conn.recv(1024)
        if not data: break
        pkt.barX,pkt.radius0,pkt.radius1,pkt.radius2,pkt.angle0,pkt.angle1,pkt.angle2,pkt.color0,pkt.color1,pkt.color2,pkt.posx0,pkt.posx1,pkt.posx2,pkt.posy0,pkt.posy1,pkt.posy2,pkt.selection = struct.unpack("iffffffiiiiiiiiii", data)
        print(f"received angle2: {pkt.angle2}")

    conn.close()

t = threading.Thread(target=server, args=(), kwargs={})
t.start()

while running:
    dt = clock.tick(60) / 1000.0

    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False
        if event.type == pygame.KEYDOWN and event.key == pygame.K_SPACE:
            last_press = time.time()
            success = is_aligned(angles, GAP_ANGLE)

    for i in range(len(angles)):
        angles[i] += speeds[i] * dt

    # --- draw virtual surface ---
    virtual = pygame.Surface((VIRTUAL_W, VIRTUAL_H))
    virtual.fill((20, 20, 20))

    
    draw_arc(virtual, intToCol(pkt.color0), (VIRTUAL_W // 2 + pkt.posx0, VIRTUAL_H // 2 + pkt.posy0), pkt.radius0, pkt.angle0, GAP_ANGLE)
    draw_arc(virtual, intToCol(pkt.color1), (VIRTUAL_W // 2 + pkt.posx1, VIRTUAL_H // 2 + pkt.posy1), pkt.radius1, pkt.angle1 , GAP_ANGLE)
    draw_arc(virtual, intToCol(pkt.color2), (VIRTUAL_W // 2 + pkt.posx2, VIRTUAL_H // 2 + pkt.posy2), pkt.radius2, pkt.angle2 , GAP_ANGLE)
    
    rect = pygame.Rect(pkt.barX-10, 170/2-2, 20, 4)
    pygame.draw.rect(virtual, (200, 200, 200), rect, border_radius=3)


    # scale to real window
    pygame.transform.scale(virtual, (WINDOW_W, WINDOW_H), screen)
    pygame.display.flip()


pygame.quit()