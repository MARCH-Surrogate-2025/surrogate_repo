cd cpp
mkdir -p build
cd build
# cmake ..
cmake -DCMAKE_PREFIX_PATH=$MY_INSTALL_DIR ../..
make -j4
