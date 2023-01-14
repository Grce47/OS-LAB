gcd() (
    ! (( $1 % $2 )) && echo $2 || gcd $2 $(( $1 % $2 ))
)
ans=1
while read p
do
    tmp=$( echo $p | rev )
    ans=$(( ans * tmp / $( gcd $ans $tmp ) ))
done < $1
echo $ans