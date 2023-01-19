while read p;do
[[ "$p" == *"$2"* ]]&&sed "s/\([a-zA-Z]\)\([^a-zA-Z]*\)\([a-zA-Z]\)/\U\1\2\L\3/g"<<<"${p^^}"||echo "$p";done<$1