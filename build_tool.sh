pushd ./include/gRPC
./grpc_cpp_build_tool.sh
./grpc_python_build_tool.sh
popd

mkdir -p build
cd build
cmake ..
make -j4
