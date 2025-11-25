

# DotNET/src/mono/

SrcDir=$(pwd)/../../artifacts
DestDir=~/Documents/Code/MonoSDK_Temp

rm -rf $DestDir
mkdir -p $DestDir

cp -Rf  $SrcDir/obj/mono/osx.arm64.$BuildType/out/include/mono-2.0 $DstDir/include
cp -Rf  $SrcDir/obj/mono/osx.arm64.$BuildType/out/lib $DstDir/lib
cp -Rf  $SrcDir/obj/mono/osx.arm64.$BuildType/out/bin $DstDir/bin
cp -Rf  $SrcDir/bin/runtime/net10.0-osx-$BuildType-arm64 $DstDir/runtime
cp -Rf  $SrcDir/bin/mono/osx.arm64.$BuildType/IL/ $DstDir/runtime
