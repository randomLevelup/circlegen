em++ index.cpp cgfill.cpp cgparse.cpp cgproc.cpp -o ../circlegen.js \
     -I ../../include -O2 -s WASM=1 \
     -s EXPORTED_FUNCTIONS='["_malloc", "_free", "__Z16processImageDataPcii", "__Z13freeImageDataPh"]' \
     -s EXPORTED_RUNTIME_METHODS='["ccall", "cwrap"]' \
     -s ALLOW_MEMORY_GROWTH=1