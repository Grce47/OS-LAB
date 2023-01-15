isInvalidword()(
    while read p
    do
        if [[ $1 =~ $p ]]
        then
            echo $p
            echo 1
            exit 1
        fi  
    done < fruits.txt
    echo 0
)
validate()(
    tmp=$(isInvalidword $1)
    if (( ${#1}<5||${#1}>20 ))
    then
        echo "NO"
    elif [[ ${1:0:1} != [a-z]&&${1:0:1} != [A-Z] ]]
    then
        echo "NO"
    elif ! [[ "$1"=~^[0-9a-zA-Z]+$ ]]
    then
        echo "NO"
    elif ! [[ "$1"=~[0-9] ]]
    then 
        echo "NO"
    elif (($tmp==1))
    then
        echo "NO"
    else
        echo "YES"
    fi 
)
while read p
do
    validate $p
done < $1
