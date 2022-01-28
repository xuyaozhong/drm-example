#include "font2img.h"
#include "font.h"

extern struct bitmap_font font;

char* debug_char[] = {
" ________ ",
" _______X ",
" ______X_ ",
" ______XX ",
" _____X__ ",
" _____X_X ",
" _____XX_ ",
" _____XXX ",
" ____X___ ",
" ____X__X ",
" ____X_X_ ",
" ____X_XX ",
" ____XX__ ",
" ____XX_X ",
" ____XXX_ ",
" ____XXXX ",
" ___X____ ",
" ___X___X ",
" ___X__X_ ",
" ___X__XX ",
" ___X_X__ ",
" ___X_X_X ",
" ___X_XX_ ",
" ___X_XXX ",
" ___XX___ ",
" ___XX__X ",
" ___XX_X_ ",
" ___XX_XX ",
" ___XXX__ ",
" ___XXX_X ",
" ___XXXX_ ",
" ___XXXXX ",
" __X_____ ",
" __X____X ",
" __X___X_ ",
" __X___XX ",
" __X__X__ ",
" __X__X_X ",
" __X__XX_ ",
" __X__XXX ",
" __X_X___ ",
" __X_X__X ",
" __X_X_X_ ",
" __X_X_XX ",
" __X_XX__ ",
" __X_XX_X ",
" __X_XXX_ ",
" __X_XXXX ",
" __XX____ ",
" __XX___X ",
" __XX__X_ ",
" __XX__XX ",
" __XX_X__ ",
" __XX_X_X ",
" __XX_XX_ ",
" __XX_XXX ",
" __XXX___ ",
" __XXX__X ",
" __XXX_X_ ",
" __XXX_XX ",
" __XXXX__ ",
" __XXXX_X ",
" __XXXXX_ ",
" __XXXXXX ",
" _X______ ",
" _X_____X ",
" _X____X_ ",
" _X____XX ",
" _X___X__ ",
" _X___X_X ",
" _X___XX_ ",
" _X___XXX ",
" _X__X___ ",
" _X__X__X ",
" _X__X_X_ ",
" _X__X_XX ",
" _X__XX__ ",
" _X__XX_X ",
" _X__XXX_ ",
" _X__XXXX ",
" _X_X____ ",
" _X_X___X ",
" _X_X__X_ ",
" _X_X__XX ",
" _X_X_X__ ",
" _X_X_X_X ",
" _X_X_XX_ ",
" _X_X_XXX ",
" _X_XX___ ",
" _X_XX__X ",
" _X_XX_X_ ",
" _X_XX_XX ",
" _X_XXX__ ",
" _X_XXX_X ",
" _X_XXXX_ ",
" _X_XXXXX ",
" _XX_____ ",
" _XX____X ",
" _XX___X_ ",
" _XX___XX ",
" _XX__X__ ",
" _XX__X_X ",
" _XX__XX_ ",
" _XX__XXX ",
" _XX_X___ ",
" _XX_X__X ",
" _XX_X_X_ ",
" _XX_X_XX ",
" _XX_XX__ ",
" _XX_XX_X ",
" _XX_XXX_ ",
" _XX_XXXX ",
" _XXX____ ",
" _XXX___X ",
" _XXX__X_ ",
" _XXX__XX ",
" _XXX_X__ ",
" _XXX_X_X ",
" _XXX_XX_ ",
" _XXX_XXX ",
" _XXXX___ ",
" _XXXX__X ",
" _XXXX_X_ ",
" _XXXX_XX ",
" _XXXXX__ ",
" _XXXXX_X ",
" _XXXXXX_ ",
" _XXXXXXX ",
" X_______ ",
" X______X ",
" X_____X_ ",
" X_____XX ",
" X____X__ ",
" X____X_X ",
" X____XX_ ",
" X____XXX ",
" X___X___ ",
" X___X__X ",
" X___X_X_ ",
" X___X_XX ",
" X___XX__ ",
" X___XX_X ",
" X___XXX_ ",
" X___XXXX ",
" X__X____ ",
" X__X___X ",
" X__X__X_ ",
" X__X__XX ",
" X__X_X__ ",
" X__X_X_X ",
" X__X_XX_ ",
" X__X_XXX ",
" X__XX___ ",
" X__XX__X ",
" X__XX_X_ ",
" X__XX_XX ",
" X__XXX__ ",
" X__XXX_X ",
" X__XXXX_ ",
" X__XXXXX ",
" X_X_____ ",
" X_X____X ",
" X_X___X_ ",
" X_X___XX ",
" X_X__X__ ",
" X_X__X_X ",
" X_X__XX_ ",
" X_X__XXX ",
" X_X_X___ ",
" X_X_X__X ",
" X_X_X_X_ ",
" X_X_X_XX ",
" X_X_XX__ ",
" X_X_XX_X ",
" X_X_XXX_ ",
" X_X_XXXX ",
" X_XX____ ",
" X_XX___X ",
" X_XX__X_ ",
" X_XX__XX ",
" X_XX_X__ ",
" X_XX_X_X ",
" X_XX_XX_ ",
" X_XX_XXX ",
" X_XXX___ ",
" X_XXX__X ",
" X_XXX_X_ ",
" X_XXX_XX ",
" X_XXXX__ ",
" X_XXXX_X ",
" X_XXXXX_ ",
" X_XXXXXX ",
" XX______ ",
" XX_____X ",
" XX____X_ ",
" XX____XX ",
" XX___X__ ",
" XX___X_X ",
" XX___XX_ ",
" XX___XXX ",
" XX__X___ ",
" XX__X__X ",
" XX__X_X_ ",
" XX__X_XX ",
" XX__XX__ ",
" XX__XX_X ",
" XX__XXX_ ",
" XX__XXXX ",
" XX_X____ ",
" XX_X___X ",
" XX_X__X_ ",
" XX_X__XX ",
" XX_X_X__ ",
" XX_X_X_X ",
" XX_X_XX_ ",
" XX_X_XXX ",
" XX_XX___ ",
" XX_XX__X ",
" XX_XX_X_ ",
" XX_XX_XX ",
" XX_XXX__ ",
" XX_XXX_X ",
" XX_XXXX_ ",
" XX_XXXXX ",
" XXX_____ ",
" XXX____X ",
" XXX___X_ ",
" XXX___XX ",
" XXX__X__ ",
" XXX__X_X ",
" XXX__XX_ ",
" XXX__XXX ",
" XXX_X___ ",
" XXX_X__X ",
" XXX_X_X_ ",
" XXX_X_XX ",
" XXX_XX__ ",
" XXX_XX_X ",
" XXX_XXX_ ",
" XXX_XXXX ",
" XXXX____ ",
" XXXX___X ",
" XXXX__X_ ",
" XXXX__XX ",
" XXXX_X__ ",
" XXXX_X_X ",
" XXXX_XX_ ",
" XXXX_XXX ",
" XXXXX___ ",
" XXXXX__X ",
" XXXXX_X_ ",
" XXXXX_XX ",
" XXXXXX__ ",
" XXXXXX_X ",
" XXXXXXX_ ",
" XXXXXXXX ",
};



int rendercharonscreen32(uint8_t* screen, int screenw, int screenh, char c, int x_l, int y_top, uint32_t color, int scale)
{
	unsigned w,h;
	uint32_t topleft, hoffset,scaleoffset;

        if(c < 32 || c > 126)
                return -1;
        w = font.Widths[c];
        h = font.Height;

//	printf("w = %d,h = %d\n",w, h);
	
        unsigned char *bitmap = font.Bitmap + c * font.Height;


//	for(int i = 0; i < font.Height ; i++)
//	{
//		printf("%s\n", debug_char[bitmap[i]]);
//	}

	topleft =  y_top * screenw * 4 + x_l * 4;
	
        int j = 0, k = 0;
        for(j = 0; j < font.Height; j++)
        {
//		printf("%s ", debug_char[bitmap[j]]);
                for(k = 0; k < font.Width; k++)
                {
			
                        if( bitmap[j] & (0x80 >> k))
                        {
		
				hoffset = topleft + k * 4 * scale; // start point (top,left) + scale offset in bitmap font 
				for(int tmpsv = 0; tmpsv <  scale ; tmpsv++) // vertical scale
				{
					scaleoffset = hoffset + ( screenw * 4 ) * tmpsv;
					for(int tmpsh = 0; tmpsh <  scale ; tmpsh++) //horizontal scale
					{
                                		*(uint32_t*)&screen[scaleoffset + tmpsh * 4] = color;
					}
				}
                        }
                }
		topleft += screenw * 4 * scale;
//		printf("\n");
        }
	return 0;
}
