i=0
while read line
do
    array[ $i ]="$line"        
    (( i++ ))
done < <(ls $1)

dest=$2
source=$1
shift
shift

RO=1
COL=0

for i in "${array[@]}"
do
    sample=$(jq --slurp . < $source/$i)
    file=$(echo $i | sed "s/jsonl/csv/")
    rm -f "$dest/$file"
    t=0
    flag=$((${#@}-1))
    string="\""
    for arg in ${@}; do
        if (( $t != $flag )) 
        then
            string=$string"$arg\", \"" 
        else
            string=$string"$arg\""
        fi
        (( t++ ))
    done
    echo $string >> $dest/$file
    for row in $(echo "${sample}" | jq -r '.[] | @base64'); do
        _jq() {
        echo ${row} | base64 --decode | jq -r ${1}
        }       
        
        string="\""
        t=0
        flag=$((${#@}-1))
        for arg in ${@}; do
            if (( $t != $flag )) 
            then
                string=$string$(_jq ".$arg")\", 
                string=$string\"
            else
                string=$string$(_jq ".$arg")\"
            fi
            (( t++ ))
        done
        echo $string >> $dest/$file
    done
done