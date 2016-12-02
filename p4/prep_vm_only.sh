./clean.sh
cd submission
cp *.C *.H ../
cd ..
cp kernel_vm.C kernel.C
make -f makefile.linux64 clean
make -f makefile.linux64
./copykernel.sh
bochs -f bochsrc.bxrc

