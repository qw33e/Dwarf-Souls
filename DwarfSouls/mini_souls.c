#include <gb/gb.h>
#include <stdint.h>
#include <stdio.h>

#include "map_map.h"
#include "map_tiles.h"

uint8_t sprite_data[] = {
    0x00,0x3c,0x3c,0x7d,0x3c,0x15,0x34,0x09,0xa5,0x7e,0x3c,0x01,0x00,0x3c,0x24,0x24, /*player model */
    0x3c,0x3c,0x42,0x7e,0xbd,0xc3,0x05,0x03,0x8d,0xc3,0x19,0x07,0x06,0x3e,0x18,0x78  /*attack particle*/
};
UINT8 joypadPrevious;

joypads_t joypads;

/*frame counter, useful for animations and shit. Ev stores the frames where we want to call a function (an event happens), which will redirect to evf
evc just tells you where you are on the writing to ev*/
uint8_t frc=1;
uint8_t ev[256];
void (*evf[256])();
uint8_t evc=0;

/*player x and y coordinates + direction*/
int16_t plx=64;
int16_t ply=64;
int8_t plright=0;
int8_t pldown=0;

/*checks  if the player can move*/
uint8_t can_move=1;

void event(uint8_t frame,  void (*func)()) {
    if (frc>255-frame) ev[evc]=frc-(255-frame);
    else ev[evc]=frc+frame;
    evf[evc]=func;
    if (frc==255) frc=0;
    else frc++;
}






/*animations block, please label carefully*/
void animation_1() {
    can_move=1;
}









void main(void) {
    DISPLAY_ON;
    SHOW_BKG;
    /*set_bkg_data(0, 24u, map_tiles);
    set_bkg_tiles(0,0,20,18,map_map);*/
    BGP_REG = OBP0_REG = OBP1_REG = 0xE4;
    set_sprite_data(0, 2, sprite_data);
    set_sprite_tile(0,0);
    set_sprite_tile(1,1);
    move_sprite(0, (0 << 3) + 64, 64);
    move_sprite(1, (0 << 3) + 64, 64);
    SHOW_SPRITES;

    NR52_REG = 0x80; // is 1000 0000 in binary and turns on sound
    NR50_REG = 0x77; // sets the volume for both left and right channel just set to max 0x77
    NR51_REG = 0xFF; // is 1111 1111 in binary, select which chanels we want to use in this case all of them. One bit for the L one bit for the R of all four channels
    
    while(1) {
            if (can_move==1)  {
            if ((joypad() & J_LEFT) && (joypad() & J_UP)) scroll_sprite(0, -1.5, -1.5), plx--, ply--, plright=-1, pldown=-1, set_sprite_prop(0,32);
            else if ((joypad() & J_LEFT) && (joypad() & J_DOWN)) scroll_sprite(0, -1.5, 1.5), plx--, ply++, plright=-1, pldown=1, set_sprite_prop(0,32);
            else if ((joypad() & J_RIGHT) && (joypad() & J_UP)) scroll_sprite(0, 1.5, -1.5), plx++, ply--, plright=1, pldown=-1, set_sprite_prop(0,0);
            else if ((joypad() & J_RIGHT) && (joypad() & J_DOWN)) scroll_sprite(0, 1.5, 1.5), plx++, ply++, plright=1, pldown=1, set_sprite_prop(0,0);
            else if (joypad() & J_LEFT) scroll_sprite(0, -1.5, 0), plx--, plright=-1, pldown=0, set_sprite_prop(0,32);
            else if (joypad() & J_RIGHT) scroll_sprite(0, 1.5, 0), plx++, plright=1, pldown=0, set_sprite_prop(0,0);
            else if (joypad() & J_UP) scroll_sprite(0, 0, -1.5), ply--, plright=0, pldown=-1;
            else if (joypad() & J_DOWN) scroll_sprite(0, 0, 1.5), ply++, plright=0, pldown=1;
            if (joypad() & J_A){
                NR10_REG=0X00;
                NR11_REG=0X50;
                NR12_REG=0X43;
                NR13_REG=0X73;
                NR14_REG=0X86;
                event(30, animation_1);
                move_sprite(1, (0 << 3) + plx+8*plright, (0 << 3) + ply+8*pldown);
                can_move=0;
            }
            }

            if (frc==255) frc=1;
            else frc+=1;
            for (volatile int i=0; i<255; i++) {
                if (ev[i]==frc) (*evf[i])(), ev[i]=0;

            }
        joypadPrevious=joypad();
        wait_vbl_done();
    }
}