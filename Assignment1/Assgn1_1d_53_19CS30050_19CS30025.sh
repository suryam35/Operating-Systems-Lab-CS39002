cd data1d/temp/;mkdir ../../files_mod;for file in *.txt
do awk '{print NR,$0}' $file|sed "s/ /,/g">../../files_mod/$file;done