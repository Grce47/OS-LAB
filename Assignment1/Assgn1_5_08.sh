print_comments(){
    echo $1
    line_no=1
    multiline_comment_double=0
    multiline_comment_single=0
    while read -r line; do
        if [[ "$line" =~ ^\ *\"\"\".*\"\"\" ]]
        then
            echo "$line_no: $line"
        elif [[ "$line" =~ ^.*\"\"\" ]] && [[ $multiline_comment_double -eq 1 ]] 
        then
            echo "  $line"
            multiline_comment_double=0
        elif [[ "$line" =~ ^.*\"\"\" ]] && [[ $multiline_comment_double -eq 0 ]]
        then
            echo "$line_no: $line"
            multiline_comment_double=1
        elif [[ "$line" =~ ^.*\'\'\'.*\'\'\' ]]
        then
            echo "$line_no: $line"
        elif [[ "$line" =~ ^.*\'\'\' ]] && [[ $multiline_comment_single -eq 1 ]] 
        then
            echo "  $line"
            multiline_comment_single=0
        elif [[ "$line" =~ ^.*\'\'\' ]] && [[ $multiline_comment_single -eq 0 ]]
        then
            echo "$line_no: $line"
            multiline_comment_single=1
        elif [[ $multiline_comment_double -eq 1 ]]
        then
            echo "  $line"
        elif [[ $multiline_comment_single -eq 1 ]]
        then
            echo "  $line"
        elif [[ "$line" =~ \#.*$ ]] && ! [[ "$line" =~ print\(\".*\#*.*\"\)[^\#]*$ ]]
        then
            echo "$line_no: $line"
        fi
        (( line_no++ ))
    done <$file
}


for file in `find "$1" -type f -name "*.py"`
do
    print_comments $file
    
    echo
done
