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
            .default_value(300);

    m_argParser.add_argument( "--dy", ARG_DPI_Y )
            .help( UTF8STDSTR( _( "The Y DPI of the image" ) ) )
            .scan<'i', int>()
            .default_value(300);

    m_argParser.add_argument( "--ol", ARG_OUTPUT_LAYER )
            .default_value( std::string() )
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

    // potrace_bitmap_t* potrace_bitmap = bm_new( m_NB_Image.GetWidth(), m_NB_Image.GetHeight() );
    potrace_bitmap_t* potrace_bitmap = bm_new( 102.4, 102.4 );
    if( !potrace_bitmap )
    {
        // TODO error message
        wxFprintf( stderr, _( "Failed to create bitmap image\n" ) );
        return 1;
    }

#if 0
    /* fill the bitmap with data */
    for( int y = 0; y < m_NB_Image.GetHeight(); y++ )
    {
        for( int x = 0; x < m_NB_Image.GetWidth(); x++ )
        {
            unsigned char pixel = m_NB_Image.GetGreen( x, y );
            BM_PUT( potrace_bitmap, x, y, pixel ? 0 : 1 );
        }
    }
#endif

    WX_STRING_REPORTER reporter;
    std::string aData = m_argParser.get<std::string>(ARG_OUTPUT);
    BITMAPCONV_INFO    converter( aData, reporter );

    converter.ConvertBitmap( potrace_bitmap, aFormat, a_Dpi_X, a_Dpi_Y, aLayer );

    if( reporter.HasMessage() ) {

    }

    return 0;
}
