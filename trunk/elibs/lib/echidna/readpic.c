/*
 * READPIC.C
 *
 * COPYRIGHT (c) 1992 Echidna.
 * PROGRAMMER : Gregg A. Tavares
 *    VERSION : 00.000
 *    CREATED : 09/30/94
 *   MODIFIED : 12/22/94
 *       TABS : 05 09
 *
 *	     \|///-_
 *	     \oo///_
 *	-----w/-w------
 *	 E C H I D N A
 *	---------------
 *
 * DESCRIPTION
 *		
 *
 * HISTORY
 *
*/

#include "platform.h"
#include "switches.h"
#include "echidna/ensure.h"

#include "echidna/readgfx.h"
#include "echidna/memfile.h"
#include "echidna/checkglu.h"
#include "echidna/eerrors.h"
#include "readpic.h"

/**************************** C O N S T A N T S ***************************/

#define PIC_UNCOMPRESSED     0x00
#define PIC_MIXED_RUN_LENGTH 0x02

#define PIC_CHANNEL_RED_BIT   0x80
#define PIC_CHANNEL_GREEN_BIT 0x40
#define PIC_CHANNEL_BLUE_BIT  0x20
#define PIC_CHANNEL_ALPHA_BIT 0x10

/******************************** T Y P E S *******************************/

typedef struct PICHeader
{
  int32 magic;
  int32 version;
  char  comment[80];
  char  id[4];
  int16 width;
  int16 height;
  int32 ratio;
  int16 fields;
  int16 pad;
} PICHeader;

typedef struct PICChannel
{
  int8 chained;
  int8 size;
  int8 type;
  int8 channel;
}
PICChannel;

int loadPIC32Bit(BlockO32BitPixels* blockPtr, MEMFILE* mf)
{
    PICHeader   phead;
    PICChannel  channel[4];
    int         i;
	int			alphaChannel = 0;

    if (MEMFILE_Read(mf, &phead, sizeof( phead )) != sizeof (phead))
    {
		SetGlobalErr (ERR_GENERIC);
		GEcatf ("Invalid PIC file");
		goto cleanup;
    }

    phead.magic   = MSBFToNative32Bit (phead.magic);
    phead.version = MSBFToNative32Bit (phead.version);
    phead.width   = MSBFToNative16Bit (phead.width);
    phead.height  = MSBFToNative16Bit (phead.height);
    phead.ratio   = MSBFToNative32Bit (phead.ratio);
    phead.fields  = MSBFToNative16Bit (phead.fields);

    for(i = 0; i < 4; i++)
    {
        if ( MEMFILE_Read(mf, &channel[i], sizeof(PICChannel)) !=  sizeof (PICChannel))
        {
			SetGlobalErr (ERR_GENERIC);
    		GEcatf ("Invalid PIC file (2)");
    		goto cleanup;
        }

        #if 0
    		EL_printf("\n");
            EL_printf("\t\tchannel[%d].chained = %d\n", i, channel[i].chained);
            EL_printf("\t\tchannel[%d].size    = %d\n", i, channel[i].size);
            EL_printf("\t\tchannel[%d].type    = %d\n", i, channel[i].type);
            EL_printf("\t\tchannel[%d].channel = %d\n", i, channel[i].channel);
    		EL_printf("\n");
        #endif

        if (channel[i].channel & PIC_CHANNEL_ALPHA_BIT)
		{
			blockPtr->channels = 1;
        }
		
        if (!channel[i].chained)
        {
            break;
        }
    }

    if (i == 4)
    {
		SetGlobalErr (ERR_GENERIC);
        GEcatf ("Too Many Channels");
        goto cleanup;
    }

    {
        long bufferWidth  = phead.width;
        long bufferHeight = phead.height;
        long bufferSize;
		int  c;
		int  y;
        pixel32* pEOB;

        if((bufferWidth <= 0 ) || (bufferHeight <= 0 ))
        {
			SetGlobalErr (ERR_GENERIC);
      		GEcatf ("Invalid PIC file (3), bad size");
            goto cleanup;
        }

      	bufferSize = bufferWidth * bufferHeight * sizeof (pixel32);

        blockPtr->rgba   = (pixel32 *) malloc (bufferSize);
        blockPtr->width  = bufferWidth;
        blockPtr->height = bufferHeight;
		
        if (!blockPtr->rgba)
        {
            SetGlobalErr (ERR_GENERIC);
            GEcatf ("Out of Memory loading tga");
            goto cleanup;
        }
		
		memset (blockPtr->rgba, 255, bufferSize);

        pEOB = blockPtr->rgba + bufferWidth * bufferHeight;

        for(y = 0; y < bufferHeight; y++)
        {
            for(c = 0; c < i + 1; c++)
            {
                pixel32*  pPixel;
                uint8     channels = channel[c].channel;

        	    pPixel = blockPtr->rgba + y * bufferWidth;

                if (channels == 0)
                {
					SetGlobalErr (ERR_GENERIC);
                    GEcatf("bad channel flags\n");
                    goto cleanup;
                }

                switch(channel[c].type)
                {
                case PIC_UNCOMPRESSED:
                    {
                        long x;

                        for(x = 0; x < bufferWidth; x++)
                        {
                            if (channels & PIC_CHANNEL_RED_BIT)   { pPixel->red   = MEMFILE_getc(mf); }
                            if (channels & PIC_CHANNEL_GREEN_BIT) { pPixel->green = MEMFILE_getc(mf); }
                            if (channels & PIC_CHANNEL_BLUE_BIT)  { pPixel->blue  = MEMFILE_getc(mf); }
                            if (channels & PIC_CHANNEL_ALPHA_BIT) { pPixel->alpha = MEMFILE_getc(mf); }
                            pPixel++;
                        }
                    }
                    break;
                case PIC_MIXED_RUN_LENGTH:
                    {
                        long x = 0;

                        while (x < bufferWidth)
                        {
                            long count = MEMFILE_getc(mf);

                            if(count > 128)
                            {
                                uint8 r,g,b,a;

                                if (channels & PIC_CHANNEL_RED_BIT)   { r = MEMFILE_getc(mf); }
                                if (channels & PIC_CHANNEL_GREEN_BIT) { g = MEMFILE_getc(mf); }
                                if (channels & PIC_CHANNEL_BLUE_BIT)  { b = MEMFILE_getc(mf); }
                                if (channels & PIC_CHANNEL_ALPHA_BIT) { a = MEMFILE_getc(mf); }

                                count -= 127;

                                if (pPixel + count > pEOB)
                                {
									SetGlobalErr (ERR_GENERIC);
                                    GEcatf ("error reading file");
                                    goto cleanup;
                                }

                                while (count > 0)
                                {
                                    if (channels & PIC_CHANNEL_RED_BIT)   { pPixel->red   = r; }
                                    if (channels & PIC_CHANNEL_GREEN_BIT) { pPixel->green = g; }
                                    if (channels & PIC_CHANNEL_BLUE_BIT)  { pPixel->blue  = b; }
                                    if (channels & PIC_CHANNEL_ALPHA_BIT) { pPixel->alpha = a; }

                                    pPixel++;
                                    x++;
                                    count--;
                                }
                            }
                            else if (count == 128)
                            {
                                uint8 r,g,b,a;

                                count  = MEMFILE_getc(mf) * 256;
                                count += MEMFILE_getc(mf);

                                if (channels & PIC_CHANNEL_RED_BIT)   { r = MEMFILE_getc(mf); }
                                if (channels & PIC_CHANNEL_GREEN_BIT) { g = MEMFILE_getc(mf); }
                                if (channels & PIC_CHANNEL_BLUE_BIT)  { b = MEMFILE_getc(mf); }
                                if (channels & PIC_CHANNEL_ALPHA_BIT) { a = MEMFILE_getc(mf); }

                                if (pPixel + count > pEOB)
                                {
									SetGlobalErr (ERR_GENERIC);
                                    GEcatf ("error reading file (2)");
                                    goto cleanup;
                                }

                                while (count > 0)
                                {
                                    if (channels & PIC_CHANNEL_RED_BIT)   { pPixel->red   = r; }
                                    if (channels & PIC_CHANNEL_GREEN_BIT) { pPixel->green = g; }
                                    if (channels & PIC_CHANNEL_BLUE_BIT)  { pPixel->blue  = b; }
                                    if (channels & PIC_CHANNEL_ALPHA_BIT) { pPixel->alpha = a; }

                                    pPixel++;
                                    x++;
                                    count--;
                                }
                            }
                            else /* if (count < 128) */
                            {
                                count++;

                                if (pPixel + count > pEOB)
                                {
									SetGlobalErr (ERR_GENERIC);
                                    GEcatf ("error reading file (3)");
                                    goto cleanup;
                                }

                                while (count > 0)
                                {
                                    if (channels & PIC_CHANNEL_RED_BIT)   { pPixel->red   = MEMFILE_getc(mf); }
                                    if (channels & PIC_CHANNEL_GREEN_BIT) { pPixel->green = MEMFILE_getc(mf); }
                                    if (channels & PIC_CHANNEL_BLUE_BIT)  { pPixel->blue  = MEMFILE_getc(mf); }
                                    if (channels & PIC_CHANNEL_ALPHA_BIT) { pPixel->alpha = MEMFILE_getc(mf); }

                                    pPixel++;
                                    x++;
                                    count--;
                                }
                            }
                        }
                    }
                    break;
                default:
					SetGlobalErr (ERR_GENERIC);
                    GEcatf ("error reading file (3)");
                    goto cleanup;
                }
            }
        }
    }

    return TRUE;

cleanup:
    if (blockPtr->rgba)
    {
        free (blockPtr->rgba);
    }

    return FALSE;
}

