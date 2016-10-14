./clean.sh
cp frame_* p2-naive/
mv frame_* p2-hard/
cd p2-naive
make -f makefile.linux64
./copykernel.sh
cd ../p2-hard
make -f makefile.linux64
./copykernel.sh
