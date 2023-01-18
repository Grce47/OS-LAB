file_does_not_exists=1;
header="Date(dd-mm-yy),Category,Amount,Name"


# OPTIMIZE THIS
# Checking if main.csv file exists ((check if -f flag is neccessary))
# convert if to boolean
if [ -f "main.csv" ]
then
    file_does_not_exists=0;
    read -r firstline<main.csv

    # check if main.csv header matches with desired header
    if [[ $firstline != $header ]]
    then
        rm main.csv
        file_does_not_exists=1;
    fi
fi

# code to create new main.csv if file_does_not_exists==1 and add desired header
(($file_does_not_exists==1))&&touch main.csv;(($file_does_not_exists==1))&&echo $header>>main.csv;args=("$@");IFS=",";to_insert=("${args[-4]}","${args[-3]}","${args[-2]}","${args[-1]}");echo "${to_insert[*]}">>main.csv;echo "inserts ${to_insert[*]} into the csv";

while getopts ':c:n:s:h' OPTION; do
  case "${OPTION}" in
    c)awk -F, -v val="$OPTARG" '$2==val {sum+=$3} END {if(sum == "") {print 0} else {print sum}}' main.csv;;n)awk -F, -v val="$OPTARG" '$4==val {sum+=$3} END {if(sum == "") {print 0} else {print sum}}' main.csv;;s)var=0;(($OPTARG=="Category"))&&var=2;(($OPTARG=="Date"))&&var=1;(($OPTARG=="Amount"))&&var=3;(($OPTARG=="Name"))&&var=4;(($var!=0))&&tail -n +2 main.csv|sort -t, -k$var,$var -n>temp.del;(($var!=0))&&cat <temp.del>main.csv;(($var!=0))&&sed -i "1i\\$header" main.csv;(($var!=0))&&rm *.del;;
    h)
# TODO : COMPLETE THIS FLAG FUNCTIONALITY
      echo "h flag called"
      ;;
    # ?)
    #   exit 1
    #   ;;
  esac
done
shift "$(($OPTIND -1))"
