# cut --complement -d ":" -f 7 lcm.txt --output-delimiter=" "

# ls -l | awk '{print $1}'

declare -a res

arr=$(cat $1)

echo $arr

echo ${arr[1]}

j=0
for i in $arr
do
echo $i, $j
res[j]=$i
((j++))
done

# echo ${res[1]}
for i in ${res[@]}
do
echo $i
done