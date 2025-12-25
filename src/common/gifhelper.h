#ifndef GIFHELPER_H
#define GIFHELPER_H
#ifdef _WIN32
#pragma once
#endif

#include "vtf/vtf.h"

struct GifFileType;

//-----------------------------------------------------------------------------
// Purpose: Simple utility for decoding GIFs
//-----------------------------------------------------------------------------
class CGIFHelper
{
public:
	CGIFHelper( void );
	~CGIFHelper( void ) { CloseImage(); }

	bool BOpenImage( CUtlBuffer &bufImage );
	void CloseImage( void );

	void DestroyTexture( void );

	bool BIsProcessed( void ) const { return m_bProcessed; }

	bool BNextFrame( void ); // iterates to the next frame, returns true if we have just looped
	int  GetFrameCount( void ) const;
	int  GetSelectedFrame( void ) const { return m_iSelectedFrame; }
	bool BShouldIterateFrame( void ) const { return m_dIterateTime < Plat_FloatTime(); }

	// !!! Methods below will only work when the texture has been fully processed !!! //

	// Main method for retrieving selected frame texture data.
	// The output texture format is IMAGE_FORMAT_DXT1_RUNTIME.
	uint8 *FrameData( void );

	// Gets the resolution of the texture.
	void   FrameSize( int &nWide, int &nTall ) const;

private:
	// Background worker for processing GIFs to textures
	class CGifTextureProcThread : public CThread
	{
	public:
		CGifTextureProcThread( CGIFHelper *pOuter );

	protected:
		virtual int Run( void );

	private:
		CGIFHelper *m_pOuter;
	};

	GifFileType *m_pImage;
	IVTFTexture *m_pTexture;
	bool         m_bProcessed;

	int m_iSelectedFrame;

	double m_dIterateTime;

	CGifTextureProcThread m_textureProcThread;
};

#endif //GIFHELPER_H