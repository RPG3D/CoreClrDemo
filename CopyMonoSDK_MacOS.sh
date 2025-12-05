

# place at DotNet source root directory

SrcDir=$(pwd)/artifacts
DestDir=~/Documents/Code/MonoSDK_Temp

BuildType=Debug

rm -rf $DestDir
mkdir -p $DestDir

cp -Rf  $SrcDir/obj/mono/osx.arm64.$BuildType/out/include/mono-2.0/ $DestDir/include
cp -Rf  $SrcDir/obj/mono/osx.arm64.$BuildType/out/lib/ $DestDir/lib
cp -Rf  $SrcDir/obj/mono/osx.arm64.$BuildType/out/bin/ $DestDir/bin
cp -Rf  $SrcDir/bin/runtime/net10.0-osx-$BuildType-arm64/ $DestDir/runtime
cp -Rf  $SrcDir/bin/mono/osx.arm64.$BuildType/IL/ $DestDir/runtime
