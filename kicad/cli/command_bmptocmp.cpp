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

CLI::BMP_TO_CMP_COMMAND::BMP_TO_CMP_COMMAND() : COMMAND( "bmp2cmp" )
{
    m_argParser.add_description( UTF8STDSTR( _( "Convert bitmap images to component footprints" ) ) );
}

int CLI::BMP_TO_CMP_COMMAND::doPerform( KIWAY& aKiway )
{
    return 0;
}
