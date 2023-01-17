gcd() (
    ! (( $1 % $2 )) && echo $2 || gcd $2 $(( $1 % $2 ))
)
ans=1
while read p
do
    tmp=$( echo $p | rev )
    mult=$(bc<<<"$ans*$tmp")
    gc=$(bc<<<"$( gcd $ans $tmp )")  
    ans=$(bc<<<"$mult/$gc")
done < $1
echo $ans