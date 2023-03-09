#include <gb/gb.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
/*to draw individual dots and lines*/
#include <gb/drawing.h>

#include "map_map.h"
#include "map_tiles.h"

#define camera_max_y ((map_mapHeight - 18) * 8) 
#define camera_max_x ((map_mapWidth - 20) * 8) 

#define MIN(A,B) ((A)<(B)?(A):(B))

uint8_t joy;

uint8_t sprite_data[480] = {
    0x00,0x3c,0x3c,0x7d,0x3c,0x15,0x34,0x09,0xa5,0x7e,0x3c,0x01,0x00,0x3c,0x24,0x24, /*player model */
    0x3c,0x3c,0x42,0x7e,0xbd,0xc3,0x05,0x03,0x8d,0xc3,0x19,0x07,0x06,0x3e,0x18,0x78, /*attack particle*/
    0x39,0x38,0x4c,0x74,0xbe,0xc2,0xb7,0xc9,0x6f,0x59,0xbf,0xc1,0x42,0x7e,0x3d,0x3c, /*roll animation*/
    0x00,0x3c,0x3c,0x41,0x3c,0x29,0x34,0x3d,0xa5,0xdb,0x3c,0x3d,0x00,0x3c,0x24,0x00,  /*attack model*/

/*owl*/
    0xfe,0xaa,0xff,0xa9,0x42,0x7e,0xff,0x81,0xff,0x81,0xef,0x91,0xd7,0xa9,0x7e,0x42, /*leg*/
    0xfb,0xfc,0x9d,0x86,0xbd,0x86,0xfe,0x83,0xce,0xb3,0xdf,0xa1,0xf3,0x80,0xf7,0x80, /*wing1*/
    0xff,0x81,0xcf,0xb1,0xdf,0xa1,0xf3,0x8d,0xf7,0x89,0xff,0x81,0x7d,0x43,0x3f,0x3f, /*wing2*/
    0x7f,0x7f,0x80,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x07,0xf8,0x1f,0xe0,0x3f,0xc0, /*ass*/
    0x3f,0xc0,0x3f,0xc0,0x3f,0xc0,0x3f,0xc0,0x3f,0xc1,0x1e,0xe7,0x08,0xff,0x10,0xf9, /*belly*/
    0xfc,0x9f,0xff,0x83,0xff,0x90,0xef,0xb8,0xd7,0xfc,0xfe,0xb9,0x7d,0x42,0x3f,0x3f, /*head*/

    0x7e,0x00,0xe7,0x5a,0xe7,0x5a,0xff,0x5a,0xe7,0x3c,0x66,0x3c,0x7e,0x18,0x3c,0x00, //enemy

    0x08,0x08,0x04,0x0c,0x7a,0x76,0xfd,0x83,0xfd,0x83,0x7a,0x76,0x04,0x0c,0x08,0x08 //arrow
};

//to tell what tiles cause collision
uint8_t collision_detection[]={
    0x04,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e
};

//to tell what tile has the bonfire (please change if better solution is found: the bonfire alway has the same graphics)
uint16_t bonfire=1770;

//to tell which positions cause text to appear upon interation (pos_x+map_width*pos_y)
uint16_t interact[]={
    1289, 1290
};

UINT8 joypadPrevious;
//indicates current room
uint8_t room=0;
// current and old positions of the camera in pixels
uint16_t camera_x=160;
uint16_t camera_y=144;
uint16_t old_camera_x=160;
uint16_t old_camera_y=144;
// current and old position of the map in tiles
uint8_t map_pos_x=20;
uint8_t map_pos_y=18;
uint8_t old_map_pos_x=20;
uint8_t old_map_pos_y=18;
// redraw flag, indicates that camera position was changed
uint8_t redraw;
//  gives the amount of sprites on screen (minus the two dedicated to the player) //
uint8_t sprite_num=0;
/* for checking random shit (better than malloc at least)*/
int mptr=0;

/*replacement for the floor function in divisions*/
int floor(int x, int div) {
    return (x-(x%div))/div;
}

void set_camera() {
    // update hardware scroll position
    SCY_REG = camera_y; SCX_REG = camera_x; 
    // up or down
    map_pos_y = (uint8_t)(camera_y >> 3u);
    if (map_pos_y != old_map_pos_y) { 
        if (camera_y < old_camera_y) {
            set_bkg_submap(map_pos_x, map_pos_y, MIN(21u, map_mapWidth-map_pos_x), 1, map_map, map_mapWidth);
        } else {
            if ((map_mapHeight - 18u) > map_pos_y) set_bkg_submap(map_pos_x, map_pos_y + 18u, MIN(21u, map_mapWidth-map_pos_x), 1, map_map, map_mapWidth);     
        }
        old_map_pos_y = map_pos_y; 
    }
    // left or right
    map_pos_x = (uint8_t)(camera_x >> 3u);
    if (map_pos_x != old_map_pos_x) {
        if (camera_x < old_camera_x) {
            set_bkg_submap(map_pos_x, map_pos_y, 1, MIN(19u, map_mapHeight - map_pos_y), map_map, map_mapWidth);     
        } else {
            if ((map_mapWidth - 20u) > map_pos_x) set_bkg_submap(map_pos_x + 20u, map_pos_y, 1, MIN(19u, map_mapHeight - map_pos_y), map_map, map_mapWidth);     
        }
        old_map_pos_x = map_pos_x;
    }
    // set old camera position to current camera position
    old_camera_x = camera_x, old_camera_y = camera_y;
}

joypads_t joypads;

/*frame counter, useful for animations and shit. Ev stores the frames where we want to call a function (an event happens), which will redirect to evf
evc just tells you where you are on the writing to ev*/
uint8_t frc=1;
uint8_t ev[256];
uint8_t eva[256][6];
void (*evf[256])();
uint8_t evc=0;
//measures the point we're at in evf and eva
uint8_t needle=0;

/*player x and y coordinates + direction and momentum and stamina and health*/
int16_t plx=236; //84+152
int16_t ply=208; //80+128
int8_t plxs=0;
int8_t plys=0;
int8_t plright=0;
int8_t pldown=1;
uint8_t stamina=144;
uint8_t health=144;
//look, it's just souls let's not lie
uint8_t pressure=18;
//weapon and armour equipped
uint8_t weapon=0;
uint8_t armour=0;
//total Healthy boy, Magic, Strength, Dexterity, Intelligence, Forest Affinity
uint8_t stats[6]={1,1,1,1,1,1};
//for the tile in front of an entity
uint16_t front_tile=0;

//lookup tables for weapon, armor, rings, etc.
//weapon
uint8_t wbasestr(){
    if (weapon==0) return 80;
    
    else return 0;
}
uint8_t wscalestr(){
    if (weapon==0) return 20;
    
    else return 0;
}
uint8_t wbasemag(){
    if (weapon==0) return 0;

    else return 0;
}
uint8_t wscalemag(){
    if (weapon==0) return 0;

    else return 0;
}
uint8_t wbasedex(){
    if (weapon==0) return 0;

    else return 0;
}
uint8_t wscaledex(){
    if (weapon==0) return 0;

    else return 0;
}
uint8_t wbaseint(){
    if (weapon==0) return 0;

    else return 0;
}
uint8_t wscaleint(){
    if (weapon==0) return 0;

    else return 0;
}
uint8_t wbaseaff(){
    if (weapon==0) return 60;

    else return 0;
}
uint8_t wscaleaff(){
    if (weapon==0) return 60;

    else return 0;
}
uint8_t wbasehlt(){
    if (weapon==0) return 0;

    else return 0;
}
uint8_t wscalehlt(){
    if (weapon==0) return 0;

    else return 0;
}

//enemies
uint8_t en_astr(int n){
    if (n==0) return 4;

    else return 1;
}
uint8_t en_amag(int n){
    if (n==0) return 3;

    else return 1;
}
uint8_t en_adex(int n){
    if (n==0) return 3;

    else return 1;
}
uint8_t en_aint(int n){
    if (n==0) return 3;

    else return 1;
}
uint8_t en_aaff(int n){
    if (n==0) return 1;
    
    else return 1;
}
uint8_t en_ahlt(int n){
    if (n==0) return 3;

    else return 1;
}

/*checks  if the player can move*/
//0=player cannot move  1=normal movement  2=menu  3=weapons  4=armour  5=items  6=friends 7=interact (text) 8=bonfire
uint8_t can_move=1;

//same things but for other entities (for now, max 32)
int8_t entype[32];
int16_t enx[32];
int16_t eny[32];
int8_t enxs[32];
int8_t enys[32];
int8_t enright[32];
int8_t endown[32];
uint8_t enstamina[32];
uint8_t enhealth[32];
uint8_t encan_move[32];
//for amount of enemies to be rendered
uint8_t en_num=0;

/*function to add and substract health and stamina*/
void phealth(int stam){
    /*for (volatile int i=0;i<stam;i++) {
        plot(health+8+1+i, 5, BLACK, SOLID);
    } */
    health+=stam;
}
void mhealth(int stam){
    /*for (volatile int i=0;i<stam;i++) {
        plot(health+8-i, 5, WHITE, SOLID);
    } */
    health-=stam;
}
void pstamina(int stam){
    /*for (volatile int i=0;i<stam;i++) {
        plot(stamina+8+1+i, 5, BLACK, SOLID);
    } */
    stamina+=stam;
}
void mstamina(int stam){
    /*for (volatile int i=0;i<stam;i++) {
        plot(stamina+8-i, 5, WHITE, SOLID);
    } */
    stamina-=stam;
}


//to  push enemies
void enpush(){
    enxs[eva[needle][0]]+=eva[needle][1];
    enys[eva[needle][0]]+=eva[needle][2];
}
void enstop(){
    enxs[eva[needle][0]]=0;
    enys[eva[needle][0]]=0;
    if (encan_move[eva[needle][0]]!=9) encan_move[eva[needle][0]]=0;
}


/*Used to set event, enter the number of frames you want to wait, followed by the function*/
void event(uint8_t frame,  void (*func)()) {
    //there are 128 event slots, in the case where this isn't enough, you can double it
    if (evc==127) evc=0;
    else evc++;
    if (frc>255-frame) ev[evc]=frc-(255-frame);
    else ev[evc]=frc+frame;
    evf[evc]=func;
}




//to make enemies
//always put enemies after the other entities
void make_enemy(uint8_t type, uint8_t tile, uint16_t x, uint16_t y){
    set_sprite_tile(sprite_num+2,tile);
    move_sprite(sprite_num+2,x, y);
    entype[en_num]=type;
    enx[en_num]=x+plx-84;
    eny[en_num]=y+ply-80;
    enxs[en_num]=0;
    enys[en_num]=0;
    enright[en_num]=0;
    endown[en_num]=1;
    enhealth[en_num]=100;
    enstamina[en_num]=100;
    encan_move[en_num]=1;
    en_num++;
    sprite_num++;
}

void kill_enemy(uint8_t num) {
    move_sprite(num+2+(sprite_num-en_num),255, 255);
    enhealth[num]=0;
    enxs[num]=0;
    eny[num]=0;
    enx[num]=0;
    eny[num]=0;
    encan_move[num]=9;

    if (entype[num]==0) pressure++;
}

//when the player hits something
void hit(uint8_t num) {
    if (enhealth[num]<(stats[0]*wscalehlt()+wbasehlt())/en_ahlt(entype[num])+(stats[1]*wscalemag()+wbasemag())/en_amag(entype[num])+(stats[2]*wscalestr()+wbasestr())/en_astr(entype[num])+(stats[3]*wscaledex()+wbasedex())/en_adex(entype[num])+(stats[4]*wscaleint()+wbaseint())/en_aint(entype[num])) kill_enemy(num);
    else enhealth[num]-=(stats[0]*wscalehlt()+wbasehlt())/en_ahlt(entype[num])+(stats[1]*wscalemag()+wbasemag())/en_amag(entype[num])+(stats[2]*wscalestr()+wbasestr())/en_astr(entype[num])+(stats[3]*wscaledex()+wbasedex())/en_adex(entype[num])+(stats[4]*wscaleint()+wbaseint())/en_aint(entype[num])/en_aint(num);
}



/*animations block, please label carefully*/
void attack_4() {
    can_move=1;
    move_sprite(1, 160,160);
}
void attack_3() {
    NR10_REG=0X00;
    NR11_REG=0X40;
    NR12_REG=0X33;
    NR13_REG=0X63;
    NR14_REG=0X46;
    if ((plright==-1) && (pldown==-1)) {
        move_sprite(1,84-8,80);
        for (int i=0; i<en_num; i++) {
                    if ((plx-8<=enx[i]+8) && (enx[i]<=plx-8+8) && (ply<=eny[i]+8) && (eny[i]<=ply+8)) {
                        encan_move[i]=0, enxs[i]+=plright, enys[i]+=pldown;
                        eva[evc][0]=i;
                        hit(i);
                        event(150, enstop);
                    }
                }
    }
    else if ((plright==0) && (pldown==-1)) {
        move_sprite(1,84-8,80-8);
        for (int i=0; i<en_num; i++) {
                    if ((plx-8<=enx[i]+8) && (enx[i]<=plx-8+8) && (ply-8<=eny[i]+8) && (eny[i]<=ply-8+8)) {
                        encan_move[i]=0, enxs[i]+=plright, enys[i]+=pldown;
                        eva[evc][0]=i;
                        hit(i);
                        event(150, enstop);
                    }
                }
    }
    else if ((plright==1) && (pldown==-1)) {
        move_sprite(1,84,80-8);
        for (int i=0; i<en_num; i++) {
                    if ((plx<=enx[i]+8) && (enx[i]<=plx+8) && (ply-8<=eny[i]+8) && (eny[i]<=ply-8+8)) {
                        encan_move[i]=0, enxs[i]+=plright, enys[i]+=pldown;
                        eva[evc][0]=i;
                        hit(i);
                        event(150, enstop);
                    }
                }
    }
    else if ((plright==1) && (pldown==0)) {
        move_sprite(1,84+8,80-8);
        for (int i=0; i<en_num; i++) {
                    if ((plx+8<=enx[i]+8) && (enx[i]<=plx+8+8) && (ply-8<=eny[i]+8) && (eny[i]<=ply-8+8)) {
                        encan_move[i]=0, enxs[i]+=plright, enys[i]+=pldown;
                        eva[evc][0]=i;
                        hit(i);
                        event(150, enstop);
                    }
                }
    }
    else if ((plright==1) && (pldown==1)) {
        move_sprite(1,84+8,80);
        for (int i=0; i<en_num; i++) {
                    if ((plx+8<=enx[i]+8) && (enx[i]<=plx+8+8) && (ply<=eny[i]+8) && (eny[i]<=ply+8)) {
                        encan_move[i]=0, enxs[i]+=plright, enys[i]+=pldown;
                        eva[evc][0]=i;
                        hit(i);
                        event(150, enstop);
                    }
                }
    }
    else if ((plright==0) && (pldown==1)) {
        move_sprite(1,84+8,80+8);
        for (int i=0; i<en_num; i++) {
                    if ((plx+8<=enx[i]+8) && (enx[i]<=plx+8+8) && (ply+8<=eny[i]+8) && (eny[i]<=ply+8+8)) {
                        encan_move[i]=0, enxs[i]+=plright, enys[i]+=pldown;
                        eva[evc][0]=i;
                        hit(i);
                        event(150, enstop);
                    }
                }
    }
    else if ((plright==-1) && (pldown==1)) {
        move_sprite(1,84,80+8);
        for (int i=0; i<en_num; i++) {
                    if ((plx<=enx[i]+8) && (enx[i]<=plx+8) && (ply+8<=eny[i]+8) && (eny[i]<=ply+8+8)) {
                        encan_move[i]=0, enxs[i]+=plright, enys[i]+=pldown;
                        eva[evc][0]=i;
                        hit(i);
                        event(150, enstop);
                    }
                }
    }
    else if ((plright==-1) && (pldown==0)) {
        move_sprite(1,84-8,80+8);
        for (int i=0; i<en_num; i++) {
                    if ((plx-8<=enx[i]+8) && (enx[i]<=plx-8+8) && (ply+8<=eny[i]+8) && (eny[i]<=ply+8+8)) {
                        encan_move[i]=0, enxs[i]+=plright, enys[i]+=pldown;
                        eva[evc][0]=i;
                        hit(i);
                        event(150, enstop);
                    }
                }
    }
    event(5,attack_4);
}
void attack_2() {
    NR10_REG=0X00;
    NR11_REG=0X40;
    NR12_REG=0X33;
    NR13_REG=0X63;
    NR14_REG=0X46;
    event(5, attack_3);
    move_sprite(1,84+8*plright,80+8*pldown);
    for (int i=0; i<en_num; i++) {
                    if ((plx+8*plright<=enx[i]+8) && (enx[i]<=plx+8*plright+8) && (ply+8*pldown<=eny[i]+8) && (eny[i]<=ply+8*pldown+8)) {
                        encan_move[i]=0, enxs[i]+=plright, enys[i]+=pldown;
                        eva[evc][0]=i;
                        hit(i);
                        event(150, enstop);
                    }
                }
}
void attack_1() {
    NR10_REG=0X00;
    NR11_REG=0X40;
    NR12_REG=0X33;
    NR13_REG=0X63;
    NR14_REG=0X46;
    event(5, attack_2);
    if (plright==1) set_sprite_prop(1,0);
                else set_sprite_prop(1,32);
                if ((plright==-1) && (pldown==-1)) {
                    move_sprite(1,84,80-8);
                    for (int i=0; i<en_num; i++) {
                    if ((plx<=enx[i]+8) && (enx[i]<=plx+8) && (ply-8<=eny[i]+8) && (eny[i]<=ply-8+8)) {
                        encan_move[i]=0, enxs[i]+=plright, enys[i]+=pldown;
                        eva[evc][0]=i;
                        hit(i);
                        event(150, enstop);
                    }
                }
                }
                else if ((plright==0) && (pldown==-1)) {
                    move_sprite(1,84+8,80-8);
                    for (int i=0; i<en_num; i++) {
                    if ((plx+8<=enx[i]+8) && (enx[i]<=plx+8+8) && (ply-8<=eny[i]+8) && (eny[i]<=ply-8+8)) {
                        encan_move[i]=0, enxs[i]+=plright, enys[i]+=pldown;
                        eva[evc][0]=i;
                        hit(i);
                        event(150, enstop);
                    }
                }
                }
                else if ((plright==1) && (pldown==-1)) {
                    move_sprite(1,84+8,80);
                    for (int i=0; i<en_num; i++) {
                    if ((plx+8<=enx[i]+8) && (enx[i]<=plx+8+8) && (ply<=eny[i]+8) && (eny[i]<=ply+8)) {
                        encan_move[i]=0, enxs[i]+=plright, enys[i]+=pldown;
                        eva[evc][0]=i;
                        hit(i);
                        event(150, enstop);
                    }
                }
                }
                else if ((plright==1) && (pldown==0)) {
                    move_sprite(1,84+8,80+8);
                    for (int i=0; i<en_num; i++) {
                    if ((plx+8<=enx[i]+8) && (enx[i]<=plx+8+8) && (ply+8<=eny[i]+8) && (eny[i]<=ply+8+8)) {
                        encan_move[i]=0, enxs[i]+=plright, enys[i]+=pldown;
                        eva[evc][0]=i;
                        hit(i);
                        event(150, enstop);
                    }
                }
                }
                else if ((plright==1) && (pldown==1)) {
                    move_sprite(1,84,80+8);
                    for (int i=0; i<en_num; i++) {
                    if ((plx<=enx[i]+8) && (enx[i]<=plx+8) && (ply+8<=eny[i]+8) && (eny[i]<=ply+8+8)) {
                        encan_move[i]=0, enxs[i]+=plright, enys[i]+=pldown;
                        eva[evc][0]=i;
                        hit(i);
                        event(150, enstop);
                    }
                }
                }
                else if ((plright==0) && (pldown==1)) {
                    move_sprite(1,84-8,80+8);
                    for (int i=0; i<en_num; i++) {
                    if ((plx-8<=enx[i]+8) && (enx[i]<=plx-8+8) && (ply+8<=eny[i]+8) && (eny[i]<=ply+8+8)) {
                        encan_move[i]=0, enxs[i]+=plright, enys[i]+=pldown;
                        eva[evc][0]=i;
                        hit(i);
                        event(150, enstop);
                    }
                }
                }
                else if ((plright==-1) && (pldown==1)) {
                    move_sprite(1,84-8,80);
                    for (int i=0; i<en_num; i++) {
                    if ((plx-8<=enx[i]+8) && (enx[i]<=plx-8+8) && (ply<=eny[i]+8) && (eny[i]<=ply+8)) {
                        encan_move[i]=0, enxs[i]+=plright, enys[i]+=pldown;
                        eva[evc][0]=i;
                        hit(i);
                        event(150, enstop);
                    }
                }
                }
                else if ((plright==-1) && (pldown==0)) {
                    move_sprite(1,84-8,80-8);
                    for (int i=0; i<en_num; i++) {
                    if ((plx-8<=enx[i]+8) && (enx[i]<=plx-8+8) && (ply-8+8*pldown<=eny[i]+8) && (eny[i]<=ply-8+8)) {
                        encan_move[i]=0, enxs[i]+=plright, enys[i]+=pldown;
                        eva[evc][0]=i;
                        hit(i);
                        event(150, enstop);
                    }
                }
                }
    set_sprite_tile(0,0);
}

void roll_1() {
    camera_x-=plxs, camera_y-=plys, plx-=plxs, ply-=plys, can_move=1, plys=0, plxs=0, set_sprite_tile(0,0);
}









void main(void) {
    DISPLAY_OFF;
    SHOW_BKG;
    set_bkg_data(0, 17u, map_tiles);

    map_pos_x = 20;
    map_pos_y = 18; 
    old_map_pos_x = 19;
    old_map_pos_y = 17;
    set_bkg_submap(map_pos_x, map_pos_y, 20, 18, map_map, map_mapWidth);
    DISPLAY_ON;
    
    camera_x = 160;
    camera_y = 144;
    old_camera_x = camera_x; old_camera_y = camera_y;

    redraw = FALSE;

    SCX_REG = camera_x; SCY_REG = camera_y; 


    BGP_REG = OBP0_REG = OBP1_REG = 0xE4;
    set_sprite_data(0, 11u, sprite_data);
    set_sprite_tile(0,0);
    set_sprite_tile(1,1);
    move_sprite(0,84, 80);
    move_sprite(1,160,160);

    for (needle=0; needle<3; needle++){
        set_sprite_tile(2*needle+2,needle+4);
        set_sprite_tile(2*needle+3,needle+4);
        set_sprite_prop(2*needle+3,32);
        move_sprite(2*needle+2, 72, 8*needle+16);
        move_sprite(2*needle+3, 96, 8*needle+16);
    }
    for (needle=0; needle<3; needle++){
        set_sprite_tile(2*(needle+3)+2,needle+3+4);
        set_sprite_tile(2*(needle+3)+3,needle+3+4);
        set_sprite_prop(2*(needle+3)+3,32);
        move_sprite(2*(needle+3)+2, 80, 8*needle+20);
        move_sprite(2*(needle+3)+3, 88, 8*needle+20);
    }
    set_sprite_tile(0,0);
    sprite_num=12;

    //enemies
    make_enemy(0,10,108,120);
    make_enemy(0,10,148,120);
    SHOW_SPRITES;
    /* makes the health and stamina bars*/
    /*for (volatile int i=0; i<144; i++){
    plot(i+8, 5, DKGREY, SOLID);
    } */
    /*for (volatile int i=0; i<144; i++){
    plot(i+8, 2, BLACK, SOLID);
    } */

    NR52_REG = 0x80; // is 1000 0000 in binary and turns on sound
    NR50_REG = 0x77; // sets the volume for both left and right channel just set to max 0x77
    NR51_REG = 0xFF; // is 1111 1111 in binary, select which chanels we want to use in this case all of them. One bit for the L one bit for the R of all four channels
    
    while(1) {
        if (can_move==1)  {
            if ((joypad() & J_A) && !(joypadPrevious & J_A) && (stamina>50)){
                NR10_REG=0X00;
                NR11_REG=0X50;
                NR12_REG=0X43;
                NR13_REG=0X73;
                NR14_REG=0X86;
                event(10, attack_1);
                can_move=0;
                mstamina(50);
                set_sprite_tile(0,3);
            }
        }
            if (can_move==1)  {
                if (joypad() & J_LEFT)plx--, plright=-1, set_sprite_prop(0,32), camera_x--, redraw = TRUE;
                else if (joypad() & J_RIGHT)plx++, plright=1, set_sprite_prop(0,0), camera_x++, redraw = TRUE;
                else if ((joypad() & J_UP)||(joypad() & J_DOWN)) plright=0;
                if (joypad() & J_UP)ply--,pldown=-1, camera_y--, redraw = TRUE;
                else if (joypad() & J_DOWN)ply++,pldown=1, camera_y++, redraw = TRUE;
                else if ((joypad() & J_LEFT)||(joypad() & J_RIGHT)) pldown=0;
            if ((joypad() & J_B) && (stamina>50)) {
            mstamina(50);
            plxs=2*plright;
            plys=2*pldown;
            redraw=TRUE;
            can_move=0;
            event(10, roll_1);
            set_sprite_tile(0,2);
            }
            //interact
            if ((joypad() & J_SELECT) && !(joypadPrevious & J_SELECT)){
                //bonfire
                if (((plx+4-4*plright)/8)+map_mapWidth*((ply+4-4*pldown)/8)+plright+map_mapWidth*pldown==bonfire) {
                    //reset states
                    sprite_num-=en_num;
                    en_num=0;
                    if (room==0) make_enemy(0,10,108+160-camera_x,120+144-camera_y), make_enemy(0,10,148+160-camera_x,120+144-camera_y);
                    
                    camera_x = 230;
                    camera_y = 254;
                    set_camera();
                    redraw=TRUE;
                    for(mptr=0; mptr<20; mptr++) printf("\n");
                    printf("Level: %d\n\nHealthy boy: %d\nMagic: %d\nStrength: %d\nDexterity: %d\nIntelligence: %d\n\nForest\nAffinity: %d\n\nPressure: %d\nRequired: %d\n\n\n", stats[0]+stats[1]+stats[2]+stats[3]+stats[4]+stats[5], stats[0], stats[1], stats[2],stats[3],stats[4],stats[5], pressure,(stats[0]+stats[1]+stats[2]+stats[3]+stats[4]+stats[5])*3 );
                    set_sprite_data(0, sprite_num+2, sprite_data);
                    set_sprite_tile(sprite_num+2,11);
                    move_sprite(sprite_num+2,22,50);
                    sprite_num++;
                    can_move=8;
                }
                //interact
                for (needle=0;needle<sizeof(*interact);needle++){
                if (((plx+4-4*plright)/8)+map_mapWidth*((ply+4-4*pldown)/8)+plright+map_mapWidth*pldown==interact[needle]) {
                    camera_x = 250;
                    camera_y = 294;
                    set_camera();
                    redraw=TRUE;
                    for(mptr=0; mptr<20; mptr++) printf("\n");
                    can_move=7;
                    //couldn't make all the text fit in ram, so it goes in memory instead WARNING: VERY LONG, PLEASE COLLAPSE
                    if (room==0) {
                        if (needle==0) {
                            printf("Hello small warf\n\n\n\n\n\n\n\n\n\n");
                        }
                        else if (needle==1) {
                            printf("Goodbye small warf\n\n\n\n\n\n\n\n\n\n");
                        }
                    }
                break;
                }
                }
            }
            //open meny
            if ((joypad() & J_START) && !(joypadPrevious & J_START)){
                camera_x = 230;
                camera_y = 294;
                set_camera();
                redraw=TRUE;
                for(mptr=0; mptr<20; mptr++) printf("\n");
                printf("Little Dwarf\n\n\nWeapon\n\nArmour\n\nItems\n\nBest friend");
                set_sprite_data(0, sprite_num+2, sprite_data);
                set_sprite_tile(sprite_num+2,11);
                move_sprite(sprite_num+2,22,66);
                sprite_num++;
                can_move=2;
            }
             joypadPrevious=joypad();
            }
            //for the menu movement
            else if (can_move==2) {
                /*here I use camera_y when moving the arrow. This does not move the camera, as you need to use set_camera() to actualize it
                I onliny use it to measure the position of the arrow without having to make another variable  (good practices)*/
                if ((joypad() & J_UP) && !(joypadPrevious & J_UP)) scroll_sprite(sprite_num+1, 0, -16), camera_y--;
                else if ((joypad() & J_DOWN) && !(joypadPrevious & J_DOWN)) scroll_sprite(sprite_num+1, 0, 16), camera_y++;
                else if ((joypad() & J_START) && !(joypadPrevious & J_START)){
                camera_x = plx-76;
                camera_y = ply-64;
                set_camera();
                redraw=TRUE;
                set_sprite_data(0, 11u, sprite_data);
                move_sprite(sprite_num+1,160,160);
                sprite_num--;
                can_move=1;
                DISPLAY_OFF;
                SHOW_BKG;
                set_bkg_data(0, 17u, map_tiles);
                set_bkg_submap(old_map_pos_x, old_map_pos_y, 20, 18, map_map, map_mapWidth);
                DISPLAY_ON;
            }
            else if ((joypad() & J_A) && !(joypadPrevious & J_A)){
                if (camera_y==294) printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\nList of weapons");
                else if (camera_y==295) printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\nList of armour");
                else if (camera_y==296) printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\nList of items");
                else if (camera_y==297) printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\nList of friends");
            }
            else if ((joypad() & J_B) && !(joypadPrevious & J_B)){
                printf("\n\n\n\n\nLittle Dwarf\n\n\nWeapon\n\nArmor\n\nItems\n\nBest friend");
                can_move=2;
            }
                joypadPrevious=joypad();
            }
            
            //text interaction
            else if (can_move==7) {
                if ((joypad() & J_SELECT) && !(joypadPrevious & J_SELECT)){
                camera_x = plx-76;
                camera_y = ply-64;
                set_camera();
                redraw=TRUE;
                can_move=1;
                DISPLAY_OFF;
                SHOW_BKG;
                set_bkg_data(0, 17u, map_tiles);
                set_bkg_submap(old_map_pos_x, old_map_pos_y, 20, 18, map_map, map_mapWidth);
                DISPLAY_ON;
            }
                joypadPrevious=joypad();
            }

            //bonfire
            else if (can_move==8) {
                /*here I use camera_y when moving the arrow. This does not move the camera, as you need to use set_camera() to actualize it
                I onliny use it to measure the position of the arrow without having to make another variable  (good practices)*/
                if ((joypad() & J_UP) && !(joypadPrevious & J_UP)) scroll_sprite(sprite_num+1, 0, -8), camera_y--;
                else if ((joypad() & J_DOWN) && !(joypadPrevious & J_DOWN)) scroll_sprite(sprite_num+1, 0, 8), camera_y++;
                else if ((joypad() & J_SELECT) && !(joypadPrevious & J_SELECT)){
                camera_x = plx-76;
                camera_y = ply-64;
                set_camera();
                redraw=TRUE;
                set_sprite_data(0, 11u, sprite_data);
                move_sprite(sprite_num+1,160,160);
                sprite_num--;
                can_move=1;
                DISPLAY_OFF;
                SHOW_BKG;
                set_bkg_data(0, 17u, map_tiles);
                set_bkg_submap(old_map_pos_x, old_map_pos_y, 20, 18, map_map, map_mapWidth);
                DISPLAY_ON;
            }
            else if ((joypad() & J_A) && !(joypadPrevious & J_A)){
                if (pressure>=(stats[0]+stats[1]+stats[2]+stats[3]+stats[4]+stats[5])*3) {
                    pressure-=(stats[0]+stats[1]+stats[2]+stats[3]+stats[4]+stats[5])*3;
                    if (camera_y==254) stats[0]++;
                    if (camera_y==255) stats[1]++;
                    if (camera_y==256) stats[2]++;
                    if (camera_y==257) stats[3]++;
                    if (camera_y==258) stats[4]++;
                    camera_x = plx-76;
                    camera_y = ply-64;
                    set_camera();
                    redraw=TRUE;
                    set_sprite_data(0, 11u, sprite_data);
                    move_sprite(sprite_num+1,160,160);
                    sprite_num--;
                    can_move=1;
                    DISPLAY_OFF;
                    SHOW_BKG;
                 set_bkg_data(0, 17u, map_tiles);
                 set_bkg_submap(old_map_pos_x, old_map_pos_y, 20, 18, map_map, map_mapWidth);
                 DISPLAY_ON;
                }
            }
                joypadPrevious=joypad();
            }

            //enemy movement and collision
            for (needle=0;needle<en_num;needle++) {
                if ((enxs[needle]!=0)||(enys[needle]!=0)){
                enx[needle]+=enxs[needle];
                eny[needle]+=enys[needle];
                scroll_sprite(sprite_num-en_num+needle+2,enxs[needle],enys[needle]);
                front_tile=map_mapPLN0[floor(enx[needle],8)+map_mapWidth*floor(eny[needle],8)];
                for (mptr=0;mptr<sizeof collision_detection / sizeof collision_detection[0];mptr++){
                if (front_tile==collision_detection[mptr]){
                    enxs[needle]=0;
                    enys[needle]=0;
                    break;
                }
            }
            }
            }


            if ((plxs)||(plys)) redraw=TRUE;
            camera_x+=plxs, camera_y+=plys, plx+=plxs, ply+=plys;
            if (frc==255) frc=1;
            else frc++;
            if (stamina<144) stamina++/*, plot(stamina+8, 5, DKGREY, SOLID)*/;
            //events
            for (needle=0; needle<=127; needle++) {
                if (ev[needle]==frc) (*evf[needle])(), ev[needle]=0;
            }
        if (redraw) {
            //collision detection
            mptr=0;
            front_tile=map_mapPLN0[floor(plx+4-4*plright,8)+map_mapWidth*floor(ply+4-4*pldown,8)+plright+map_mapWidth*pldown];
            for (needle=0;needle<sizeof collision_detection / sizeof collision_detection[0];needle++){
            if (front_tile==collision_detection[needle]){
                mptr=1;
                break;
            }
            }
    
            if (mptr==0) {        
            for (needle=0; needle<sprite_num; needle++){
                if ((needle<(sprite_num-en_num)) || (encan_move[needle-(sprite_num-en_num)]!=9)) scroll_sprite(needle+2, old_camera_x-camera_x,old_camera_y-camera_y);
            }
            
            }
            else {
                if (can_move==0) plx-=plxs, ply-=plys, camera_x-=plxs, camera_y-=plys;
                else plx-=plright,ply-=pldown, camera_x-=plright, camera_y-=pldown;
            }
            redraw = FALSE;
            set_camera();
        }
        wait_vbl_done();
    }
}