./clean.sh
cp -r p5-provided/ p5-grading/
mv submission/* p5-grading/
cd p5-grading
mv kernel_term.C kernel.C
make -f makefile.linux64 clean
make -f makefile.linux64
./copykernel.sh
bochs -f bochsrc.bxrc
