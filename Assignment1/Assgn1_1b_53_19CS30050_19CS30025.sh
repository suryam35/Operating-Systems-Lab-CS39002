mkdir 1.b.files.out;cd 1.b.files;for file in *.txt 
do sort -n $file>../1.b.files.out/$file;done;cat ../1.b.files.out/*.txt|sort -n|uniq -c|awk '{print $2,$1}'>../1.b.out.txt
