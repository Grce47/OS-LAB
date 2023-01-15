isInvalidword()(while read p;do
        [[ ${1,,} =~ ${p,,} ]]&&{ echo 1;exit 1;};done<fruits.txt;echo 0;
)
validate()(
    (((${#1}<5||${#1}>20))||[[ ${1:0:1} != [a-zA-Z] ]]||! [[ "$1" =~ ^[0-9a-zA-Z]+$ ]]||! [[ "$1" =~ [0-9] ]]||(($(isInvalidword $1)==1)))&&echo "NO"||echo "YES";
)
while read p;do validate $p;done<$1
