#include "LutPlugin.hpp"
#include "LutProcess.hpp"
#include "LutDefinitions.hpp"

#include <boost/filesystem/operations.hpp>
#include <boost/gil/gil_all.hpp>
#include <boost/filesystem.hpp>

namespace bfs = boost::filesystem;

namespace tuttle {
namespace plugin {
namespace lut {

using namespace boost::gil;
static const std::string kLutHelpString = "<b>Image Luter</b> is used to lut components of an image.  <br />";

LutPlugin::LutPlugin( OfxImageEffectHandle handle )
	: ImageEffectGilPlugin( handle )
{
	_sFilename = fetchStringParam( kInputFilename );
}

/**
 * @brief The overridden render function
 * @param[in]   args     Rendering parameters
 */
void LutPlugin::render( const OFX::RenderArguments& args )
{
	if( !_lutReader.readOk() )
	{
		std::string str;
		_sFilename->getValue( str );
		if( ! bfs::exists( str ) )
		{
			BOOST_THROW_EXCEPTION( exception::FileNotExist()
				<< exception::filename(str) );
		}
		if( ! _lutReader.read( str ) )
		{
			BOOST_THROW_EXCEPTION( exception::File()
				<< exception::user( "Unable to read lut file." ) );
		}
		_lut3D.reset( new TetraInterpolator(), _lutReader );
	}
	if( !_lutReader.readOk() )
	{
		BOOST_THROW_EXCEPTION( exception::Unknown() );
	}
	doGilRender<LutProcess>( *this, args );
}

void LutPlugin::changedParam( const OFX::InstanceChangedArgs& args, const std::string& paramName )
{
	if( paramName == kHelp )
	{
		sendMessage( OFX::Message::eMessageMessage,
		             "", // No XML resources
		             kLutHelpString );
	}
	else if( paramName == kInputFilename )
	{
		std::string str;
		_sFilename->getValue( str );
		if( bfs::exists( str ) )
		{
			if( ! _lutReader.read( str ) )
			{
				BOOST_THROW_EXCEPTION( exception::File() << exception::user( "Unable to read lut file..." ) );
			}
			_lut3D.reset( new TetraInterpolator(), _lutReader );
		}
	}
}

}
}
}
