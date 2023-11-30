cd ../../ 

cmake -B build-ios -G Xcode -D CMAKE_BUILD_TYPE=Release -D CMAKE_SYSTEM_NAME=iOS -D CMAKE_OSX_DEPLOYMENT_TARGET=15.0 -D CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY="Apple Development" -D CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM=VN2HUYH284 

cd ./build-ios 

cmake --build . --config=Release -- -allowProvisioningUpdates