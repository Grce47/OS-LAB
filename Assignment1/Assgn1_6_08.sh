prime=(0 0)

for i in {2..1000001}
do 
    prime[i]=1; 
done

for i in {2..1000001}
do
    if [ ${prime[i]} -eq 0 ]
    then 
        continue
    fi
    for (( j=i*i; j<1000001 ; j=j+i ))
    do
        prime[j]=0;
    done
done

while read p
do
    for (( i=2;i<=$p;i++ ))
    do 
        if [ ${prime[i]} -eq 1 ]
        then
            echo -n $i" "
        fi    
    done
    echo
done < input.txt