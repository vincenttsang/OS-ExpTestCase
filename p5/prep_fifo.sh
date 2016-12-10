rm -rf p5-temp/
cp -r p5-provided/ p5-temp/
sudo chown -R guest:guest p5-temp/
cp submission/* p5-temp/
cd p5-temp
mv kernel_fifo.C kernel.C
make -f makefile.linux64 clean
make -f makefile.linux64
sh ./copykernel.sh
bochs -f bochsrc.bxrc
