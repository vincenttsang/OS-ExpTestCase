./clean.sh
cd submission
cp frame_pool.C frame_pool.H page_table.C page_table.H ../
cd ..
make -f makefile.linux64 clean
make -f makefile.linux64
./copykernel.sh
