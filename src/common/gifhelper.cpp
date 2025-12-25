#include "cbase.h"
#include "gifhelper.h"
#include "gif_lib.h"

//-----------------------------------------------------------------------------
// Purpose: giflib read callback for pulling data from CUtlBuffers
//-----------------------------------------------------------------------------
static int GifReadData( GifFileType *pImage, GifByteType *pubBuffer, int cubBuffer )
{
    auto &bufImage = *( CUtlBuffer * )pImage->UserData;

    int nBytesToRead = Min( cubBuffer, bufImage.GetBytesRemaining() );
    if ( nBytesToRead > 0 )
        bufImage.Get( pubBuffer, nBytesToRead );

    return nBytesToRead;
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CGIFHelper::CGIFHelper( void )
    : m_pImage( NULL )
    , m_pTexture( NULL )
    , m_bProcessed( false )
    , m_iSelectedFrame( 0 )
    , m_dIterateTime( 0.0 )
    , m_textureProcThread( this )
{
}

//-----------------------------------------------------------------------------
// Purpose:
// Output : false on open/slurp failure, true on success
//-----------------------------------------------------------------------------
bool CGIFHelper::BOpenImage( CUtlBuffer &bufImage )
{
    if ( m_pImage )
    {
        CloseImage();
    }

    int nError;
    m_pImage = DGifOpen( &bufImage, GifReadData, &nError );
    if ( !m_pImage )
    {
        Warning( "Failed to open GIF image: %s\n", GifErrorString( nError ) );
        return false;
    }

    if ( DGifSlurp( m_pImage ) != GIF_OK )
    {
        Warning( "Failed to slurp GIF image: %s\n", GifErrorString( m_pImage->Error ) );
        CloseImage();
        return false;
    }

    // Texture resolution must be a power of two
	int nTexWide = CeilPow2( m_pImage->SWidth );
	int nTexTall = CeilPow2( m_pImage->SHeight );

	m_pTexture = CreateVTFTexture();
    m_pTexture->Init(
        nTexWide, nTexTall, 1,
		IMAGE_FORMAT_RGB888, // Will be converted to DXT1 when the texture is processed
        TEXTUREFLAGS_POINTSAMPLE | TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT |
        TEXTUREFLAGS_NOMIP | TEXTUREFLAGS_SINGLECOPY | TEXTUREFLAGS_PROCEDURAL,
        m_pImage->ImageCount
    );

    m_textureProcThread.Start();

    return true;
}

//-----------------------------------------------------------------------------
// Purpose: Free all GIF resources
//-----------------------------------------------------------------------------
void CGIFHelper::CloseImage( void )
{
    DestroyTexture();

    if ( m_pImage )
    {
        int nError;
        if ( DGifCloseFile( m_pImage, &nError ) != GIF_OK )
        {
            DevWarning( "Failed to close GIF image: %s\n", GifErrorString( nError ) );
        }
        m_pImage = NULL;
    }

    m_bProcessed = false;
    m_iSelectedFrame = 0;
    m_dIterateTime = 0.0;
}

//-----------------------------------------------------------------------------
// Purpose: You can call this to free texture resources if you copied the data
//          somewhere else
//-----------------------------------------------------------------------------
void CGIFHelper::DestroyTexture( void )
{
    if ( m_textureProcThread.IsAlive() )
    {
        m_textureProcThread.Stop();
    }
    if ( m_pTexture )
    {
        DestroyVTFTexture( m_pTexture );
        m_pTexture = NULL;
    }
}

//-----------------------------------------------------------------------------
// Purpose: Advances the current frame index
// Output : true - looped back to frame 0
//-----------------------------------------------------------------------------
bool CGIFHelper::BNextFrame( void )
{
    if ( !m_pImage )
        return false;

    m_iSelectedFrame++;

    if ( m_iSelectedFrame >= m_pImage->ImageCount )
    {
        // Loop
        m_iSelectedFrame = 0;
    }

    GraphicsControlBlock gcb;
    if ( DGifSavedExtensionToGCB( m_pImage, m_iSelectedFrame, &gcb ) == GIF_OK )
    {
        // simulates web browsers "throttling" short time delays so
        // gif animation speed is similar to Steam's
        constexpr double k_dMinTime = .02, k_dDefaultTime = .1; // Chrome defaults

        double dDelayTime = gcb.DelayTime * .01;
        m_dIterateTime = ( dDelayTime < k_dMinTime ? k_dDefaultTime : dDelayTime ) + Plat_FloatTime();
    }

    return m_iSelectedFrame == 0;
}

//-----------------------------------------------------------------------------
// Purpose: Gets the total number of frames in the open image
// Output : int
//-----------------------------------------------------------------------------
int CGIFHelper::GetFrameCount( void ) const
{
    return m_pImage ? m_pImage->ImageCount : 0;
}


//-----------------------------------------------------------------------------
// Purpose: Returns base address of the selected frame texture data.
//-----------------------------------------------------------------------------
uint8 *CGIFHelper::FrameData( void )
{
	return ( m_pTexture && m_bProcessed ) ? m_pTexture->ImageData( m_iSelectedFrame, 0, 0 ) : NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Gets the resolution of the texture.
//-----------------------------------------------------------------------------
void CGIFHelper::FrameSize( int &nWide, int &nTall ) const
{
    nWide = ( m_pTexture && m_bProcessed ) ? m_pTexture->Width()  : 0;
	nTall = ( m_pTexture && m_bProcessed ) ? m_pTexture->Height() : 0;
}


//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CGIFHelper::CGifTextureProcThread::CGifTextureProcThread( CGIFHelper *pOuter )
	: CThread(), m_pOuter( pOuter )
{
    SetName( "CGifTextureProcThread" );
}

//-----------------------------------------------------------------------------
// Purpose: Background worker that converts GIF frames into IVTFTexture
//-----------------------------------------------------------------------------
int CGIFHelper::CGifTextureProcThread::Run( void )
{
    GifFileType *pImage = m_pOuter->m_pImage;
    IVTFTexture *pTexture = m_pOuter->m_pTexture;
    if ( !pImage || !pTexture )
        return -1;

    constexpr int k_nBytesPerPixel = 3; //RGB888

    const int nScreenWide = pImage->SWidth;
    const int nScreenTall = pImage->SHeight;

    const int cubScreenStride = nScreenWide * k_nBytesPerPixel;

    const int cubComposite = cubScreenStride * nScreenTall;

    // Alloc our working buffers
    uint8 *pubComposite = ( uint8 * )stackalloc( cubComposite );
    uint8 *pubPrevious = ( uint8 * )stackalloc( cubComposite );
    Q_memset( pubComposite, 0, cubComposite );
    Q_memset( pubPrevious, 0, cubComposite );

    // Process every GIF frame
    for ( int iFrame = 0; iFrame < pImage->ImageCount; ++iFrame )
    {
        GifImageDesc &imageDesc = pImage->SavedImages[ iFrame ].ImageDesc;
        ColorMapObject *pColorMap = imageDesc.ColorMap ? imageDesc.ColorMap : pImage->SColorMap;

        int nTransparentIndex = NO_TRANSPARENT_COLOR;
        int nDisposalMethod = DISPOSAL_UNSPECIFIED;
        GraphicsControlBlock gcb;
        if ( DGifSavedExtensionToGCB( pImage, iFrame, &gcb ) == GIF_OK )
        {
            nTransparentIndex = gcb.TransparentColor;
            nDisposalMethod = gcb.DisposalMode;
        }

        // Draw over previous composite
        Q_memcpy( pubComposite, pubPrevious, cubComposite );

        // Render this frame onto the composite
        auto lambdaRenderFrameToComposite = [ & ]( int nRowOffset = 0, int nRowIncrement = 1 )
        {
            int iPixel = nRowOffset * imageDesc.Width;
            for ( int y = nRowOffset; y < imageDesc.Height; y += nRowIncrement )
            {
                const int nScreenY = y + imageDesc.Top;
                if ( nScreenY >= nScreenTall ) { iPixel += imageDesc.Width; continue; }

                uint8 *pubScanLine = pubComposite + ( nScreenY * cubScreenStride ) + ( imageDesc.Left * k_nBytesPerPixel );
                for ( int x = 0; x < imageDesc.Width; ++x, ++iPixel )
                {
                    const int nScreenX = x + imageDesc.Left;
                    if ( nScreenX >= nScreenWide ) continue;

                    GifByteType idx = pImage->SavedImages[ iFrame ].RasterBits[ iPixel ];
                    if ( idx < pColorMap->ColorCount && idx != nTransparentIndex )
                    {
                        const GifColorType &color = pColorMap->Colors[ idx ];
                        pubScanLine[ 0 ] = color.Red;
                        pubScanLine[ 1 ] = color.Green;
                        pubScanLine[ 2 ] = color.Blue;
                    }
                    pubScanLine += k_nBytesPerPixel;
                }
            }
        };

        if ( imageDesc.Interlace )
        {
            // https://giflib.sourceforge.net/gifstandard/GIF89a.html#interlacedimages
            constexpr int k_nRowOffsets[] = { 0, 4, 2, 1 };
            constexpr int k_nRowIncrements[] = { 8, 8, 4, 2 };
            for ( int nPass = 0; nPass < 4; ++nPass )
            {
                lambdaRenderFrameToComposite( k_nRowOffsets[ nPass ], k_nRowIncrements[ nPass ] );
            }
        }
        else
        {
            lambdaRenderFrameToComposite();
        }

        const int cubTextureStride = pTexture->RowSizeInBytes( 0 );
        uint8 *pubTextureBase = pTexture->ImageData( iFrame, 0, 0 );
        // Do bilinear resampling from GIF image size to VTF texture size
        for ( int y = 0; y < pTexture->Height(); ++y )
        {
            // Compute fractional xy positions from source
            float flScreenY = ( ( y + 0.5f ) * nScreenTall / ( float )pTexture->Height() ) - .5f;

            int y0 = ( int )floor( flScreenY );
            int y1 = Min( y0 + 1, nScreenTall - 1 );
            float fy = flScreenY - y0;
            y0 = Max( y0, 0 );

            uint8 *pubScanLine = pubTextureBase + y * cubTextureStride;

            for ( int x = 0; x < pTexture->Width(); ++x )
            {
                float flScreenX = ( ( x + 0.5f ) * nScreenWide / ( float )pTexture->Width() ) - .5f;

                int x0 = ( int )floor( flScreenX );
                int x1 = Min( x0 + 1, nScreenWide - 1 );
                float fx = flScreenX - x0;
                x0 = Max( x0, 0 );

                // Get closest 2x2 neighborhood of pixels
                const uint8 *p00 = pubComposite + ( y0 * cubScreenStride ) + ( x0 * k_nBytesPerPixel );
                const uint8 *p10 = pubComposite + ( y0 * cubScreenStride ) + ( x1 * k_nBytesPerPixel );
                const uint8 *p01 = pubComposite + ( y1 * cubScreenStride ) + ( x0 * k_nBytesPerPixel );
                const uint8 *p11 = pubComposite + ( y1 * cubScreenStride ) + ( x1 * k_nBytesPerPixel );
                // Do interp each channel
                for ( int c = 0; c < k_nBytesPerPixel; ++c )
                {
                    float c00 = p00[ c ];
                    float c10 = p10[ c ];
                    float c01 = p01[ c ];
                    float c11 = p11[ c ];
                    float c0 = c00 + fx * ( c10 - c00 );
                    float c1 = c01 + fx * ( c11 - c01 );
                    pubScanLine[ x * k_nBytesPerPixel + c ] = ( uint8 )( c0 + fy * ( c1 - c0 ) );
                }
            }
        }

        // Handle disposal for the next frame
        switch ( nDisposalMethod )
        {
        case DISPOSE_BACKGROUND:
            // Fill previous with background color
            if ( pImage->SBackGroundColor < pImage->SColorMap->ColorCount )
            {
                const GifColorType &color = pImage->SColorMap->Colors[ pImage->SBackGroundColor ];
                const int nFillWide = Min( imageDesc.Width, nScreenWide - imageDesc.Left );
                const int nFillTall = Min( imageDesc.Height, nScreenTall - imageDesc.Top );

                for ( int y = 0; y < nFillTall; ++y )
                {
                    uint8 *pubScanLine = pubPrevious + ( ( y + imageDesc.Top ) * cubScreenStride ) + imageDesc.Left * k_nBytesPerPixel;

                    for ( int x = 0; x < nFillWide; ++x )
                    {
                        pubScanLine[ 0 ] = color.Red;
                        pubScanLine[ 1 ] = color.Green;
                        pubScanLine[ 2 ] = color.Blue;
                        pubScanLine += k_nBytesPerPixel;
                    }
                }
            }
            break;
        case DISPOSE_PREVIOUS:
            // Keep previous composite
            break;
        case DISPOSAL_UNSPECIFIED:
        case DISPOSE_DO_NOT:
        default:
            // Copy current composite to previous
            Q_memcpy( pubPrevious, pubComposite, cubComposite );
            break;
        }
    }

    pTexture->ConvertImageFormat( IMAGE_FORMAT_DXT1_RUNTIME, false );

    m_pOuter->m_bProcessed = true;

    return 0;
}