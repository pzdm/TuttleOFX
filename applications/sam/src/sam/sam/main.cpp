#include <tuttle/common/exceptions.hpp>

#include <tuttle/host/Core.hpp>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/throw_exception.hpp>

namespace bfs = boost::filesystem;

bfs::path retrieveToolFullPath( const std::string& toolName, const std::vector<bfs::path>& searchPaths )
{
	const std::string toolFilename( std::string("sam-") + toolName );
	
	BOOST_FOREACH( const bfs::path& p, searchPaths )
	{
		const bfs::path toolFullPath = p / toolFilename;
		if( bfs::exists( toolFullPath ) )
		{
			return toolFullPath;
		}
	}

	/// @todo exception ?
	TUTTLE_CERR(
		"Sam command \"" << toolName << "\" not found."
		"\n"
		);
	// displayAvailableCommands();
	exit( -1 );
}

std::vector<bfs::path> retrieveSearchPaths( const bfs::path& samDirectory )
{
	std::vector<bfs::path> searchPaths;
	if( const char* env_ptr = std::getenv("SAM_PATH") )
	{
		const std::string env( env_ptr );
		const std::vector<std::string> paths = boost::program_options::split_unix( env, ":;" );
		searchPaths.insert( searchPaths.end(), paths.begin(), paths.end() );
	}
	
	searchPaths.push_back( samDirectory / "sam" );
	searchPaths.push_back( samDirectory );
	
	return searchPaths;
}

std::vector<bfs::path> retrieveAllSamCommands( const bfs::path& dir )
{
	std::vector<bfs::path> res;
	if( ! bfs::is_directory(dir) )
		return res;
	
	bfs::directory_iterator dir_iter(dir), dir_end;
	for(;dir_iter != dir_end; ++dir_iter)
	{
		bfs::path cmd = *dir_iter;
		if( boost::starts_with( cmd.filename().string(), "sam-" ) )
		{
			res.push_back( cmd );
		}
	}
	return res;
}

std::vector<bfs::path> retrieveAllSamCommands( const std::vector<bfs::path>& dirs )
{
	typedef std::map<bfs::path,bfs::path> PathMap;
	PathMap allRes;
	BOOST_FOREACH( const bfs::path& dir, dirs )
	{
		std::vector<bfs::path> dirRes = retrieveAllSamCommands( dir );
		BOOST_FOREACH( const bfs::path& cmd, dirRes )
		{
			if( allRes.find( cmd.filename() ) == allRes.end() )
			{
				allRes[cmd.filename()] = cmd;
			}
		}
	}
	std::vector<bfs::path> res;
	res.reserve( allRes.size() );
	BOOST_FOREACH( PathMap::const_reference p, allRes )
	{
		res.push_back( p.second );
	}
	return res;
}

int main( int argc, char** argv )
{
	if( argc <= 1 ) // no argument
	{
		TUTTLE_COUT(
			"sam: missing operands.\n"
			"'sam --help' for more informations.\n"
			);
		exit( -1 );
	}

	try
	{
		namespace bpo = boost::program_options;
	
		std::vector<std::string> cl_options;
		for( int i = 1; i < argc; ++i )
		{
			const std::string s( argv[i] );
			if( ! boost::starts_with( s, "-" ) )
			{
				break;
			}
			cl_options.push_back( s );
		}
		
		// Declare the supported options.
		bpo::options_description infoOptions( "\tOptions" );
		infoOptions.add_options()
			("help,h"       , "show node help")
			("version,v"    , "display version") // which version?
			("commands,c"   , "show list of all available sam commands")
			("commands-list" , "show list of all available sam commands") /// @todo: secret
			("binaries-list" , "show list of all available sam binaries") /// @todo: secret
		;
		bpo::options_description all_options;
		all_options.add(infoOptions);

		bpo::variables_map sam_vm;
		bpo::store( bpo::command_line_parser(cl_options).options(all_options).run(), sam_vm );
		if( const char* env_ptr = std::getenv("SAM_OPTIONS") )
		{
			std::vector<std::string> envOptions;
			std::string env( env_ptr );
			envOptions.push_back( env );
			bpo::store( bpo::command_line_parser(envOptions).options(all_options).run(), sam_vm );
		}

		bpo::notify( sam_vm );

		if( sam_vm.count("help") )
		{
			TUTTLE_COUT( "TuttleOFX project [http://sites.google.com/site/tuttleofx]" );
			TUTTLE_COUT( "" );
			TUTTLE_COUT( "NAME" );
			TUTTLE_COUT( "\tsam - A set of command line tools." );
			TUTTLE_COUT( "" );
			TUTTLE_COUT( "SYNOPSIS" );
			TUTTLE_COUT( "\tsam COMMAND [options]..." );
			TUTTLE_COUT( "" );
			TUTTLE_COUT( "DESCRIPTION" );
			TUTTLE_COUT( infoOptions );
			exit( 0 );
		}
		if( sam_vm.count("version") )
		{
			TUTTLE_COUT( "TuttleOFX Host - version " << TUTTLE_HOST_VERSION_STR );
			exit( 0 );
		}
		
		bfs::path fullsam( argv[0] );
		if( fullsam.is_relative() )
		{
			fullsam = bfs::current_path() / fullsam;
		}
		bfs::path samDirectory( fullsam.parent_path() );

		const std::vector<bfs::path> searchPaths = retrieveSearchPaths( samDirectory );

		if( sam_vm.count("commands") )
		{
			TUTTLE_COUT( "" );
			TUTTLE_COUT( "COMMANDS" );
			
			const std::vector<bfs::path> cmds = retrieveAllSamCommands( searchPaths );
			BOOST_FOREACH( const bfs::path& c, cmds )
			{
				TUTTLE_COUT( "\t" << c.filename().string().substr(4) );
//				const int res = system( (c.string()+" --brief").c_str() );
				TUTTLE_COUT( "" );
			}
			TUTTLE_COUT( "" );

			exit( 0 );
		}
		if( sam_vm.count("binaries-list") )
		{
			const std::vector<bfs::path> cmds = retrieveAllSamCommands( searchPaths );
			BOOST_FOREACH( const bfs::path& c, cmds )
			{
				TUTTLE_COUT( c.string() );
			}
			exit( 0 );
		}
		if( sam_vm.count("commands-list") )
		{
			const std::vector<bfs::path> cmds = retrieveAllSamCommands( searchPaths );
			BOOST_FOREACH( const bfs::path& c, cmds )
			{
				TUTTLE_COUT( c.filename().string() );
			}
			exit( 0 );
		}
		
		const std::string sam_tool( argv[cl_options.size() + 1] );
		std::vector<std::string> sam_cmd;
		for( int i = cl_options.size() + 2; i < argc; ++i )
		{
			const std::string s( argv[i] );
			sam_cmd.push_back( s );
		}
		
		bfs::path toolFullPath = retrieveToolFullPath( sam_tool, searchPaths );
		
		const std::string fullcmd( toolFullPath.string() + " " + boost::algorithm::join( sam_cmd, " " ) );
		
//		TUTTLE_TCOUT_VAR( fullcmd );
		return system( fullcmd.c_str() );
	}
	catch( const boost::program_options::error& e )
	{
		TUTTLE_CERR( "sam");
		TUTTLE_CERR( "Error: " << e.what() );
		exit( -2 );
	}
	catch( ... )
	{
		TUTTLE_CERR( "sam");
		TUTTLE_CERR( boost::current_exception_diagnostic_information() );
		exit( -2 );
	}
	return 0;
}

