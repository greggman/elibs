/*************************************************************************
 *                                                                       *
 *                              MEMFILE.CPP                              *
 *                                                                       *
 *************************************************************************

                          Copyright 1996 Echidna

   DESCRIPTION


   PROGRAMMERS


   FUNCTIONS

   TABS : 5 9

   HISTORY
		03/28/96 : Created.

 *************************************************************************/

/**************************** I N C L U D E S ****************************/

#include "platform.h"
#include "switches.h"
#include "echidna/ensure.h"

#include "echidna/memfile.h"
#include "echidna/eio.h"

/*************************** C O N S T A N T S ***************************/


/******************************* T Y P E S *******************************/


/************************** P R O T O T Y P E S **************************/


/***************************** G L O B A L S *****************************/


/****************************** M A C R O S ******************************/


/**************************** R O U T I N E S ****************************/

MEMFILE *MEMFILE_Load (const char *filename)
{
	MEMFILE* mf = NULL;
	int		fh;
	long	len;
	
	fh = EIO_ReadOpen(filename); 
	if (fh != (-1))
	{
		len = EIO_FileLength(fh);
		if (len)
		{
			mf = (MEMFILE*)malloc (len + sizeof (MEMFILE));
			if (mf)
			{
				memset (mf, 0, sizeof (MEMFILE));

				mf->buffer = ((uint8*)mf)+sizeof (MEMFILE);
			
				mf->curPtr    = mf->buffer;
				mf->size      = len;
				mf->bytesLeft = len;
	
				EIO_Read (fh, mf->buffer, len);
			}
		}
		EIO_Close (fh);
	}
	return mf;
}

void MEMFILE_Close (MEMFILE *mf)
{
	free (mf);
}

int MEMFILE_Read(MEMFILE *mf, void *buf, long len)
{
	if (mf->bytesLeft)
	{
		long	copyBytes;

		copyBytes = (mf->bytesLeft < len) ? mf->bytesLeft : len;

		memcpy (buf, mf->curPtr, copyBytes);

		mf->bytesLeft -= copyBytes;
		mf->curPtr    += copyBytes;

		return copyBytes;
	}
	return 0;
}

int MEMFILE_Seek (MEMFILE *mf, long pos, int type)
{
	switch (type)
	{
	case SEEK_SET:
		mf->curPtr    = mf->buffer + pos;
		mf->bytesLeft = mf->size   - pos;
		break;
	case SEEK_CUR:
		mf->curPtr    += pos;
		mf->bytesLeft -= pos;
		break;
	case SEEK_END:
		mf->curPtr     = mf->buffer + mf->size + pos;
		mf->bytesLeft  = -pos;
		break;
	}
	return mf->size - mf->bytesLeft;
}


