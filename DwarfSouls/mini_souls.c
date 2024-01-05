#include <gb/gb.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "map_map.h"
#include "map_tiles.h"
#include "text_tiles.h"
#include "win_map.h"

#define camera_max_y ((map_mapHeight - 18) * 8) 
#define camera_max_x ((map_mapWidth - 20) * 8) 

#define MIN(A,B) ((A)<(B)?(A):(B))

uint8_t joy;

uint8_t sprite_data[480];

//to tell what tiles cause collision
uint8_t collision_detection[]={
    0x05,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f
};

//to tell what tile has the bonfire (please change if better solution is found: the bonfire alway has the same graphics)
uint16_t bonfire=1770;
//to tell what ROOM the previous bonfire was in
uint8_t prev_bonfire=0;
//to tell which positions cause text to appear upon interation (pos_x+map_width*pos_y)
uint16_t interact[20];

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
//look plx and ply used to be signed integers, along with enx and eny. I can't figure out why so I made them unsigned. This might cause problems
//if so just change them back to signed and stop asking questions
uint16_t plx=236; //84+152
uint16_t ply=208; //80+128
int8_t plxs=0;
int8_t plys=0;
int8_t plright=0;
int8_t pldown=1;
uint8_t max_stamina=144;
uint8_t max_health=144;
uint8_t stamina=144;
uint8_t health=144;
//look, it's just souls let's not lie
uint8_t pressure=18;
//weapon equipped
uint8_t weapon=1;
//Full weapon list is found in the function weapon_print
uint8_t weapon_list[40]={1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
//item equipped
uint8_t item=1;
//Full item list is found in the function item_print
uint8_t item_list[40]={1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
//number of root left (estus lol)
uint8_t root_ocean=3;
//total Healthy boy, Magic, Strength, Dexterity, Intelligence, Forest Affinity
uint8_t stats[6]={1,1,1,1,1,1};
//for the tile in front of an entity
uint16_t front_tile=0;
/*checks  if the player can move*/
//0=player cannot move 1=normal movement  2=attacking  3=weapons 4=items  5=friends 7=interact (text) 8=bonfire 9=menu
uint8_t can_move=1;



void change_sprite(int num,int val1,int val2,int val3,int val4,int val5,int val6,int val7,int val8,int val9,int val10,int val11,int val12,int val13,int val14,int val15,int val16) {
        sprite_data[(num*16)+0]=val1;
        sprite_data[(num*16)+1]=val2;
        sprite_data[(num*16)+2]=val3;
        sprite_data[(num*16)+3]=val4;
        sprite_data[(num*16)+4]=val5;
        sprite_data[(num*16)+5]=val6;
        sprite_data[(num*16)+6]=val7;
        sprite_data[(num*16)+7]=val8;
        sprite_data[(num*16)+8]=val9;
        sprite_data[(num*16)+9]=val10;
        sprite_data[(num*16)+10]=val11;
        sprite_data[(num*16)+11]=val12;
        sprite_data[(num*16)+12]=val13;
        sprite_data[(num*16)+13]=val14;
        sprite_data[(num*16)+14]=val15;
        sprite_data[(num*16)+15]=val16;
        set_sprite_data(0, 9u, sprite_data);
}

//lookup tables for weapon, armor, rings, etc.
//weapon
uint8_t wbasestr(){
    if (weapon==1) return 80;
    else if (weapon==2) return 1;
    
    else return 0;
}
uint8_t wscalestr(){
    if (weapon==1) return 20;
    else if (weapon==2) return 1;
    
    else return 0;
}
uint8_t wbasemag(){
    if (weapon==1) return 0;
    else if (weapon==2) return 0;

    else return 0;
}
uint8_t wscalemag(){
    if (weapon==1) return 0;
    else if (weapon==2) return 0;

    else return 0;
}
uint8_t wbasedex(){
    if (weapon==1) return 0;
    else if (weapon==2) return 0;

    else return 0;
}
uint8_t wscaledex(){
    if (weapon==1) return 0;
    else if (weapon==2) return 0;

    else return 0;
}
uint8_t wbaseint(){
    if (weapon==1) return 0;
    else if (weapon==2) return 0;

    else return 0;
}
uint8_t wscaleint(){
    if (weapon==1) return 0;
    else if (weapon==2) return 0;

    else return 0;
}
uint8_t wbaseaff(){
    if (weapon==1) return 60;
    else if (weapon==2) return 0;

    else return 0;
}
uint8_t wscaleaff(){
    if (weapon==1) return 60;
    else if (weapon==2) return 0;

    else return 0;
}
uint8_t wbasehlt(){
    if (weapon==1) return 0;
    else if (weapon==2) return 0;

    else return 0;
}
uint8_t wscalehlt(){
    if (weapon==1) return 0;
    else if (weapon==2) return 0;

    else return 0;
}

//to print the lists where necessary
void weapon_print(int n) {
    printf("\n\n\n\n\n\n\n\n\n\nList of weapons");
    for (volatile int i=0; i<6; i++) {
        if (weapon_list[i+(7*n)]==1) printf("\n\nBRKN Sword");
        else if (weapon_list[i+(7*n)]==2) printf("\n\nLondor Hammer");
        else printf("\n\n");
    }
}
void item_print(int n) {
    printf("\n\n\n\n\n\n\n\n\n\nList of items");
    for (volatile int i=0; i<6; i++) {
        if (item_list[i+(7*n)]==1) printf("\n\nRoot Ocean "), printf("%d", root_ocean);
        else printf("\n\n");
    }
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

//same things but for other entities (for now, max 32)
uint8_t entype[32];
//look plx and ply used to be signed integers, along with enx and eny. I can't figure out why so I made them unsigned. This might cause problems
//if so just change them back to signed and stop asking questions
uint16_t enx[32];
uint16_t eny[32];
int8_t enxs[32];
int8_t enys[32];
//enemy hurt box x and y
uint8_t enattx[32];
uint8_t enatty[32];
uint8_t enhealth[32];
uint8_t encan_move[32];
//for amount of enemies to be rendered
uint8_t en_num=0;

/*function to add and substract health and stamina*/
/*why is the variable called stam ? Buddy do I look like a smart person I have no idea*/
void phealth(int stam){
    if (health+stam>max_health) health=max_health;
    else health+=stam;
    for (volatile int i=(health/(max_health/20));i>0;i--) {
        win_mapPLN0[i-1]=0x01;
    }
    set_win_tiles(0,0,win_mapWidth,win_mapHeight,win_map);
    redraw=TRUE;
}
//mhealth has the death code
void mhealth(int stam){
    //death code
    if (stam>=health) {
        camera_x = 230;
        camera_y = 254;
        set_camera();
        redraw=TRUE;
        HIDE_WIN;
        printf("\n\n\n\n\n\n\n\nYOU DIED\n\n\n\n\n");
        can_move=8;
        room=prev_bonfire;
        root_ocean=3;
        sprite_num++;
        if (room==0) plx=236, ply=208;
    }
    else {
    health-=stam;
    for (volatile int i=20-(health/(max_health/20));i>=0;i--) {
        win_mapPLN0[19-i]=0x00;
    }
    set_win_tiles(0,0,win_mapWidth,win_mapHeight,win_map);
    redraw=TRUE;
    }
}
void pstamina(int stam){
    stamina+=stam;
    if (stamina==max_stamina) {
        win_mapPLN0[39]=0x01;
        win_mapPLN0[38]=0x01;
    }
    else if ((stamina%(max_stamina/20))==0) {
        win_mapPLN0[(stamina/(max_stamina/20))+18]=0x01;
        win_mapPLN0[(stamina/(max_stamina/20))+19]=0x01;
        set_win_tiles(0,0,win_mapWidth,win_mapHeight,win_map);
        redraw=TRUE;
    }
}
void mstamina(int stam){
    stamina-=stam;
    for (volatile int i=20-(stamina/(max_stamina/20));i>=0;i--) {
        win_mapPLN0[39-i]=0x00;
    }
    set_win_tiles(0,0,win_mapWidth,win_mapHeight,win_map);
    redraw=TRUE;
}


//to  push enemies
void enpush(){
    enxs[eva[needle][0]]+=eva[needle][1];
    enys[eva[needle][0]]+=eva[needle][2];
}
void enstop(){
    enxs[eva[needle][0]]=0;
    enys[eva[needle][0]]=0;
    if (encan_move[eva[needle][0]]!=9) encan_move[eva[needle][0]]=1;
}

//to push the player
void plpush(){
    plxs+=eva[needle][0];
    plys+=eva[needle][1];
    can_move=0;
}
void plstop(){
    camera_x-=plxs, camera_y-=plys, plx-=plxs, ply-=plys;
    plxs=0;
    plys=0;
    can_move=1;
    change_sprite(0,0x00,0x3c,0x3c,0x7d,0x3c,0x15,0x34,0x09,0xa5,0x7e,0x3c,0x01,0x00,0x3c,0x24,0x24);
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
    enattx[en_num]=0;
    enatty[en_num]=0;
    enhealth[en_num]=100;
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
void root_1() {
    if (can_move==2){
    can_move=1;
    change_sprite(0,0x00,0x3c,0x3c,0x7d,0x3c,0x15,0x34,0x09,0xa5,0x7e,0x3c,0x01,0x00,0x3c,0x24,0x24);
    if (root_ocean>0) phealth(40), root_ocean--;
    }
}

void londor_hammer_attack_2() {
    if (can_move==2){
    can_move=1;
    move_sprite(1, 0,0);
    }
}

void londor_hammer_attack_1() {
    if (can_move==2){
    NR10_REG=0X00;
    NR11_REG=0X40;
    NR12_REG=0X33;
    NR13_REG=0X63;
    NR14_REG=0X46;
    event(5, londor_hammer_attack_2);
    move_sprite(1,84+8*plright,80+8*pldown);
    for (int i=0; i<en_num; i++) {
                    if ((plx+8*plright<=enx[i]+8) && (enx[i]<=plx+8*plright+8) && (ply+8*pldown<=eny[i]+8) && (eny[i]<=ply+8*pldown+8)) {
                        encan_move[i]=0, enxs[i]+=plright, enys[i]+=pldown;
                        hit(i);
                        event(60, enstop);
                        eva[evc][0]=i;
                    }
                }
    }
    change_sprite(0,0x00,0x3c,0x3c,0x7d,0x3c,0x15,0x34,0x09,0xa5,0x7e,0x3c,0x01,0x00,0x3c,0x24,0x24);
}

void attack_4() {
    if (can_move==2){
    can_move=1;
    move_sprite(1, 0,0);
    }
}
void attack_3() {
    if (can_move==2){
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
                        hit(i);
                        event(10, enstop);
                        eva[evc][0]=i;
                    }
                }
    }
    else if ((plright==0) && (pldown==-1)) {
        move_sprite(1,84-8,80-8);
        for (int i=0; i<en_num; i++) {
                    if ((plx-8<=enx[i]+8) && (enx[i]<=plx-8+8) && (ply-8<=eny[i]+8) && (eny[i]<=ply-8+8)) {
                        encan_move[i]=0, enxs[i]+=plright, enys[i]+=pldown;
                        hit(i);
                        event(10, enstop);
                        eva[evc][0]=i;
                    }
                }
    }
    else if ((plright==1) && (pldown==-1)) {
        move_sprite(1,84,80-8);
        for (int i=0; i<en_num; i++) {
                    if ((plx<=enx[i]+8) && (enx[i]<=plx+8) && (ply-8<=eny[i]+8) && (eny[i]<=ply-8+8)) {
                        encan_move[i]=0, enxs[i]+=plright, enys[i]+=pldown;
                        hit(i);
                        event(10, enstop);
                        eva[evc][0]=i;
                    }
                }
    }
    else if ((plright==1) && (pldown==0)) {
        move_sprite(1,84+8,80-8);
        for (int i=0; i<en_num; i++) {
                    if ((plx+8<=enx[i]+8) && (enx[i]<=plx+8+8) && (ply-8<=eny[i]+8) && (eny[i]<=ply-8+8)) {
                        encan_move[i]=0, enxs[i]+=plright, enys[i]+=pldown;
                        hit(i);
                        event(10, enstop);
                        eva[evc][0]=i;
                    }
                }
    }
    else if ((plright==1) && (pldown==1)) {
        move_sprite(1,84+8,80);
        for (int i=0; i<en_num; i++) {
                    if ((plx+8<=enx[i]+8) && (enx[i]<=plx+8+8) && (ply<=eny[i]+8) && (eny[i]<=ply+8)) {
                        encan_move[i]=0, enxs[i]+=plright, enys[i]+=pldown;
                        hit(i);
                        event(10, enstop);
                        eva[evc][0]=i;
                    }
                }
    }
    else if ((plright==0) && (pldown==1)) {
        move_sprite(1,84+8,80+8);
        for (int i=0; i<en_num; i++) {
                    if ((plx+8<=enx[i]+8) && (enx[i]<=plx+8+8) && (ply+8<=eny[i]+8) && (eny[i]<=ply+8+8)) {
                        encan_move[i]=0, enxs[i]+=plright, enys[i]+=pldown;
                        hit(i);
                        event(10, enstop);
                        eva[evc][0]=i;
                    }
                }
    }
    else if ((plright==-1) && (pldown==1)) {
        move_sprite(1,84,80+8);
        for (int i=0; i<en_num; i++) {
                    if ((plx<=enx[i]+8) && (enx[i]<=plx+8) && (ply+8<=eny[i]+8) && (eny[i]<=ply+8+8)) {
                        encan_move[i]=0, enxs[i]+=plright, enys[i]+=pldown;
                        hit(i);
                        event(10, enstop);
                        eva[evc][0]=i;
                    }
                }
    }
    else if ((plright==-1) && (pldown==0)) {
        move_sprite(1,84-8,80+8);
        for (int i=0; i<en_num; i++) {
                    if ((plx-8<=enx[i]+8) && (enx[i]<=plx-8+8) && (ply+8<=eny[i]+8) && (eny[i]<=ply+8+8)) {
                        encan_move[i]=0, enxs[i]+=plright, enys[i]+=pldown;
                        hit(i);
                        event(10, enstop);
                        eva[evc][0]=i;
                    }
                }
    }
    event(5,attack_4);
    }
}
void attack_2() {
    if (can_move==2){
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
                        hit(i);
                        event(10, enstop);
                        eva[evc][0]=i;
                    }
                }
    }
}
void attack_1() {
    if (can_move==2){
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
                        hit(i);
                        event(10, enstop);
                        eva[evc][0]=i;
                    }
                }
                }
                else if ((plright==0) && (pldown==-1)) {
                    move_sprite(1,84+8,80-8);
                    for (int i=0; i<en_num; i++) {
                    if ((plx+8<=enx[i]+8) && (enx[i]<=plx+8+8) && (ply-8<=eny[i]+8) && (eny[i]<=ply-8+8)) {
                        encan_move[i]=0, enxs[i]+=plright, enys[i]+=pldown;
                        hit(i);
                        event(10, enstop);
                        eva[evc][0]=i;
                    }
                }
                }
                else if ((plright==1) && (pldown==-1)) {
                    move_sprite(1,84+8,80);
                    for (int i=0; i<en_num; i++) {
                    if ((plx+8<=enx[i]+8) && (enx[i]<=plx+8+8) && (ply<=eny[i]+8) && (eny[i]<=ply+8)) {
                        encan_move[i]=0, enxs[i]+=plright, enys[i]+=pldown;
                        hit(i);
                        event(10, enstop);
                        eva[evc][0]=i;
                    }
                }
                }
                else if ((plright==1) && (pldown==0)) {
                    move_sprite(1,84+8,80+8);
                    for (int i=0; i<en_num; i++) {
                    if ((plx+8<=enx[i]+8) && (enx[i]<=plx+8+8) && (ply+8<=eny[i]+8) && (eny[i]<=ply+8+8)) {
                        encan_move[i]=0, enxs[i]+=plright, enys[i]+=pldown;
                        hit(i);
                        event(10, enstop);
                        eva[evc][0]=i;
                    }
                }
                }
                else if ((plright==1) && (pldown==1)) {
                    move_sprite(1,84,80+8);
                    for (int i=0; i<en_num; i++) {
                    if ((plx<=enx[i]+8) && (enx[i]<=plx+8) && (ply+8<=eny[i]+8) && (eny[i]<=ply+8+8)) {
                        encan_move[i]=0, enxs[i]+=plright, enys[i]+=pldown;
                        hit(i);
                        event(10, enstop);
                        eva[evc][0]=i;
                    }
                }
                }
                else if ((plright==0) && (pldown==1)) {
                    move_sprite(1,84-8,80+8);
                    for (int i=0; i<en_num; i++) {
                    if ((plx-8<=enx[i]+8) && (enx[i]<=plx-8+8) && (ply+8<=eny[i]+8) && (eny[i]<=ply+8+8)) {
                        encan_move[i]=0, enxs[i]+=plright, enys[i]+=pldown;
                        hit(i);
                        event(10, enstop);
                        eva[evc][0]=i;
                    }
                }
                }
                else if ((plright==-1) && (pldown==1)) {
                    move_sprite(1,84-8,80);
                    for (int i=0; i<en_num; i++) {
                    if ((plx-8<=enx[i]+8) && (enx[i]<=plx-8+8) && (ply<=eny[i]+8) && (eny[i]<=ply+8)) {
                        encan_move[i]=0, enxs[i]+=plright, enys[i]+=pldown;
                        hit(i);
                        event(10, enstop);
                        eva[evc][0]=i;
                    }
                }
                }
                else if ((plright==-1) && (pldown==0)) {
                    move_sprite(1,84-8,80-8);
                    for (int i=0; i<en_num; i++) {
                    if ((plx-8<=enx[i]+8) && (enx[i]<=plx-8+8) && (ply-8+8*pldown<=eny[i]+8) && (eny[i]<=ply-8+8)) {
                        encan_move[i]=0, enxs[i]+=plright, enys[i]+=pldown;
                        hit(i);
                        event(10, enstop);
                        eva[evc][0]=i;
                    }
                }
                }
    change_sprite(0,0x00,0x3c,0x3c,0x7d,0x3c,0x15,0x34,0x09,0xa5,0x7e,0x3c,0x01,0x00,0x3c,0x24,0x24);
    }
}

void roll_1() {
    camera_x-=plxs, camera_y-=plys, plx-=plxs, ply-=plys, can_move=1, plys=0, plxs=0, change_sprite(0,0x00,0x3c,0x3c,0x7d,0x3c,0x15,0x34,0x09,0xa5,0x7e,0x3c,0x01,0x00,0x3c,0x24,0x24);
}


void en_attack2() {
    if (encan_move[eva[needle][0]]==2) {
    move_sprite(1, 0,0);
    encan_move[eva[needle][0]]=1;
}
}
void en_attack1() {
    if (encan_move[eva[needle][0]]==3) {
        encan_move[eva[needle][0]]=2;
    move_sprite(1,(enattx[eva[needle][0]]-plx)+84,(enatty[eva[needle][0]]-ply)+80);
    //attack other enemies (attacking player is comprised in "all movement" section)
    for (mptr=0; mptr<en_num;mptr++) {
                        if ((abs((enx[mptr]&255)-eva[needle][1])<5) && (abs((eny[mptr]&255)-eva[needle][2])<5) && (eva[needle][0]!=mptr)&&((encan_move[mptr]==1)||(encan_move[mptr]==2))) {
                            enxs[mptr]=((enx[mptr]-enx[eva[needle][0]])>0)*2-1;
                            enys[mptr]=((eny[mptr]-eny[eva[needle][0]])>0)*2-1;
                            encan_move[mptr]=0;
                            event(20, enstop);
                            eva[evc][0]=mptr;
                            break;
                        }
                    }
    event(30, en_attack2);
    eva[evc][0]=eva[needle][0];
}
}

void room0() {
    DISPLAY_OFF;
    SHOW_BKG;
    HIDE_WIN;
    bonfire=1770;

    interact[0]=1289, interact[1]=1290;//both tiles of the owl
    interact[2]=1410;//item pickup
    set_camera();
    set_bkg_data(0, 18u, map_tiles);
    set_bkg_submap(map_pos_x, map_pos_y, 20, 18, map_map, map_mapWidth);

    move_win(7,128);  
    set_win_tiles(0,0,win_mapWidth,win_mapHeight,win_map);
    

    DISPLAY_ON;
    SHOW_WIN;
    change_sprite(0,0x00,0x3c,0x3c,0x7d,0x3c,0x15,0x34,0x09,0xa5,0x7e,0x3c,0x01,0x00,0x3c,0x24,0x24); /*player model */
    //0x39,0x38,0x4c,0x74,0xbe,0xc2,0xb7,0xc9,0x6f,0x59,0xbf,0xc1,0x42,0x7e,0x3d,0x3c, /*roll animation*/
    //0x00,0x3c,0x3c,0x41,0x3c,0x29,0x34,0x3d,0xa5,0xdb,0x3c,0x3d,0x00,0x3c,0x24,0x00,  /*attack model*/
    change_sprite(1,0x3c,0x3c,0x42,0x7e,0xbd,0xc3,0x05,0x03,0x8d,0xc3,0x19,0x07,0x06,0x3e,0x18,0x78); /*attack particle*/
    //0x08,0x08,0x04,0x0c,0x7a,0x76,0xfd,0x83,0xfd,0x83,0x7a,0x76,0x04,0x0c,0x08,0x08 //arrow

/*owl*/
    change_sprite(2,0xfe,0xaa,0xff,0xa9,0x42,0x7e,0xff,0x81,0xff,0x81,0xef,0x91,0xd7,0xa9,0x7e,0x42); /*leg*/
    change_sprite(3,0xfb,0xfc,0x9d,0x86,0xbd,0x86,0xfe,0x83,0xce,0xb3,0xdf,0xa1,0xf3,0x80,0xf7,0x80); /*wing1*/
    change_sprite(4,0xff,0x81,0xcf,0xb1,0xdf,0xa1,0xf3,0x8d,0xf7,0x89,0xff,0x81,0x7d,0x43,0x3f,0x3f); /*wing2*/
    change_sprite(5,0x7f,0x7f,0x80,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x07,0xf8,0x1f,0xe0,0x3f,0xc0); /*ass*/
    change_sprite(6,0x3f,0xc0,0x3f,0xc0,0x3f,0xc0,0x3f,0xc0,0x3f,0xc1,0x1e,0xe7,0x08,0xff,0x10,0xf9); /*belly*/
    change_sprite(7,0xfc,0x9f,0xff,0x83,0xff,0x90,0xef,0xb8,0xd7,0xfc,0xfe,0xb9,0x7d,0x42,0x3f,0x3f); /*head*/

    change_sprite(8,0x7e,0x00,0xe7,0x5a,0xe7,0x5a,0xff,0x5a,0xe7,0x3c,0x66,0x3c,0x7e,0x18,0x3c,0x00); //enemy
    
    sprite_num=0;
    en_num=0;
    for (needle=0; needle<3; needle++){
        set_sprite_tile(2*needle+2,needle+2);
        set_sprite_tile(2*needle+3,needle+2);
        set_sprite_prop(2*needle+3,32);
        move_sprite(2*needle+2, 72+160-camera_x, (8*needle+16)+144-camera_y);
        move_sprite(2*needle+3, 96+160-camera_x, (8*needle+16)+144-camera_y);
    }
    for (needle=0; needle<3; needle++){
        set_sprite_tile(2*(needle+3)+2,needle+3+2);
        set_sprite_tile(2*(needle+3)+3,needle+3+2);
        set_sprite_prop(2*(needle+3)+3,32);
        move_sprite(2*(needle+3)+2, 80+160-camera_x, (8*needle+20)+144-camera_y);
        move_sprite(2*(needle+3)+3, 88+160-camera_x, (8*needle+20)+144-camera_y);
    }
    sprite_num=12;
    make_enemy(0,8,108+160-camera_x,120+144-camera_y);
    make_enemy(0,8,148+160-camera_x,120+144-camera_y);
    SHOW_SPRITES;
    redraw=TRUE;
}

void soft_room0() {
    DISPLAY_OFF;
    SHOW_BKG;
    set_camera();
    set_bkg_data(0, 18u, map_tiles);
    set_bkg_submap(map_pos_x, map_pos_y, 20, 18, map_map, map_mapWidth);
    DISPLAY_ON;
    SHOW_WIN;
    redraw=TRUE;
}

void main(void) {
    map_pos_x = 20;
    map_pos_y = 18; 
    old_map_pos_x = 19;
    old_map_pos_y = 17;
    
    camera_x = 160;
    camera_y = 144;
    old_camera_x = camera_x; old_camera_y = camera_y;

    redraw = FALSE;

    SCX_REG = camera_x; SCY_REG = camera_y; 


    BGP_REG = OBP0_REG = OBP1_REG = 0xE4;
    

    room0();
    set_sprite_tile(0,0);
    set_sprite_tile(1,1);
    move_sprite(0,84, 80);
    move_sprite(1,160,160);

    NR52_REG = 0x80; // is 1000 0000 in binary and turns on sound
    NR50_REG = 0x77; // sets the volume for both left and right channel just set to max 0x77
    NR51_REG = 0xFF; // is 1111 1111 in binary, selects which chanels we want to use, in this case all of them. One bit for the L one bit for the R of all four channels
    
    while(1) {
            //attack being put first fixes some problems so fuck you it stays there
            if (can_move==1)  {
            if ((joypad() & J_A) && !(joypadPrevious & J_A) && (stamina>50)){
                NR10_REG=0X00;
                NR11_REG=0X50;
                NR12_REG=0X43;
                NR13_REG=0X73;
                NR14_REG=0X86;
                if (weapon==1) event(20, attack_1), can_move=2,mstamina(50);
                else if (weapon==2) event(1, londor_hammer_attack_1), can_move=2,mstamina(1);
                change_sprite(0,0x00,0x3c,0x3c,0x41,0x3c,0x29,0x34,0x3d,0xa5,0xdb,0x3c,0x3d,0x00,0x3c,0x24,0x00);
            }
        }
            if (can_move==1)  {
                if (joypad() & J_LEFT)plx--,plright=-1, set_sprite_prop(0,32), camera_x--, redraw = TRUE;
                else if (joypad() & J_RIGHT)plx++, plright=1, set_sprite_prop(0,0), camera_x++, redraw = TRUE;
                else if ((joypad() & J_UP)||(joypad() & J_DOWN)) plright=0;
                if (joypad() & J_UP)ply--, pldown=-1, camera_y--, redraw = TRUE;
                else if (joypad() & J_DOWN)ply++, pldown=1, camera_y++, redraw = TRUE;
                else if ((joypad() & J_LEFT)||(joypad() & J_RIGHT)) pldown=0;
                if ((joypad() & J_LEFT)||(joypad() & J_RIGHT)||(joypad() & J_UP)||(joypad() & J_DOWN)) {
                    if (frc%14==0) move_sprite(0,84,81);
                    else if (frc%7==0) move_sprite(0,84,80);
                }
            if ((joypad() & J_B) && (stamina>50)) {
            mstamina(50);
            plxs=2*plright;
            plys=2*pldown;
            redraw=TRUE;
            can_move=0;
            event(10, roll_1);
            change_sprite(0,0x39,0x38,0x4c,0x74,0xbe,0xc2,0xb7,0xc9,0x6f,0x59,0xbf,0xc1,0x42,0x7e,0x3d,0x3c);
            }
            //interact
            if ((joypad() & J_SELECT) && !(joypadPrevious & J_SELECT)){
                //bonfire
                if (((plx+4-4*plright)>>3)+map_mapWidth*((ply+4-4*pldown)>>3)+plright+map_mapWidth*pldown==bonfire) {
                    prev_bonfire=room;
                    //reset states
                    camera_x = 230;
                    camera_y = 254;
                    root_ocean=3;
                    set_camera();
                    redraw=TRUE;
                    HIDE_WIN;
                    set_bkg_data(0, 26u, text_tiles);
                    for(mptr=0; mptr<20; mptr++) printf("\n");
                    printf("Level: %d\n\nHealthy boy: %d\nMagic: %d\nStrength: %d\nDexterity: %d\nIntelligence: %d\n\nForest\nAffinity: %d\n\nPressure: %d\nRequired: %d\n\n\n", stats[0]+stats[1]+stats[2]+stats[3]+stats[4]+stats[5], stats[0], stats[1], stats[2],stats[3],stats[4],stats[5], pressure,(stats[0]+stats[1]+stats[2]+stats[3]+stats[4]+stats[5])*3 );
                    change_sprite(1,0x08,0x08,0x04,0x0c,0x7a,0x76,0xfd,0x83,0xfd,0x83,0x7a,0x76,0x04,0x0c,0x08,0x08);
                    set_sprite_data(0, sprite_num+2, sprite_data);
                    set_sprite_tile(sprite_num+2,1);
                    move_sprite(sprite_num+2,22,50);
                    sprite_num++;
                    can_move=8;
                }
                //interact
                for (needle=0;needle<20;needle++){
                if (((plx+4-4*plright)>>3)+map_mapWidth*((ply+4-4*pldown)>>3)+plright+map_mapWidth*pldown==interact[needle]) {
                    camera_x = 250;
                    camera_y = 294;
                    set_camera();
                    redraw=TRUE;
                    for(mptr=0; mptr<20; mptr++) printf("\n");
                    can_move=7;
                    //couldn't make all the text fit in ram, so it goes in memory instead WARNING: VERY LONG, PLEASE COLLAPSE
                    HIDE_WIN;
                    if (room==0) {
                        if (needle==0) {
                            printf("Hello small warf\n\n\n\n\n\n\n\n\n\n");
                        }
                        else if (needle==1) {
                            printf("Goodbye small warf\n\n\n\n\n\n\n\n\n\n");
                        }
                        else if (needle==2) {
                            map_mapPLN0[1410]=0x00;
                            printf("Got the hammer of londor\n\n\n\n\n\n\n\n\n\n");
                            for (needle=0;needle<40;needle++) {
                                if ((weapon_list[needle]==0)||(weapon_list[needle]==2)) {
                                    weapon_list[needle]=2;
                                    break;
                                }
                            }
                            can_move=9;
                            //so that it doesn't fuck the sprites by sending it to menu
                            sprite_num++;
                        }
                    }
                break;
                }
                }
                //item use
                if (can_move==1) {
                    //root ocean
                    if (item==1) {
                        can_move=2;
                        NR10_REG=0X20;
                        NR11_REG=0X10;
                        NR12_REG=0Xa0;
                        NR13_REG=0X60;
                        NR14_REG=0Xe6;
                        event(60, root_1);
                        change_sprite(0,0x00,0x3c,0x3c,0x41,0x3c,0x29,0x34,0x3d,0xa5,0xdb,0x3c,0x3d,0x00,0x3c,0x24,0x00);
                    }
                }
            }
            //open meny
            if ((joypad() & J_START) && !(joypadPrevious & J_START)){
                camera_x = 230;
                camera_y = 294;
                set_camera();
                redraw=TRUE;
                HIDE_WIN;
                for(mptr=0; mptr<20; mptr++) printf("\n");
                set_bkg_data(0, 26u, text_tiles);
                printf("Little Dwarf\n\n\nWeapon\n\nItems\n\nBest friend\n\n%d", pressure);
                change_sprite(1,0x08,0x08,0x04,0x0c,0x7a,0x76,0xfd,0x83,0xfd,0x83,0x7a,0x76,0x04,0x0c,0x08,0x08);
                    set_sprite_data(0, sprite_num+2, sprite_data);
                    set_sprite_tile(sprite_num+2,1);
                move_sprite(sprite_num+2,22,66);
                sprite_num++;
                can_move=9;
            }
             joypadPrevious=joypad();
            }

            //for weapon menu movement
            else if (can_move==3) {
                if ((joypad() & J_UP) && !(joypadPrevious & J_UP)) scroll_sprite(sprite_num+1, 0, -16), camera_y--;
                else if ((joypad() & J_DOWN) && !(joypadPrevious & J_DOWN)) scroll_sprite(sprite_num+1, 0, 16), camera_y++;
                else if ((joypad() & J_START) && !(joypadPrevious & J_START)){
                camera_x = plx-76;
                camera_y = ply-64;
                can_move=1;
                move_sprite(sprite_num+1,160,160);
                sprite_num--;
                change_sprite(1,0x3c,0x3c,0x42,0x7e,0xbd,0xc3,0x05,0x03,0x8d,0xc3,0x19,0x07,0x06,0x3e,0x18,0x78);
                if (room==0) soft_room0();
            }
                else if ((joypad() & J_LEFT) && !(joypadPrevious & J_LEFT)){
                    if (needle>0){
                        needle--;
                        weapon_print(needle);
                    }
                }
                else if ((joypad() & J_RIGHT) && !(joypadPrevious & J_RIGHT)){
                        needle++;
                        weapon_print(needle);
                }
                else if ((joypad() & J_A) && !(joypadPrevious & J_A)){
                if (camera_y==292) weapon=weapon_list[0+(needle*7)];
                else if (camera_y==293) weapon=weapon_list[1+(needle*7)];
                else if (camera_y==294) weapon=weapon_list[2+(needle*7)];
                else if (camera_y==295) weapon=weapon_list[3+(needle*7)];
                else if (camera_y==296) weapon=weapon_list[4+(needle*7)];
                else if (camera_y==297) weapon=weapon_list[5+(needle*6)];
                set_bkg_data(0, 26u, text_tiles);
                printf("\n\n\n\n\nLittle Dwarf\n\n\nWeapon\n\nItems\n\nBest friend\n\n%d", pressure);
                can_move=9;
            }
                else if ((joypad() & J_B) && !(joypadPrevious & J_B)){
                set_bkg_data(0, 26u, text_tiles);
                printf("\n\n\n\n\nLittle Dwarf\n\n\nWeapon\n\nItems\n\nBest friend\n\n%d", pressure);
                can_move=9;
            }
                joypadPrevious=joypad();
            }

            //for item menu movement
            else if (can_move==4) {
                if ((joypad() & J_UP) && !(joypadPrevious & J_UP)) scroll_sprite(sprite_num+1, 0, -16), camera_y--;
                else if ((joypad() & J_DOWN) && !(joypadPrevious & J_DOWN)) scroll_sprite(sprite_num+1, 0, 16), camera_y++;
                else if ((joypad() & J_START) && !(joypadPrevious & J_START)){
                camera_x = plx-76;
                camera_y = ply-64;
                can_move=1;
                move_sprite(sprite_num+1,160,160);
                sprite_num--;
                change_sprite(1,0x3c,0x3c,0x42,0x7e,0xbd,0xc3,0x05,0x03,0x8d,0xc3,0x19,0x07,0x06,0x3e,0x18,0x78);
                if (room==0) soft_room0();
            }
                else if ((joypad() & J_LEFT) && !(joypadPrevious & J_LEFT)){
                    if (needle>0){
                        needle--;
                        item_print(needle);
                    }
                }
                else if ((joypad() & J_RIGHT) && !(joypadPrevious & J_RIGHT)){
                        needle++;
                        item_print(needle);
                }
                else if ((joypad() & J_A) && !(joypadPrevious & J_A)){
                if (camera_y==292) item=item_list[0+(needle*7)];
                else if (camera_y==293) item=item_list[1+(needle*7)];
                else if (camera_y==294) item=item_list[2+(needle*7)];
                else if (camera_y==295) item=item_list[3+(needle*7)];
                else if (camera_y==296) item=item_list[4+(needle*7)];
                else if (camera_y==297) item=item_list[5+(needle*6)];
                set_bkg_data(0, 26u, text_tiles);
                printf("\n\n\n\n\nLittle Dwarf\n\n\nWeapon\n\nItems\n\nBest friend\n\n%d", pressure);
                can_move=9;
            }
                else if ((joypad() & J_B) && !(joypadPrevious & J_B)){
                set_bkg_data(0, 26u, text_tiles);
                printf("\n\n\n\n\nLittle Dwarf\n\n\nWeapon\n\nItems\n\nBest friend\n\n%d", pressure);
                can_move=9;
            }
                joypadPrevious=joypad();
            }
            
            //for the menu movement
            else if (can_move==9) {
                /*here I use camera_y when moving the arrow. This does not move the camera, as you need to use set_camera() to actualize it
                I onliny use it to measure the position of the arrow without having to make another variable  (good practices)*/
                if ((joypad() & J_UP) && !(joypadPrevious & J_UP)) scroll_sprite(sprite_num+1, 0, -16), camera_y--;
                else if ((joypad() & J_DOWN) && !(joypadPrevious & J_DOWN)) scroll_sprite(sprite_num+1, 0, 16), camera_y++;
                else if ((joypad() & J_START) && !(joypadPrevious & J_START)){
                camera_x = plx-76;
                camera_y = ply-64;
                can_move=1;
                move_sprite(sprite_num+1,160,160);
                sprite_num--;
                change_sprite(1,0x3c,0x3c,0x42,0x7e,0xbd,0xc3,0x05,0x03,0x8d,0xc3,0x19,0x07,0x06,0x3e,0x18,0x78);
                if (room==0) soft_room0();
            }
            else if ((joypad() & J_A) && !(joypadPrevious & J_A)){
                HIDE_WIN;
                if (camera_y==294) {
                    can_move=3;
                    needle=0;
                    weapon_print(needle);
                }
                if (camera_y==295) {
                    can_move=4;
                    needle=0;
                    item_print(needle);
                }
                else if (camera_y==296) printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\nList of friends");
            }
            else if ((joypad() & J_B) && !(joypadPrevious & J_B)){
                HIDE_WIN;
                set_bkg_data(0, 26u, text_tiles);
                printf("\n\n\n\n\nLittle Dwarf\n\n\nWeapon\n\nItems\n\nBest friend\n\n%d", pressure);
                can_move=9;
            }
                joypadPrevious=joypad();
            }
            
            //text interaction
            else if (can_move==7) {
                if ((joypad() & J_SELECT) && !(joypadPrevious & J_SELECT)){
                camera_x = plx-76;
                camera_y = ply-64;
                can_move=1;
                move_sprite(sprite_num+1,160,160);
                if (room==0) room0();
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
                can_move=1;
                move_sprite(sprite_num+1,160,160);
                phealth(max_health);
                if (room==0) room0();
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
                    can_move=1;
                    move_sprite(sprite_num+1,160,160);
                    phealth(max_health);
                    if (room==0) room0();
                }
            }
                joypadPrevious=joypad();
            }

            //all movement
            if (can_move<3){
            //enemy movement and collision
            for (needle=0;needle<en_num;needle++) {
                    //checks if what the tile horizontal is available, and then the tile vertical to the enemy
                    front_tile=map_mapPLN0[((enx[needle]+4+enxs[needle])>>3)+map_mapWidth*((eny[needle]+4+enys[needle])>>3)];
                    for (mptr=0;mptr<sizeof collision_detection / sizeof collision_detection[0];mptr++){
                    if (front_tile==collision_detection[mptr]){
                        if (encan_move[needle]==0) enxs[needle]=-enxs[needle], enys[needle]=-enys[needle];
                        else enxs[needle]=0, enys[needle]=0;
                        break;
                    }
                    }
                    //so enemies don't collide with each other
                    for (mptr=0; mptr<en_num;mptr++) {
                        if ((abs(enx[needle]+enxs[needle]-enx[mptr])<5) && (abs(eny[needle]+enys[needle]-eny[mptr])<5) && (needle!=mptr)) {
                            enxs[needle]=0;
                            enys[needle]=0;
                            break;
                        }
                    }
                enx[needle]+=enxs[needle];
                eny[needle]+=enys[needle];
                scroll_sprite(sprite_num-en_num+needle+2,enxs[needle],enys[needle]);

            if (encan_move[needle]==1) {
                    if (abs(plx-enx[needle])>abs(ply-eny[needle])) {
                        if (plx>enx[needle]) enxs[needle]=1, enys[needle]=0;
                        else enxs[needle]=-1, enys[needle]=0;
                    }
                    else if (ply>eny[needle]) enys[needle]=1, enxs[needle]=0;
                    else enys[needle]=-1, enxs[needle]=0;

                    //enemy attack
                    if ((abs(plx-enx[needle])<12) && (abs(ply-eny[needle])<12)) {
                        enxs[needle]=0;
                        enys[needle]=0;
                        //look man I'm writing 3 so the function for attack can be interrupted, don't question it
                        encan_move[needle]=3;
                        event(20, en_attack1);
                        eva[evc][0]=needle;
                        enattx[needle]=plx;
                        enatty[needle]=ply;
                        NR10_REG=0X00;
                        NR11_REG=0X50;
                        NR12_REG=0X43;
                        NR13_REG=0X73;
                        NR14_REG=0X86;
                    }
                }
            //enemy to player hit detection (don't ask why it's here)
            else if (encan_move[needle]==2) {
                //player position is 16bit, enatt is 8 so we need to put it on the same scale
                //player takes hit
                if ((abs((plx&255)-enattx[needle])<5)&&(abs((ply&255)-enatty[needle])<5)&&(can_move!=0)){
                can_move=0;
                change_sprite(0,0x00,0x3c,0x3c,0x41,0x3c,0x29,0x34,0x3d,0xa5,0xdb,0x3c,0x3d,0x00,0x3c,0x24,0x00);
                mhealth(3);
                plxs=(plx>enx[needle])*2-1;
                plys=(ply>eny[needle])*2-1;

                event(20, plstop);             
                    }
    }
            }
            

            if ((plxs)||(plys)) redraw=TRUE;
            camera_x+=plxs, camera_y+=plys, plx+=plxs, ply+=plys;
            if (frc==255) frc=1;
            else frc++;
            if (stamina<max_stamina) pstamina(1);
            //events
            for (needle=0; needle<=127; needle++) {
                if (ev[needle]==frc) (*evf[needle])(), ev[needle]=0;
            }
            }
        if (redraw) {
            //collision detection
            mptr=0;
            front_tile=map_mapPLN0[((plx+4+4*plright)>>3)+(map_mapWidth*((ply+7+pldown)>>3))];
            for (needle=0;needle<sizeof collision_detection / sizeof collision_detection[0];needle++){
            if (front_tile==collision_detection[needle]){
                mptr=1;
                break;
            }
            }
            if (mptr==1) {
                if (can_move==0) plx-=plxs, ply-=plys, camera_x-=plxs, camera_y-=plys;
                else plx-=plright,ply-=pldown, camera_x-=plright, camera_y-=pldown;
            }
            scroll_sprite(1, old_camera_x-camera_x,old_camera_y-camera_y);
            for (needle=0; needle<sprite_num; needle++){
                if ((needle<(sprite_num-en_num)) || (encan_move[needle-(sprite_num-en_num)]!=9)) scroll_sprite(needle+2, old_camera_x-camera_x,old_camera_y-camera_y);
            }
            redraw = FALSE;
            set_camera();
        }
        wait_vbl_done();
    }
}