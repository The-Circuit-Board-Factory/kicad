/*
 * this program source code file is part of kicad, a free eda cad application.
 *
 * copyright (c) 2022 mark roszko <mark.roszko@gmail.com>
 * copyright the kicad developers, see authors.txt for contributors.
 *
 * this program is free software: you can redistribute it and/or modify it
 * under the terms of the gnu general public license as published by the
 * free software foundation, either version 3 of the license, or (at your
 * option) any later version.
 *
 * this program is distributed in the hope that it will be useful, but
 * without any warranty; without even the implied warranty of
 * merchantability or fitness for a particular purpose.  see the gnu
 * general public license for more details.
 *
 * you should have received a copy of the gnu general public license along
 * with this program.  if not, see <http://www.gnu.org/licenses/>.
 */

#include "command_bmptocmp.h"
#include <cli/exit_codes.h>
#include <kiface_base.h>
#include <layer_ids.h>
#include <string_utils.h>
#include <wx/crt.h>
#include <wx/dir.h>

#include <macros.h>
#include <wx/tokenzr.h>

#include <bitmap2cmp_frame.h>
#include <bitmap2component.h>
#include <bitmap2cmp_panel.h>
#include <bitmap2cmp_settings.h>
#include <potracelib.h>
#include <bitmap.h>
//#include <bitmap_io.h>

#define ARG_OUTPUT_FORMAT "--output-format"
#define ARG_DPI_X "--dpi-x"
#define ARG_DPI_Y "--dpi-y"
#define ARG_OUTPUT_LAYER "--output-layer"
#define ARG_OUTPUT "--output"

#define DEFAULT_DPI 300

CLI::BMP_TO_CMP_COMMAND::BMP_TO_CMP_COMMAND() : COMMAND( "bmp2cmp" )
{
    m_argParser.add_description( UTF8STDSTR( _( "Convert bitmap images to component footprints" ) ) );

    addCommonArgs( true, false, false, false );

    m_argParser.add_argument( "-o", ARG_OUTPUT )
            .default_value( std::string() )
            .help( UTF8STDSTR( _( "The file to output the converted component to" ) ) );

    m_argParser.add_argument( "--of", ARG_OUTPUT_FORMAT )
            .default_value( std::string() )
            .help( UTF8STDSTR( _( "The format to output the bitmap to" ) ) );

    m_argParser.add_argument( "--dx", ARG_DPI_X )
            .help( UTF8STDSTR( _( "The X DPI of the image" ) ) )
            .scan<'i', int>()
            .default_value(DEFAULT_DPI);

    m_argParser.add_argument( "--dy", ARG_DPI_Y )
            .help( UTF8STDSTR( _( "The Y DPI of the image" ) ) )
            .scan<'i', int>()
            .default_value(DEFAULT_DPI);

    m_argParser.add_argument( "--ol", ARG_OUTPUT_LAYER )
            .default_value( "F.Silkscreen" )
            .help( UTF8STDSTR( _( "The layer the resulting footprint is on" ) ) );
}

int CLI::BMP_TO_CMP_COMMAND::doPerform( KIWAY& aKiway )
{
    // cli arguments

    // std::string format = m_argParser.get<std::string>( ARG_OUTPUT_FORMAT );
    // TODO hardcoded to footprint only for now
    OUTPUT_FMT_ID aFormat = FOOTPRINT_FMT;

    /*
    if( aFormat == FOOTPRINT_FMT )
    {
        switch( m_layerCtrl->GetSelection() )
        {
        case 0: layer = wxT( "F.Cu" );      break;
        case 1: layer = wxT( "F.SilkS" );   break;
        case 2: layer = wxT( "F.Mask" );    break;
        case 3: layer = wxT( "Dwgs.User" ); break;
        case 4: layer = wxT( "Cmts.User" ); break;
        case 5: layer = wxT( "Eco1.User" ); break;
        case 6: layer = wxT( "Eco2.User" ); break;
        case 7: layer = wxT( "F.Fab" );     break;
        }
    }
    */
    int a_Dpi_X = m_argParser.get<int>(ARG_DPI_X);
    int a_Dpi_Y = m_argParser.get<int>(ARG_DPI_Y);
    const wxString aLayer = From_UTF8(m_argParser.get<std::string>(ARG_OUTPUT_LAYER).c_str());

    // convert image
    wxImage           m_Pict_Image;
    wxBitmap          m_Pict_Bitmap;
    wxImage           m_Greyscale_Image;
    wxBitmap          m_Greyscale_Bitmap;
    wxImage           m_NB_Image;
    wxBitmap          m_BN_Bitmap;
    IMAGE_SIZE        m_outputSizeX;
    IMAGE_SIZE        m_outputSizeY;
    double            m_aspectRatio;

    m_Pict_Image.Destroy();

    if( !m_Pict_Image.LoadFile(m_argInput))
    {
        // LoadFile has its own UI, no need for further failure notification here
        return false;
    }

    m_Pict_Bitmap = wxBitmap( m_Pict_Image );

    // Determine image resolution in DPI (does not existing in all formats).
    // the resolution can be given in bit per inches or bit per cm in file

    int imageDPIx = m_Pict_Image.GetOptionInt( wxIMAGE_OPTION_RESOLUTIONX );
    int imageDPIy = m_Pict_Image.GetOptionInt( wxIMAGE_OPTION_RESOLUTIONY );

    if( imageDPIx > 1 && imageDPIy > 1 )
    {
        if( m_Pict_Image.GetOptionInt( wxIMAGE_OPTION_RESOLUTIONUNIT ) == wxIMAGE_RESOLUTION_CM )
        {
            imageDPIx = KiROUND( imageDPIx * 2.54 );
            imageDPIy = KiROUND( imageDPIy * 2.54 );
        }
    }
    else    // fallback to a default value (DEFAULT_DPI)
    {
        imageDPIx = imageDPIy = DEFAULT_DPI;
    }

//    m_InputXValueDPI->SetLabel( wxString::Format( wxT( "%d" ), imageDPIx ) );
//    m_InputYValueDPI->SetLabel( wxString::Format( wxT( "%d" ), imageDPIy ) );
//
    int h  = m_Pict_Bitmap.GetHeight();
    int w  = m_Pict_Bitmap.GetWidth();
    m_aspectRatio = (double) w / h;

    m_outputSizeX.SetOriginalDPI( imageDPIx );
    m_outputSizeX.SetOriginalSizePixels( w );
    m_outputSizeY.SetOriginalDPI( imageDPIy );
    m_outputSizeY.SetOriginalSizePixels( h );

    // Update display to keep aspect ratio
    //wxCommandEvent dummy;
    //OnSizeChangeX( dummy );

    //updateImageInfo();

    //m_GreyscalePicturePanel->SetVirtualSize( w, h );
    //m_InitialPicturePanel->SetVirtualSize( w, h );
    //m_BNPicturePanel->SetVirtualSize( w, h );

    m_Greyscale_Image.Destroy();
    m_Greyscale_Image = m_Pict_Image.ConvertToGreyscale( );

    if( m_Pict_Bitmap.GetMask() )
    {
        for( int x = 0; x < m_Pict_Bitmap.GetWidth(); x++ )
        {
            for( int y = 0; y < m_Pict_Bitmap.GetHeight(); y++ )
            {
                if( m_Pict_Image.GetRed( x, y ) == m_Pict_Image.GetMaskRed()
                        && m_Pict_Image.GetGreen( x, y ) == m_Pict_Image.GetMaskGreen()
                        && m_Pict_Image.GetBlue( x, y ) == m_Pict_Image.GetMaskBlue() )
                {
                    m_Greyscale_Image.SetRGB( x, y, 255, 255, 255 );
                }
            }
        }
    }

    const wxString outputFile = From_UTF8(m_argParser.get<std::string>(ARG_OUTPUT).c_str());
    FILE* outfile = wxFopen( outputFile, wxT( "w" ) );

    if( !outfile )
    {
        // wxMessageBox( wxString::Format( _( "File '%s' could not be created." ), m_outFileName ) );
        wxFprintf( stderr,  wxString::Format( _( "File '%s' could not be created." ), outputFile ));
        return 1;
    }

    std::string buffer;


    potrace_bitmap_t* potrace_bitmap = bm_new( m_NB_Image.GetWidth(), m_NB_Image.GetHeight() );
    if( !potrace_bitmap )
    {
        // TODO error message
        wxFprintf( stderr, _( "Failed to create bitmap image\n" ) );
        return 1;
    }

    /* fill the bitmap with data */
    for( int y = 0; y < m_NB_Image.GetHeight(); y++ )
    {
        for( int x = 0; x < m_NB_Image.GetWidth(); x++ )
        {
            unsigned char pixel = m_NB_Image.GetGreen( x, y );
            BM_PUT( potrace_bitmap, x, y, pixel ? 0 : 1 );
        }
    }

    WX_STRING_REPORTER reporter;
    BITMAPCONV_INFO    converter( buffer, reporter );

    wxFprintf(stdout, _("Outputting footprint\n"));

    converter.ConvertBitmap( potrace_bitmap, aFormat, a_Dpi_X, a_Dpi_Y, aLayer );

    if( reporter.HasMessage() ) {
        wxFprintf(stderr, _( reporter.GetMessages() ) );
    }


    fputs( buffer.c_str(), outfile );
    fclose( outfile );




    return 0;
}
