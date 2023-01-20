while read p;do
sed -n "s/\b$2\b//;t;q1"<<<"$p"&&sed "s/\([a-zA-Z]\)\([^a-zA-Z]*\)\([a-zA-Z]\)/\U\1\2\L\3/g"<<<"${p^^}"||echo "$p";done<$1
