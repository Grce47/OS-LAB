declare -A dept_dict name_dict;while read -a p;do
dept_dict[${p[1]}]=$((dept_dict[${p[1]}]+1));name_dict[${p[0]}]=$((name_dict[${p[0]}]+1));done<$1;for k in $(for x in ${!dept_dict[@]};do echo $x;done |sort);do
echo "${dept_dict[$k]} ${k}";done |sort -rns| while read cnt dept;do
echo "${dept} ${cnt}";done;echo;for k in ${!name_dict[@]};do
((${name_dict[$k]}>1))&&echo $k;done;cnt=0;for k in ${!name_dict[@]};do
((${name_dict[$k]}==1))&&cnt=$((cnt+1));done;echo $cnt;
