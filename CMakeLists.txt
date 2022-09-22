cmake_minimum_required( VERSION 3.10 )

# set the project name
project( FF7BattleModelToFBX VERSION 1.0 )

set( m_MainDir "${CMAKE_CURRENT_SOURCE_DIR}" )
set( m_SrcDir "${m_MainDir}/src" )


file( GLOB SrcFiles "${m_SrcDir}/private/*.cpp" )
file( GLOB HdrFiles "${m_SrcDir}/public/*.h" )

# add the executable
add_executable( FF7BattleModelToFBX ${SrcFiles} ${HdrFiles} )