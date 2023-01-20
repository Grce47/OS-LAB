declare -A dept_dict name_dict;while read -a p;do
dept_dict[${p[1]}]=$((dept_dict[${p[1]}]+1));name_dict[${p[0]}]=$((name_dict[${p[0]}]+1));done<$1;for k in ${!dept_dict[@]};do
echo "$k ${dept_dict[$k]}";done |sort -k2rn,2;echo;for k in ${!name_dict[@]};do
((${name_dict[$k]}==1))&&cnt=$((cnt+1))||echo $k;done;echo $cnt;
