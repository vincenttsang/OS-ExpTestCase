rm p2-naive/frame_pool.C p2-naive/frame_pool.H
rm p2-hard/frame_pool.C p2-hard/frame_pool.H
cp frame_* p2-naive/
mv frame_* p2-hard/
cd p2-naive
make -f makefile.linux64
./copykernel.sh
cd ../p2-hard
make -f makefile.linux64
./copykernel.sh
