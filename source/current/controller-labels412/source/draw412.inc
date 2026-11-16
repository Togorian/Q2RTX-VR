// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
// SPDX-License-Identifier: GPL-2.0-or-later
// Four private one-cell markers are intercepted here, never sent to the font.
// Raster strokes use existing UI coordinates and the ordinary fill primitive.
static void b2bm_draw412(int x,int y,int flags,const char *text,uint32_t colour)
{
    static const unsigned char rows[4][7]={
        {65,34,20,8,20,34,65},      // Cross
        {28,34,65,65,65,34,28},     // Circle
        {127,65,65,65,65,65,127},   // Square
        {8,20,20,34,34,65,127}     // Triangle
    };
    char plain[160];size_t n=strlen(text);
    if(n>=sizeof(plain)){UI_DrawString(x,y,flags,text);return;}
    bool shapes=false;
    for(size_t i=0;i<=n;i++){
        unsigned c=(unsigned char)text[i];
        bool shape=c>=1 && c<=4;shapes|=shape;plain[i]=shape?' ':text[i];
    }
    UI_DrawString(x,y,flags,plain);
    if(!shapes)return;
    if((flags&UI_CENTER)==UI_CENTER)x-=(int)n*CHAR_WIDTH/2;
    else if(flags&UI_RIGHT)x-=(int)n*CHAR_WIDTH;
    for(size_t i=0;i<n;i++){
        unsigned c=(unsigned char)text[i];if(c<1 || c>4)continue;
        for(int yy=0;yy<7;yy++)for(int xx=0;xx<7;xx++)
            if(rows[c-1][yy]&(1u<<xx))R_DrawFill32(x+(int)i*CHAR_WIDTH+xx,y+yy,1,1,colour);
    }
}
