/*
 * PHOTOSHP.C
 *
 *  COPYRIGHT : 1994 Echidna.
 * PROGRAMMER : Gregg A. Tavares
 *    VERSION : 00.000
 *    CREATED : 09/13/94
 *   MODIFIED : 09/15/94
 *       TABS : 05 09
 *
 *       \|///-_
 *       \oO///_
 *  -----w/-w------
 *   E C H I D N A
 *  ---------------
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

#include "echidna/eerrors.h"
#include "echidna/readgfx.h"
#include "echidna/memfile.h"
#include "photoshp.h"

/**************************** C O N S T A N T S ***************************/


/******************************** T Y P E S *******************************/

typedef struct
{
    uint32  signature;
    uint16  version;
    uint8   reserved[6];
    uint16  channels;
    uint32  rows;
    uint32  columns;
    uint16  depth;
    uint16  mode;
}
PSHeader;

typedef struct
{
    int32  top;
    int32  left;
    int32  bottom;
    int32  right;
    uint16  channels;
}
PSLayerInfo;

typedef struct
{
    uint8  top[4];
    uint8  left[4];
    uint8  bottom[4];
    uint8  right[4];
    uint8  channels[2];
}
PSLayerInfoRead;

/* -- blendmodes
norm' = normal
dark' = darken
lite' = lighten
hue ' = hue
sat ' = saturation
colr' = color
lum ' = luminosity
mul ' = multiply
scrn' = screen
diss' = dissolve
over' = overlay
hLit' = hard light
sLit' = soft light
diff' = difference
smud' = exlusion
div ' = color dodge
idiv' = color burn
*/

typedef struct
{
    uint32  signature;
    uint32  blendmode;  //
    uint8   opacity;    // 0 - 255 (255 = 100%)
    uint8   clipping;   // 0 = base, 1= non-base?
    uint8   flags;      // bit 0 = transparency protected
                        // bit 1 = visible
                        // bit 2 = obsolete
                        // bit 3 = 1 for Photoshop 5.0 and later, tells if
                        // bit 4 has useful information
                        // bit 4 = pixel data irrelevant to appearance of document
    uint8   filler;
    uint32  extraDataLength;
}
PSLayerMore;

typedef struct
{
    int32   top;
    int32   left;
    int32   right;
    int32   bottom;
    uint8   defaultColor;
    uint8   flags;  // bit 0 = position relative to layer
                    // bit 1 = layer mask disabled
                    // bit 2 = invert layer mask when blending
    uint8   padding[2];
}
PSLayerMaskData;

typedef struct
{
    int16   channelID;  // 0 = red, 1 = green, etc..  -1 = transparency, -2 = user supplied mask
    uint32  channelDataLength;
}
PSChannelLengthInfo;

typedef struct
{
    uint8   channelID[2];  // 0 = red, 1 = green, etc..  -1 = transparency, -2 = user supplied mask
    uint8   channelDataLength[4];
}
PSChannelLengthInfoRead;

typedef struct
{
    uint8   source[4];
    uint8   dest[4];
}
PSChannelCSDR;

typedef struct
{
    PSChannelLengthInfo     cli;
    PSChannelCSDR           csdr;
    uint8*                  pData;
}
PSChannelInfo;

typedef struct
{
    uint32  size;
    uint8   source[4];
    uint8   dest[4];
}
PSLayerGrayInfo;

typedef struct
{
    PSLayerInfo     li;
    PSChannelInfo*  pChannelInfo;
    PSLayerMore     lm;
    PSLayerMaskData mask;
    PSLayerGrayInfo gray;
    char*           pName;
}
PSLayerInfoHolder;

/****************************** G L O B A L S *****************************/


/******************************* M A C R O S ******************************/


/***************************** R O U T I N E S ****************************/

static short unpack (uint8 *buffer, MEMFILE *mf, long width, long lines, long mod)
{
    char     i;
    short    j;
    long     bytecount;
    long     d;
    long     count;
    long     line;

    for (line = 0; line < lines; line++)
    {
        bytecount = 0;
        while (bytecount < width)
        {
        // read a char & check for eof

            d = MEMFILE_getc(mf);
            if (d == EOF)
            {
                return(1);
            }
            i = (char)d;

            if(i < 0)
            {
            // run data

                count = -i + 1;

                d = MEMFILE_getc(mf);
                if(d == EOF)
                {
                    return(1);
                }

                for(j = 0; j < count; j++)
                {
                    *buffer = (uint8)d;
                    buffer += mod;
                }
            }
            else
            {
            // dump data

                count = i + 1;

                for (j = 0; j < count; j++)
                {
                    d = MEMFILE_getc(mf);
                    if(d == EOF)
                    {
                        return(1);
                    }
                    *buffer = (uint8)d;
                    buffer += mod;
                }
            }

            bytecount += count;
        }
    }
    return 0;
}
// unpack

/*********************************************************************
 *
 * loadPhotoshop32Bit
 *
 * SYNOPSIS
 *      void  loadPhotoshop32Bit (BlockO24BitPixels *blockPtr, char *fileName)
 *
 * PURPOSE
 *      Attempts to load the specified photoshop 2.5 file.
 *
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
int loadPhotoshop32Bit (BlockO32BitPixels *blockPtr, MEMFILE *mf)
{
    int          success = FALSE;
    uint8        header[4+2+6+2+4+4+2+2];
    uint8       **channelBufs = NULL;
    short        r;
    short        i;
    short        line;
    long         width;
    long         height;
    long         channels;
    long         dsize;
    PSHeader     psh;
    PSLayerInfoHolder *pLayerInfos = NULL;
    short        numLayers;
    int          bHasMergedAlpha = FALSE;
    uint8*       pTrans = NULL;

// read the header

    if (MEMFILE_Read(mf, &header, sizeof(header)) != sizeof(header))
    {
        SetGlobalErr (ERR_GENERIC);
        GEcatf ("loadPhotoshop:Bad Header");
        goto cleanup;
    }

    psh.signature = ((uint32)header[ 0] << 24) | ((uint32)header[ 1] << 16) | ((uint32)header[ 2] << 8) | ((uint32)header[ 3]);
    psh.version   = ((uint16)header[ 4] <<  8) | ((uint16)header[ 5]);
    psh.channels  = ((uint16)header[12] <<  8) | ((uint16)header[13]);
    psh.rows      = ((uint32)header[14] << 24) | ((uint32)header[15] << 16) | ((uint32)header[16] << 8) | ((uint32)header[17]);
    psh.columns   = ((uint32)header[18] << 24) | ((uint32)header[19] << 16) | ((uint32)header[20] << 8) | ((uint32)header[21]);
    psh.depth     = ((uint16)header[22] <<  8) | ((uint16)header[23]);
    psh.mode      = ((uint16)header[24] <<  8) | ((uint16)header[25]);

// Verify the format

    if (psh.signature != 0x38425053)
    {
        SetGlobalErr (ERR_GENERIC);
        GEcatf1 ("loadPhotoshop:Cannot identify signature (%08lx)", psh.signature);
        goto cleanup;
    }
    if (psh.version != 1)
    {
        SetGlobalErr (ERR_GENERIC);
        GEcatf1 ("loadPhotoshop:Bad version number (%d)", psh.version);
        goto cleanup;
    }
    if (psh.channels < 3)
    {
        SetGlobalErr (ERR_GENERIC);
        GEcatf1 ("loadPhotoshop:Wrong number of channels (%d)", psh.channels);
        goto cleanup;
    }
    if (psh.depth != 8)
    {
        SetGlobalErr (ERR_GENERIC);
        GEcatf1 ("loadPhotoshop:Wrong depth (%d)", psh.depth);
        goto cleanup;
    }
    if (psh.mode != 3)
    {
        SetGlobalErr (ERR_GENERIC);
        GEcatf1 ("loadPhotoshop:Wrong Mode (not RGB(%d))", psh.mode);
        goto cleanup;
    }

    // get size of color mode data
    if (MEMFILE_Read(mf, &dsize, 4) != 4)
    {
        SetGlobalErr (ERR_GENERIC);
        GEcatf ("Error reading mode data");
        goto cleanup;
    }
    BigLong2Native (dsize);
    // skip color mode data
    MEMFILE_Seek(mf, dsize, SEEK_CUR);

    // get size of image resources
    if (MEMFILE_Read(mf, &dsize,4) != 4)
    {
        SetGlobalErr (ERR_GENERIC);
        GEcatf ("Error reading resource data");
        goto cleanup;
    }
    BigLong2Native (dsize);
    // skip image rsources
    MEMFILE_Seek(mf, dsize, SEEK_CUR);

    // get size of layers and masks
    if (MEMFILE_Read(mf, &dsize, 4) != 4)
    {
        SetGlobalErr (ERR_GENERIC);
        GEcatf ("Error reading reserved data");
        goto cleanup;
    }
    BigLong2Native (dsize);

    //  MEMFILE_Seek(mf, dsize, SEEK_CUR);

    // check for size of layer section
    if (dsize > 0)
    {
        MEMFILE  layermfX;
        MEMFILE* layermf = &layermfX;
        MEMFILE  extramfX;
        MEMFILE* extramf = &extramfX;
        long layerInfoSize;
        short layerNdx;

        // make a mini memory file for the layer data
        MEMFILE_Init(layermf, mf->curPtr, dsize);

        // skip the layer data in the main file
        MEMFILE_Seek(mf, dsize, SEEK_CUR);

        if (MEMFILE_Read(layermf, &layerInfoSize, 4) != 4)
        {
            SetGlobalErr (ERR_GENERIC);
            GEcatf ("Error reading layerInfoSize");
            goto cleanup;
        }

        BigLong2Native (layerInfoSize);

        if (MEMFILE_Read(layermf, &numLayers, 2) != 2)
        {
            SetGlobalErr (ERR_GENERIC);
            GEcatf ("Error reading numLayers");
            goto cleanup;
        }

        BigWord2Native (numLayers);
        if (numLayers < 0)
        {
            numLayers = -numLayers;
            bHasMergedAlpha = TRUE;
        }

        if (numLayers)
        {
            pLayerInfos = calloc(numLayers, sizeof(PSLayerInfoHolder));
            if (!pLayerInfos)
            {
                SetGlobalErr (ERR_GENERIC);
                GEcatf ("Error allocating Layer infos");
                goto cleanup;
            }

            for (layerNdx = 0; layerNdx < numLayers; ++layerNdx)
            {
                PSLayerInfoRead lir;
                int ch;
                long layerMaskDataSize;

                if (MEMFILE_Read(layermf, &lir, sizeof (PSLayerInfoRead)) != sizeof(PSLayerInfoRead))
                {
                    SetGlobalErr (ERR_GENERIC);
                    GEcatf ("Error reading Layer info");
                    goto cleanup;
                }

                #if __LSBFIRST__
                    #define MSBFToNative32Bit4(value) (  \
                       (uint32)value[0] * 256 * 256 * 256 + \
                       (uint32)value[1] * 256 * 256 + \
                       (uint32)value[2] * 256 + \
                       (uint32)value[3] )
                    #define MSBFToNative16Bit2(value) (  \
                       (uint16)value[0] * 256 + \
                       (uint16)value[1] )
                #else
                    #define MSBFToNative32Bit4(value) (  \
                       (uint32)value[3] * 256 * 256 * 256 + \
                       (uint32)value[2] * 256 * 256 + \
                       (uint32)value[1] * 256 + \
                       (uint32)value[0] )
                    #define MSBFToNative16Bit2(value) (  \
                       (uint16)value[1] * 256 + \
                       (uint16)value[0] )
                #endif


                pLayerInfos[layerNdx].li.top      = MSBFToNative32Bit4(lir.top);
                pLayerInfos[layerNdx].li.left     = MSBFToNative32Bit4(lir.left);
                pLayerInfos[layerNdx].li.bottom   = MSBFToNative32Bit4(lir.bottom);
                pLayerInfos[layerNdx].li.right    = MSBFToNative32Bit4(lir.right);
                pLayerInfos[layerNdx].li.channels = MSBFToNative16Bit2(lir.channels);

                pLayerInfos[layerNdx].pChannelInfo = calloc(pLayerInfos[layerNdx].li.channels, sizeof (PSChannelInfo));
                if (!pLayerInfos[layerNdx].pChannelInfo)
                {
                    SetGlobalErr (ERR_GENERIC);
                    GEcatf ("Error allocating channel length infos");
                    goto cleanup;
                }

                for (ch = 0; ch < pLayerInfos[layerNdx].li.channels; ++ch)
                {
                    PSChannelLengthInfoRead clir;

                    if (MEMFILE_Read(layermf, &clir, sizeof (PSChannelLengthInfoRead)) != sizeof(PSChannelLengthInfoRead))
                    {
                        SetGlobalErr (ERR_GENERIC);
                        GEcatf ("Error reading channel length info");
                        goto cleanup;
                    }

                    pLayerInfos[layerNdx].pChannelInfo[ch].cli.channelID         = MSBFToNative16Bit2(clir.channelID);
                    pLayerInfos[layerNdx].pChannelInfo[ch].cli.channelDataLength = MSBFToNative32Bit4(clir.channelDataLength);
                }

                if (MEMFILE_Read(layermf, &pLayerInfos[layerNdx].lm, sizeof (PSLayerMore)) != sizeof(PSLayerMore))
                {
                    SetGlobalErr (ERR_GENERIC);
                    GEcatf ("Error reading Layer more");
                    goto cleanup;
                }

                BigLong2Native(pLayerInfos[layerNdx].lm.signature);
                BigLong2Native(pLayerInfos[layerNdx].lm.blendmode);
                BigLong2Native(pLayerInfos[layerNdx].lm.extraDataLength);

                if (pLayerInfos[layerNdx].lm.extraDataLength == 0)
                {
                    SetGlobalErr (ERR_GENERIC);
                    GEcatf ("Error layer extra data length is zero");
                    goto cleanup;
                }

                // make a mini file for the extra data
                MEMFILE_Init(extramf, layermf->curPtr, pLayerInfos[layerNdx].lm.extraDataLength);
                // skip the extra data for this layer in the layer section
                MEMFILE_Seek(layermf, pLayerInfos[layerNdx].lm.extraDataLength, SEEK_CUR);

                if (pLayerInfos[layerNdx].lm.signature != 0x3842494D)
                {
                    SetGlobalErr (ERR_GENERIC);
                    GEcatf ("Error Layer more signature not 8BIM");
                    goto cleanup;
                }

                if (MEMFILE_Read(extramf, &layerMaskDataSize, 4) != 4)
                {
                    SetGlobalErr (ERR_GENERIC);
                    GEcatf ("Error reading layerMaskDataSize");
                    goto cleanup;
                }

                BigLong2Native (layerMaskDataSize);

                if (layerMaskDataSize)
                {
                    if (MEMFILE_Read(extramf, &pLayerInfos[layerNdx].mask, sizeof (PSLayerMaskData)) != sizeof(PSLayerMaskData))
                    {
                        SetGlobalErr (ERR_GENERIC);
                        GEcatf ("Error reading Layer mask data");
                        goto cleanup;
                    }

                    BigLong2Native(pLayerInfos[layerNdx].mask.top);
                    BigLong2Native(pLayerInfos[layerNdx].mask.left);
                    BigLong2Native(pLayerInfos[layerNdx].mask.bottom);
                    BigLong2Native(pLayerInfos[layerNdx].mask.right);
                }

                if (MEMFILE_Read(extramf, &pLayerInfos[layerNdx].gray, sizeof (PSLayerGrayInfo)) != sizeof(PSLayerGrayInfo))
                {
                    SetGlobalErr (ERR_GENERIC);
                    GEcatf ("Error reading Layer gray info");
                    goto cleanup;
                }

                BigLong2Native(pLayerInfos[layerNdx].gray.size);

                for (ch = 0; ch < pLayerInfos[layerNdx].li.channels; ++ch)
                {
                    if (MEMFILE_Read(extramf, &pLayerInfos[layerNdx].pChannelInfo[ch].csdr, sizeof (PSChannelCSDR)) != sizeof(PSChannelCSDR))
                    {
                        SetGlobalErr (ERR_GENERIC);
                        GEcatf ("Error reading channel CSDR");
                        goto cleanup;
                    }
                }

                {
                    uint8 slen;
                    uint8 strbuf[257];

                    MEMFILE_Read(extramf, &slen, 1);
                    MEMFILE_Read(extramf, strbuf, (((int)(slen + 1) + 3) & 0x1FC) - 1);

                    strbuf[slen] = '\0';

                    pLayerInfos[layerNdx].pName = strdup(strbuf);
                    if (!pLayerInfos[layerNdx].pName)
                    {
                        SetGlobalErr (ERR_GENERIC);
                        GEcatf ("Error allocating layer name");
                        goto cleanup;
                    }
                }

            }

            if (bHasMergedAlpha)
            {

                long tWidth  = pLayerInfos[0].li.right - pLayerInfos[0].li.left;
                long tHeight = pLayerInfos[0].li.bottom - pLayerInfos[0].li.top;
                short        compress;

                pTrans = (uint8*)malloc(tWidth * tHeight);
                if (!pTrans)
                {
                    SetGlobalErr (ERR_GENERIC);
                    GEcatf ("Error allocating transparency");
                    return FALSE;
                }

                if (MEMFILE_Read(layermf, &compress, 2) != 2)
                {
                    SetGlobalErr (ERR_GENERIC);
                    GEcatf ("Error reading compression type for transparency");
                    return FALSE;
                }
                BigWord2Native (compress);

                if (!compress)
                {
                    if (MEMFILE_Read(layermf, pTrans, tWidth * tHeight) != tWidth * tHeight)
                    {
                        SetGlobalErr (ERR_GENERIC);
                        GEcatf ("Error reading transparency");
                        return FALSE;
                    }
                }
                else
                {
                    long yy;

                    for (yy = 0; yy < tHeight; ++yy)
                    {
                        uint16  lineLen;
                        uint8*  pDst = pTrans + yy * tWidth;

                        if (MEMFILE_Read(layermf, &lineLen, 2) != 2)
                        {
                            SetGlobalErr (ERR_GENERIC);
                            GEcatf ("Error reading lineLen for transparency");
                            return FALSE;
                        }
                        BigWord2Native (lineLen);

                        while (lineLen)
                        {
                            char i;
                            int d = MEMFILE_getc(layermf);
                            int j;
                            int count;
                            lineLen--;

                            if (d == EOF)
                            {
                                SetGlobalErr (ERR_GENERIC);
                                GEcatf ("Error uncompressing transparency (1)");
                                return FALSE;
                            }

                            i = (char)d;

                            if (i < 0)
                            {
                                // run data

                                count = -i + 1;

                                    lineLen--;
                                d = MEMFILE_getc(layermf);
                                if(d == EOF)
                                {
                                    return(1);
                                }

                                for(j = 0; j < count; j++)
                                {
                                    *pDst++ = (uint8)d;
                                }
                            }
                            else
                            {
                                // dump data

                                count = i + 1;

                                for (j = 0; j < count; j++)
                                {
                                    lineLen--;
                                    d = MEMFILE_getc(layermf);
                                    if(d == EOF)
                                    {
                                        return(1);
                                    }
                                    *pDst++ = (uint8)d;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    width    = psh.columns;
    height   = psh.rows;
    channels = psh.channels - 3;

    blockPtr->width    = width;
    blockPtr->height   = height;
    blockPtr->channels = channels;

    {
        long    channelSize;

        channelBufs = (uint8 **)malloc ((channels + 3) * sizeof (uint8 *));
        if (!channelBufs)
        {
            SetGlobalErr (ERR_GENERIC);
            GEcatf ("Out of memory reading photoshop file");
            goto cleanup;
        }

        channelSize = width * height;

        blockPtr->rgba = (pixel32 *)malloc (channelSize * 4);
        if (!blockPtr->rgba)
        {
            SetGlobalErr (ERR_GENERIC);
            GEcatf ("Out of memory reading photoshop file");
            goto cleanup;
        }

        memset (blockPtr->rgba, 255, channelSize * 4);

        channelBufs[0] = &blockPtr->rgba[0].red;
        channelBufs[1] = &blockPtr->rgba[0].green;
        channelBufs[2] = &blockPtr->rgba[0].blue;

        if (channels)
        {
            channelBufs[3] = &blockPtr->rgba[0].alpha;
            channels = 1;
        }
    }

    {
        short        compress;

        // check compression
        if (MEMFILE_Read(mf, &compress, 2) != 2)
        {
            SetGlobalErr (ERR_GENERIC);
            GEcatf ("Error reading compression type");
            return FALSE;
        }
        BigWord2Native (compress);

    // uncompress the body

        if (!compress)
        {
        // load uncompressed

            long    channel;

            for (channel = 0; channel < channels + 3; channel++)
            {
                for(line = 0; line < height; line++)
                {
                    for(i = 0; i < width; i++)
                    {
                        if (EOF == (r = MEMFILE_getc(mf)))
                        {
                            SetGlobalErr (ERR_GENERIC);
                            GEcatf ("Error reading pic data");
                            goto cleanup;
                        }
                        *channelBufs[channel] = (uint8)r;
                        channelBufs[channel] += 4;
                    }
                }
            }
        }
        else
        {
        // load compressed
            long    junk;
            long    channel;

            junk = psh.rows*psh.channels*2;
            MEMFILE_Seek (mf, junk, SEEK_CUR);  // Skip line size info

            for (channel = 0; channel < channels + 3; channel++)
            {
                if (unpack(channelBufs[channel], mf, width, height, 4))
                {
                    SetGlobalErr (ERR_GENERIC);
                    GEcatf ("Error reading pic data");
                    goto cleanup;
                }
            }
        }
    }

    // if we have transparency and there was no alpha, use the transparency
#if 0
    if (bHasMergedAlpha && channels == 3)
    {
        long tWidth  = pLayerInfos[0].li.right - pLayerInfos[0].li.left;
        long tHeight = pLayerInfos[0].li.bottom - pLayerInfos[0].li.top;

        long yy;
        for (yy = 0; yy < tHeight; ++yy)
        {
            pixel32* pDst = &blockPtr->rgba[(pLayerInfos[0].li.top + yy) * width + pLayerInfos[0].li.left];
            uint8*   pSrc = pTrans + yy * tWidth;
            long xx;

            for (xx = 0; xx < tWidth; ++xx)
            {
                pDst->alpha = *pSrc;
                ++pSrc;
                ++pDst;
            }
        }
    }
#endif

//  flipBuffer (blockPtr->rgba, blockPtr->width * 4, blockPtr->height);

    success = TRUE;

cleanup:
    if (pTrans)  {  free (pTrans);  }

    if (pLayerInfos)
    {
        int layerNdx;

        for (layerNdx = 0; layerNdx < numLayers; ++layerNdx)
        {
            if (pLayerInfos[layerNdx].pChannelInfo)
            {
                int ch;

                for (ch = 0; ch < pLayerInfos[layerNdx].li.channels; ++ch)
                {
                    if (pLayerInfos[layerNdx].pChannelInfo[ch].pData)
                    {
                        free (pLayerInfos[layerNdx].pChannelInfo[ch].pData);
                    }
                }
                free (pLayerInfos[layerNdx].pChannelInfo);
            }
            if (pLayerInfos[layerNdx].pName)
            {
                free (pLayerInfos[layerNdx].pName);
            }
        }
    }
    if (channelBufs)    free (channelBufs);

    if (!success)
    {
        if (blockPtr->rgba) free (blockPtr->rgba);
    }

    return success;
}
// loadPhotoshop32Bit


