cmake_minimum_required( VERSION 3.10 )

# set the project name
project( FF7BattleModelToFBX VERSION 1.0 )

set( m_MainDir ${CMAKE_CURRENT_SOURCE_DIR} )
set( m_SrcDir ${m_MainDir}/src )

# Need these strings for FBX filepaths
if( CMAKE_GENERATOR_PLATFORM MATCHES "Win32" )
    set( m_Platform "x86" )
elseif( CMAKE_GENERATOR_PLATFORM MATCHES "x64" )
    set( m_Platform "x64" )
else()
    message( FATAL_ERROR "Unhandled platform type ${CMAKE_GENERATOR_PLATFORM}" )
    set( m_Platform "x64" )
endif()

# Build currently only works as x86 because of inline assembly code
if( NOT CMAKE_GENERATOR_PLATFORM MATCHES "Win32" )
    message( FATAL_ERROR "Non Win32 builds not currently supported, as inline asm exists in code" )
endif()

file( GLOB SrcFiles ${m_SrcDir}/private/*.cpp )
file( GLOB HdrFiles ${m_SrcDir}/public/*.h )

# add the executable
add_executable( FF7BattleModelToFBX ${SrcFiles} ${HdrFiles} )

set_property( GLOBAL PROPERTY VS_STARTUP_PROJECT FF7BattleModelToFBX )

target_include_directories( FF7BattleModelToFBX PUBLIC ${m_SrcDir}/public )

# FBX
# Make sure to use the appropriate runtime library for your version of Visual Studio, and for the processor architecture of your target application. 
# The FBX libraries are organized into subdirectories under <yourFBXSDKpath>\lib according to their Visual Studio version, processor type, and build mode:
# <yourFBXSDKpath>\lib\<compiler_version>\<processor_type>\<build_mode>
set( FBX_RootDir   "E:/Autodesk/FBX/FBX SDK/2020.3.2" CACHE FILEPATH "Root directory of FBX SDK, which includes the /include/ and /lib/ folders" )
set( FBX_VSVersion "vs2019"                           CACHE STRING   "Version of VS compiler. Must match the directory name in FBX_RootDir/lib/" ) 
set( FBX_Config    "debug"                            CACHE STRING   "Release type must be set for FBX to pull from the right lib directory"     ) 
set_property( CACHE FBX_VSVersion PROPERTY STRINGS vs2017 vs2019 )
set_property( CACHE FBX_Config    PROPERTY STRINGS debug release )

# 6. In the property tree on the left of the Property Pages dialog, choose Configuration Properties > C/C++ > General.
# 7. In the properties sheet on the right of the dialog, select <Edit..> in the Additional Include Directories drop-down box. The Additional Include Directories dialog appears.
# 8. In the list box on the top of the Additional Include Directories dialog, click the top blank line until you see a control to browse for directories.
# 9. Use this control to add the full path for the <yourFBXSDKpath>\include directory and click OK.
target_include_directories( FF7BattleModelToFBX PUBLIC ${FBX_RootDir}/include )
# (steps 10 & 11 are only relevant if linking with a static library)
# 12. In the Property Pages dialog box, choose Linker > General.
# 13. In the Additional Library Dependencies drop-down box, select <Edit..>. The Additional Library Directories dialog appears.
# 14. Enter the full path for the <yourFBXSDKpath>\lib folder of the FBX SDK distribution.
target_link_directories( FF7BattleModelToFBX PUBLIC ${FBX_RootDir}/lib )
# 15. In the Property Pages dialog box, choose Linker > Input. In the Additional Dependencies drop-down box, select <Edit>. The Additional Dependencies dialog appears. 
#     Add the appropriate FBX SDK library. For a table describing the available FBX SDK library files, see Configuring the FBX SDK for Windows.
set( m_libfbxsdkPath ${FBX_RootDir}/lib/${FBX_VSVersion}/${m_Platform}/${FBX_Config} )
target_link_libraries( FF7BattleModelToFBX PRIVATE ${m_libfbxsdkPath}/libfbxsdk.lib )
# If you are using a dynamic library version of the FBX SDK, add FBXSDK_SHARED to the preprocessor definitions of your project. 
# To do this in Visual Studio, in your project, select: 
# PROJECT > Properties > Configuration Properties > C/C++ > Preprocessor, and edit the Preprocessor Definitions field.
add_compile_definitions( FBXSDK_SHARED )
# (step 16 is only relevant if linking with a static library)

# Need to copy file to output
add_custom_command(
    TARGET FF7BattleModelToFBX POST_BUILD           # Adds a post-build event to FF7BattleModelToFBX
    COMMAND ${CMAKE_COMMAND} -E copy_if_different   # which executes "cmake - E copy_if_different..."
        ${m_libfbxsdkPath}/libfbxsdk.dll            # <--this is in-file
        $<TARGET_FILE_DIR:FF7BattleModelToFBX>)     # <--this is out-file path