for((i=1;i<=150;i++))
do
	echo $RANDOM,$RANDOM,$RANDOM,$RANDOM,$RANDOM,$RANDOM,$RANDOM,$RANDOM,$RANDOM,$RANDOM;done>$1;cat $1|awk -v c1=$2 -F, '{print $c1}'|if grep -q $3
then
	echo "YES";else
	echo "NO";fi
