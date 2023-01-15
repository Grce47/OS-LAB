isInvalidword()(while read p;do
        [[ $1 =~ $p ]]&&{ echo 1;exit 1;};done<fruits.txt;echo 0;
)
validate()(
    ((${#1}<5||${#1}>20))&&{ echo "NO";exit 1;};[[ ${1:0:1} != [a-z]&&${1:0:1} != [A-Z] ]]&&{ echo "NO";exit 1;};! [[ "$1"=~^[0-9a-zA-Z]+$ ]]&&{ echo "NO";exit 1;};! [[ "$1"=~[0-9] ]]&&{ echo "NO";exit 1;};(($(isInvalidword $1)==1))&&{ echo "NO";exit 1;};echo "YES"
)
while read p;do validate $p;done<$1
