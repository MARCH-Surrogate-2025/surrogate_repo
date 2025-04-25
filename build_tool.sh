pushd ./include/gRPC
./grpc_cpp_build_tool.sh
#./grpc_python_build_tool.sh
popd

mkdir -p build
cd build
cmake -DCMAKE_PREFIX_PATH=$MY_INSTALL_DIR ..
make -j4
