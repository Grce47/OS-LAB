mkdir -p $2;rm $2/{a..z}.txt||true;touch $2/{a..z}.txt;for line in $(cat $1/*.txt|sort);do echo "$line">>"$2/${line:0:1}.txt";done
