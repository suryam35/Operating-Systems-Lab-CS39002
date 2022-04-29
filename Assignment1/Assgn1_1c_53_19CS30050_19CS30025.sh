find data1c/data -type f|awk -F. '{if($2!="")print $2}'|sort -u|while read file
do
	mkdir data1c/$file;find data1c/data -type f -name "*.$file" -exec mv -t data1c/$file -i '{}' +;done;mkdir data1c/NIL;find data1c/data -type f ! -name "*.*" -exec mv -t data1c/NIL -i '{}' +;rm -r data1c/data