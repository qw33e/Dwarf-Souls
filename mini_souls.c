#include <gb/gb.h>
#include <stdint.h>
#include <stdio.h>
/*to draw individual dots and lines*/
#include <gb/drawing.h>

#include "map_map.h"
#include "map_tiles.h"

#define camera_max_y ((map_mapHeight - 18) * 8) 
#define camera_max_x ((map_mapWidth - 20) * 8) 

#define MIN(A,B) ((A)<(B)?(A):(B))

uint8_t joy;

uint8_t sprite_data[] = {
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

/*bonfire*/
    0x08,0x08,0x3c,0x18,0x66,0x3c,0xff,0x42,0xd7,0xa9,0xff,0x81,0x3c,0x42,0x3c,0x3c, /*base*/
    0x3a,0x2a,0xbe,0xa6,0x7e,0x46,0xfe,0x84,0x75,0x4f,0x6f,0x5f,0xfb,0x9d,0x5e,0x7a /*top*/
};

//to tell what tiles cause collision
uint8_t collision_detection[]={
    0x04,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e
};
UINT8 joypadPrevious;
// current and old positions of the camera in pixels
uint16_t camera_x, camera_y, old_camera_x, old_camera_y;
// current and old position of the map in tiles
uint8_t map_pos_x, map_pos_y, old_map_pos_x, old_map_pos_y;
// redraw flag, indicates that camera position was changed
uint8_t redraw;
//  gives the amount of sprites on screen (minus the two dedicated to the player) //
uint8_t sprite_num=0;
/* for checking random shit (better than malloc at least)*/
int mptr=0;

/*replacement for the floor function in divisions*/
int floor(int x, int div) {
    int i=0;
    while(i*div<x){
        i++;
    }
    i--;
    return i;
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
void (*evf[256])();
uint8_t evc=0;

/*player x and y coordinates + direction and momentum and stamina and health*/
int16_t plx=83;
int16_t ply=71;
int8_t plxs=0;
int8_t plys=0;
int8_t plright=0;
int8_t pldown=1;
uint8_t stamina=144;
uint8_t health=144;
//for the tile in front of the player
uint16_t front_tile=0;

/*checks  if the player can move*/
uint8_t can_move=1;

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

/*Used to set event, enter the number of frames you want to wait, followed by the function*/
void event(uint8_t frame,  void (*func)()) {
    if (frc>255-frame) ev[evc]=frc-(255-frame);
    else ev[evc]=frc+frame;
    evf[evc]=func;
    if (evc==255) evc=0;
    else evc++;
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
    if ((plright==-1) && (pldown==-1)) move_sprite(1,84-8,80);
    else if ((plright==0) && (pldown==-1)) move_sprite(1,84-8,80-8);
    else if ((plright==1) && (pldown==-1)) move_sprite(1,84,80-8);
    else if ((plright==1) && (pldown==0)) move_sprite(1,84+8,80-8);
    else if ((plright==1) && (pldown==1)) move_sprite(1,84+8,80);
    else if ((plright==0) && (pldown==1)) move_sprite(1,84+8,80+8);
    else if ((plright==-1) && (pldown==1)) move_sprite(1,84,80+8);
    else if ((plright==-1) && (pldown==0)) move_sprite(1,84-8,80+8);
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
                if ((plright==-1) && (pldown==-1)) move_sprite(1,84,80-8);
                else if ((plright==0) && (pldown==-1)) move_sprite(1,84+8,80-8);
                else if ((plright==1) && (pldown==-1)) move_sprite(1,84+8,80);
                else if ((plright==1) && (pldown==0)) move_sprite(1,84+8,80+8);
                else if ((plright==1) && (pldown==1)) move_sprite(1,84,80+8);
                else if ((plright==0) && (pldown==1)) move_sprite(1,84-8,80+8);
                else if ((plright==-1) && (pldown==1)) move_sprite(1,84-8,80);
                else if ((plright==-1) && (pldown==0)) move_sprite(1,84-8,80-8);
    set_sprite_tile(0,0);
}

void roll_1() {
    can_move=1, plys=0, plxs=0, set_sprite_tile(0,0);
}









void main(void) {
    DISPLAY_OFF;
    SHOW_BKG;
    set_bkg_data(0, 15u, map_tiles);

    map_pos_x = map_pos_y = 0; 
    old_map_pos_x = old_map_pos_y = 255;
    set_bkg_submap(map_pos_x, map_pos_y, 20, 18, map_map, map_mapWidth);
    DISPLAY_ON;
    
    camera_x = camera_y = 0;
    old_camera_x = camera_x; old_camera_y = camera_y;

    redraw = FALSE;

    SCX_REG = camera_x; SCY_REG = camera_y; 


    BGP_REG = OBP0_REG = OBP1_REG = 0xE4;
    set_sprite_data(0, 12u, sprite_data);
    set_sprite_tile(0,0);
    set_sprite_tile(1,1);
    move_sprite(0,84, 80);
    move_sprite(1,160,160);

    for (int i=0; i<3; i++){
        set_sprite_tile(2*i+2,i+4);
        set_sprite_tile(2*i+3,i+4);
        set_sprite_prop(2*i+3,32);
        move_sprite(2*i+2, 72, 8*i+16);
        move_sprite(2*i+3, 96, 8*i+16);
    }
    for (int i=0; i<3; i++){
        set_sprite_tile(2*(i+3)+2,i+3+4);
        set_sprite_tile(2*(i+3)+3,i+3+4);
        set_sprite_prop(2*(i+3)+3,32);
        move_sprite(2*(i+3)+2, 80, 8*i+20);
        move_sprite(2*(i+3)+3, 88, 8*i+20);
    }
    set_sprite_tile(14,10);
    set_sprite_tile(15,11);
    move_sprite(14,84, 104);
    move_sprite(15,84,96);
    sprite_num=14;
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
                if (joypad() & J_LEFT)plx--, plright=-1, set_sprite_prop(0,32), camera_x--, redraw = TRUE;
                else if (joypad() & J_RIGHT)plx++, plright=1, set_sprite_prop(0,0), camera_x++, redraw = TRUE;
                else if ((joypad() & J_UP)||(joypad() & J_DOWN)) plright=0;
                if (joypad() & J_UP)ply--,pldown=-1, camera_y--, redraw = TRUE;
                else if (joypad() & J_DOWN)ply++,pldown=1, camera_y++, redraw = TRUE;
                else if ((joypad() & J_LEFT)||(joypad() & J_RIGHT)) pldown=0;
            if ((joypad() & J_A) && (stamina>50)){
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
            if ((joypad() & J_B) && (stamina>50)) {
            mstamina(50);
            plxs=2*plright;
            plys=2*pldown;
            redraw=TRUE;
            can_move=0;
            event(10, roll_1);
            set_sprite_tile(0,2);
            }
            }

            if ((plxs)||(plys)) redraw=TRUE;
            camera_x+=plxs, camera_y+=plys, plx+=plxs, ply+=plys;
            if (frc==255) frc=1;
            else frc+=1;
            if (stamina<144) stamina++/*, plot(stamina+8, 5, DKGREY, SOLID)*/;
            for (volatile int i=0; i<255; i++) {
                if (ev[i]==frc) (*evf[i])(), ev[i]=0;
            }
        if (redraw) {
            mptr=0;
            front_tile=map_mapPLN0[floor(plx+4*plright-3,8)+map_mapWidth*floor(ply+4*pldown-3,8)];
            for (int i=0;i<sizeof collision_detection / sizeof collision_detection[0];i++){
            if (front_tile==collision_detection[i]){
                mptr=1;
                break;
            }
            }
            if (mptr==0) {        
            for (int i=0; i<sprite_num; i++){
                scroll_sprite(i+2, old_camera_x-camera_x,old_camera_y-camera_y);
            }
            
            }
            else plx-=plright,ply-=pldown, camera_x-=plright, camera_y-=pldown;
            redraw = FALSE;
            set_camera();
        }
        wait_vbl_done();
        joypadPrevious=joypad();
    }
}