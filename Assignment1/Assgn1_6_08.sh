prime=(0,0);for((i=2;i<1000001;i++));do
prime[i]=1;done;for((i=2;i<1000001;i++));do
((${prime[i]}==0))&&continue;for((j=i*i;j<1000001;j+=i));do
prime[j]=0;done;done;echo -n"">output.txt;while read p;do
for((i=2;i<=$p;i++));do 
((${prime[i]}==1&&p%i==0))&&echo -n $i" ";done;echo;done<input.txt>>output.txt