x=$1;for((i=2;i<=$x;i++));do
	while(($x%$i==0));do echo $i;x=`expr $x/$i`;done;done

