source /home/jupiter/emsdk/emsdk_env.fish

em++ index.cpp cgfill.cpp cgparse.cpp cgproc.cpp -o ../circlegen.js \
     -I ../../include -I ../../include/eigen3 \
     -O2 -s WASM=1 \
     -s EXPORTED_FUNCTIONS='["_malloc", "_free", "__Z16processImageDataPcii", "__Z13freeImageDataPh"]' \
     -s EXPORTED_RUNTIME_METHODS='["ccall", "cwrap"]' \
     -s ALLOW_MEMORY_GROWTH=1