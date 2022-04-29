REQ_HEADERS="traceparent,Host,Accept";curl https://www.example.com/>example.html;curl -i http://ip.jsontest.com/;for header in ${REQ_HEADERS//,/ }
do 
	curl http://headers.jsontest.com/|jq -r ".$header";done;cd jsonfiles;for file in *.json
do
	curl -d "json=$(cat $file)" -X POST http://validate.jsontest.com/|grep "validate"|if grep -q "true"
	then
		echo $file>>../valid.txt;else
		echo $file>>../invalid.txt;fi;done;sort ../valid.txt -o ../valid.txt;sort ../invalid.txt -o ../invalid.txt