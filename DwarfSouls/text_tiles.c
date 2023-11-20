const unsigned char text_tiles[] =
{
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x3c,0x3c,0x66,0x66,0x6e,0x6e,0x76,0x76,0x66,0x66,0x66,0x66,0x3c,0x3c,//0
0x0c,0x0c,0x1c,0x1c,0x3c,0x3c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,//1
0x00,0x00,0x3c,0x3c,0x66,0x66,0x0e,0x0e,0x1c,0x1c,0x38,0x38,0x70,0x70,0x7e,0x7e,//2
0x00,0x00,0x7e,0x7e,0x0c,0x0c,0x18,0x18,0x3c,0x3c,0x06,0x06,0x46,0x46,0x3c,0x3c,//3
0x00,0x00,0x0c,0x0c,0x1c,0x1c,0x2c,0x2c,0x4c,0x4c,0x7e,0x7e,0x0c,0x0c,0x0c,0x0c,//4
0x00,0x00,0x7e,0x7e,0x60,0x60,0x7c,0x7c,0x06,0x06,0x06,0x06,0x46,0x46,0x3c,0x3c,//5
0x00,0x00,0x1c,0x1c,0x20,0x20,0x60,0x60,0x7c,0x7c,0x66,0x66,0x66,0x66,0x3c,0x3c,//6
0x00,0x00,0x7e,0x7e,0x06,0x06,0x0e,0x0e,0x1c,0x1c,0x18,0x18,0x18,0x18,0x18,0x18,//7
0x00,0x00,0x3c,0x3c,0x66,0x66,0x66,0x66,0x3c,0x3c,0x66,0x66,0x66,0x66,0x3c,0x3c,//8
0x00,0x00,0x3c,0x3c,0x66,0x66,0x66,0x66,0x3e,0x3e,0x06,0x06,0x0c,0x0c,0x38,0x38,//9
};
//2: 0x00,0x00,0x3c,0x3c,0x66,0x66,0x0e,0x0e,0x1c,0x1c,0x38,0x38,0x70,0x70,0x7e,0x7e

//Only the tiles past line 18 (19,20,etc) actually matter as text, the rest is filler. This is because
//of a quirk of how text and ram work in GBDK. Text only gets initialised in RAM the first time
//it is called. Every time it is called after, it is assumed to still be in the same place, intact.
//This can be a problem when you call on different tiles in RAM that overwrite that spot,
//because the text will never come back to replace it, it will go on as if he text was still there.
//To counteract this, when text is called, tiles in RAM are replace so that the 'overwriting' tiles
//are actually in the shape of text, therefore giving the illusion that the text is still there.

//This is annoying as fuck and took me months to understand AND NONE OF IT IS WRITTEN ANYWHERE
//SO PLEASE PLEASE HEAD THIS IF YOU WISH TO MAKE YOUR OWN GAME. THE ACTUAL DOCUMENTATION DOES
//NOT REFLECT HOW THE PROGRAM WORKS. IT IS MYSTICAL AND ANCIENT AND WE ARE FOOLS IF WE THINK WE
//CAN EVER HOPE TO UNDERSTAND IT