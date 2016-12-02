./clean.sh
cd submission
cp *.C *.H ../../p3-grading/p3/submission
cd ../../p3-grading/p3/
#make -f makefile.linux64 clean
#make -f makefile.linux64
./prep.sh
bochs -f bochsrc.bxrc
cd ../../p4-grading/
